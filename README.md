# dapLINK
Tired of digging in a drawer looking for a JTAG probe only to find your board had the wrong connector? You're in luck.
dapLINK is an embedded firmware project that enhances developer productivity. Every modern development board should 
have a circuit that runs the dapLINK firmware. The firmware runs on many reference circuits including the ARMmbed HDK 
and Freescale OpenSDA.

**Sounds too good to be true? Keep reading.**

The dapLINK application is a USB composite device compatible with Windows, Mac and Linux that exposes three USB classes:
- CDC: virtual serial port connection for re-targeting stdio
- HID: a CMSIS-DAP SWD/JTAG tunnel between the PC and your MCU
- MSC: drag-n-drop flash programming via SWD/JTAG

dapLINK is an open-source (Apache 2.0) project that is actively being developed by ARM, it's partners and the developer 
community. Contributions are always welcome.


# Getting Started
Here are some of the known [development boards capable of running the dapLINK application]
(https://developer.mbed.org/platforms/?interface=8)
If your development board isn't listed there may be getting started notes elsewhere.

###  Using dapLINK
When connecting your dapLINK powered development board to a computer the *.htm file found on the drive should notify 
you if an update is available when clicked (internet connection required). Otherwise, all [firmware and releases 
notes are found here](https://github.com/ARMmbed)

### Building dapLINK
**Step 1.** Install dependencies
 1. Install Python 2.7.x
 2. Install [pyYAML](https://github.com/yaml/pyyaml)
 3. Install [Setuptools](https://pypi.python.org/pypi/distribute)
 4. Install [Jinja2](https://pypi.python.org/pypi/Jinja2)

**Step 2.** Create the project files using [project_generator](https://github.com/0xc0170/project_generator). All 
commands are ran from the **tools** directory
```
 tools>python project_generator/export.py -f records/projects.yaml
```

**Step 3.** Build all projects that were generated
```
 tools>python build.py
```

### adding dapLINK
Adding dapLINK to a new (and sometimes existing) development board should be quite easy. For more information see the 
[dapLINK porting guide](https://github.com/ARMmbed)

## Contact
For discussing the development of the CMSIS-DAP Interface Firmware please join our [mbed-devel mailing list]
(https://groups.google.com/forum/?fromgroups#!forum/mbed-devel).
[Porting the FW to new boards](http://mbed.org/handbook/cmsis-dap-interface-firmware)


## Notes

**Some things left to sort out**

* mbed_ser driver installer to not require hardware be connected
* Change USB to expose HID without needing the CDC driver
* Implement XON/XOFF flow control
* Incorporate mbed build system
* Automate basic cross OS tests
* Change application offset address to 0x8000 for all targets with a bootloader
* Only erase sectors that code fits in (when needed)
* Use media eject for MSD
* Verify semi-hosting on HID connection
* RAM allocation for virtual file-system hidden files and folders
* version.txt file for offline, non html-access to the bootloader and CMSIS-DAP version
* Add hex file support. Exists on master but needs verification and testing
* Add srec file support
* etc...

**Tests**
A baseline for testing the dap firmware should cover:

1. program hello world echo application onto target MCU
 * copy using cmd line method
 * copy using drag n drop method
 * Top 3 browsers for each OS
2. flash with pyOCD
 * change demo app with package to be hello world echo

Note: Test should be completed when echo is received so CDC support is inclusive to all tests.

Note: To automate bootloader testing it may be worth putting a sequence in the dap firmware to execute the bootloader (ie: erase vector table at offset and reset)

**Directory**
* **bootloader** - source files that are only used by the bootloader
* **dap**- source files that are only used by the CMSIS-DAP application
* **flash** - Source files to build position independent flash algorithms. [Adding new flash algorithms](http://keil.com/support/man/docs/ulink2/ulink2_su_newalgorithms.htm)
* **shared** - A software component that is used by __both__ bootloader and interface
* **tools** - python files for building and testing the projects as well as exporting project files for debugging

```
+---source
    +---bootloader
        +---hal
            +---mcu
    +---daplink
        +---hal
            +---mcu
    +---common
        +---cmsis_core
            +---freescale
            +---nxp
        +---cmsis_dap
            +---hal
                +---mcu
        +---flash_algorithms
            +---hal
                +---mcu
        +---shared
            +---mcu
        +---rtos
        +---usb
+---tools
    +---project_generator
    +---records
    +---test
```
