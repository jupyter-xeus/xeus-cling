#############################################################################
# Copyright (c) 2016, Johan Mabille, Loic Gouarin, Sylvain Corlay           #
# Copyright (c) 2016, QuantStack                                            #
#                                                                           #
# Distributed under the terms of the BSD 3-Clause License.                  #
#                                                                           #
# The full license is in the file LICENSE, distributed with this software.  #
#############################################################################

import unittest
import jupyter_kernel_test


class XCppTests(jupyter_kernel_test.KernelTests):

    kernel_name = 'xcpp14'

    # language_info.name in a kernel_info_reply should match this
    language_name = 'c++'

    # Code in the kernel's language to write "hello, world" to stdout
    code_hello_world = '#include <iostream>\nstd::cout << "hello, world" << std::endl;'''

    # Pager: code that should display something (anything) in the pager
    code_page_something = "?std::vector"

    # Samples of code which generate a result value (ie, some text
    # displayed as Out[n])
    code_execute_result = [
        {
            'code': '6 * 7',
            'result': '42'
        }
    ]

    # Samples of code which should generate a rich display output, and
    # the expected MIME type
    code_display_data = [
        {
            'code': '#include <string>\n#include "xcpp/xdisplay.hpp"\nstd::string test("foobar");\nxcpp::display(test);',
            'mime': 'text/plain'
        }
    ]

    # You can also write extra tests. We recommend putting your kernel name
    # in the method name, to avoid clashing with any tests that
    # jupyter_kernel_test adds in the future.
    def test_xcpp_stderr(self):
        reply, output_msgs = self.execute_helper(code='#include <iostream>\nstd::cerr << "oops" << std::endl;')
        self.assertEqual(output_msgs[0]['msg_type'], 'stream')
        self.assertEqual(output_msgs[0]['content']['name'], 'stderr')
        self.assertEqual(output_msgs[0]['content']['text'], 'oops')

if __name__ == '__main__':
    unittest.main()
