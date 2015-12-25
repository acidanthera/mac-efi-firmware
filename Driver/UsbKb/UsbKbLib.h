/** @file
  Copyright (c) 2004 - 2008, Intel Corporation
  Portions Copyright (C) 2005 - 2015, Apple Inc
  All rights reserved.  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php/
  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#ifndef USB_KB_LIB_H_
#define USB_KB_LIB_H_

#include "UsbKbInternal.h"

// UsbKbCheckForKey
/** Check whether there is key pending.

  @param[in] UsbKbDev  The USB_KB_DEV instance.

  @retval EFI_SUCCESS    Success
  @retval EFI_NOT_READY  Device is not ready
**/
EFI_STATUS
UsbKbCheckForKey (
  IN USB_KB_DEV  *UsbKbDev
  );

// KbdReportStatusCode
/** Report Status Code in Usb Bot Driver

  @param[in] DevicePath  Use this to get Device Path
  @param[in] CodeType    Status Code Type
  @param[in] CodeValue   Status Code Value
**/
VOID
KbdReportStatusCode (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevicePath,
  IN EFI_STATUS_CODE_TYPE      CodeType,
  IN EFI_STATUS_CODE_VALUE     Value
  );

// IsUSBKeyboard
/** Uses USB I/O to check whether the device is a USB Keyboard device.

  @param[in] UsbIo  Points to a USB I/O protocol instance.

  @return  Returned is whether the device is a USB Keyboard device.
**/
BOOLEAN
IsUsbKeyboard (
  IN EFI_USB_IO_PROTOCOL  *UsbIo
  );

// InitUSBKeyboard
/** Initialize USB Keyboard device and all private data structures.

  @param[in] UsbKbDev  The USB_KB_DEV instance.

  @retval EFI_SUCCESS       Success
  @retval EFI_DEVICE_ERROR  Hardware Error
**/
EFI_STATUS
InitUsbKeyboard (
  IN USB_KB_DEV  *UsbKbDev
  );

// KeyboardHandler
/** Handler function for USB Keyboard's asynchronous interrupt transfer.

  @param[in] Data        A pointer to a Buffer that is filled with key data which is
                         retrieved via asynchronous interrupt transfer.
  @param[in] DataLength  Indicates the size of the data Buffer.
  @param[in] Context     Pointing to USB_KB_DEV instance.
  @param[in] Result      Indicates the result of the asynchronous interrupt transfer.

  @retval EFI_SUCCESS       Success
  @retval EFI_DEVICE_ERROR  Hardware Error
**/
EFI_STATUS
EFIAPI
KeyboardHandler (
  IN VOID    *Data,
  IN UINTN   DataLength,
  IN VOID    *Context,
  IN UINT32  Result
  );

// UsbKbRecoveryHandler
/** Timer handler for Delayed Recovery timer.

  @param[in] Event   The Delayed Recovery event.
  @param[in] Points  to the USB_KB_DEV instance.
**/
VOID
EFIAPI
UsbKbRecoveryHandler (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  );

// UsbParseKey
/** Retrieves a key character after parsing the raw data in keyboard Buffer.

  @param[in, out] UsbKbDev  The USB_KB_DEV instance.
  @param[out]     KeyChar   Points to the Key character after key parsing.

  @retval EFI_SUCCESS    Success
  @retval EFI_NOT_READY  Device is not ready
**/
EFI_STATUS
UsbParseKey (
  IN OUT USB_KB_DEV  *UsbKbDev,
  OUT    UINT8       *KeyChar
  );

// UsbKeyCodeToEfiScanCode
/** Converts USB Keyboard code to EFI Scan Code.

  @param[in]  UsbKbDev  The USB_KB_DEV instance.
  @param[in]  KeyChar   Indicates the key code that will be interpreted.
  @param[out] Key       A pointer to a Buffer that is filled in with the keystroke information for the key
                        that was pressed.

  @retval EFI_NOT_READY  Device is not ready
  @retval EFI_SUCCESS    Success
**/
EFI_STATUS
UsbKeyCodeToEfiScanCode (
  IN  USB_KB_DEV     *UsbKbDev,
  IN  UINT8          KeyChar,
  OUT EFI_INPUT_KEY  *Key
  );

// InitUsbKbBuffer
/** Resets USB Keyboard Buffer.

  @param[in, out] KeyboardBuffer  Points to the USB Keyboard Buffer.

  @retval EFI_SUCCESS  Success
**/
EFI_STATUS
InitUsbKbBuffer (
  IN OUT USB_KB_BUFFER  *KeyboardBuffer
  );

// IsUsbKbBufferEmpty
/** Check whether USB Keyboard Buffer is empty.

  @param[in] KeyboardBuffer  USB Keyboard Buffer.

  @return  Returned is whether USB Keyboard Buffer is empty.
**/
BOOLEAN
IsUsbKbBufferEmpty (
  IN USB_KB_BUFFER  KeyboardBuffer
  );

// IsUsbKbBufferFull
/** Check whether USB Keyboard Buffer is full.

  @param[in] KeyboardBuffer  USB Keyboard Buffer.

  @return  Returned is whether USB Keyboard Buffer is full.
**/
BOOLEAN
IsUsbKbBufferFull (
  IN USB_KB_BUFFER  KeyboardBuffer
  );

// InsertKeyCode
/** Inserts a key code into keyboard Buffer.

  @param[in, out] KeyboardBuffer  Points to the USB Keyboard Buffer.
  @param[in]      Key             Key code
  @param[in]      Down            Special key

  @retval EFI_SUCCESS  Success
**/
EFI_STATUS
InsertKeyCode (
  IN OUT USB_KB_BUFFER  *KeyboardBuffer,
  IN     UINT8          Key,
  IN     UINT8          Down
  );

// RemoveKeyCode
/** Pops a key code off from keyboard Buffer.

  @param[in, out] KeyboardBuffer  Points to the USB Keyboard Buffer.
  @param[in]      UsbKey          Points to the Buffer that contains a usb key code.

  @retval EFI_SUCCESS       Success
  @retval EFI_DEVICE_ERROR  Hardware Error
**/
EFI_STATUS
RemoveKeyCode (
  IN OUT USB_KB_BUFFER  *KeyboardBuffer,
  OUT    USB_KEY        *UsbKey
  );

// UsbKbRepeatHandler
/** Timer handler for Repeat Key timer.

  @param[in] Event    The Repeat Key event.
  @param[in] Context  Points to the USB_KB_DEV instance.
**/
VOID
EFIAPI
UsbKbRepeatHandler (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  );

// SetKeyLed
/** Sets USB Keyboard LED state.

  @param[in] UsbKbDev  The USB_KB_DEV instance.

  @retval EFI_SUCCESS  Success
**/
EFI_STATUS
SetKeyLed (
  IN USB_KB_DEV  *UsbKbDev
  );

#endif // USB_KB_LIB_H_
