QT       -= core
QT       -= gui

TARGET = gingko
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

DEFINES += SDL=2 RELEASE=351

QMAKE_CFLAGS += -std=gnu99

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
    llcolor.c \
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
    unixcomm.c \
    uraid.c \
    rpc.c \
    ufn.c \
    z2.c \
    eqf.c \
    fp.c \
    ubf1.c \
    ubf2.c \
    ubf3.c \
    uutils.c \
    perrno.c \
    foreign.c \
    lisp2c.c \
    osmsg.c \
    dbgtool.c \
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
    kbdsubrs.c \
    ether_common.c \
    ether_sunos.c \
    ether_nethub.c \
    tty.c \
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
    vdate.c



