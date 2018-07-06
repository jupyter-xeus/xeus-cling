.. Copyright (c) 2017, Johan Mabille, Loic Gouarin and Sylvain Corlay

   Distributed under the terms of the BSD 3-Clause License.

   The full license is in the file LICENSE, distributed with this software.

.. raw:: html

   <style>
   .rst-content .section>img {
       border: 1px solid #e1e4e5;
   }
   </style>

Magic commands
==============

Magics are special commands for the kernel that are not part of the C++
programming language.

There are defined with the symbol ``%`` for a line magic and ``%%`` for a cell
magic.

A few magics are available in xeus-cling. In the future, user-defined magics
will also be enabled.

%%file
------

This magic command copies the content of the cell in a file named `filename`.

.. code::

    %%file [-a] filename

- Example

.. image:: file_magic.png

- Optional argument:

+------------+---------------------------------+
| -a         | append the content to the file. |
+------------+---------------------------------+

%timeit
-------

Measure the execution time execution for a line statement (`%timeit`) or for a
block of statements (`%%timeit`)

- Usage in line mode

.. code::

    %timeit [-n<N> -r<R> -p<P>] statement

- Usage in cell mode

.. code::

    %%timeit [-n<N> -r<R> -p<P>]
    statements

- Example

.. image:: timeit_magic.png

- Optional arguments:

+------------+---------------------------------------------------------------------------------------------------------+
| -n         | execute the given statement <N> times in a loop. If this value is not given, a fitting value is chosen. |
+------------+---------------------------------------------------------------------------------------------------------+
| -r         | repeat the loop iteration <R> times and take the best result. Default: 7                                |
+------------+---------------------------------------------------------------------------------------------------------+
| -p         | use a precision of <P> digits to display the timing result. Default: 3                                  |
+------------+---------------------------------------------------------------------------------------------------------+
