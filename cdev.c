#pragma cdev cdevMain

#include <types.h>
#include <stddef.h>
#include <gsos.h>
#include <orca.h>
#include <quickdraw.h>
#include <window.h>
#include <control.h>
#include <lineedit.h>
#include <desk.h>

#define MachineCDEV     1
#define BootCDEV        2
#define InitCDEV        4
#define CloseCDEV       5
#define EventsCDEV      6
#define CreateCDEV      7
#define AboutCDEV       8
#define RectCDEV        9
#define HitCDEV         10
#define RunCDEV         11
#define EditCDEV        12

#define imageURLTxt         2
#define urlLine             3
#define mountBtn            1
//#define optionsPopUp      6
//#define trianglePic       7

extern void FreeAllCDevMem(void);

char urlBuf[257];

WindowPtr wPtr = NULL;

void DoMount(void)
{
    GetLETextByID(wPtr, urlLine, (StringPtr)&urlBuf);

    //TODO
}

void DoHit(long ctlID, CtlRecHndl ctlHandle)
{
    CtlRecHndl oldMenuBar;
    Word menuItem;

    if (!wPtr)  /* shouldn't happen */
        return;

    if (ctlID == mountBtn) {
        DoMount();
    }

    return;
}

long DoMachine(void)
{
    return 1;
}

void DoEdit(Word op)
{
    CtlRecHndl ctl;
    GrafPortPtr port;
    
    if (!wPtr)
        return;
    port = GetPort();
    SetPort(wPtr);
    
    ctl = FindTargetCtl();
    if (toolerror() || GetCtlID(ctl) != urlLine)
        goto ret;

    switch (op) {
    case cutAction:     LECut((LERecHndl) GetCtlTitle(ctl));
                        if (LEGetScrapLen() > 0)
                            LEToScrap();
                        break;
    case copyAction:    LECopy((LERecHndl) GetCtlTitle(ctl));
                        if (LEGetScrapLen() > 0)
                            LEToScrap();
                        break;
    case pasteAction:   LEFromScrap();
                        LEPaste((LERecHndl) GetCtlTitle(ctl));
                        break;
    case clearAction:   LEDelete((LERecHndl) GetCtlTitle(ctl));
                        break;
    }

ret:
    SetPort(port);
}

void DoCreate(WindowPtr windPtr)
{
    int mode;
    
    wPtr = windPtr;
    mode = (GetMasterSCB() & scbColorMode) ? 640 : 320;
    NewControl2(wPtr, resourceToResource, mode);
}

LongWord cdevMain (LongWord data2, LongWord data1, Word message)
{
    long result = 0;

    switch(message) {
    case MachineCDEV:   result = DoMachine();               break;
    case HitCDEV:       DoHit(data2, (CtlRecHndl)data1);    break;
    case EditCDEV:      DoEdit(data1 & 0xFFFF);             break;
    case CreateCDEV:    DoCreate((WindowPtr)data1);         break;
    case CloseCDEV:     wPtr = NULL;                        break;
    }

ret:
    FreeAllCDevMem();
    return result;
}
