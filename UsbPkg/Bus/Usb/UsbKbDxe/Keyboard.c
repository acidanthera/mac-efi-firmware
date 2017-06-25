/*++

Copyright (c) 2004 - 2009, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  Keyboard.c

Abstract:

  Helper functions for USB Keyboard Driver

Revision History

--*/

#include "Keyboard.h"

#include <IndustryStandard/AppleHid.h>
#include <IndustryStandard/Usb.h>
#include <Library/PcdLib.h>
// APPLE_KEY_DESCRIPTOR
typedef struct {
  APPLE_KEY_CODE AppleKeyCode;
  EFI_INPUT_KEY  InputKey;
  EFI_INPUT_KEY  ShiftedInputKey;
} APPLE_KEY_DESCRIPTOR;

// gAppleHidUsbKbUsageKeyMap
/// The default United States key map for Apple keyboards.
extern APPLE_KEY_DESCRIPTOR gAppleKeyMap[];

STATIC KB_MODIFIER  KB_Mod[8] = {
  { MOD_CONTROL_L,  0xe0 }, // 11100000
  { MOD_CONTROL_R,  0xe4 }, // 11100100
  { MOD_SHIFT_L,    0xe1 }, // 11100001
  { MOD_SHIFT_R,    0xe5 }, // 11100101
  { MOD_ALT_L,      0xe2 }, // 11100010
  { MOD_ALT_R,      0xe6 }, // 11100110
  { MOD_WIN_L,      0xe3 }, // 11100011
  { MOD_WIN_R,      0xe7 }, // 11100111
};

BOOLEAN
IsUSBKeyboard (
  IN  EFI_USB_IO_PROTOCOL       *UsbIo
  )
/*++

  Routine Description:
    Uses USB I/O to check whether the device is a USB Keyboard device.

  Arguments:
    UsbIo:    Points to a USB I/O protocol instance.

  Returns:

--*/
{
  EFI_STATUS                    Status;
  EFI_USB_INTERFACE_DESCRIPTOR  InterfaceDescriptor;

  //
  // Get the Default interface descriptor, currently we
  // assume it is interface 1
  //
  Status = UsbIo->UsbGetInterfaceDescriptor (
                    UsbIo,
                    &InterfaceDescriptor
                    );

  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  if (InterfaceDescriptor.InterfaceClass == CLASS_HID &&
      InterfaceDescriptor.InterfaceSubClass == SUBCLASS_BOOT &&
      InterfaceDescriptor.InterfaceProtocol == PROTOCOL_KEYBOARD
      ) {

    return TRUE;
  }

  return FALSE;
}

EFI_STATUS
AppleGetProtocolRequest (
  IN EFI_USB_IO_PROTOCOL     *UsbIo,
  IN UINT8                   Interface,
  OUT UINT8                   *Protocol
  )
/*++

  Routine Description:
    Get Hid Protocol Request

  Arguments:
    UsbIo             -   EFI_USB_IO_PROTOCOL
    Interface         -   Which interface the caller wants to get protocol
    Protocol          -   Protocol value returned.

  Returns:
    EFI_SUCCESS
    EFI_DEVICE_ERROR
    EFI_TIMEOUT

--*/
{
  UINT32                  Status;
  EFI_STATUS              Result;
  EFI_USB_DEVICE_REQUEST  Request;

  //
  // Fill Device request packet
  //
  Request.RequestType = USB_HID_CLASS_GET_REQ_TYPE;
  Request.Request = EFI_USB_GET_PROTOCOL_REQUEST;
  Request.Value   = 0;
  Request.Index   = Interface;
  Request.Length  = 1;

  // BUG: Should use EfiUsbDataIn.

  Result = UsbIo->UsbControlTransfer (
                    UsbIo,
                    &Request,
                    EfiUsbDataOut,
                    PcdGet32 (PcdUsbTransferTimeoutValue),
                    Protocol,
                    sizeof (UINT8),
                    &Status
                    );

  return Result;
}

EFI_STATUS
InitUSBKeyboard (
  IN USB_KB_DEV   *UsbKeyboardDevice
  )
/*++

  Routine Description:
    Initialize USB Keyboard device and all private data structures.

  Arguments:
    UsbKeyboardDevice    The USB_KB_DEV instance.

  Returns:
    EFI_SUCCESS      - Success
    EFI_DEVICE_ERROR - Hardware Error
--*/
{
  UINT8               Protocol;
  UINT8               ReportId;
  UINT8               Duration;

  REPORT_STATUS_CODE_WITH_DEVICE_PATH (
    EFI_PROGRESS_CODE,
    (EFI_PERIPHERAL_KEYBOARD | EFI_P_KEYBOARD_PC_SELF_TEST),
    UsbKeyboardDevice->DevicePath
    );

  InitUSBKeyBuffer (&(UsbKeyboardDevice->KeyboardBuffer));

  AppleGetProtocolRequest (
    UsbKeyboardDevice->UsbIo,
    UsbKeyboardDevice->InterfaceDescriptor.InterfaceNumber,
    &Protocol
    );
  //
  // Sets boot protocol for the USB Keyboard.
  // This driver only supports boot protocol.
  // !!BugBug: How about the device that does not support boot protocol?
  //
  if (Protocol != BOOT_PROTOCOL) {
    UsbSetProtocolRequest (
      UsbKeyboardDevice->UsbIo,
      UsbKeyboardDevice->InterfaceDescriptor.InterfaceNumber,
      BOOT_PROTOCOL
      );
  }
  //
  // the duration is indefinite, so the endpoint will inhibit reporting forever,
  // and only reporting when a change is detected in the report data.
  //

  //
  // idle value for all report ID
  //
  ReportId = 0;
  //
  // idle forever until there is a key pressed and released.
  //
  Duration = 0;
  UsbSetIdleRequest (
    UsbKeyboardDevice->UsbIo,
    UsbKeyboardDevice->InterfaceDescriptor.InterfaceNumber,
    ReportId,
    Duration
    );

  UsbKeyboardDevice->CtrlOn     = 0;
  UsbKeyboardDevice->AltOn      = 0;
  UsbKeyboardDevice->ShiftOn    = 0;
  UsbKeyboardDevice->NumLockOn  = 1;
  UsbKeyboardDevice->CapsOn     = 0;
  UsbKeyboardDevice->ScrollOn   = 0;

  ZeroMem (UsbKeyboardDevice->LastKeyCodeArray, sizeof (UINT8) * 8);

  //
  // Sync the initial state of lights
  //
  SetKeyLED (UsbKeyboardDevice);

  //
  // Set a timer for repeat keys' generation.
  //
  if (UsbKeyboardDevice->RepeatTimer) {
    gBS->CloseEvent (UsbKeyboardDevice->RepeatTimer);
    UsbKeyboardDevice->RepeatTimer = 0;
  }

  gBS->CreateEvent (
         EVT_TIMER | EVT_NOTIFY_SIGNAL,
         TPL_CALLBACK,
         USBKeyboardRepeatHandler,
         UsbKeyboardDevice,
         &UsbKeyboardDevice->RepeatTimer
         );

  if (UsbKeyboardDevice->DelayedRecoveryEvent) {
    gBS->CloseEvent (UsbKeyboardDevice->DelayedRecoveryEvent);
    UsbKeyboardDevice->DelayedRecoveryEvent = 0;
  }

  gBS->CreateEvent (
         EVT_TIMER | EVT_NOTIFY_SIGNAL,
         TPL_NOTIFY,
         USBKeyboardRecoveryHandler,
         UsbKeyboardDevice,
         &UsbKeyboardDevice->DelayedRecoveryEvent
         );

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
KeyboardHandler (
  IN  VOID          *Data,
  IN  UINTN         DataLength,
  IN  VOID          *Context,
  IN  UINT32        Result
  )
/*++

  Routine Description:
    Handler function for USB Keyboard's asynchronous interrupt transfer.

  Arguments:
    Data       A pointer to a buffer that is filled with key data which is
               retrieved via asynchronous interrupt transfer.
    DataLength Indicates the size of the data buffer.
    Context    Pointing to USB_KB_DEV instance.
    Result     Indicates the result of the asynchronous interrupt transfer.

  Returns:
    EFI_SUCCESS      - Success
    EFI_DEVICE_ERROR - Hardware Error
--*/
{
  STATIC UINT8 InvalidKeyCodeBuffer[] = {
    0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01
  };

  USB_KB_DEV          *UsbKeyboardDevice;
  EFI_USB_IO_PROTOCOL *UsbIo;
  UINT8               *CurKeyCodeBuffer;
  UINT8               *OldKeyCodeBuffer;
  UINT8               CurModifierMap;
  UINT8               OldModifierMap;
  UINT8               Index;
  UINT8               Index2;
  BOOLEAN             Down;
  BOOLEAN             KeyRelease;
  BOOLEAN             KeyPress;
  UINT8               NewRepeatKey;
  UINT32              UsbStatus;
  INT8                NumberOfKeyCodes;
  APPLE_KEY_CODE      AppleKeyCodes[8];
  INTN                CompareResult;

  ASSERT (Context);

  NewRepeatKey      = 0;
  UsbKeyboardDevice = (USB_KB_DEV *) Context;
  UsbIo             = UsbKeyboardDevice->UsbIo;

  //
  // Analyzes the Result and performs corresponding action.
  //
  if (Result != EFI_USB_NOERROR) {
    //
    // Some errors happen during the process
    //
    REPORT_STATUS_CODE_WITH_DEVICE_PATH (
      EFI_ERROR_CODE | EFI_ERROR_MINOR,
      (EFI_PERIPHERAL_KEYBOARD | EFI_P_EC_INPUT_ERROR),
      UsbKeyboardDevice->DevicePath
      );

    //
    // stop the repeat key generation if any
    //
    UsbKeyboardDevice->RepeatKey = 0;

    gBS->SetTimer (
          UsbKeyboardDevice->RepeatTimer,
          TimerCancel,
          USBKBD_REPEAT_RATE
          );

    if ((Result & EFI_USB_ERR_STALL) == EFI_USB_ERR_STALL) {
      UsbClearEndpointHalt (
        UsbIo,
        UsbKeyboardDevice->IntEndpointDescriptor.EndpointAddress,
        &UsbStatus
        );
    }

    //
    // Delete & Submit this interrupt again
    //

    UsbIo->UsbAsyncInterruptTransfer (
             UsbIo,
             UsbKeyboardDevice->IntEndpointDescriptor.EndpointAddress,
             FALSE,
             0,
             0,
             NULL,
             NULL
             );

    gBS->SetTimer (
          UsbKeyboardDevice->DelayedRecoveryEvent,
          TimerRelative,
          EFI_USB_INTERRUPT_DELAY
          );

    return EFI_DEVICE_ERROR;
  }

  if (DataLength == 0 || Data == NULL) {
    return EFI_SUCCESS;
  }

  CurKeyCodeBuffer  = (UINT8 *) Data;
  OldKeyCodeBuffer  = UsbKeyboardDevice->LastKeyCodeArray;

  //
  // checks for new key stroke.
  // if no new key got, return immediately.
  //
  for (Index = 0; Index < 8; Index++) {
    if (OldKeyCodeBuffer[Index] != CurKeyCodeBuffer[Index]) {
      break;
    }
  }

  if (Index == 8) {
    return EFI_SUCCESS;
  }

  //
  // check if CurKeyCodeBuffer is valid
  //
  if (CurKeyCodeBuffer == &InvalidKeyCodeBuffer[0]) {
    return EFI_DEVICE_ERROR;
  }

  CompareResult = CompareMem (
                    (VOID *)CurKeyCodeBuffer,
                    (VOID *)&InvalidKeyCodeBuffer[0],
                    sizeof (InvalidKeyCodeBuffer)
                    );

  if (CompareResult == 0) {
    return EFI_DEVICE_ERROR;
  }

  CurModifierMap   = CurKeyCodeBuffer[0];
  NumberOfKeyCodes = 0;

  //
  // Pass valid keycodes to the Key Map Database protocol
  //
  for (Index = 2; Index < 8; Index++) {
    if (USB_HID_KB_KP_VALID_KEYCODE (CurKeyCodeBuffer[Index])) {
      AppleKeyCodes[NumberOfKeyCodes] = APPLE_HID_USB_KB_KP_USAGE (
                                          CurKeyCodeBuffer[Index]
                                          );

      ++NumberOfKeyCodes;
    }
  }

  UsbKeyboardDevice->KeyMapDb->SetKeyStrokeBufferKeys (
                                 UsbKeyboardDevice->KeyMapDb,
                                 UsbKeyboardDevice->KeyMapDbIndex,
                                 (APPLE_MODIFIER_MAP)CurModifierMap,
                                 NumberOfKeyCodes,
                                 &AppleKeyCodes[0]
                                 );

  //
  // Parse the modifier key
  //
  CurModifierMap  = CurKeyCodeBuffer[0];
  OldModifierMap  = OldKeyCodeBuffer[0];

  //
  // handle modifier key's pressing or releasing situation.
  //
  for (Index = 0; Index < 8; Index++) {

    if ((CurModifierMap & KB_Mod[Index].Mask) != (OldModifierMap & KB_Mod[Index].Mask)) {
      //
      // if current modifier key is up, then
      // CurModifierMap & KB_Mod[Index].Mask = 0;
      // otherwize it is a non-zero value.
      // Inserts the pressed modifier key into key buffer.
      //
      Down = (UINT8) (CurModifierMap & KB_Mod[Index].Mask);
      InsertKeyCode (&(UsbKeyboardDevice->KeyboardBuffer), KB_Mod[Index].Key, Down);
    }
  }

  //
  // handle normal key's releasing situation
  //
  KeyRelease = FALSE;
  for (Index = 2; Index < 8; Index++) {

    if (!USBKBD_VALID_KEYCODE (OldKeyCodeBuffer[Index])) {
      continue;
    }

    KeyRelease = TRUE;
    for (Index2 = 2; Index2 < 8; Index2++) {

      if (!USBKBD_VALID_KEYCODE (CurKeyCodeBuffer[Index2])) {
        continue;
      }

      if (OldKeyCodeBuffer[Index] == CurKeyCodeBuffer[Index2]) {
        KeyRelease = FALSE;
        break;
      }
    }

    if (KeyRelease) {
      InsertKeyCode (
        &(UsbKeyboardDevice->KeyboardBuffer),
        OldKeyCodeBuffer[Index],
        0
        );
      //
      // the original reapeat key is released.
      //
      if (OldKeyCodeBuffer[Index] == UsbKeyboardDevice->RepeatKey) {
        UsbKeyboardDevice->RepeatKey = 0;
      }
    }
  }

  //
  // original repeat key is released, cancel the repeat timer
  //
  if (UsbKeyboardDevice->RepeatKey == 0) {
    gBS->SetTimer (
          UsbKeyboardDevice->RepeatTimer,
          TimerCancel,
          USBKBD_REPEAT_RATE
          );
  }

  //
  // handle normal key's pressing situation
  //
  KeyPress = FALSE;
  for (Index = 2; Index < 8; Index++) {

    if (!USBKBD_VALID_KEYCODE (CurKeyCodeBuffer[Index])) {
      continue;
    }

    KeyPress = TRUE;
    for (Index2 = 2; Index2 < 8; Index2++) {

      if (!USBKBD_VALID_KEYCODE (OldKeyCodeBuffer[Index2])) {
        continue;
      }

      if (CurKeyCodeBuffer[Index] == OldKeyCodeBuffer[Index2]) {
        KeyPress = FALSE;
        break;
      }
    }

    if (KeyPress) {
      InsertKeyCode (&(UsbKeyboardDevice->KeyboardBuffer), CurKeyCodeBuffer[Index], 1);
      //
      // NumLock pressed or CapsLock pressed
      //
      if (CurKeyCodeBuffer[Index] == 0x53 || CurKeyCodeBuffer[Index] == 0x39) {
        UsbKeyboardDevice->RepeatKey = 0;
      } else {
        NewRepeatKey = CurKeyCodeBuffer[Index];
        //
        // do not repeat the original repeated key
        //
        UsbKeyboardDevice->RepeatKey = 0;
      }
    }
  }

  //
  // Update LastKeycodeArray[] buffer in the
  // Usb Keyboard Device data structure.
  //
  for (Index = 0; Index < 8; Index++) {
    UsbKeyboardDevice->LastKeyCodeArray[Index] = CurKeyCodeBuffer[Index];
  }

  //
  // If have new key pressed, update the RepeatKey value, and set the
  // timer to repeate delay timer
  //
  if (NewRepeatKey != 0) {
    //
    // sets trigger time to "Repeat Delay Time",
    // to trigger the repeat timer when the key is hold long
    // enough time.
    //
    gBS->SetTimer (
          UsbKeyboardDevice->RepeatTimer,
          TimerRelative,
          USBKBD_REPEAT_DELAY
          );
    UsbKeyboardDevice->RepeatKey = NewRepeatKey;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
USBParseKey (
  IN OUT  USB_KB_DEV  *UsbKeyboardDevice,
  OUT     UINT8       *KeyChar
  )
/*++

  Routine Description:
    Retrieves a key character after parsing the raw data in keyboard buffer.

  Arguments:
    UsbKeyboardDevice    The USB_KB_DEV instance.
    KeyChar              Points to the Key character after key parsing.

  Returns:
    EFI_SUCCESS   - Success
    EFI_NOT_READY - Device is not ready
--*/
{
  USB_KEY UsbKey;

  *KeyChar = 0;

  while (!IsUSBKeyboardBufferEmpty (&UsbKeyboardDevice->KeyboardBuffer)) {
    //
    // pops one raw data off.
    //
    RemoveKeyCode (&(UsbKeyboardDevice->KeyboardBuffer), &UsbKey);

    if (!UsbKey.Down) {
      switch (UsbKey.KeyCode) {

      //
      // CTRL release
      //
      case 0xe0:
      case 0xe4:
        UsbKeyboardDevice->CtrlOn = 0;
        break;

      //
      // Shift release
      //
      case 0xe1:
      case 0xe5:
        UsbKeyboardDevice->ShiftOn = 0;
        break;

      //
      // Alt release
      //
      case 0xe2:
      case 0xe6:
        UsbKeyboardDevice->AltOn = 0;
        break;

      default:
        break;
      }

      continue;
    }

    //
    // Analyzes key pressing situation
    //
    switch (UsbKey.KeyCode) {

    //
    // CTRL press
    //
    case 0xe0:
    case 0xe4:
      UsbKeyboardDevice->CtrlOn = 1;
      continue;
      break;

    //
    // Shift press
    //
    case 0xe1:
    case 0xe5:
      UsbKeyboardDevice->ShiftOn = 1;
      continue;
      break;

    //
    // Alt press
    //
    case 0xe2:
    case 0xe6:
      UsbKeyboardDevice->AltOn = 1;
      continue;
      break;

    case 0xe3:
    case 0xe7:
      continue;
      break;

    case 0x53:
      UsbKeyboardDevice->NumLockOn ^= 1;
      //
      // Turn on the NumLock light on KB
      //
      SetKeyLED (UsbKeyboardDevice);
      continue;
      break;

    case 0x39:
      UsbKeyboardDevice->CapsOn ^= 1;
      //
      // Turn on the CapsLock light on KB
      //
      SetKeyLED (UsbKeyboardDevice);
      continue;
      break;

    //
    // PrintScreen,Pause,Application,Power
    // keys are not valid EFI key
    //

      // TODO: Verify
    case 0x46:
    //
    // fall through
    //
    case 0x48:
    //
    // fall through
    //
    case 0x65:
    //
    // fall through
    //
    case 0x66:
    //
    // fall through
    //
      continue;
      break;

    default:
      break;
    }

    *KeyChar = UsbKey.KeyCode;
    return EFI_SUCCESS;
  }

  return EFI_NOT_READY;
}


EFI_STATUS
USBKeyCodeToEFIScanCode (
  IN  USB_KB_DEV      *UsbKeyboardDevice,
  IN  UINT8           KeyChar,
  OUT EFI_INPUT_KEY   *Key
  )
/*++

  Routine Description:
    Converts USB Keyboard code to EFI Scan Code.

  Arguments:
    UsbKeyboardDevice    The USB_KB_DEV instance.
    KeyChar              Indicates the key code that will be interpreted.
    Key                  A pointer to a buffer that is filled in with
                         the keystroke information for the key that
                         was pressed.
  Returns:
    EFI_NOT_READY - Device is not ready
    EFI_SUCCESS   - Success
--*/
{
  UINT8               Index;

  if (!USBKBD_VALID_KEYCODE (KeyChar)) {
    return EFI_NOT_READY;
  }

  //
  // valid USB Key Code starts from 4
  //
  Index = (UINT8) (KeyChar - 4);

  if (Index >= USB_KEYCODE_MAX_MAKE) {
    return EFI_NOT_READY;
  }

  Key->ScanCode = gAppleKeyMap[Index].InputKey.ScanCode;

  if (UsbKeyboardDevice->ShiftOn
   || ((UsbKeyboardDevice->Unknown & 0x22) != 0)) {

    Key->UnicodeChar = gAppleKeyMap[Index].ShiftedInputKey.UnicodeChar;

  } else {

    Key->UnicodeChar = gAppleKeyMap[Index].InputKey.UnicodeChar;
  }

  if (UsbKeyboardDevice->CapsOn) {

    if ((Key->UnicodeChar >= 'a' && Key->UnicodeChar <= 'z')
     || (Key->UnicodeChar >= 'A' && Key->UnicodeChar <= 'Z')) {

      Key->UnicodeChar = gAppleKeyMap[Index].ShiftedInputKey.UnicodeChar;

    }
  }

  if (KeyChar >= 0x59 && KeyChar <= 0x63) {

    if (UsbKeyboardDevice->NumLockOn && !UsbKeyboardDevice->ShiftOn) {

      Key->ScanCode = SCAN_NULL;

    } else {

      Key->UnicodeChar = 0x00;
    }
  }

  if (KeyChar >= 0x4F && KeyChar <= 0x51) {

    Index -= 5;

    Key->UnicodeChar = gAppleKeyMap[Index].InputKey.UnicodeChar;

  }

  if (KeyChar == 0x52) {

    Key->ScanCode    = SCAN_UP;
    Key->UnicodeChar = 0x01;

  } else if (KeyChar >= 0x3A && KeyChar <= 0x45) {

    Key->UnicodeChar = gAppleKeyMap[Index].InputKey.UnicodeChar;

  }
  //
  // Translate the CTRL-Alpha characters to their corresponding control value  (ctrl-a = 0x0001 through ctrl-Z = 0x001A)
  //
  if (UsbKeyboardDevice->CtrlOn
   || ((UsbKeyboardDevice->Unknown & 0x11) != 0)) {
    if (Key->UnicodeChar == 'e') {
      Key->UnicodeChar = 0x05;
    } else if (Key->UnicodeChar == 'c') {
      Key->UnicodeChar = 0x03;
    } else if (Key->UnicodeChar == 'a') {
      Key->UnicodeChar = 0x01;
    }
  }

  if (Key->UnicodeChar == 0 && Key->ScanCode == SCAN_NULL) {
    return EFI_NOT_READY;
  }

  return EFI_SUCCESS;

}


EFI_STATUS
InitUSBKeyBuffer (
  IN OUT  USB_KB_BUFFER   *KeyboardBuffer
  )
/*++

  Routine Description:
    Resets USB Keyboard Buffer.

  Arguments:
    KeyboardBuffer - Points to the USB Keyboard Buffer.

  Returns:
    EFI_SUCCESS - Success
--*/
{
  ZeroMem (KeyboardBuffer, sizeof (USB_KB_BUFFER));

  KeyboardBuffer->bHead = KeyboardBuffer->bTail;

  return EFI_SUCCESS;
}

BOOLEAN
IsUSBKeyboardBufferEmpty (
  IN  USB_KB_BUFFER   *KeyboardBuffer
  )
/*++

  Routine Description:
    Check whether USB Keyboard buffer is empty.

  Arguments:
    KeyboardBuffer - USB Keyboard Buffer.

  Returns:

--*/
{
  //
  // meet FIFO empty condition
  //
  return (BOOLEAN) (KeyboardBuffer->bHead == KeyboardBuffer->bTail);
}


BOOLEAN
IsUSBKeyboardBufferFull (
  IN  USB_KB_BUFFER   *KeyboardBuffer
  )
/*++

  Routine Description:
    Check whether USB Keyboard buffer is full.

  Arguments:
    KeyboardBuffer - USB Keyboard Buffer.

  Returns:

--*/
{
  return (BOOLEAN)(((KeyboardBuffer->bTail + 1) % (MAX_KEY_ALLOWED + 1)) ==
                                                        KeyboardBuffer->bHead);
}


EFI_STATUS
InsertKeyCode (
  IN OUT  USB_KB_BUFFER *KeyboardBuffer,
  IN      UINT8         Key,
  IN      UINT8         Down
  )
/*++

  Routine Description:
    Inserts a key code into keyboard buffer.

  Arguments:
    KeyboardBuffer - Points to the USB Keyboard Buffer.
    Key            - Key code
    Down           - Special key
  Returns:
    EFI_SUCCESS - Success
--*/
{
  USB_KEY UsbKey;

  //
  // if keyboard buffer is full, throw the
  // first key out of the keyboard buffer.
  //
  if (IsUSBKeyboardBufferFull (KeyboardBuffer)) {
    RemoveKeyCode (KeyboardBuffer, &UsbKey);
  }

  KeyboardBuffer->buffer[KeyboardBuffer->bTail].KeyCode = Key;
  KeyboardBuffer->buffer[KeyboardBuffer->bTail].Down    = Down;

  //
  // adjust the tail pointer of the FIFO keyboard buffer.
  //
  KeyboardBuffer->bTail = (UINT8) ((KeyboardBuffer->bTail + 1) % (MAX_KEY_ALLOWED + 1));

  return EFI_SUCCESS;
}

EFI_STATUS
RemoveKeyCode (
  IN OUT  USB_KB_BUFFER *KeyboardBuffer,
  OUT     USB_KEY       *UsbKey
  )
/*++

  Routine Description:
    Pops a key code off from keyboard buffer.

  Arguments:
    KeyboardBuffer -  Points to the USB Keyboard Buffer.
    UsbKey         -  Points to the buffer that contains a usb key code.

  Returns:
    EFI_SUCCESS      - Success
    EFI_DEVICE_ERROR - Hardware Error
--*/
{
  if (IsUSBKeyboardBufferEmpty (KeyboardBuffer)) {
    return EFI_DEVICE_ERROR;
  }

  UsbKey->KeyCode = KeyboardBuffer->buffer[KeyboardBuffer->bHead].KeyCode;
  UsbKey->Down    = KeyboardBuffer->buffer[KeyboardBuffer->bHead].Down;

  //
  // adjust the head pointer of the FIFO keyboard buffer.
  //
  KeyboardBuffer->bHead = (UINT8) ((KeyboardBuffer->bHead + 1) % (MAX_KEY_ALLOWED + 1));

  return EFI_SUCCESS;
}

EFI_STATUS
SetKeyLED (
  IN  USB_KB_DEV    *UsbKeyboardDevice
  )
/*++

  Routine Description:
    Sets USB Keyboard LED state.

  Arguments:
    UsbKeyboardDevice - The USB_KB_DEV instance.

  Returns:
    EFI_SUCCESS - Success
--*/
{
  LED_MAP Led;
  UINT8   ReportId;

  //
  // Set each field in Led map.
  //
  Led.NumLock    = 0;
  Led.CapsLock   = (UINT8) UsbKeyboardDevice->CapsOn;
  Led.ScrollLock = 0;
  Led.Resrvd     = 0;

  ReportId       = 0;
  //
  // call Set Report Request to lighten the LED.
  //
  UsbSetReportRequest (
    UsbKeyboardDevice->UsbIo,
    UsbKeyboardDevice->InterfaceDescriptor.InterfaceNumber,
    ReportId,
    HID_OUTPUT_REPORT,
    1,
    (UINT8 *) &Led
    );

  return EFI_SUCCESS;
}

VOID
EFIAPI
USBKeyboardRepeatHandler (
  IN    EFI_EVENT    Event,
  IN    VOID         *Context
  )
/*++

  Routine Description:
    Timer handler for Repeat Key timer.

  Arguments:
    Event   - The Repeat Key event.
    Context - Points to the USB_KB_DEV instance.

  Returns:

--*/
{
  USB_KB_DEV  *UsbKeyboardDevice;

  UsbKeyboardDevice = (USB_KB_DEV *) Context;

  //
  // Do nothing when there is no repeat key.
  //
  if (UsbKeyboardDevice->RepeatKey != 0) {
    //
    // Inserts one Repeat key into keyboard buffer,
    //
    InsertKeyCode (
      &(UsbKeyboardDevice->KeyboardBuffer),
      UsbKeyboardDevice->RepeatKey,
      1
      );

    //
    // set repeate rate for repeat key generation.
    //
    gBS->SetTimer (
          UsbKeyboardDevice->RepeatTimer,
          TimerRelative,
          USBKBD_REPEAT_RATE
          );

  }
}

VOID
EFIAPI
USBKeyboardRecoveryHandler (
  IN    EFI_EVENT    Event,
  IN    VOID         *Context
  )
/*++

  Routine Description:
    Timer handler for Delayed Recovery timer.

  Arguments:
    Event   -  The Delayed Recovery event.
    Context -  Points to the USB_KB_DEV instance.

  Returns:

--*/
{

  USB_KB_DEV          *UsbKeyboardDevice;
  EFI_USB_IO_PROTOCOL *UsbIo;
  UINT8               PacketSize;

  UsbKeyboardDevice = (USB_KB_DEV *) Context;

  UsbIo             = UsbKeyboardDevice->UsbIo;

  PacketSize        = (UINT8) (UsbKeyboardDevice->IntEndpointDescriptor.MaxPacketSize);

  UsbIo->UsbAsyncInterruptTransfer (
          UsbIo,
          UsbKeyboardDevice->IntEndpointDescriptor.EndpointAddress,
          TRUE,
          UsbKeyboardDevice->IntEndpointDescriptor.Interval,
          PacketSize,
          KeyboardHandler,
          UsbKeyboardDevice
          );
}
