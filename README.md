# sbrp_heur
An implementation of the ALNS metaheuristic for the stochastic bicycle repositioning problem. This is the algorithm that provided an upper bound to the problem in [Technical Report](https://www.cirrelt.ca/documentstravail/cirrelt-2024-26.pdf)

## Building the code in Linux

1. Clone the repository and add executable permission to a script that will call CMake for you:

```shell
git clone https://github.com/lucasparada20/sbrp_heur.git
cd sbrp_heur
chmod u+x cmake_script_heur.sh
```
2. Build the code by typing:

```bash
./cmake_script_heur.sh
```

## Running the code

Inside the sbrp directory, you will find a script run_heur.sh with sample command line calls. The format is:

* instance_file : the instance to solve.
* instance_type : dins or pcg. dins denotes instances from [Dell'Amico et al., (2018)](https://doi.org/10.1016/j.trb.2018.10.015). pcg denotes the instances of our technical report. For the dins instances, resolution begins by a previously computed upper bound from an implementation of the Adaptative Large Neighborhood Search algorithm. The bounds are in sbrp/instances_dins/all_upper_bounds.txt and are read in the UbManager class from UbManager.h.
* epsilon, delta : the parameters for cost computations.

In run_heur.sh, the first example calls the executable with the instance Chicago_20_1.txt from [Dell'Amico et al., (2018)](https://doi.org/10.1016/j.trb.2018.10.015), and using epsilon = delta = 0.2 

```bash
build/exec_exact instance_file=instances_dins/Chicago_20_1.txt epsilon=0.2 delta=0.2 opt_cuts=2 instance_type=dins algorithm=dl
```
The second example in run.sh calls the executable with the instance ssbrp_30_20_0.txt from the [Technical Report](https://www.cirrelt.ca/documentstravail/cirrelt-2024-26.pdf), using epsilon = delta = 0.2, hybrid optimality cuts and the DL-shaped method. 

```bash
build/exec_exact instance_file=instances_pcg/ssbrp_30_20_0.txt epsilon=0.2 delta=0.2 opt_cuts=3  instance_type=pcg algorithm=dl
```
## Ouput
Upon calling the examples in run.sh, you should see the following output.
