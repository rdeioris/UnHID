# UnHID

UnHID is an Unreal Engine plugin for accessing the low-level HID (Human Interface Device) api of your operating system (Windows, Mac and Linux are supported).

It allows advanced access to keyboards, mices, gamepads, joysticks, lighting and illumination devices, touchscreens, haptics and everything covered by the HID standard (https://www.usb.org/hid)

While generally applied to USB devices, HID works over various transports (included bluetooth and I2C). This plugin is transport agnostic.

## Main features

* Enumeration and filtering of HID devices
* Async receive of messages (reports)
* Support for Feature Reports (required for lighting and illumination)
* Access to Descriptor Reports
* Parsing api, included automatic offseting and sizing based on Report Descriptors
* A virtual input system that can be injected with the data coming from the HID devices
* Debug tools for managing devices, accessing the official HID tables (HUT) and testing the virtual input system.

## Quickstart

## Commercial Support
