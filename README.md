# xeus-cling

`xeus-cling` is a Jupyter kernel for C++ based on the C++ interpreter [cling](https://github.com/root-project/cling) and
the native implementation of the Jupyter protocol [xeus](https://github.com/QuantStack/xeus).

This project is in development phase and is highly instable!

## Installation

### Dependencies

``xeus-cling`` depends on [xeus](https://github.com/QuantStack/xeusa) and [cling](https://github.com/root-project/cling).

It is highly recommended to install ``cling`` and ``xeus`` with conda, on a new installation or new environment:

```bash
conda install cling xeus -c QuantStack -c conda-forge
```

You can then install the Jupyter notebook:

```bash
conda install notebook -c conda-forge
```

The installation should be done in this order. Indeed, `xeus-cling` requires its dependencies to be built with the same
compiler and same C library as the one used to build `cling`. The `QuantStack` channel provides a `xeus`, `cling` and
their dependencies built with gcc-6. These dependencies include `zeromq`, which is also a dependency of the notebook.
So if you have already installed the notebook, or any package depending on `zeromq`, the `zeromq` package from the `QuantStack`
channel will not be installed, resulting in errors when starting a xeus-cling kernel in the notebook. This is why we recommmend
to install `xeus` and `cling` on a new conda installation or environment.

### Build xeus-cling

Once the dependencies are installed, you can build xeus-cling from sources and install it:

```bash
cmake -DCMAKE_INSTALL_PREFIX=your_conda_path -DCMAKE_INSTALL_LIBDIR=your_conda_path/lib
make && make install
```

