# ![xeus-cling](http://quantstack.net/assets/images/xeus-cling.svg)

[![Binder](https://img.shields.io/badge/launch-binder-brightgreen.svg)](https://beta.mybinder.org/v2/gh/QuantStack/xeus-cling/0.0.6?filepath=notebooks/xcpp.ipynb)
[![Join the Gitter Chat](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/QuantStack/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

`xeus-cling` is a Jupyter kernel for C++ based on the C++ interpreter [cling](https://github.com/root-project/cling) and
the native implementation of the Jupyter protocol [xeus](https://github.com/QuantStack/xeus).

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

### A C++ notebook

You can now make use of the C++ programming language in the Jupyter notebook.

![A C++ notebook](notebook.png)

### Quick help and tab completion.

Quick help is shown on the pager with the special `?` magic. 

![Help](help.png)

Content for the help is available for the standard library and the QuantStack packages.

### Jupyter interactive widgets

A C++ backend for the Jupyter interactive widgets is available in the [`xwidgets`](https://github.com/QuantStack/xwidgets/) package.

![Widgets](widgets.gif)

## Dependencies

``xeus-cling`` depends on

 - [xeus](https://github.com/QuantStack/xeus)
 - [xtl](https://github.com/QuantStack/xtl)
 - [cling](https://github.com/root-project/cling)
 - [pugixml](https://github.com/zeux/pugixml)

`xeus-cling` requires its dependencies to be built with the same compiler and same C runtime as the one used to build `cling`. 

The `QuantStack` channel provides a `xeus`, `cling` and their dependencies built with gcc-6. We highly recommend installing
these dependencies from QuantStack in a clean conda installation or environment.

## License

We use a shared copyright model that enables all contributors to maintain the
copyright on their contributions.

This software is licensed under the BSD-3-Clause license. See the [LICENSE](LICENSE) file for details.
