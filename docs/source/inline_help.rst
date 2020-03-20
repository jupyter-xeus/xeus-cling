.. Copyright (c) 2017, Johan Mabille, Loic Gouarin and Sylvain Corlay

   Distributed under the terms of the BSD 3-Clause License.

   The full license is in the file LICENSE, distributed with this software.

.. raw:: html

   <style>
   .rst-content .section>img {
       border: 1px solid #e1e4e5;
   }
   </style>

Inline documentation
====================

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
tag file for your library must be placed under the ``xeus-cling`` "data"
directory of the installation prefix, namely

.. code::

   PREFIX/share/xeus-cling/tagfiles

For ``xeus-cling`` to be able to make use of that information, a JSON
configuration file must be placed under the ``xeus-cling`` `configuration`
directory of the installation prefix, namely

.. code::

   PREFIX/etc/xeus-cling/tags.d

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

The xhale_ Python package can be used to convert the sphinx inventory files
produced breathe into doxygen tag files.

.. image:: xtensor.png

.. _cppreference: https://en.cppreference.com
.. _`relevant section`: https://www.stack.nl/~dimitri/doxygen/manual/external.html
.. _breathe: https://breathe.readthedocs.io
.. _xhale: https://xhale.readthedocs.io
