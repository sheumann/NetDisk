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
#include "mounturl.h"
#include "version.h"

#define TCPIP_REQUEST_NAME "\pTCP/IP~STH~NetDisk~"

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
static pascal Word mountRequestProc(Word reqCode, void *dataIn, void *dataOut);
static pascal Word tcpipRequestProc(Word reqCode, void *dataIn, void *dataOut);

static void ejectAll(void);

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
    
    /* Accept requests to mount URLs */
    AcceptRequests(NETDISK_REQUEST_NAME, userid(), &mountRequestProc);

    /* Accept requests (notifications) from Marinetti */
    AcceptRequests(TCPIP_REQUEST_NAME, userid(), &tcpipRequestProc);

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

static void doMountURL(struct MountURLRec *mountURLRec) {
    DAccessRecGS controlRec = {5};
    controlRec.code = MountURL;
    controlRec.list = (pointer)mountURLRec;
    controlRec.requestCount = mountURLRec->byteCount;

    for (unsigned i = 0; i < NDIBS; i++) {
        Word devNum = dibs[i].DIBDevNum;
        if (devNum == 0)
            continue;
    
        controlRec.devNum = devNum;
        DControl(&controlRec);
        
        if (mountURLRec->result != DISK_ALREADY_MOUNTED)
            return;
    }
    
    mountURLRec->result = NO_DIBS_AVAILABLE;
}

/*
 * Request procedure called to mount a disk image by URL.
 */
#pragma databank 1
#pragma toolparms 1
static pascal Word mountRequestProc(Word reqCode, void *dataIn, void *dataOut) {
    if (reqCode == MountURL) {
        doMountURL((struct MountURLRec *) dataIn);
        return 0x8000;
    }
    
    return 0;
}
#pragma toolparms 0
#pragma databank 0


/*
 * Procedure to "eject" all our disks (called when network goes down).
 */
static void ejectAll(void) {
    DAccessRecGS controlRec = {5};
    controlRec.code = eject;
    controlRec.list = NULL;
    controlRec.requestCount = 0;
        
    for (unsigned i = 0; i < NDIBS; i++) {
        controlRec.devNum = dibs[i].DIBDevNum;
        if (controlRec.devNum == 0)
            continue;

        DControl(&controlRec);
    }
}

/*
 * Request procedure called by Marinetti with its notifications.
 * If the network has gone down, we unmount all the disks.
 */
#pragma databank 1
#pragma toolparms 1
static pascal Word tcpipRequestProc(Word reqCode, void *dataIn, void *dataOut) {
    if (reqCode == TCPIPSaysNetworkDown) {
        ejectAll();
    }
    
    return 0;
}
#pragma toolparms 0
#pragma databank 0
