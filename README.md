DS2PlusPlus
-----------

This library and all of its constitutent elements are licensed under the LGPL v3.0.

###Contents###

####`libds2`###

A Qt/C++ library to interface with BMW control units.

####`ds2-dump`####

A quick program that can: process DPP-JSON files, run arbitarary commands against an ECU, and identify an ECU.

####`dpp-json`####

ECU definition files.

####`dpp-tools`####

Tools for validating the DPP-JSON files contained within.

####`tests`###

Tests, of course.  Written using QTestLib.  Running `make check` will excute the tests.

###Getting Started###

####Requirements###

* Qt 5.2 or newer with the `serialport` and `sql` modules.

####Building Everything###

Use Qt Creator for GUI goodness.  From the commandline use qmake (ex: `qmake DS2PlusPlus.pro`).
