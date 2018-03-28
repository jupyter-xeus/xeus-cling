.. Copyright (c) 2017, Johan Mabille, Loic Gouarin and Sylvain Corlay

   Distributed under the terms of the BSD 3-Clause License.

   The full license is in the file LICENSE, distributed with this software.

.. raw:: html

   <style>
   .rst-content .section>img {
       border: 1px solid #e1e4e5;
   }
   </style>

Accessing documentation
=======================

The standard library
--------------------

The ``xeus-cling`` kernel allows users to access help on functions and classes
of the standard library.

In a code cell, typing ``?std::vector`` will simply display the help page on
vector from the cppreference_ website.

.. image:: help.png

Enabling the quick-help feature for third-party libraries
---------------------------------------------------------

The quick help feature can be enabled for other libraries. To do so, a doxygen
tag file for your library must be placed under the ``xcpp`` `data` directory of
the installation prefix, namely

.. code::

   PREFIX/share/xcpp/tagfiles

For ``xeus-cling`` to be able to make use of that information, a JSON
configuration file must be placed under the ``xcpp`` `configuration` directory
of the installation prefix, namely

.. code::

   PREFIX/etc/xcpp/tags.d

.. note::

   For more information on how to generate tag files for a doxygen
   documentation, check the `relevant section`_ of the doxygen documentation.

The format for the JSON configuration file is the following

.. code:: json

   {
       "url": "Base URL for the documentation",
       "tagfile": "Name of the doxygen tagfile"
   }

For example the JSON configuration file for the documentation of the standard
library is

.. code:: json

    {
        "url": "https://en.cppreference.com/w/",
        "tagfile": "cppreference-doxygen-web.tag.xml"
    }

.. note::

   We recommend that you only use the ``https`` protocol for the URL. Indeed,
   when the notebook is served over ``https``, content from unsecure sources
   will not be rendered.

The case of breathe and sphinx documentation
--------------------------------------------

Another popular documentation system is the combination of doxygen and sphinx,
thanks for the breathe_ package, which generates sphinx documentation using the
XML output of doxygen.

A Python script for converting a sphinx inventory into a doxygen tag file is
available with xeus-cling. From there, the enabling of the live documentation
in xeus-cling works in the same fashion.

The case  of the ``xtensor`` library was handled with the conversion of the
sphinx inventory.

.. image:: xtensor.png

.. _cppreference: https://en.cppreference.com
.. _`relevant section`: https://www.stack.nl/~dimitri/doxygen/manual/external.html
.. _breathe: https://breathe.readthedocs.io
