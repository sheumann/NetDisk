#include <stdlib.h>
#include <string.h>
#include <gsos.h>

#define MountURL 0x8080

int main(int argc, char **argv) {
    DAccessRecGS controlRec = {5};
    
    if (argc < 3)
        return;
    
    controlRec.devNum = strtoul(argv[1], NULL, 0);
    controlRec.code = MountURL;
    controlRec.list = argv[2];
    controlRec.requestCount = strlen(argv[2]) + 1;
    
    DControl(&controlRec);
}
