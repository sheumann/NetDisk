#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <locator.h>
#include "mounturl.h"
#include "netdiskerror.h"

static void usage(void) {
    fprintf(stderr, "Usage: mounturl [-c] url\n");
    exit(EXIT_FAILURE);
}


int main(int argc, char **argv) {
    struct MountURLRec mountURLRec = {sizeof(struct MountURLRec)};
    int i = 1;
    int flags = flgUseCache;
    
    while (i < argc && argv[i][0] == '-') {
        switch(argv[i][1]) {
        case 'c':
                flags &= ~flgUseCache;
                break;

        default:
                usage();
        }
        i++;
    }
    
    if (i >= argc)
        usage();
    
    mountURLRec.result = NETDISK_NOT_PRESENT;
    mountURLRec.url = argv[i];
    mountURLRec.flags = flags;
    
    SendRequest(MountURL, sendToName|stopAfterOne, (Long)NETDISK_REQUEST_NAME,
                (Long)&mountURLRec, NULL);
    
    if (mountURLRec.result != OPERATION_SUCCESSFUL) {
        char *errString = ErrorString(mountURLRec.result);
        if (strchr(errString, '*') == NULL) {
            fprintf(stderr, "%s\n", errString);
        } else {
            fprintf(stderr, "NetDisk error %u.\n", mountURLRec.result);
        }
        return 1;
    }
    
    return 0;
}
