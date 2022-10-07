# nRFICE
### nRF5340 with ICE40UP5K including JLink OB for instant BLE and FPGA development with zero dongles
In order to develop a number of comercial products, we (here at Hurley Research in Santa Cruz CA) needed a simple bluetooth 5 plus FPGA platform. nRFICE is meant to be a maximally simple solution for launching projects that require MCU plus FPGA and certainly if Bluetooth including mesh is needed.
### Arduino Uno compatibility
nRFICE is physically compatible with Arduino Uno shields, but take care that the GPIO level is 3.3V.
### JLink OB
Comercial eval/dev boards from chip manufactures, such as Nordic Semi, typically have the JLink debugger hardware embedded on the board for convenience. We have included JLink OB on nRFICE, allowing developers to get started without any additional hardare, programming dongles, etc. Out of box demos for both nRF5340 and ICE40UP5K can be loaded and debugged immediately with a single usb cable, or over BLE with the nRFICE android app.
