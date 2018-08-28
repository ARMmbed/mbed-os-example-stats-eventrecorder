# Getting started with CPU Stats on uVision Event Recorder

This guide reviews the steps required to get Mbed OS CPU statistics on EventRecorder.

Please install [mbed CLI](https://github.com/ARMmbed/mbed-cli#installing-mbed-cli).

## Import the example application

From the command-line, import the example:

```
mbed import mbed-os-example-stats-eventrecorder
cd mbed-os-example-stats-eventrecorder
```

### Update the linker script for your target device as per `linker_change_example.patch` (K64F)
Linker script update is required to place the event recorder in uninitialized memory.

### Now export to uVision

Invoke `mbed export`, and specify the name of your platform and uVision

```
mbed export -m K64F -i uVision
```

### Enable EventRecorder with the help of below mentioned link
https://www.keil.com/pack/doc/compiler/EventRecorder/html/er_use.html

### Build and execute program
When executing this example in the µVision debugger, use the menu command View - Analysis Windows - Event Recorder to open the Event Recorder window. 

### Known Issues
Redefinition of _RTE_ - `RTE_build_issue.patch` Use this patch to fix the build issue