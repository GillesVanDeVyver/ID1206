gcc -c green.c -g
gcc -g test.c -o test.o green.o
gdb test.o
