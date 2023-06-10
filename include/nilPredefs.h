#pragma once
#include "nilConfig.h"

namespace nil {

  //! USB Usage Page IDs.
  //! See See USB HID Usage Tables version 1.1, page 15
  enum USBUsagePage
  {
    USBUsagePage_Desktop = 0x01
  };

  //! USB Generic Desktop Page usage IDs.
  //! See USB HID Usage Tables version 1.1, page 27
  enum USBDesktopUsage
  {
    USBDesktopUsage_Mice = 0x02,
    USBDesktopUsage_Joysticks = 0x04,
    USBDesktopUsage_Gamepads = 0x05,
    USBDesktopUsage_Keyboards = 0x06,
    USBDesktopUsage_MultiAxes = 0x08
  };

  //! Known USB vendor IDs that might be useful.
  enum USBKnownVendor : uint16_t
  {
    USBVendor_Microsoft = 0x045E, //!< Microsoft
    USBVendor_Logitech = 0x046D, //!< Logitech
    USBVendor_Sony = 0x054C, //!< Sony
    USBVendor_Razer = 0x1532, //!< Razer
    USBVendor_Nacon = 0x146B, //!< Nacon
    USBVendor_Kingston = 0x0951, //!< Kingston/HyperX
    USBVendor_Corsair = 0x1B1C, //!< Corsair
    USBVendor_Apple = 0x05AC, //!< Apple (+ Keychron?)
    USBVendor_Alienware = 0x04F2, //!< Alienware
    USBVendor_Metadot = 0x24F0, //!< Metadot (Das Keyboard)
    USBVendor_Ducky = 0x04D9, //!< Ducky
    USBVendor_Roccat = 0x1E7D, //!< Roccat
    USBVendor_SteelSeries = 0x1038 //!< SteelSeries
  };

  enum KnownDeviceType
  {
    KnownDevice_Unknown = 0,
    KnownDevice_DualShock4,
    KnownDevice_DualSense
  };

  struct KnownDeviceRecord
  {
    USBKnownVendor vid;
    uint16_t pid;
    KnownDeviceType type;
    utf8String name;
    uint8_t reportID_Serial;
  };

  enum HIDConnectionType
  {
    HIDConnection_Unknown = 0,
    HIDConnection_USB,
    HIDConnection_Bluetooth
  };

}