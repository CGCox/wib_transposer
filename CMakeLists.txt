

# This is a skeleton CMakeLists.txt file, auto-generated on
# Wed Oct 19 17:55:03 CEST 2022.  The developer(s) of this package should delete
# this comment as well as adding dependent targets, packages,
# etc. specific to the package. For details on how to write a package,
# please see
# https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/

cmake_minimum_required(VERSION 3.12)
project(wib_transposer VERSION 0.1.0)

find_package(daq-cmake REQUIRED)
find_package(fmt REQUIRED)
find_package(CLI11 REQUIRED)
find_package(detdataformats REQUIRED)

daq_setup_environment()


find_package(Boost COMPONENTS unit_test_framework REQUIRED)

##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_library

daq_add_library( *.cpp LINK_LIBRARIES fmt::fmt) # Any source files and/or dependent libraries to link in not yet determined

##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_application

daq_add_application(simple_read_protowib simple_read_protowib.cxx LINK_LIBRARIES ${PROJECT_NAME} CLI11::CLI11 detdataformats::detdataformats) # Any libraries to link in not yet determined
daq_add_application(block_read_protowib block_read_protowib.cxx LINK_LIBRARIES ${PROJECT_NAME} CLI11::CLI11 detdataformats::detdataformats ) # Any libraries to link in not yet determined
daq_add_application(simple_read_dunewib simple_read_dunewib.cxx LINK_LIBRARIES ${PROJECT_NAME} CLI11::CLI11 detdataformats::detdataformats) # Any libraries to link in not yet determined  
daq_add_application(block_read_dunewib block_read_dunewib.cxx LINK_LIBRARIES ${PROJECT_NAME} CLI11::CLI11 detdataformats::detdataformats) # Any libraries to link in not yet determined
daq_add_application(print_block_read_dunewib print_block_read_dunewib.cxx LINK_LIBRARIES ${PROJECT_NAME} CLI11::CLI11 detdataformats::detdataformats) # Any libraries to link in not yet determined  

##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_unit_test

# daq_add_unit_test(Placeholder_test LINK_LIBRARIES)  # Placeholder_test should be replaced with real unit tests

##############################################################################

daq_install()

