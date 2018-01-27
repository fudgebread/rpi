# rpi
Projects for the rpi v1 (B+). These projects should be usable on any pi
provided the correct toolchain is used. Toolchains were build using
yocto with the ipk package manager to create development packages.

# Projects
Below is a list of projects for the rpi. Each project has it's own TODOs
since they are a work-in-progress.

## GPIO
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
See LICENCE.md
