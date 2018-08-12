#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <gsos.h>
#include "mounturl.h"

int main(int argc, char **argv) {
    struct MountURLRec mountURLRec;
    DAccessRecGS controlRec = {5};
    
    if (argc < 3)
        return;
    
    mountURLRec.byteCount = sizeof(mountURLRec);
    mountURLRec.url = argv[2];
    
    controlRec.devNum = strtoul(argv[1], NULL, 0);
    controlRec.code = MountURL;
    controlRec.list = (Pointer)&mountURLRec;
    controlRec.requestCount = sizeof(mountURLRec);
    
    DControl(&controlRec);
    
    if (mountURLRec.result != OPERATION_SUCCESSFUL) {
        fprintf(stderr, "MountURL error %u\n", mountURLRec.result);
    }
}
