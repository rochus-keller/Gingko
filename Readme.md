This repository is an experimental fork of the [Maiko Medley Interlisp virtual machine](https://github.com/Interlisp/maiko).

The present goal is to reduce complexity of the VM and make it easier to build and run on all supported platforms. 
One idea is to replace the OS and display dependencies by a Qt (i.e. [LeanQt](https://github.com/rochus-keller/LeanQt/)) 
based plattform abstraction layer (as e.g. done for [Oberon System 3](https://github.com/rochus-keller/OberonSystem3)).

To avoid confusions with the original project, this version of the VM is called "Gingko" (in reference to [Ginkgo](https://en.wikipedia.org/wiki/Ginkgo)) instead of "Maiko"[^1].

#### Status on 2024-10-04

- Downloaded original source code from https://github.com/Interlisp/maiko, commit 91e0cea by 2024-09-17.
- Reduced the file tree to only the *.c and *.h files required to build the SDL2 executable "ldesdl" and combined these files into one directory.
- Added a qmake (*.pro) file able to build the executable; to make the build work I added vdate.c generated by the original build, fixed an issue on main.c:835, and changed the path in version.h:15.
- The resulting executable is able to run the full.sysout image found in the medley/loadups subdirectory of the [original deployment](github.com/.../medley-full-linux-x86_64-240926-e1989850_240513-4becc6ad.tgz) on Linux i386, using e.g. the command `./gingko -noscroll -sc 1024x768 <path-to>full.sysout`.

#### Status on 2024-10-05

- Refactored interpreter loop
- Defined SDL and RELEASE in version.h
- A lot of changes to make the code C99 compatible
- The VM can now be simply built using the cc *.c command (see below)

#### Precompiled versions

Not available at this time.

#### How to build

Download this repository, and then either ...

- open a terminal in the Gingko directory and run `gcc *.c -std=c99 -lSDL2 -lm -o gingko`
- or open the file Gingko.pro in Qt Creator and run the build. 

This has been tested on Linux so far.

#### Additional Credits

See README_Maiko.md for more information about the original Maiko VM, and the files LICENSE and NOTICE for 
licensing information.


[^1]: actually "Geiko" would probably have been more terminologically obvious, but the term has an undesirable connotation; the name "Ginkgo" itself is [derived from a misspelling of the Japanese pronunciation "gin kyō"](https://en.wikipedia.org/wiki/Ginkgo_biloba#Etymology) (銀杏), meaning "silver apricot," a reference to the tree's fruit; in botanical contexts, both "Ginkgo" and "Gingko" are used interchangeably; according to Wikipedia, "Gingko" (i.e. Ginkgo) is a "living fossil", which fits very well with Interlisp and Medley ;-)

