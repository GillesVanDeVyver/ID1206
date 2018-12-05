# Gnuplot script for plotting data in file "lru.dat", "random.dat" and "optimal.dat"
set terminal png
set output "randOptLRU.png"

set title "Page replacement using random, optimal and LRU policy"

set key right center

set xlabel "Frames in memory"
set ylabel "Hit ratio"

set xrange [0:100]
set yrange [0:1]

plot "random.dat" u 1:2 w linespoints title "Random" , \
     "optimal.dat" u 1:2 w linespoints title "Optimal" , \
     "lru.dat" u 1:2 w linespoints title "LRU"
