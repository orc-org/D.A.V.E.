## GNSS Module Setup (SparkFun NEO-F10N)

This guide walks through configuring the SparkFun NEO-F10N GNSS module via USB on an NVIDIA Jetson Orin Nano running JetPack 6.0 (Linux Kernel 5.15).

---

### 1. Hardware Requirements & Wiring

* **Device:** SparkFun NEO-F10N GNSS Breakout
* **Interface:** USB-C (uses the On-Board CH340 USB-to-Serial bridge)
* **Cable Requirement:** Must use a **USB Data Cable** (power-only cables will power the LED but fail to enumerate hardware).

---

### 2. Kernel Driver Setup (`ch341.ko`)

JetPack 6 does not ship with the `ch341` USB-to-serial driver by default. You must compile and load the module against local Tegra kernel headers.

#### A. Install Build Tools
```bash
sudo apt update
sudo apt install -y git build-essential ncurses-dev xz-utils libssl-dev bc flex bison

B. Compile the Module

Create a temporary build directory, download the standard Linux kernel 5.15 driver source, and compile:

mkdir -p ~/ch341_build && cd ~/ch341_build

# Fetch the v5.15 driver source matching JetPack 6
wget [https://raw.githubusercontent.com/torvalds/linux/v5.15/drivers/usb/serial/ch341.c](https://raw.githubusercontent.com/torvalds/linux/v5.15/drivers/usb/serial/ch341.c)

# Create Makefile pointing to internal kernel headers
cat << 'EOF' > Makefile
obj-m += ch341.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
EOF

# Compile
make

C. Install & Load Module

Copy the built .ko module to system drivers and register it:

# Copy module
sudo cp ch341.ko /lib/modules/$(uname -r)/kernel/drivers/usb/serial/

# Update dependencies & load module
sudo depmod -a
sudo modprobe ch341

3. Disable Interfering Services (brltty)

Ubuntu's brltty daemon automatically hijacks serial-to-USB devices upon connection, causing /dev/ttyUSB0 to disconnect immediately. Uninstall it:

sudo apt remove -y brltty

4. User Permissions (Non-Root Access)

To allow ROS 2 nodes to access /dev/ttyUSB0 without sudo, grant your user account access to the dialout group:

sudo usermod -a -G dialout $USER

5. Verification & Testing

    Unplug and re-plug the USB-C cable into the Jetson.

    Confirm the serial device is active:

ls -l /dev/ttyUSB0

can test using test_gps.py.
