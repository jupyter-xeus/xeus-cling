.. Copyright (c) 2017, Johan Mabille, Loic Gouarin and Sylvain Corlay

   Distributed under the terms of the BSD 3-Clause License.

   The full license is in the file LICENSE, distributed with this software.

Build options
=============

Build flags
-----------

You can specify additional build flags that will be used by ``xeus-cling``
to compile the code in the notebook. To do so, you need to edit the kernelspec
file (usually ``share/jupyter/kernels/xcppSTD/kernel.json``, where ``STD`` is the
version of the cpp standard) and add the build flags in the ``argv`` array.

For instance, if you want to pass the ``-pthread -lpthread`` flags to ``xeus-cling``
and compile C++14 code, the C++14 kernelpec file becomes:

.. code::
    
    {
        "display_name": "C++14",
        "argv": [
            "/home/yoyo/miniconda3/envs/xwidgets/bin/xcpp",
            "-f",
            "{connection_file}",
            "-std=c++14",
            "-pthread",
            "lpthread"
        ],
        "language": "C++14"
    }

Using third-party libraries
---------------------------

When building a binary, you usually specify the include directories and the library path
of third-party libraries in the build tool. The library will be loaded upon binary execution.

``xeus-cling`` is slightly different, it allows you to specify both include directories and
library path, however you need to load the library explicitly. This is done with special
pragma commands that you can use in a code cell in a Jupyter Notebook:

- ``#pragma cling add_include_path("inc_directory")``
- ``#pragma cling add_library_path("lib_directory")"``
- ``#pragma cling load("libname")``

