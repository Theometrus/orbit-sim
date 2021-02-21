# Preface
This project was a delve into the world of multithreading in C. It's a solar system simulator featuring real-world gravitational equations. 

## Demonstration
![](./gifs/orbits.gif)

# Technical
## Compiling | Makefile
I tried to make the user's life as easy as possible with the Universal Makefile™. One can compile various parts of the program, tests, and run the benchmark - all using the makefile. The following commands are available:

1. make 
2. make nbody
3. make nbody-gui
4. make nbody-test
5. make benchmark
6. make clean
7. make nbody-asan

Most of these are fairly self explanatory. 

*make* by itself compiles both nbodycli and nbodygui. 

*make clean* removes previously compiled files. 

*make nbody-asan* compiles the CLI version of nbody with the -fsanitize=address flag, as nbody-asan.

Finally, my pride and joy, *make benchmark* is an all-in-one command which will compile the files it needs, run benchmarks on the CLI version of the program (altering the number of threads and number of bodies), and save the data and GNUPlot output in the *benchmarking* folder. Beautiful, isn't it?

<br>

## Running
### Commands
The program contains CLI and GUI versions. After the appropriate compilation step, one can perform the following commands *in the root folder of the directory*:

* ./nbody &lt;num_iters&gt; &lt;dt&gt; &lt;(-b &lt;num_bodies&gt;) | (-f &lt;filename&gt;)&gt; &lt;num_threads&gt;

    Please note, the *num_threads* parameter is optional. If you omit it, the program will run with 1 thread. This command will execute the CLI version of nbody. It will print out the initial and final energies of the system after completion. 

<br>

* ./nbody-gui &lt;view_width&gt; &lt;view_height&gt; &lt;num_iters&gt; &lt;dt&gt; &lt;(-b &lt;num_bodies&gt;) | (-f &lt;filename&gt;)&gt; &lt;scale&gt; &lt;num_threads&gt;

    Once again, the *num_threads* parameter is optional. This will run the GUI version of the program. The random bodies version is honestly more of a ballpit than a solar system, however. It will spawn celestial bodies with random masses, at random positions, and with random velocities. This doesn't really result in meaningful orbits as the bodies just end up flying towards/away from each other usually (since the balance of distance, velocity, and mass has to be *just right* to get heavenly bodies into a stable orbit).

<br>

* ./nbody-test

<br>

For example, from the root folder: ./nbody-gui 1024 576 10000 1000 -f planets.csv 0.000000001 4

<br>

### Recommendations
I'd like to share some values I have discovered to work well in my tests of the GUI. These will get planets to display nicely:

* scale = 0.000000001 for the included planets.csv file. 
* scale = 0.000000000055, width=1024, height = 576, dt=1000+ for randomly generated bodies.

<br>
<br>
