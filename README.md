# Unit Testing Example Project for Cortex-M microcontrollers

This is an example project for unit testing with qemu and ceedling.
All unit tests and coverage analysis runs on qemu.
A wrapper script (qemu-test-fixture.py) preprocess the qemu output
and produces the coverage files (*.gcno and *.gcda).

To run it, install ceedling and qemu-system-arm:

* [ceedling](https://www.throwtheswitch.org/ceedling)
* [QEMU](https://www.qemu.org/)

## How it works.

We use ceedling to build the unit tests. The configuration in project.yml
file uses QEMU to run the tests.

just run ceedling on the top folder.

```
$> ceedling

--------------------
OVERALL TEST SUMMARY
--------------------
TESTED:  3
PASSED:  1
FAILED:  1
IGNORED: 1

---------------------
BUILD FAILURE SUMMARY
---------------------
Unit test failures.

```

The unit tests are linked with an small startup code (startup.c) and a
linker script (linker_script.ld) which provide the basic runtime for 
qemu to run the unit tests.

This crt outputs via semihosting the unit test results and the coverage
information. This is preprocessed by the qemu-test-fixture.py script and 
returned to ceedling.
