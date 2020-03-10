Advantech support board README
==============================

Supported Hardware
------------------

 * SOM-3567 (CoreBoot 4.10) only bring up
 * SOM-7567 (CoreBoot 4.10) only bring up
 * SOM-5992 (CoreBoot 4.10) only bring up

Build Requirements
------------------

Build Coreboot
==============


Download CoreBoot Source
------------------------

git clone https://github.com/coreboot/coreboot.git
cd coreboot


Download 3rd Party submodule
----------------------------

git submodule update --init --checkout


Compile relalted build tools
----------------------------

make crossgcc CPUS=16
  * It spends long time to build. Please waiting.


Select target board to build
----------------------------

make menuconfig
Optional:
    Mainboard menu
      Mainboard vendor  -> Advantech
      Mainboard model   -> SOM-3567
    
    Payload menu
      Add a Payload     -> SeaBIOS
      
make distclean  (If change board select)
make clean      (rebuild)

Build completed
---------------

\build\coreboot.rom


Website and Mailing List
------------------------


