# UnHID

![UnHID Logo](Docs/Images/UnHID_Logo256.png)

UnHID is an Unreal Engine plugin for accessing the low-level HID (Human Interface Device) api of your operating system (Windows, Mac and Linux are supported).

It allows advanced access to keyboards, mices, gamepads, joysticks, lighting and illumination devices, touchscreens, haptics, steering wheels and everything covered by the HID standard (https://www.usb.org/hid)

While generally applied to USB devices, HID works over various transports (included bluetooth and I2C). This plugin is transport agnostic.

Join the discord server for support: https://discord.gg/NWyXyf3tMq

## Main features

* Enumeration and filtering of HID devices
* Async receive of messages (reports)
* Support for Feature Reports (required for lighting and illumination)
* Access to Descriptor Reports
* Parsing api, included automatic offseting and sizing based on Report Descriptors
* A virtual input system that can be injected with the data coming from the HID devices
* Debug tools for managing devices, accessing the official HID tables (HUT) and testing the virtual input system.

## Usage

We are going to cover 3 examples:

* Managing the Eject key of a multimedia keyboard
* Reading the analog axis of a flight simulator joystick
* Setting the color of lights of a gaming mouse
* Injecting data in the virtual input system

### Step0: checking everything is working with the UnHID Dashboard

Once the plugin is installed (if you are using a gthub source release, just clone it into the directory Plugins/ of your C++ project and regenerate the solution) you will find the Dashboard in
the Tools/Debug menu:

The Dashboard shows all of the detected HID devices on your system and by clicking on "Connect" you can start receiving (or sending) messages ("reports" in HID slang).

NOTE: operating systems have various protection mechanisms for keyboards and mice, so if you plan to interact with them read the notes [below](#-Dealing-with-security-limitations-on-keyboards-and-mice) 

If you have a gamepad or joystick connected to your system, those are good candidates for a check:

### Step1: Enumerating and Opening a Device

### Step2: Parsing Reports

### Step3: Dealing with a flight simulator joystick

### Step4: Setting led colors on a gaming mouse

### Step5: Injecting input events from the flight simulator joystick

## Debug tools

## API

## Dealing with security limitations on keyboards and mice

## Commercial Support

Commercial support is offered by Blitter S.r.l. (Italy). Contact the admin of the discord server (rdeioris).
