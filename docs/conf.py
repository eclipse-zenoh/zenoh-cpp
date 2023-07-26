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

# -- Project information -----------------------------------------------------
project = 'zenoh-cpp'
copyright = '2017, 2023 ZettaScale Technology'
author = 'ZettaScale Zenoh team'
release = '0.10.0.0'

# -- General configuration ---------------------------------------------------
master_doc = 'index'
html_theme = 'sphinx_rtd_theme'

extensions = [ "breathe" ]

breathe_default_project = "zenohcpp"