#
# Copyright (c) 2023 ZettaScale Technology
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# http://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
# which is available at https://www.apache.org/licenses/LICENSE-2.0.
#
# SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
#
# Contributors:
#   ZettaScale Zenoh Team, <zenoh@zettascale.tech>
#

# -- Run doxygen if executing on readthedocs -------------------------------
# see https://breathe.readthedocs.io/en/latest/readthedocs.html
import subprocess, os
read_the_docs_build = os.environ.get('READTHEDOCS', None) == 'True'
if read_the_docs_build:
     subprocess.call('doxygen', shell=True)
     breathe_projects = {"zenohcpp": "xml"}

# -- Project information -----------------------------------------------------
project = 'zenoh-cpp'
copyright = '2017, 2023 ZettaScale Technology'
author = 'ZettaScale Zenoh team'
# Extract the release number from the version.txt file
with open("../version.txt", "rt") as f:
    release = f.read()

# -- General configuration ---------------------------------------------------
master_doc = 'index'
html_theme = 'sphinx_rtd_theme'
cpp_index_common_prefix = ['z::', 'zenoh::', 'zenohcxx::', 'zenohc::', 'zenohpico::']

extensions = [ "breathe" ]

breathe_default_project = "zenohcpp"
breathe_order_parameters_first = True
