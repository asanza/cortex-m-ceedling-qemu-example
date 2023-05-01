# Unit Testing Example Project for Cortex-M microcontrollers

This is an example project for unit testing with qemu and ceedling.

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