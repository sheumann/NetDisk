#ifndef DISKCOPY42_H
#define DISKCOPY42_H

#define DC42_DISK_NAME_LEN 64
#define DC42_DATA_OFFSET 84

/* Expected magic value in "private" header field (taken as little-endian) */
#define DC42_MAGIC 0x0001 

struct DiskCopy42Header {
    char diskName[DC42_DISK_NAME_LEN]; /* p-string */
    LongWord dataSize; /* big-endian */
    LongWord tagSize;  /* big-endian */
    LongWord dataChecksum;
    LongWord tagChecksum;
    Byte diskFormat;
    Byte formatByte;
    Word private;
};

#endif
