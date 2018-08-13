#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <locator.h>
#include "mounturl.h"

int main(int argc, char **argv) {
    if (argc < 2)
        return;
    
    struct MountURLRec mountURLRec = {sizeof(struct MountURLRec)};
    mountURLRec.result = NETDISK_NOT_PRESENT;
    mountURLRec.url = argv[1];
    
    SendRequest(MountURL, sendToName|stopAfterOne, (Long)NETDISK_REQUEST_NAME,
                (Long)&mountURLRec, NULL);
    
    if (mountURLRec.result != OPERATION_SUCCESSFUL) {
        fprintf(stderr, "MountURL error %u\n", mountURLRec.result);
    }
}
