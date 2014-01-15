//
// Logitech Gaming LED SDK
//
// Copyright (C) 2011-2012 Logitech. All rights reserved.
//

#pragma once

// Device types for LogiLedSaveCurrentLighting, LogiLedSetLighting, LogiLedRestoreLighting
const int LOGITECH_LED_MOUSE = 0x0001;
const int LOGITECH_LED_KEYBOARD = 0x0002;
const int LOGITECH_LED_ALL = LOGITECH_LED_MOUSE | LOGITECH_LED_KEYBOARD;

BOOL LogiLedInit();
BOOL LogiLedSaveCurrentLighting(int deviceType);
BOOL LogiLedSetLighting(int deviceType, int redPercentage, int greenPercentage, int bluePercentage);
BOOL LogiLedRestoreLighting(int deviceType);
void LogiLedShutdown();
