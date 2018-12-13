gcc -c green.c
gcc test.c -o test.o green.o -lm

./test.o
