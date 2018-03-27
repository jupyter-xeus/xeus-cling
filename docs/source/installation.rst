.. Copyright (c) 2017, Johan Mabille, Loic Gouarin and Sylvain Corlay

   Distributed under the terms of the BSD 3-Clause License.

   The full license is in the file LICENSE, distributed with this software.

Installation
============

With Conda
----------

``xeus-cling`` has been packaged on all platforms for the conda package manager.

.. code::

    conda install xeus-cling notebook -c QuantStack -c conda-forge

From Source
-----------

``xeus-cling`` depends on the following libraries:

 - `xeus`
 - `xtl`
 - `cling`
 - `pugxml`
 - `cppzmq`
 - `cxxopts`
 - `nlohmann_json`
 - `dirent` (on Windows only)

We have packaged all these dependencies for the conda package manager. The simplest way to install them with conda is to run:

.. code::

    conda install cmake zeromq cppzmq cryptopp xtl -c conda-forge .

On Linux platform, you will also need:

.. code::

    conda install libuuid -c conda-forge

Once you have installed the dependencies, you can build and install `xeus`:

.. code::

    cmake -D BUILD_EXAMPLES=ON -D CMAKE_BUILD_TYPE=Release .
    make
    make install
