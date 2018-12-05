set terminal png
set output "tlb.png"

set title "TLB benchmark"

set key right center

set xlabel "number of pages"

set ylabel "time in s"

#use log scale if we use doubling of number of pages set logscale x 2

plot "tlb64.dat" u 1:2 w linespoints title "Page size 64 bytes", \
     "tlb4k.dat" u 1:2 w linespoints title "Page size 4K bytes", \
     "dummy.dat" u 1:2 w linespoints title "Dummy"
