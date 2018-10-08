#ifndef DRIVER_H
#define DRIVER_H

#include <types.h>

#define DEVICE_MFM_DRIVE 0x0017  /* This is the ID used for FDHD SuperDrives */

#define NDIBS 16

/* device information block */
struct DIB {
    void *linkPtr;
    void *entryPtr;
    Word characteristics;
    LongWord blockCount;
    char devName[32];
    Word slotNum;
    Word unitNum;
    Word version;
    Word deviceID;
    Word headlink;
    Word forwardLink;
    void *extendedDIBPtr;
    Word DIBDevNum;
};

/* list of DIBs (argument to INSTALL_DRIVER) */
struct DIBList {
    LongWord count;
    struct DIB *dibPointers[NDIBS];
};

/* GS/OS direct page structure */
struct GSOSDP {
    Word deviceNum;
    Word callNum;
    Byte *bufferPtr; 
    LongWord requestCount;
    LongWord transferCount;
    LongWord blockNum;
    Word blockSize;
    Word fstNum;
    Word volumeID;
    Word cachePriority;
    void *cachePointer;
    struct DIB *dibPointer;
};
#define statusListPtr bufferPtr
#define statusCode fstNum
#define controlListPtr bufferPtr
#define controlCode fstNum

/* GS/OS driver call numbers */
#define Driver_Startup  0x0000
#define Driver_Open     0x0001
#define Driver_Read     0x0002
#define Driver_Write    0x0003
#define Driver_Close    0x0004
#define Driver_Status   0x0005
#define Driver_Control  0x0006
#define Driver_Flush    0x0007
#define Driver_Shutdown 0x0008

/* Driver_Status subcalls */
#define Get_Device_Status     0x0000
#define Get_Config_Parameters 0x0001
#define Get_Wait_Status       0x0002
#define Get_Format_Options    0x0003
#define Get_Partition_Map     0x0004

/* Custom Driver_Control subcalls */
#define Mount_URL 0x8080
#define Switch_To_DOS_Order 0x8081


/* Status list record for Get_DeviceStatus */
typedef struct DeviceStatusRec {
    Word statusWord;
    LongWord numBlocks;
} DeviceStatusRec;

extern struct DIB dibs[NDIBS];
extern struct DIBList dibList;

extern void *gsosDP;

void InitDIBs(void);
Word DriverDispatch(Word callNum, struct GSOSDP *dp);

#endif
