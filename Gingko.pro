QT       -= core
QT       -= gui

TARGET = gingko
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

QMAKE_CFLAGS += -std=c99 -fno-strict-aliasing
# NOTE: if we leave away -fno-strict-aliasing, the VM crashes randomly
# NOTE: the original build also has -g3 and -std=gnu99, but this has no influence on stability

LIBS += -lSDL2

SOURCES += \
    arithops.c \
    arrayops.c \
    bin.c \
    binds.c \
    bitblt.c \
    bbtsub.c \
    blt.c \
    car-cdr.c \
    chardev.c \
    common.c \
    conspage.c \
    mkcell.c \
    draw.c \
    findkey.c \
    fvar.c \
    xc.c \
    gc.c \
    gc2.c \
    gvar2.c \
    hardrtn.c \
    inet.c \
    intcall.c \
    lineblt8.c \
    lsthandl.c \
    llstk.c \
    loopsops.c \
    lowlev1.c \
    lowlev2.c \
    misc7.c \
    mvs.c \
    return.c \
    rplcons.c \
    shift.c \
    subr.c \
    sxhash.c \
    miscn.c \
    subr0374.c \
    timer.c \
    typeof.c \
    unwind.c \
    vars3.c \
    z2.c \
    eqf.c \
    fp.c \
    ubf1.c \
    ubf2.c \
    ubf3.c \
    uutils.c \
    perrno.c \
    lisp2c.c \
    testtool.c \
    kprint.c \
    byteswap.c \
    main.c \
    initsout.c \
    storage.c \
    allocmds.c \
    vmemsave.c \
    mkatom.c \
    ldsout.c \
    dspsubrs.c \
    initdsp.c \
    dsk.c \
    ufs.c \
    dir.c \
    keyevent.c \
    initkbd.c \
    gcscan.c \
    gcarray.c \
    gccode.c \
    gcfinal.c \
    gcrcell.c \
    gchtfind.c \
    gcmain3.c \
    gcr.c \
    gcoflow.c \
    sdl.c \
    usrsubr.c \
    display.c \
    vmem_alloc.c

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
    emlglob.h \
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
    keyboard.h \
    keyeventdefs.h \
    kprintdefs.h \
    ldsoutdefs.h \
    lineblt8defs.h \
    lisp2cdefs.h \
    lispemul.h \
    lispmap.h \
    lispver2.h \
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
    perrnodefs.h \
    pilotbbt.h \
    platform.h \
    print.h \
    return.h \
    returndefs.h \
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
    ttydefs.h \
    typeofdefs.h \
    ubf1defs.h \
    ubf2defs.h \
    ubf3defs.h \
    ufsdefs.h \
    unwinddefs.h \
    usrsubrdefs.h \
    uutilsdefs.h \
    vars3defs.h \
    version.h \
    vmemsave.h \
    vmemsavedefs.h \
    xcdefs.h \
    xrdoptdefs.h \
    z2defs.h \
    tinydir.h



