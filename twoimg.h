#ifndef TWOIMG_H
#define TWOIMG_H

#define TWO_IMG_MAGIC ('2' | 'I'<<8 | (LongWord)'M'<<16 | (LongWord)'G'<<24)

#define IMAGE_FORMAT_DOS_33_ORDER 0
#define IMAGE_FORMAT_PRODOS_ORDER 1
#define IMAGE_FORMAT_NIBBLIZED    2

struct TwoImgHeader {
    LongWord twoImgID;
    LongWord appTag;
    Word headerLength;
    Word version;
    LongWord imgFormat;
    LongWord flag;
    LongWord nBlocks;
    LongWord dataOffset;
    LongWord dataLength;
    /* Remaining header fields are unimportant for us */
};

#endif
