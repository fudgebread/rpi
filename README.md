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
$ scp <package>.ipk root@<ip-address>:/home/root
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
	
### Limitations
Current limitations are:
- GPIO ranges currently limited to rpi v1 (2-26).

### TODOs
List of features to implement:
- Add polling functionality (read/write)
- Cleaup build artefacts

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
