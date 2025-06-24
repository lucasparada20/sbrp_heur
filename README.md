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

3. Retrieve the instances from the [sbrp_exact](https://github.com/lucasparada20/sbrp_exact) repository.
    
## Running the code

Inside the sbrp_heur directory, you will find a script run_heur.sh with sample command line calls. The format is:

* instance_file : the instance to solve.
* instance_type : dins or pcg. dins denotes instances from [Dell'Amico et al., (2018)](https://doi.org/10.1016/j.trb.2018.10.015). pcg denotes the instances of our technical report. 
* epsilon, delta : the parameters for cost computations.

In run_heur.sh, the first example calls the executable with the instance Chicago_20_2.txt from [Dell'Amico et al., (2018)](https://doi.org/10.1016/j.trb.2018.10.015), using epsilon = delta = 0.2 and setting 10 times 25k ALNS iterations.

```bash
build/exec_heur instance_file=instances_dins/Chicago_20_2.txt epsilon=0.2 delta=0.2 iterations=25000 instance_type=dins
```
The second example in run_heur.sh calls the executable with the instance ssbrp_30_20_0.txt from the [Technical Report](https://www.cirrelt.ca/documentstravail/cirrelt-2024-26.pdf), using epsilon = delta = 0.2, and setting 10 times 25k ALNS iterations.

```bash
build/exec_heur instance_file=instances_pcg/ssbrp_30_20_0.txt epsilon=0.2 delta=0.2 iterations=25000  instance_type=pcg
```
