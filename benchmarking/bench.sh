#!/bin/bash
echo "==========================================================================="
echo "Cleaning up previous files.."
echo
rm -f data.txt
cd ..
make nbody
make all
echo
echo "Cleanup complete!"

echo "==========================================================================="
echo "Beginning benchmark."
echo
echo "While you're waiting, here are the stats for the benchmarks:"
echo "Number of iterations: 50 (not to waste your time)"
echo "dt: 100"
echo "Number of bodies & number of threads: varies from 100-1000 and 1-4 respectively"
echo 
echo

# Num bodies
for ((b = 100; b <= 1000; b+=100))
do
    echo "$b" >> columns.txt
    #Num threads
    for ((t = 1; t < 5; t+=1))
    do
    echo -e "\e[1A\e[K>>Timing $b random bodies on a $t - threaded implementation..<<"
    (time ./nbody 50 100 -b $b $t;) |& awk 'NR==5{printf "%s\n", substr($2, 3, 5)}' >> data_$t.txt 
    done
done

paste columns.txt data_*.txt | column -s $' ' -t >> benchmarking/data.txt

for ((t = 1; t < 5; t+=1))
do
rm -f data_$t.txt
done
rm -f columns.txt

cd benchmarking
gnuplot plot.gnuplot

echo
echo "Benchmark completed. Please check the **benchmarking** folder. Results saved in data.txt. Plot created as output.png."
echo "==========================================================================="