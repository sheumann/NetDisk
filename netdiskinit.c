#pragma rtl

#include <stddef.h>
#include <locator.h>
#include <misctool.h>
#include <tcpip.h>
#include <orca.h>

const char bootInfoString[] = "NetDisk               v1.0a1";

Word *unloadFlagPtr;

static void setUnloadFlag(void) {
    if (unloadFlagPtr != NULL && *unloadFlagPtr == 0)
        *unloadFlagPtr = 1;
}

int main(void) {
    /*
     * Load Marinetti.
     * We may get an error if the TCPIP init isn't loaded yet, but we ignore it.
     * The tool stub is still loaded in that case, which is enough for now.
     */
    LoadOneTool(54, 0x0200);
    //if (toolerror() && toolerror() != terrINITNOTFOUND)
    //    goto error;

    /* We're not going to error out, so show boot info. */
    ShowBootInfo(bootInfoString, NULL);

    /*
     * Put Marinetti in the default TPT so its tool stub won't be unloaded,
     * even if UnloadOneTool is called on it.  Programs may still call
     * TCPIPStartUp and TCPIPShutDown, but those don't actually do
     * anything, so the practical effect is that Marinetti will always
     * be available once its init has loaded (which may not have happened
     * yet when this init loads).
     */
    SetDefaultTPT();
    
    // TODO install driver
    
    return;
    
error:
    setUnloadFlag();
    return;
}
