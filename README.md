# DIY Casiotone Bluetooth 🎹📡


This project is intended to provide a low cost alternative to the Casio [WU-BT10](https://www.casio.com/intl/electronic-musical-instruments/options/product.WU-BT10/) for Casiotone keyboard models; AP-300, AP-550, AP-750, AP-S190, AP-S200, AP-S450, CDP-S360, CT-S1-76,  CT-S1000V, CT-S1,CT-S400, CT-S410, CT-S500, LK-S450, PX-S1100, PX-S3100, PX-S5000, PX-S6000, PX-S7000

Note: This has only been tested on the Casiotone CT-S1, but should work on any Casiotone keyboard listed above provided they have both a micro USB, and an USB A port.

The device will allow you to wirelessly connect to Midi keyboard apps on Android, IOS, MacOS, etc. It will also allow you to connect to the Casio Music Space app available on [Android](https://play.google.com/store/apps/details?id=jp.co.casio.CasioMusicCity&hl=en)(tested) or [IOS](https://apps.apple.com/us/app/casio-music-space/id1561343853)(not tested)

This project is a fork of work by [sauloverissimo](https://github.com/sauloverissimo/ESP32_Host_MIDI). With code changes specifically for compatibility with Casiotone keyboards

---


## Hardware requirements

- [ESP32-S3 development board](https://www.aliexpress.com/item/1005010674295938.html). The s3-n16r8 variant will be ideal. You will not need the one with soldered pins, but it will not hinder the project either. You should be able to get this for less than $10 USD.
- USB-C OTG adaptor
- USB Micro cable
- USB A to USB C cable (This can be an inexpensive, charging only USB cable)

- A soldering iron and some solder (This requires only minimal soldering and can be done without previous soldering experience. If you don't have one, ask a friend.)

<img src="https://github.com/user-attachments/assets/55aa9c89-d56f-40d1-85db-de4259ea5d72" width="500" />


---

## Overview

This project mimics the BLE Midi functionality of the WU-BT10, allowing pairing of Casiotone keyboards with other devices; iPad, Android, MacOS, Windows, and the use of Midi piano apps, including Casio Music Space.

**Unlike the WU-BT10, this project *does not* support bluetooth audio-in, this is a limitation of the Micro USB port on the Casiotone devices, which does not USB Audio Class as a device.**

---



## Soldering

In order to initiate a handshake with the micro usb port on the keyboard, the keyboard's port must receive power from the ESP32 board. At factory settings, the OTG port does not provide power. This is easily addressed by shorting two jumper pads on the bottom of the board.

<img src="https://github.com/user-attachments/assets/d6b4349f-8bb8-4ade-8848-3ed296a593ad" width="400" />
<img src="https://github.com/user-attachments/assets/4ad86d60-96f9-4796-8f7c-5a6448239871" width="400" />



One might be temped to try a split OTG cable (such as this [this](https://www.amazon.com.au/dp/B08C5FWQND)) with an additional port for power to avoid soldering. **In testing this has not appeared to work.**



##  Setup

- This project uses the [arduino IDE](https://www.arduino.cc/en/software) software. Install this if you haven't already.
- Download this repo and extract into your arduino folder
- Copy the contents of the src folder into the DIY_Casiotone_Bluetooth/examples/Raw-USB-BLE/ folder.
- Plug the ESP32 into the computer via the com port, lablelled on the underside of the board.
- Open the Raw-USB-BLE.ino in the IDE and click upload
- The device is now ready to be plugged into the Casiotone keyboard using the cable configuration detailed below. However if you want to confirm it is working first, keep the ESP32 plugged into the computer, click the "Serial monitor" icon up the top right, and plug the ESP32's OTG port into the Casiotone keyboard's micro USB port. You should see "MIDI device connected!" in the IDE console, and key presses on your keyboard should appear in the console. You can alse test BLE connectivity. by attempting to connect a device. The ESP32 will broadcast as 'WU-BT10'.

## Cable Configuration
 
The ESP32-S3 has two USB ports, on the under side of the board, these are labelled OTG, and COM. We will use COM to power the device using the USB A port on the back of the Casiotone keyboard, we will then connect the OTG port to the micro usb port on the back of the Casiotone keyboard using an OTG cable and a micro usb cable. this is from where it will recieve the MIDI signals.

Unlike the WU-BT10, we cannot use the USB A port for MIDI out, as it uses a proprietary handshake to connect to the WU-BT 10. We can however exploit it to power our imposter device. 

---

## Troubleshooting

### Nothing happens when I plug the device into the keyboard

- Make sure you have tested while connected to the IDE with the serial monitor open.
- Try a different cable. If you are using a cheap cable, it may be for charging only.

### The ESP32 connects but easily disconnects

- This is likely to be cable related. Ensure you are using a high quality cable.


### I am experiencing input lag with BLE

- This is a limitation of the BLE protocol unfortunately, and is a reported issue even with the first party WU-BT10.

---

## Other points

- You can change the name the device advertises itself as via BLE by changing a line in the RAW-USB-BLE.ino

```
    bleMidi.begin("WU-BT10 MIDI");
```
Change this to whatever you want it to be broadcast as, but note the 'Casio Music Space' app will only connect if it 



## Case

- A case can be 3D printed using this case made by [aiekick on Makerworld](https://makerworld.com/en/models/1456361-esp32-s3-wroom-case#profileId-1517915i)
- The case will fit the board linked in the hardware section whether you have selected the one with soldered pins or not.

![Case]


## Issues

- [ ] Device does not automatically reconnect if connection is lost (e.g if the port is jostled)
- [ ] Currently requires the OTG port to be unplugged on boot, then plugged in, in order to establish a connection
