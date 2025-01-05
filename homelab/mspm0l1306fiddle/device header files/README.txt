last update 1/4/2025
-----------------------------------------------------------------------

there is no single header file from TI. currently, headers can be found as part of the offical SDK.
there is no convenient init/config documentation for the msp0l1306 currently.

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
    - ti-drivers are included in the SDK, are RTOS and multi threaded safe (unless noteed otherwise).
        driver documentation : https://software-dl.ti.com/msp430/esd/MSPM0-SDK/2_03_00_07/docs/english/drivers/mspm0g1x0x_g3x0x_api_guide/html/index.html
    - SDK also has freertos examples (for some chip models)
    - additional refrences and manuals are on the SDK download page
    - I have not found a way to just get the headers without installing the entire SDK, nor have found a register only header file.
    - will have to add the SDK to the CCS compiler include path. for me it was; project > properties > build > arm compiler > include options > add "C:/ti/mspm0_sdk_2_03_00_07/source" , must be forward slash
    - production programming overview : https://www.ti.com/video/6341315944112
    - msp0l1306 specific documentation does not exist :( (as in docs that descrive initialization/configuration steps). the msp0l1306 doesn't have anythign posted where other (and older) TI products that do have technical refrences (specifcally with configuration steps)
        normally post the docs. "See the device-specific data sheet for these details." is all TI says (the docs are not shown on their documentation listings as of Jan 2025).
        for now thigns can be configured though ti's "sysconfig" software, from then you can trace the generated file "ti_msp_dl_config.*" . 
        no chips of the msp0l series have docs, lookign at other(and popular) arm chips it appears(?) like somethign around 3 years after release before docs come out :skull-emoji:, so expect ~2026-2028? side note: sysconfig & cloud dev is a plauge. 
    - sysconfig is node js app therfore it is your moral responsibility to hate it.
    