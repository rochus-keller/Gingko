QT       = core gui

TARGET = gingko
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -lSDL2

QMAKE_CXXFLAGS += -fno-strict-aliasing

SOURCES += \
    arithops.cpp \
    arrayops.cpp \
    bin.cpp \
    binds.cpp \
    bitblt.cpp \
    bbtsub.cpp \
    blt.cpp \
    car-cdr.cpp \
    chardev.cpp \
    common.cpp \
    conspage.cpp \
    mkcell.cpp \
    draw.cpp \
    findkey.cpp \
    fvar.cpp \
    xc.cpp \
    gc.cpp \
    gc2.cpp \
    gvar2.cpp \
    hardrtn.cpp \
    inet.cpp \
    intcall.cpp \
    lineblt8.cpp \
    lsthandl.cpp \
    llcolor.cpp \
    llstk.cpp \
    loopsops.cpp \
    lowlev1.cpp \
    lowlev2.cpp \
    misc7.cpp \
    mvs.cpp \
    return.cpp \
    rplcons.cpp \
    shift.cpp \
    subr.cpp \
    sxhash.cpp \
    miscn.cpp \
    subr0374.cpp \
    timer.cpp \
    typeof.cpp \
    unwind.cpp \
    vars3.cpp \
    unixcomm.cpp \
    uraid.cpp \
    rpc.cpp \
    z2.cpp \
    eqf.cpp \
    fp.cpp \
    ubf1.cpp \
    ubf2.cpp \
    ubf3.cpp \
    uutils.cpp \
    perrno.cpp \
    lisp2c.cpp \
    osmsg.cpp \
    dbgtool.cpp \
    testtool.cpp \
    kprint.cpp \
    byteswap.cpp \
    main.cpp \
    initsout.cpp \
    storage.cpp \
    allocmds.cpp \
    vmemsave.cpp \
    mkatom.cpp \
    ldsout.cpp \
    dspsubrs.cpp \
    initdsp.cpp \
    dsk.cpp \
    ufs.cpp \
    dir.cpp \
    keyevent.cpp \
    kbdsubrs.cpp \
    initkbd.cpp \
    gcscan.cpp \
    gcarray.cpp \
    gccode.cpp \
    gcfinal.cpp \
    gcrcell.cpp \
    gchtfind.cpp \
    gcmain3.cpp \
    gcr.cpp \
    gcoflow.cpp \
    sdl.cpp \
    usrsubr.cpp \
    qtgui.cpp \
    display.cpp

HEADERS += \
    address.h \
    adr68k.h \
    allocmdsdefs.h \
    arith.h \
    arithopsdefs.h \
    array.h \
    arrayopsdefs.h \
    bb.h \
    bbtsubdefs.h \
    bindefs.h \
    bindsdefs.h \
    bitblt.h \
    bitbltdefs.h \
    bltdefs.h \
    byteswapdefs.h \
    car-cdrdefs.h \
    cell.h \
    chardevdefs.h \
    commondefs.h \
    conspagedefs.h \
    dbgtooldefs.h \
    dbprint.h \
    debug.h \
    devconf.h \
    devif.h \
    dirdefs.h \
    display.h \
    drawdefs.h \
    dskdefs.h \
    dspdata.h \
    dspifdefs.h \
    dspsubrsdefs.h \
    eqfdefs.h \
    findkeydefs.h \
    fpdefs.h \
    fvardefs.h \
    gc2defs.h \
    gcarraydefs.h \
    gccodedefs.h \
    gcdata.h \
    gcdefs.h \
    gcfinaldefs.h \
    gchtfinddefs.h \
    gcmain3defs.h \
    gcoflowdefs.h \
    gcrcelldefs.h \
    gcrdefs.h \
    gcscandefs.h \
    gvar2defs.h \
    hardrtndefs.h \
    ifpage.h \
    inetdefs.h \
    initatms.h \
    initdspdefs.h \
    initkbddefs.h \
    initsoutdefs.h \
    inlineC.h \
    intcalldefs.h \
    iopage.h \
    kbdsubrsdefs.h \
    keyboard.h \
    keyeventdefs.h \
    kprintdefs.h \
    ldsoutdefs.h \
    lineblt8defs.h \
    lisp2cdefs.h \
    lispemul.h \
    lispmap.h \
    lispver2.h \
    llcolordefs.h \
    llstkdefs.h \
    locfile.h \
    loopsopsdefs.h \
    lowlev1defs.h \
    lowlev2defs.h \
    lspglob.h \
    lsptypes.h \
    lsthandldefs.h \
    maindefs.h \
    medleyfp.h \
    misc7defs.h \
    miscndefs.h \
    miscstat.h \
    mkatomdefs.h \
    mkcelldefs.h \
    mvsdefs.h \
    my.h \
    opcodes.h \
    osmsg.h \
    osmsgdefs.h \
    perrnodefs.h \
    pilotbbt.h \
    platform.h \
    print.h \
    return.h \
    returndefs.h \
    rpcdefs.h \
    rplconsdefs.h \
    sdldefs.h \
    shiftdefs.h \
    stack.h \
    storagedefs.h \
    stream.h \
    subr0374defs.h \
    subrdefs.h \
    subrs.h \
    sxhashdefs.h \
    testtooldefs.h \
    timeout.h \
    timerdefs.h \
    tos1defs.h \
    tosfns.h \
    tosret.h \
    typeofdefs.h \
    ubf1defs.h \
    ubf2defs.h \
    ubf3defs.h \
    ufsdefs.h \
    unixcommdefs.h \
    unwinddefs.h \
    uraiddefs.h \
    uraidextdefs.h \
    usrsubrdefs.h \
    uutilsdefs.h \
    vars3defs.h \
    version.h \
    vmemsave.h \
    vmemsavedefs.h \
    xcdefs.h \
    z2defs.h \
    qtgui.h



