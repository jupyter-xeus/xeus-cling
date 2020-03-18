.. Copyright (c) 2017, Johan Mabille, Loic Gouarin and Sylvain Corlay

   Distributed under the terms of the BSD 3-Clause License.

   The full license is in the file LICENSE, distributed with this software.

.. raw:: html

   <style>
   .rst-content .section>img {
       width: 30px;
       margin-bottom: 0;
       margin-top: 0;
       margin-right: 15px;
       margin-left: 15px;
       float: left;
   }
   </style>

Installation
============

.. image:: conda.svg

Using the conda package
-----------------------

A package for xeus-cling is available on the conda package manager.

.. code::

    conda install -c conda-forge xeus-cling

.. image:: cmake.svg

From source with cmake
----------------------

You can also install ``xeus-cling`` from source with cmake. This requires that you have all the dependencies installed in the same prefix.

.. code::

    mkdir build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=/path/to/prefix ..
    make install

On Windows platforms, from the source directory:

.. code::

    mkdir build
    cd build
    cmake -G "NMake Makefiles" -DCMAKE_INSTALL_PREFIX=/path/to/prefix ..
    nmake
    nmake install

Installing the Kernel Spec
==========================

When installing xeus-cling in a given installation prefix, the corresponding Jupyter kernelspecs are installed in the same environment and are automatically picked up by Jupyter if it is installed in the same prefix. 

However, if Jupyter is installed in a different location, it will not pick up the new kernels. The xeus-cling kernels (for C++11, C++14 and C++17 respectively) can be registered with the following commands:

.. code::

   jupyter kernelspec install PREFIX/share/jupyter/xcpp11 --sys-prefix
   jupyter kernelspec install PREFIX/share/jupyter/xcpp14 --sys-prefix
   jupyter kernelspec install PREFIX/share/jupyter/xcpp17 --sys-prefix

For more information on the ``jupyter kernelspec`` command, please consult the ``jupyter_client`` documentation.
