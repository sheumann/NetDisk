#pragma noroot

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gsos.h>
#include "driver.h"
#include "driverwrapper.h"
#include "session.h"
#include "seturl.h"
#include "http.h"
#include "readtcp.h"
#include "tcpconnection.h"
#include "asmglue.h"
#include "mounturl.h"
#include "systemservices.h"
#include "version.h"

#define BLOCK_SIZE 512

/* Disk II-style track/sector layout (relevant to DOS-order images) */
#define TRACK_SIZE (BLOCK_SIZE * 8)
#define SECTOR_SIZE 256
#define DISK_II_DISK_SIZE (TRACK_SIZE * 35L)
/* The sectors making up each block within a track (using DOS 3.3 numbering) */
static const int sectorMap[2][8] = {{ 0, 13, 11, 9, 7, 5, 3,  1},
                                    {14, 12, 10, 8, 6, 4, 2, 15}};

struct DIB dibs[NDIBS] = {0};
struct DIBList dibList = {NDIBS};

struct GSOSDP *gsosDP = (void*)0x00BD00;  /* GS/OS direct page ptr */

static Word DoMountURL(struct GSOSDP *dp);
static Word DoRead(struct GSOSDP *dp);
static Word DoStatus(struct GSOSDP *dp);
static Word DoEject(struct GSOSDP *dp);
static Word DoShutdown(struct GSOSDP *dp);

void InitDIBs(void) {
    for (unsigned i = 0; i < NDIBS; i++) {
        dibs[i].linkPtr = (i < NDIBS-1) ? &dibs[i+1] : NULL;
        dibs[i].entryPtr = DriverWrapper;
            /* speed-independent, block device, read allowed, removable */
        dibs[i].characteristics = 0x03A4;
        dibs[i].blockCount = 0;
        
        int nameLen = sprintf(dibs[i].devName + 1, "NETDISK%u", i+1);
        dibs[i].devName[0] = nameLen;
        for (unsigned j = nameLen + 1; j < sizeof(dibs[i].devName); j++) {
            dibs[i].devName[j] = ' ';
        }
        
        dibs[i].slotNum = 0x8003;
        dibs[i].unitNum = i+1;
        dibs[i].version = DRIVER_VERSION;
        dibs[i].deviceID = DEVICE_MFM_DRIVE;
        
        dibList.dibPointers[i] = &dibs[i];
    }

}


#pragma databank 1
Word DriverDispatch(Word callNum, struct GSOSDP *dp) {
    Word stateReg = ForceRomIn();
    Word retVal = 0;

    switch (callNum) {
    case Driver_Startup:
        gsosDP = dp;
        break;
    
    case Driver_Open:
    case Driver_Close:
        /* Only applicable to character devices, but no error for block devs */
        break;
        
    case Driver_Read:
        retVal = DoRead(dp);
        break;
        
    case Driver_Write:
        //TODO Maybe do disk-switched logic and/or block size validation?
        retVal = drvrWrtProt;
        dp->transferCount = 0;
        break;
        
    case Driver_Status:
        switch (dp->statusCode) {
        case Get_Device_Status:
            retVal = DoStatus(dp);
            break;
            
        case Get_Config_Parameters:
            if (dp->requestCount < 2) {
                dp->transferCount = 0;
                retVal = drvrBadParm;
                break;
            }
            /* config list has length 0 */
            *(Word*)dp->statusListPtr = 0;
            dp->transferCount = 2;
            break;
            
        case Get_Wait_Status:
            if (dp->requestCount != 2) {
                dp->transferCount = 0;
                retVal = drvrBadParm;
                break;
            }
            /* always in wait mode */
            *(Word*)dp->statusListPtr = 0;
            dp->transferCount = 2;
            break;
            
        case Get_Format_Options:
            /* no format options */
            dp->transferCount = 0;
            break;
            
        case Get_Partition_Map:
            /* no partition map */
            dp->transferCount = 0;
            break;
        
        default:
            dp->transferCount = 0;
            retVal = drvrBadCode;
            break;
        }
        break;
        
    case Driver_Control:
        switch (dp->controlCode) {
        case eject:
            retVal = DoEject(dp);
            break;
            
        case setConfigParameters:
            dp->transferCount = 0;
            if (dp->requestCount < 2) {
                retVal = drvrBadParm;
                break;
            }
            /* config list should be empty (zero length) */
            if (*(Word*)dp->controlListPtr != 0) {
                retVal = drvrBadParm;
                break;
            }
            break;
            
        case setWaitStatus:
            dp->transferCount = 0;
            if (dp->requestCount != 2) {
                retVal = drvrBadParm;
                break;
            }
            /* only wait mode is valid */
            if (*(Word*)dp->controlListPtr != 0) {
                retVal = drvrBadParm;
                break;
            }
            break;
            
        case resetDevice:
        case formatDevice:
        case setFormatOptions:
        case assignPartitionOwner:
        case armSignal:
        case disarmSignal:
        case setPartitionMap:
            /* do nothing, and return no error */
            dp->transferCount = 0;
            break;
            
        case Mount_URL:
            DoMountURL(dp);
            break;
        
        default:
            dp->transferCount = 0;
            retVal = drvrBadCode;
            break;
        }
        break;
        
    case Driver_Flush:
        /* Only applicable to character devices; error for block devices */
        retVal = drvrBadReq;
        break;
    
    case Driver_Shutdown:
        retVal = DoShutdown(dp);
        break;
    
    default:
        retVal = drvrBadReq;
        break;
    }

    RestoreStateReg(stateReg);
    return retVal;
}
#pragma databank 0

/*
 * Check for a 2IMG file, and process its header if one is found.
 * Returns a non-zero error code for an invalid or unsupported 2IMG file, 
 * OPERATION_SUCCESSFUL otherwise (whether it's a 2IMG file or not).
 */
static enum NetDiskError CheckTwoImg(Session *sess) {
    struct TwoImgHeader *hdr = &sess->fileHeader.twoImgHeader;
    
    /* Check if it's a 2IMG file */
    if (hdr->twoImgID != TWO_IMG_MAGIC)
        return OPERATION_SUCCESSFUL;

    /* It's a 2IMG file, but is it one we can handle? */
    if (hdr->version > 1)
        return UNSUPPORTED_2IMG_FILE;
    if (hdr->dataLength % BLOCK_SIZE != 0)
        return UNSUPPORTED_2IMG_FILE;

    if (hdr->imgFormat == IMAGE_FORMAT_DOS_33_ORDER) {
        if (hdr->dataLength % TRACK_SIZE != 0)
            return UNSUPPORTED_2IMG_FILE;
        sess->dosOrder = TRUE;
    } else if (hdr->imgFormat != IMAGE_FORMAT_PRODOS_ORDER) {
        return UNSUPPORTED_2IMG_FILE;
    }
    
    sess->dataOffset = hdr->dataOffset;
    sess->dataLength = hdr->dataLength;
    
    /* Sweet16 apparently may generate images with erroneous dataLength of 0 */
    if (hdr->dataLength == 0) {
        sess->dataLength = hdr->nBlocks * BLOCK_SIZE;
    }

    return OPERATION_SUCCESSFUL;
}

static Word DoMountURL(struct GSOSDP *dp) {
    enum NetDiskError err;
    struct MountURLRec *mountURLRec = (struct MountURLRec *)dp->controlListPtr;

    if (mountURLRec->byteCount != sizeof(struct MountURLRec)
        || mountURLRec->byteCount != dp->requestCount)
    {
        dp->transferCount = 0;
        return drvrBadParm;
    }

    if (dp->dibPointer->extendedDIBPtr != NULL) {
        dp->transferCount = 0;
        mountURLRec->result = DISK_ALREADY_MOUNTED;
        return drvrBusy;
    }

    Session *sess = calloc(sizeof(*sess), 1);
    if (sess == NULL) {
        dp->transferCount = 0;
        mountURLRec->result = OUT_OF_MEMORY;
        return drvrNoResrc;
    }
    sess->useCache = (mountURLRec->flags & flgUseCache);
    
    err = SetURL(sess, mountURLRec->url, TRUE, FALSE);
    if (err != OPERATION_SUCCESSFUL) {
        EndNetDiskSession(sess);
        dp->transferCount = 0;
        mountURLRec->result = err;
        return drvrIOError;
    }
    
    err = DoHTTPRequest(sess, 0, sizeof(sess->fileHeader) - 1);
    if (err != OPERATION_SUCCESSFUL) {
        EndNetDiskSession(sess);
        dp->transferCount = 0;
        mountURLRec->result = err;
        return drvrIOError;
    }
    
    ReadStatus readStatus;
    InitReadTCP(sess, sizeof(sess->fileHeader.buf), &sess->fileHeader.buf);
    while ((readStatus = TryReadTCP(sess)) == rsWaiting)
        /* keep reading */ ;
    if (readStatus != rsDone) {
        EndNetDiskSession(sess);
        dp->transferCount = 0;
        mountURLRec->result = NETWORK_ERROR;
        return drvrIOError;
    }
    
    if (mountURLRec->format == formatAutoDetect 
        || mountURLRec->format == format2mg)
    {
        err = CheckTwoImg(sess);
        if (err != OPERATION_SUCCESSFUL) {
            EndNetDiskSession(sess);
            dp->transferCount = 0;
            mountURLRec->result = err;
            return drvrIOError;
        }
        if (sess->dataOffset != 0) {
            mountURLRec->format = format2mg;
        } else if (mountURLRec->format == format2mg) {
            dp->transferCount = 0;
            mountURLRec->result = NOT_SPECIFIED_IMAGE_TYPE;
            return drvrIOError;
        }
    }
    
    if (sess->dataOffset == 0) {
        /* No encapsulating disk image - treat this as raw blocks */
        sess->dataLength = sess->totalLength;
    }
    
    // TODO remove this hack in favor of better format detection
    if (mountURLRec->format == formatAutoDetect) {
        if (sess->dataLength == DISK_II_DISK_SIZE) {
            mountURLRec->format = formatDOSOrder;
        }
    }
    
    if (mountURLRec->format == formatDOSOrder) {
        if (sess->dataLength % TRACK_SIZE != 0) {
            dp->transferCount = 0;
            mountURLRec->result = NOT_SPECIFIED_IMAGE_TYPE;
            return drvrIOError;
        }
        sess->dosOrder = TRUE;
    }
    
    if (sess->dataLength % BLOCK_SIZE != 0) {
        dp->transferCount = 0;
        if (mountURLRec->format == formatAutoDetect) {
            mountURLRec->result = NOT_MULTIPLE_OF_BLOCK_SIZE;
        } else {
            mountURLRec->result = NOT_SPECIFIED_IMAGE_TYPE;
        }
        return drvrIOError;
    }

    if (mountURLRec->format == formatAutoDetect) {
        if (sess->dataLength != DISK_II_DISK_SIZE) {
            mountURLRec->format = formatRaw;
        }
    }

    dp->dibPointer->blockCount = sess->dataLength / BLOCK_SIZE;
    
    dp->dibPointer->extendedDIBPtr = sess;
    
    SetDiskSw();
    
    mountURLRec->result = OPERATION_SUCCESSFUL;
    mountURLRec->devNum = dp->dibPointer->DIBDevNum;
    return 0;
}

static Word DoRead(struct GSOSDP *dp) {
    Session *sess = dp->dibPointer->extendedDIBPtr;
    if (sess == NULL) {
        dp->transferCount = 0;
        return drvrOffLine;
    }

    //TODO disk-switched logic
    
    unsigned long readStart = dp->blockNum * dp->blockSize + sess->dataOffset;
    unsigned long readEnd = readStart + dp->requestCount - 1;
    
    /* Do sanity checks of input values */
    if (dp->blockSize == 0) {
        dp->transferCount = 0;
        return drvrBadBlock;
    }
    if (dp->requestCount % dp->blockSize != 0) {
        dp->transferCount = 0;
        return drvrBadCount;
    }
    if (readEnd < readStart || readStart / dp->blockSize != dp->blockNum)
    {
        dp->transferCount = 0;
        return drvrBadBlock;
    }

    LongWord firstBlockNum;
    LongWord endBlockNum;
    unsigned char *bufferPtr;

    Boolean useCache = sess->useCache
                       && (int)dp->cachePriority > 0
                       && (dp->fstNum & 0x8000) == 0
                       && dp->blockSize == BLOCK_SIZE;
    if (useCache) {
        firstBlockNum = dp->blockNum;
        endBlockNum = firstBlockNum + dp->requestCount / BLOCK_SIZE;
        bufferPtr = dp->bufferPtr;
        for (; dp->blockNum < endBlockNum; dp->blockNum++) {
            if (!CacheFindBlk()) {
                goto skipCache;
            }
            memmove(bufferPtr, dp->cachePointer, BLOCK_SIZE);
            bufferPtr += BLOCK_SIZE;
        }
        dp->blockNum = firstBlockNum;
        dp->transferCount = dp->requestCount;
        return 0;
skipCache:
        useCache = (dp->blockNum == firstBlockNum);
        dp->blockNum = firstBlockNum;
    }
    
    enum NetDiskError err;
    ReadStatus readStatus;

    if (!sess->dosOrder) {
        err = DoHTTPRequest(sess, readStart, readEnd);
        if (err != OPERATION_SUCCESSFUL) {
            dp->transferCount = 0;
            if (err == DIFFERENT_LENGTH) {
                if ((sess->totalLength - sess->dataOffset) % BLOCK_SIZE != 0) {
                    return drvrIOError;
                } else {
                    sess->dataLength = sess->totalLength - sess->dataOffset;
                    dp->dibPointer->blockCount = sess->dataLength / BLOCK_SIZE;
                    sess->expectedLength = sess->totalLength;
                    return drvrDiskSwitch;
                }
            } else {
                return drvrIOError;
            }
        }

        InitReadTCP(sess, dp->requestCount, dp->bufferPtr);
        while ((readStatus = TryReadTCP(sess)) == rsWaiting)
            /* keep reading */ ;
    
        dp->transferCount = dp->requestCount - sess->readCount;
    
        if (readStatus != rsDone) {
            return drvrIOError;
        }
    } else {
        if (dp->blockSize != BLOCK_SIZE) {
            dp->transferCount = 0;
            return drvrBadBlock;
        }
        
        firstBlockNum = dp->blockNum;
        endBlockNum = firstBlockNum + dp->requestCount / BLOCK_SIZE;

        int half = 0;
        dp->transferCount = 0;
        while (dp->blockNum < endBlockNum) {
            readStart = (dp->blockNum >> 3) * TRACK_SIZE
                        + sectorMap[half][dp->blockNum & 0x7] * SECTOR_SIZE
                        + sess->dataOffset;
            readEnd = readStart + (SECTOR_SIZE - 1);

            err = DoHTTPRequest(sess, readStart, readEnd);
            if (err != OPERATION_SUCCESSFUL) {
                dp->blockNum = firstBlockNum;
                return drvrIOError;
            }

            InitReadTCP(sess, SECTOR_SIZE, dp->bufferPtr + dp->transferCount);
            while ((readStatus = TryReadTCP(sess)) == rsWaiting)
                /* keep reading */ ;
    
            if (readStatus != rsDone || sess->readCount != 0) {
                dp->blockNum = firstBlockNum;
                return drvrIOError;
            }
    
            dp->transferCount += SECTOR_SIZE;
            
            half ^= 1;
            if (half == 0)
                dp->blockNum++;
        }
        
        dp->blockNum = firstBlockNum;
    }
    
    if (useCache && sess->readCount == 0) {
        bufferPtr = dp->bufferPtr;
        dp->blockNum = firstBlockNum;
        for (; dp->blockNum < endBlockNum; dp->blockNum++) {
            if (!CacheAddBlk()) {
                break;
            }
            memmove(dp->cachePointer, bufferPtr, BLOCK_SIZE);
            bufferPtr += BLOCK_SIZE;
        }
        dp->blockNum = firstBlockNum;
    }
    
    return 0;
}

/* This implements the Get_Device_Status subcall of Driver_Status */
static Word DoStatus(struct GSOSDP *dp) {
    DeviceStatusRec *dsRec = (DeviceStatusRec*)dp->statusListPtr;

    if (dp->requestCount < 2) {
        dp->transferCount = 0;
        return drvrBadParm;
    }
    //TODO disk-switched logic
    if (dp->dibPointer->extendedDIBPtr != NULL) {
        dsRec->statusWord = 0x0014;
    } else {
        dsRec->statusWord = 0;
    }
    if (dp->requestCount < 6) {
        dp->transferCount = 2;
        return 0;
    }
    dsRec->numBlocks = dp->dibPointer->blockCount;
    if (dsRec->numBlocks == 0) {
        dsRec->statusWord |= 0x8000;
    }
    dp->transferCount = 6;
    return 0;
}

static Word DoEject(struct GSOSDP *dp) {
    EndNetDiskSession(dp->dibPointer->extendedDIBPtr);
    dp->dibPointer->extendedDIBPtr = NULL;
    dp->dibPointer->blockCount = 0;
    
    dp->transferCount = 0;
    return 0;
}

static Word DoShutdown(struct GSOSDP *dp) {
    /*
     * We don't actually end the session here, because this may just be
     * a switch to P8.  If so, this driver won't work within P8, but if we
     * leave things in place it should work again once we're back in GS/OS.
     * Do end the current TCP connection, since it would probably time out.
     */
    EndTCPConnection(dp->dibPointer->extendedDIBPtr);
    
    /*
     * Return error to indicate we shouldn't be purged.
     * (I don't think we would be anyhow, since this isn't an
     * actual device driver file, but let's do this to be safe.)
     */
    return drvrIOError;
}