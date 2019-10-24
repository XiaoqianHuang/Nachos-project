cd /build.linux
make distclean
make depend
make
nachos -K > result.txt
