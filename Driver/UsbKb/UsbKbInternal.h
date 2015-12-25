/** @file
  Header file for USB Keyboard Driver's Data Structures

  Copyright (c) 2004 - 2007, Intel Corporation<BR>
  Portions Copyright (C) 2005 - 2015, Apple Inc<BR>
  All rights reserved.  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php/

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#ifndef USB_KB_H_
#define USB_KB_H_

#include <IndustryStandard/usb.h>
#include <IndustryStandard/AppleHid.h>

#include EFI_PROTOCOL_CONSUMER (UsbIo)
#include APPLE_PROTOCOL_DEPENDENCY (AppleKeyMapDatabase)
#include APPLE_PROTOCOL_PRODUCER (KeyboardInformationImpl)

#include <Library/AppleDriverLib.h>

// EFI_USB_KEYBOARD_DRIVER_GUID
#define EFI_USB_KEYBOARD_DRIVER_GUID \
  { 0xA05F5F78, 0x0FB3, 0x4D10, { 0x90, 0x90, 0xAC, 0x04, 0x6E, 0xEB, 0x7C, 0x3C } }

// MAX_KEY_ALLOWED
#define MAX_KEY_ALLOWED  32

/// @{
#define USBKBD_REPEAT_DELAY ((HZ) / 2)
#define USBKBD_REPEAT_RATE  ((HZ) / 50)
/// @}

#define CLASS_HID          3
#define SUBCLASS_BOOT      1
#define PROTOCOL_KEYBOARD  1

/// @{
#define BOOT_PROTOCOL    0
#define REPORT_PROTOCOL  1
/// @}

// USB_KEYCODE_MAX_MAKE
#define USB_KEYCODE_MAX_MAKE  0x64

// USB_KEY
typedef struct {
  UINT8 Down;
  UINT8 KeyCode;
} USB_KEY;

// USB_KB_BUFFER
typedef struct {
  USB_KEY Buffer[MAX_KEY_ALLOWED + 1];  ///< 
  UINT8   Head;                        ///< 
  UINT8   Tail;                        ///< 
} USB_KB_BUFFER;

// KB_MODIFIER
typedef struct {
  UINT8 Mask;  ///<
  UINT8 Key;   ///< 
} KB_MODIFIER;

// LED_MAP
typedef struct {
  UINT8 NumLock    : 1;
  UINT8 CapsLock   : 1;
  UINT8 ScrollLock : 1;
  UINT8 Reserved   : 5;
} LED_MAP;

/// @{
#define USB_KB_DEV_SIGNATURE  EFI_SIGNATURE_32 ('u', 'k', 'b', 'd')

// USB_KB_DEV_FROM_THIS
#define USB_KB_DEV_FROM_THIS(This) \
    CR((This), USB_KB_DEV, SimpleInput, USB_KB_DEV_SIGNATURE)
/// @}

// USB_KB_DEV
typedef struct {
  UINTN                           Signature;              ///<
  EFI_DEVICE_PATH_PROTOCOL        *DevicePath;            ///<
  EFI_EVENT                       DelayedRecoveryEvent;   ///<
  EFI_SIMPLE_TEXT_IN_PROTOCOL     SimpleInput;            ///<
  APPLE_KEY_MAP_DATABASE_PROTOCOL *KeyMapDb;              ///<
  UINTN                           KeyMapDbIndex;          ///<
  EFI_USB_IO_PROTOCOL             *UsbIo;                 ///<
  EFI_USB_DEVICE_DESCRIPTOR       DeviceDescriptor;       ///<
  EFI_USB_INTERFACE_DESCRIPTOR    InterfaceDescriptor;    ///<
  EFI_USB_ENDPOINT_DESCRIPTOR     IntEndpointDescriptor;  ///<
  USB_KB_BUFFER                   KeyboardBuffer;         ///<
  UINT8                           CtrlOn;                 ///<
  UINT8                           AltOn;                  ///<
  UINT8                           ShiftOn;                ///<
  UINT8                           NumLockOn;              ///<
  UINT8                           CapsOn;                 ///<
  UINT8                           LastKeyCodeArray[8];    ///<
  UINT8                           CurKeyChar;             ///<
  UINT8                           RepeatKey;              ///<
  EFI_EVENT                       RepeatTimer;            ///<
  EFI_UNICODE_STRING_TABLE        *ControllerNameTable;   ///<
} USB_KB_DEV;

// gEfiUsbKeyboardDriverGuid
extern EFI_GUID gEfiUsbKeyboardDriverGuid;

// gUsbKbKeyboardInformationProtocol
extern EFI_KEYBOARD_INFO_PROTOCOL gUsbKbKeyboardInformationProtocol;

#endif // USB_KB_H_
