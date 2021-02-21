# Output Settings
set terminal png size 1200,600
set output 'output.png'

# Labels, Title and Data
set key inside bottom right
set xlabel 'Number of bodies'
set ylabel 'Time (s)'
set title 'Benchmark - dt = 100, iterations = 50'
plot "data.txt" using 1:2 title '1 Thread' with linespoints, \
"data.txt" using 1:3 title '2 Threads' with linespoints, \
"data.txt" using 1:4 title '3 Threads' with linespoints, \
"data.txt" using 1:5 title '4 Threads' with linespoints