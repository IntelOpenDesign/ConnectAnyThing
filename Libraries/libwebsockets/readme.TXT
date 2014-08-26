How to setup this library

1) Please copy the libs in : arduino-1.5.3\hardware\tools\x86\i586-poky-linux-uclibc\lib
2) Open the file platform.win.txt in hardware\arduino\x86 and add the commands '-lz -lwebsockets' on the line that contains '-lm -lpthread' (revert to the normal one if you don't use the lib)
3) Please copy the libwesockets.so file on the board, in /usr/lib/