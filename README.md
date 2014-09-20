DS2PlusPlus
-----------

This library, including its code, its data, and all of its other constitutent elements, is licensed under the [GNU LGPL v3.0](http://www.gnu.org/licenses/lgpl-3.0.html).

tl;dr You're welcome to use this in a commercial, closed source application.  However any changes you make to libds2 or the ECU definition files must be made available.  Sharing is caring, right?

###Contents###

####`libds2`###

A Qt/C++ library to interface with BMW control units.

####`ds2-dump`####

A command line program that can: compile DPP-JSON files, run arbitarary commands against an control unit, and identify control units installed on a car, as well as run a series of commands and log the output to a CSV file.

####`dpp-json`####

Control unit and string table definition files.

####`dpp-tools`####

Tools for validating the DPP-JSON files contained within.

####`tests`###

Tests, of course.  Written using QTestLib.  Running `make check` will excute the tests.

###Getting Started###

####Requirements###

* Qt 5.2 or newer with the `sql` module.

####Building Everything###

Use Qt Creator for GUI goodness.  From the commandline use qmake (ex: `qmake DS2PlusPlus.pro`).
