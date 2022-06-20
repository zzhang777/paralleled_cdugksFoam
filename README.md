# Paralleled cdugksFoam

## About

This project is a work for the paper [Unified X-Space Parallelization Algorithm for Conserved Discrete Unified Gas Kinetic Scheme](https://www.sciencedirect.com/science/article/abs/pii/S0010465522001291). The open source multiscale flow solver dugksFoam is optimized with unstructured mesh,  conserved algorithm, and hybrid space parallelization to achieve better computational accuracy and efficiency.

## Features

- 1D & 2D & 3D in a single solver.
- Adopt unstructured mesh in both physical space and particle velocity space.

- Properties in each cell are updated by moments of microscopic variable
  fluxes to preserve better conservation of the macroscopic physical variables.
- Decompose each space in a hybrid manner and efficiently parallelize the
  computation.

## Installation

Before installing the code in this repository, we assume that you have installed the MPI library and OpenFOAM version 6.

Then, fetch the source code by `git clone` or `Download ZIP`.

If using git:

```shell
git clone https://github.com/zzhang777/paralleled_cdugksFoam.git
mv paralleled_cdugksFoam dugksFoam
cd dugksFoam/src
./Allwmake
```

If using ZIP package  :

```shell
#move the download file to dir wish to install
unzip paralleled_cdugksFoam-master.zip 
mv paralleled_cdugksFoam-master dugksFoam
cd dugksFoam/src
./Allwmake
```

NOTE: If permission problems occur during the installation, please run the chmod command.

## Usage

### Case setup

The case setup method in cdugksFoam is almost the same as the original dugksFoam, whose document is in `doc/` directory. Here, only different parts are present.

#### Prepare the unstructured velocity space

The original dugksFoam adopts a structured mesh in velocity space using Newton-Cotes quadrature and half-range Gauss-Hermite quadrature. This  function is implemented by configuring two files:  `constant/Xis` and `constant/weights`. The former is the velocity set, the latter is their weight.

In unstructured velocity space, the velocity mesh is generated by third-party software, such as pointwise and Gambit.
For mesh generation software that supports mesh files in OpenFOAM format, the velocity mesh can be directly exported.
For those who don't support, use mesh conversing tools provided by OpenFOAM to transform the format.

After getting the velocity mesh in OpenFOAM mesh format, named the mesh folder vMesh, and put it in the `constant/` dir.

#### Boundary conditions 

The current supported boundary conditions are diffusive wall boundary  and far field boundary.

#### Conserved algorithm

The bug “update cell center macro by macro flux” in the dugksFoam source code is fixed to replace the integration to guarantee conservation. To use this function, set the field `macroFlux` in `constant/DVMProperties` to yes.

```shell
fvDVMparas
{
    xiMax       xiMax [0 1 -1 0 0 0 0]    1;
    xiMin       xiMin [0 1 -1 0 0 0 0]   -1.766485662735688e+03;
    nDV               28;       // Number of discrete velocity, shoud be 4*Z + 1 if using compound N-C quardrature
    //set this field to yes
    macroFlux  yes;
}
```

#### Others

`xiMax`、`xiMin` and `nDV` in `fvDVMparas` in file `constant/DVMProperties` don't need to be configured. These data will be calculated in the program. 

```
fvDVMparas
{
    xiMax       xiMax [0 1 -1 0 0 0 0]    1;
    xiMin       xiMin [0 1 -1 0 0 0 0]   -1.766485662735688e+03;
    nDV               28;       // Number of discrete velocity, shoud be 4*Z + 1 if using compound N-C quardrature
    macroFlux  yes;
}
```

### Running in parallel

The X-space parallelization strategy can be configured to use physical space parallelization, velocity space parallelization, and hybrid space parallelization, respectively. 

First, one should enter the case dir and always make sure the decomposition script `multidecompose.py` is in the dir. Configure the physical space decomposition method in `system/decomposeParDict`.

Then, execute the uniform routine listed below.

1. Domain decomposition

```shell
python multidecompose.py -p M -v N
```

`M` means physical space is decomposed into M subdomains, `N` means velocity space is decomposed into N subdomains. `M = 1` when using velocity space parallelization, `N = 1` when using velocity space parallelization. After domain decomposition, the processor folder will be created under the case directory. It contains a subdomain of physical mesh, velocity mesh, and the initial field.

2. Launch MPI processes

```shell
mpirun -n M ∗ N dugksFoam -parallel -dvParallel -pd M
```

`pd` means the number of physical subdomain, `pd = 1` when running in velocity space parallelization.

3. Get computational results

```shell
reconsructPar
```

This command is provided by OpenFOAM to recombination the flow properties of different physical subdomains.  

### Post processing  

Tecplot has supported OpenFOAM data format since the 2013 release. Or you can use ParaView  to view simulation results.

## Demo

2D lid-driven cavity flow  simulation in the paper is provided in `demo` dir, the case name is cavity0.1.

## Citation

```latex
@article{zhang2022unified,
  title={{Unified X-space parallelization algorithm for conserved discrete unified gas kinetic scheme}},
  author={Zhang, Qi and Wang, Yunlan and Pan, Dongxin and Chen, Jianfeng and Liu, Sha and Zhuo, Congshan and Zhong, Chengwen},
  journal={Computer Physics Communications},
  pages={108410},
  year={2022},
  publisher={Elsevier}
}
```

## References

* [1] L. Zhu, [dugksFoam](https://github.com/zhulianhua/dugksFoam).
* [2] L. Zhu, S. Chen, Z. Guo, dugksFoam: An open source OpenFOAM solver for the Boltzmann model equation, [Comp. Phys. Commun., 213(2017) 155-164](http://www.sciencedirect.com/science/article/pii/S0010465516303642).
* [3] Z. Guo, K. Xu, R. Wang, Discrete unified gas kinetic scheme for all Knudsen number flows: low-speed isothermal case, [Phys. Rev. E, 88 (2013) 033305](http://journals.aps.org/pre/abstract/10.1103/PhysRevE.88.033305).
* [4] Z. Guo, R. Wang, K. Xu, Discrete unified gas kinetic scheme for all Knudsen number flows. II. Thermal compressible case, [Phys. Rev. E, 91(2015) 033313](http://journals.aps.org/pre/abstract/10.1103/PhysRevE.91.033313).
* [5] L. Zhu, Z. Guo, K. Xu, Discrete unified gas kinetic scheme on unstructured meshes, [Comp. Fluids, 127(2016) 211-225](http://www.sciencedirect.com/science/article/pii/S0045793016000177).
* [6] R. Yuan, C. Zhong, A conservative implicit scheme for steady state solutions of diatomic gas flow in all flow regimes, [Computer Physics Communications 247 (2020) 106972](https://www.sciencedirect.com/science/article/abs/pii/S0010465519303182).
* [7] J. Chen, S. Liu, Y. Wang, C. Zhong, Conserved discrete unified gaskinetic scheme with unstructured discrete velocity space, [Physical Review E 100 (4) (2019) 043305](https://journals.aps.org/pre/abstract/10.1103/PhysRevE.100.043305).
* [8] S. Li, Q. Li, S. Fu, J. Xu, The high performance parallel algorithm for unified gas-kinetic scheme, in: [AIP Conference Proceedings, Vol. 1786, AIP Publishing LLC, 2016, p. 180007](https://aip.scitation.org/doi/abs/10.1063/1.4967676). 
* [9] S. Tan, W. Sun, J. Wei, G. Ni, A parallel unified gas kinetic scheme for three-dimensional multi-group neutron transport, [Journal of Computational Physics 391 (2019) 37–58](https://www.sciencedirect.com/science/article/pii/S0021999119302876).
* [10] Chen, J, Liu, S, Wang, Y, Zhong, C. A Compressible Conserved Discrete Unified Gas-Kinetic Scheme with Unstructured Discrete Velocity Space for Multi-Scale Jet Flow Expanding into Vacuum Environment, [Communications in Computational Physics. 2020, 28, 1502–1535](https://global-sci.org/intro/article_detail/cicp/18109.html).
* [11] Yuan R, Liu S, Zhong C. A multi-prediction implicit scheme for steady state solutions of gas flow in all flow regimes, [Communications in Nonlinear Science and Numerical Simulation, 2021, 92: 105470](https://www.sciencedirect.com/science/article/abs/pii/S1007570420303002).
* [12] Yuan R, Liu S, Zhong C. A novel multiscale discrete velocity method for model kinetic equations, [Communications in Nonlinear Science and Numerical Simulation, 2021, 92: 105473](https://www.sciencedirect.com/science/article/abs/pii/S1007570420303038).
