#pragma rtl

#include <stddef.h>
#include <locator.h>
#include <misctool.h>
#include <tcpip.h>
#include <memory.h>
#include <gsos.h>
#include <orca.h>
#include "driver.h"
#include "installdriver.h"
#include "asmglue.h"
#include "version.h"

const char bootInfoString[] = "NetDisk               " BOOT_INFO_VERSION;

Word *unloadFlagPtr;

static struct NotificationProcRec {
    Long reserved1;
    Word reserved2;
    Word Signature;
    Long Event_flags;
    Long Event_code;
    Byte jml;
    void (*proc)(void);
} notificationProcRec;

#define NOTIFY_SHUTDOWN    0x20
#define NOTIFY_GSOS_SWITCH 0x04

static void notificationProc(void);

#define JML 0x5C


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
    if (toolerror() && toolerror() != terrINITNOTFOUND)
        goto error;

    /* Initialize the DIBs for our driver */
    InitDIBs();

    /* Install our driver */
    if (InstallDriver() != 0) {
        UnloadOneTool(54);
        goto error;
    }

    /* We're not going to error out, so show boot info. */
    ShowBootInfo(bootInfoString, NULL);

    /* Add notification proc to be called on shutdown and switches to GS/OS */
    notificationProcRec.Signature = 0xA55A;
    notificationProcRec.Event_flags = NOTIFY_SHUTDOWN | NOTIFY_GSOS_SWITCH;
    notificationProcRec.jml = JML;
    notificationProcRec.proc = notificationProc;
    NotifyProcRecGS addNotifyProcRec = {1, (ProcPtr)&notificationProcRec};
    AddNotifyProcGS(&addNotifyProcRec);

    /*
     * Put Marinetti in the default TPT so its tool stub won't be unloaded,
     * even if UnloadOneTool is called on it.  Programs may still call
     * TCPIPStartUp and TCPIPShutDown, but those don't actually do
     * anything, so the practical effect is that Marinetti will always
     * be available once its init has loaded (which may not have happened
     * yet when this init loads).
     */
    SetDefaultTPT();

    return;
    
error:
    setUnloadFlag();
    return;
}

/*
 * Notification procedure called at shutdown time or when switching to GS/OS.
 */
#pragma databank 1
static void notificationProc(void) {
    Word stateReg = ForceRomIn();

    if (notificationProcRec.Event_code & NOTIFY_GSOS_SWITCH) {
        /* Reinstall our driver on switch back to GS/OS from P8 */
        InstallDriver();
    } else if (notificationProcRec.Event_code & NOTIFY_SHUTDOWN) {
        //TODO eject disks
    }

    RestoreStateReg(stateReg);
}
#pragma databank 0

