# Gnuplot script for plotting data in file "optimal.dat"
set terminal png
set output "optimal.png"

set title "Optimal solution"

set key right center

set xlabel "Frames in memory"
set ylabel "Hit ratio"

set xrange [0:100]
set yrange [0:1]

plot "optimal.dat" u 1:2 w linespoints title "Optimal"
