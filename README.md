# RPI
Projects for the rpi v1 (B+). These projects should be usable on any pi
provided the correct toolchain is used. Toolchains were build using
yocto with the ipk package manager to create development packages.

## Getting Started
To get started (assuming code is cloned):
```sh
$ ./setup -s rpi
$ cd <project>
$ make 
$ make deploy
$ scp <package>.ipk root@<ip-address>:<root home>
```

On your rpi:
```sh
opkg install <package>.ipk
```

# Projects
Below is a list of projects for the rpi. Each project has it's own TODOs
since they are a work-in-progress.

## GPIO - userspace
Experimentation with rpi GPIO from userspace using the sysfs interface.
Current implementation creates a mock console in-which the user can do
GPIO operations:
- open, set direction, and close
- read and write

### Examples
```sh
GPIO > help
 Help
 =====
 status                - display status of all gpio 
 open <gpio>           - open <gpio>
 close <gpio>          - close gpio <gpio>
 dir <gpio> [in/out]   - set direction for gpio
 read <gpio>           - read gpio <gpio>
 write <gpio> <value>  - write value to <gpio>
GPIO > status

 GPIO | Open | Dir | Value
 -----+------+-----+------
 2    | no   | NA  | NA  
 3    | no   | NA  | NA  
 4    | no   | NA  | NA  
 5    | no   | NA  | NA  
 6    | no   | NA  | NA  
 7    | no   | NA  | NA  
 8    | no   | NA  | NA  
 9    | no   | NA  | NA  
 10   | no   | NA  | NA  
 11   | no   | NA  | NA  
 12   | no   | NA  | NA  
 13   | no   | NA  | NA  
 14   | no   | NA  | NA  
 15   | no   | NA  | NA  
 16   | no   | NA  | NA  
 17   | no   | NA  | NA  
 18   | no   | NA  | NA  
 19   | no   | NA  | NA  
 20   | no   | NA  | NA  
 21   | no   | NA  | NA  
 22   | no   | NA  | NA  
 23   | no   | NA  | NA  
 24   | no   | NA  | NA  
 25   | no   | NA  | NA  
 26   | no   | NA  | NA 
 
GPIO > open 26
GPIO > dir 26 out
GPIO > write 26 1
GPIO > write 26 0
GPIO > close 26
GPIO > quit
```
### Limitations
Current limitations are:
- GPIO ranges currently limited to rpi v1 (2-26).

### TODOs
List of features to implement:
- Add polling functionality (read/write)
- Cleanup build artefacts
- Add enum for gpios for better reporting of state
- Autotests

## GPIO - kernel
Not started yet...

# Yocto
Images and toolchains for rpi v1 (B+) built using the following layers:
- poky/meta
- poky/meta-poky
- poky/meta-yocto-bsp
- poky/meta-raspberrypi

# Docs
- https://www.raspberrypi.org/documentation/usage/gpio-plus-and-raspi2/
- https://www.raspberrypi.org/documentation/hardware/computemodule/RPI-CM-DATASHEET-V1_0.pdf

# Licence
See LICENCE
