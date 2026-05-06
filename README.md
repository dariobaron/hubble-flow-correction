# Interpretable probabilistic modelling approach to correct Hubble flow around galaxy clusters

This repository contains the code used to reproduce the results in the paper titled "Interpretable probabilistic modelling approach to correct Hubble flow around galaxy clusters".

## Software requirements

Installing this software requires:
 - GNU Make
 - GCC
 - Python 3 (with modules `virtualenv` and `pip` available)

## Installing

To use the code, clone this repository on your machine and type

```bash
make setup
```

This command will:
 - initialise the submodule
 - create a local virtual environment using Python's `virtualenv` at the sub-directory called `env` and install all the necessary Python dependencies in it
 - create the necessary local directories

After the process is completed, type

```bash
source env/bin/activate
make
```

This command will:
 - activate the previously created Python virtual environment
 - compile the C++ source code to the necessary Python modules

## Usage

The current implementation of the code can be found at the folder [notebooks](https://github.com/dariobaron/hubble-flow-correction/tree/main/notebooks).

The datasets (train and test) must be provided in HDF5 format in the folder [data](https://github.com/dariobaron/hubble-flow-correction/tree/main/data).