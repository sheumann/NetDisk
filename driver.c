#include <stdlib.h>
#include <stdio.h>
#include <gsos.h>
#include "driver.h"
#include "driverwrapper.h"
#include "session.h"
#include "seturl.h"
#include "http.h"
#include "readtcp.h"
#include "tcpconnection.h"
#include "asmglue.h"
#include "version.h"

#define BLOCK_SIZE 512

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
        dibs[i].deviceID = DEVICE_GENERIC_FLOPPY_DRIVE;
        
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
    if (hdr->imgFormat != IMAGE_FORMAT_PRODOS_ORDER)
        return UNSUPPORTED_2IMG_FILE;
    if (hdr->dataLength % BLOCK_SIZE != 0)
        return UNSUPPORTED_2IMG_FILE;

    // TODO more checks
    
    sess->dataOffset = hdr->dataOffset;

    return OPERATION_SUCCESSFUL;
}

static Word DoMountURL(struct GSOSDP *dp) {
    enum NetDiskError err;

    if (dp->dibPointer->extendedDIBPtr != NULL) {
        dp->transferCount = 0;
        return drvrBusy;
    }

    Session *sess = calloc(sizeof(*sess), 1);
    if (sess == NULL) {
        dp->transferCount = 0;
        return drvrNoResrc;
    }
    
    err = SetURL(sess, (char*)dp->controlListPtr, TRUE, FALSE);
    if (err != OPERATION_SUCCESSFUL) {
        // TODO arrange for more detailed error reporting
        EndNetDiskSession(sess);
        dp->transferCount = 0;
        return drvrIOError;
    }
    
    err = DoHTTPRequest(sess, 0, sizeof(sess->fileHeader) - 1);
    if (err != OPERATION_SUCCESSFUL) {
        // TODO arrange for more detailed error reporting
        EndNetDiskSession(sess);
        dp->transferCount = 0;
        return drvrIOError;
    }
    
    InitReadTCP(sess, sizeof(sess->fileHeader.buf), &sess->fileHeader.buf);
    while (TryReadTCP(sess) == rsWaiting)
        // TODO timeout
        /* keep reading */ ;
    //TODO detect errors
    
    err = CheckTwoImg(sess);
    if (err != OPERATION_SUCCESSFUL) {
        EndNetDiskSession(sess);
        // TODO better error
        dp->transferCount = 0;
        return drvrIOError;
    }
    
    dp->dibPointer->extendedDIBPtr = sess;
    
    //TODO report disk switch
    
    return 0;
}

static Word DoRead(struct GSOSDP *dp) {
    Session *sess = dp->dibPointer->extendedDIBPtr;
    if (sess == NULL) {
        dp->transferCount = 0;
        return drvrOffLine;
    }

    //TODO check size is multiple of a block
    //TODO disk-switched logic
    
    unsigned long readStart = dp->blockNum * dp->blockSize + sess->dataOffset;
    unsigned long readEnd = readStart + dp->requestCount - 1;
    
    enum NetDiskError err = DoHTTPRequest(sess, readStart, readEnd);
    if (err != OPERATION_SUCCESSFUL) {
        // TODO arrange for more detailed error reporting
        dp->transferCount = 0;
        return drvrIOError;
    }
    
    InitReadTCP(sess, dp->requestCount, dp->bufferPtr);
    while (TryReadTCP(sess) == rsWaiting)
        // TODO timeout
        /* keep reading */ ;
    //TODO detect errors
    
    dp->transferCount = dp->requestCount - sess->readCount;
    return 0;
}

static Word DoStatus(struct GSOSDP *dp) {
    if (dp->requestCount < 2) {
        dp->transferCount = 0;
        return drvrBadParm;
    }
    //TODO handle actual disk, and disk-switched logic
    /* no disk in drive, ... */
    if (dp->dibPointer->extendedDIBPtr != NULL) {
        ((DeviceStatusRec*)dp->statusListPtr)->statusWord = 0x8014;
    } else {
        ((DeviceStatusRec*)dp->statusListPtr)->statusWord = 0;
    }
    if (dp->requestCount < 6) {
        dp->transferCount = 2;
        return 0;
    }
    ((DeviceStatusRec*)dp->statusListPtr)->numBlocks = 0;
    dp->requestCount = 6;
    return 0;
}

static Word DoEject(struct GSOSDP *dp) {
    EndNetDiskSession(dp->dibPointer->extendedDIBPtr);
    dp->dibPointer->extendedDIBPtr = NULL;
    
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