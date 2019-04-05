
# UEFIBoy 

Gameboy/Gameboy Color emulator.   
UEFI Subtree/Fork of [Plutoboy](https://github.com/RossMeikleham/PlutoBoy)

 ![uefiIcon](/images/uefi.png?raw=true) ![BuildStatus](https://travis-ci.org/RossMeikleham/UEFIBoy.svg?branch=master)


 ![example](/images/uefi_test.gif)

# Controls (directional keys map across)
- a -> a
- s -> b
- enter -> start
- space -> select
- 1 -> decrease screen size
- 2 -> increase screen size (cpu scaling, slows down gameplay)


# Building

```
git clone https://github.com/RossMeikleham/UEFIBoy
cd UEFIBoy
docker build -t uefi -f build/UEFI/Dockerfile
docker run -v $(pwd):/mnt uefi
```
this should produve a `Plutoboy.efi` file in the bin directory.

# Autoload setup

This setup is intended if you wish to automatically run a specified game on boot without
having to enter it manually once the EFI shell has loaded.

- Building should produce `bin/Plutoboy.efi`
- Depending on the architecture being run copy Plutoboy.efi into one or more of the following directories and rename it to the filename of the efi file specified (create these directories if they don't exist)
    - x86 : `/efi/boot/BOOTIA32.EFI`
    - x64 : `/efi/boot/BOOTX64.EFI`
    - AARCH32 : `/efi/boot/BOOTARM.EFI`
    - AARCH64 : `/efi/boot/BOOTAA64.EFI`
    - Itanium : `/efi/boot/BOOTIA64.EFI`
    
For the Gameboy ROM, place it in the root of the filesystem being used (FAT32) and rename it it to `autoload.rom`

If no `autoload.rom` is present then, the emulator will exit to the shell, and you
can then run it again with a specified rom: `/efi/boot/[machine_arch] [rom_path]` e.g. `/efi/boot/BOOTX64.efi rom.gb`

# Running (QEMU)

See https://github.com/tianocore/tianocore.github.io/wiki/How-to-run-OVMF for more
information on running QEMU with OVMF.

- Building should produce `bin/Plutoboy.efi`
- Download OVMF from https://www.kraxel.org/repos/jenkins/edk2/ (extract from the .rpm file) 
- For Autoload setup:
    - Create a folder named `hda-contents` and place the `/efi/boot/` folder in the hda-contents folder
    - Place the `autoload.rom` in `hda-contents/autoload.rom`
    - run QEMU: `qemu-system-x86_64 -L . --bios OVMF.fd -hda fat:rw:hda-contents`
- For Manual setup:
        - Create a directory named `hda-contents` and place `Plutoboy.efi` file and any ROMS in the folder
    - run QEMU: `qemu-system-x86_64 -L . --bios OVMF.fd -hda fat:rw:hda-contents`
    - Once dropped into the efi shell enter `FS0:`, then run `./plutoboy.efi [rom_path]`


# Running on Real Hardware (Not Recommended)
- Running untrusted code on bare metal may not be the best idea.
- Vendors may implement UEFI differently, so this may not work.
- Follow "Running (QEMU)" but place files on a  FAT32 formatted drive/usb stick/bootable device instead of "hda-contents"
- From the machine set to booting/running from the FAT32 drive (this is different for most machines)
