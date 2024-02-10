# pico-shape-detection
Running a shape detection machine learning model using accelerometer data on the Raspberry Pi Pico W.

---

## Checklist of implementation
- [x] Read accelerometer data
- [x] Capture button presses
- [x] Store accelerometer data on button press
- [x] Make tflite-micro library interface 
- [x] Feed captured data into the model 
- [x] Display classification on LEDs 
- [x] Error Handling 

---

## Running the project

### Prequisites:
- [The pico-sdk library](https://github.com/raspberrypi/pico-sdk)
- [CMake](https://cmake.org/) & Make
- minicom
- [The tflite-micro library for the Raspberry Pi Pico](https://github.com/raspberrypi/pico-tflmicro)

For the pico-sdk, follow the guide in [Raspberry Pi Pico C/C++ SDK](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf) for Windows and Debian-based<sup>[1]</sup> systems. Since I use Arch, I followed this guide to install the sdk: [Arch Linux guide](https://loads.pickle.me.uk/2021/01/25/compiling-for-the-raspberry-pico-on-arch-linux/)

[1]: https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf#quick-setup-section

ensure that the PICO_SDK_PATH refers to the actual path of pico-sdk

```bash
echo $PICO_SDK_PATH
```

The pin numbers have been chosen according to the following wiring:

![Wiring for the Raspberry Pi Pico](pico-wiring.png)

These pin numbers can be changed as per your layout:

https://github.com/risb21/pico-shape-detection/blob/c49df2e475957c3951cbea68f9695f93edb6cc59/src/main.cpp#L16-L27

In the repository directory, create a build directory, cd into it, run cmake and then make

```bash
mkdir build && cd build
cmake ../
make -j4
```

Before uploading the program to your Raspberry Pi Pico W, check for existing ACM terminals connected to the PC

```bash
ls /dev | grep ttyACM
```

Connect the cable connected to the Raspberry Pi Pico W while pressing the `BOOTSEL` button and release once connected

Mount the RPI-RP2 and copy the compiled .uf2 file from the build directory to the Raspberry Pi Pico W

```bash
udisksctl mount -b /dev/disk/by-label/RPI-RP2
cp pico_acc.uf2 /run/media/<USER>/RPI-RP2/
```

### Reading output from serial port

Find the newly connected serial terminal

```bash
ls /dev | grep ttyACM
```

using minicom, connect to the serial terminal

<pre><code>sudo minicom -b 115200 -o -D /dev/ttyACM<b><u>n</u></b></code></pre>