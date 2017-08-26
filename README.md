# xeus-cling

`xeus-cling` is a Jupyter kernel for C++ based on the C++ interpreter [cling](https://github.com/root-project/cling) and
the native implementation of the Jupyter protocol [xeus](https://github.com/QuantStack/xeus).

This project is in development phase and is highly instable!

## Installation

xeus-cling has been packaged for the conda package manager on the linux platform. From a new miniconda3 install:

```
conda install cling -c QuantStack -c conda-forge
conda install xeus-cling -c QuantStack -c conda-forge
conda install notebook -c conda-forge
```

Or you can install it directly from the sources, if all the dependencies are already installed.

```bash
cmake -DCMAKE_INSTALL_PREFIX=your_conda_path -DCMAKE_INSTALL_LIBDIR=your_conda_path/lib
make && make install
```

## Usage

Launch the jupyter notebook with `jupyter notebook` and launch a new C++ notebook by selecting the **xeus C++14** kernel in the *new* dropdown.

## Dependencies

``xeus-cling`` depends on

 - [xeus](https://github.com/QuantStack/xeus)
 - [cling](https://github.com/root-project/cling)
 - [pugixml](https://github.com/zeux/pugixml)

`xeus-cling` requires its dependencies to be built with the same compiler and same C runtime as the one used to build `cling`. 

The `QuantStack` channel provides a `xeus`, `cling` and their dependencies built with gcc-6. We highly recommend installing
these dependencies from QuantStack in a clean conda installation or environment.
