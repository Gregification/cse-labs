last update 1/4/2025
-----------------------------------------------------------------------

there is no register level init/config documentation from TI currently.

HOW TO GET HEADER FILES
    download the MSPM0 SDK from TI. header files are in the ti/devices folder.
    see section 2.2 of the user guide for exact description of "device hgeader files" location.
    SDK userguide link: https://software-dl.ti.com/msp430/esd/MSPM0-SDK/2_03_00_07/docs/english/sdk_users_guide/doc_guide/doc_guide-srcs/sdk_users_guide.html#getting-started

HOW TO DOWNLOAD MSPM0 SDK
    1. go here : https://www.ti.com/product/MSPM0L1306#software-development
    2. scroll down and find the "software developement" seciton
    3. download the MSPM0-SDK. (done though a installer, ~1.1GB final size)

THINGS TO NOTE
    - the SDK has many other things in it; documentation, ti-drivers, random middleware, sysconfig, code examples, more libraries, etc.
    - "ti-driver" is included in the SDK, are RTOS and multi threaded safe (unless noteed otherwise). does not cover all features.
    - "driverlib" is included in the SDK, idk the offical difference but driverlib seems to be what Sysconfig uses.
    - additional refrences and manuals are on the SDK download page
    - will have to add the SDK to the CCS compiler include path.
    - production programming overview : https://www.ti.com/video/6341315944112
    - msp0l1306 specific documentation does not exist :( . expect ~2028? if ever. side note: sysconfig & cloud dev is a plauge. 
    - sysconfig is node js app therfore it is your moral responsibility to hate it.
    