/** @file
  Helper functions for USB Keyboard Driver

  Copyright (C) 2004 - 2009, Intel Corporation<BR>
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php/

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include <AppleEfi.h>
#include <AppleMisc.h>

#include <Library/UsbDxeLib.h>
#include "UsbKbLib.h"

#include "UsbKbInternal.h"

// KB_KEY
typedef struct {
  UINT16 ScanCode;        ///< 
  CHAR16 Unicode;         ///< 
  CHAR16 ShiftedUnicode;  ///< 
} KB_KEY;

// mKeyConvertionTable
/// USB Key Code to Efi key mapping table
/// Format:<efi scan code>, <unicode without shift>, <unicode with shift>
STATIC KB_KEY mKeyConvertionTable[USB_KEYCODE_MAX_MAKE] = {
  { SCAN_NULL,      'a',                 'A'                   },  // 0x04
  { SCAN_NULL,      'b',                 'B'                   },  // 0x05
  { SCAN_NULL,      'c',                 'C'                   },  // 0x06
  { SCAN_NULL,      'd',                 'D'                   },  // 0x07
  { SCAN_NULL,      'e',                 'E'                   },  // 0x08
  { SCAN_NULL,      'f',                 'F'                   },  // 0x09
  { SCAN_NULL,      'g',                 'G'                   },  // 0x0A
  { SCAN_NULL,      'h',                 'H'                   },  // 0x0B
  { SCAN_NULL,      'i',                 'I'                   },  // 0x0C
  { SCAN_NULL,      'j',                 'J'                   },  // 0x0D
  { SCAN_NULL,      'k',                 'K'                   },  // 0x0E
  { SCAN_NULL,      'l',                 'L'                   },  // 0x0F
  { SCAN_NULL,      'm',                 'M'                   },  // 0x10
  { SCAN_NULL,      'n',                 'N'                   },  // 0x11
  { SCAN_NULL,      'o',                 'O'                   },  // 0x12
  { SCAN_NULL,      'p',                 'P'                   },  // 0x13
  { SCAN_NULL,      'q',                 'Q'                   },  // 0x14
  { SCAN_NULL,      'r',                 'R'                   },  // 0x15
  { SCAN_NULL,      's',                 'S'                   },  // 0x16
  { SCAN_NULL,      't',                 'T'                   },  // 0x17
  { SCAN_NULL,      'u',                 'U'                   },  // 0x18
  { SCAN_NULL,      'v',                 'V'                   },  // 0x19
  { SCAN_NULL,      'w',                 'W'                   },  // 0x1A
  { SCAN_NULL,      'x',                 'X'                   },  // 0x1B
  { SCAN_NULL,      'y',                 'Y'                   },  // 0x1C
  { SCAN_NULL,      'z',                 'Z'                   },  // 0x1D
  { SCAN_NULL,      '1',                 '!'                   },  // 0x1E
  { SCAN_NULL,      '2',                 '@'                   },  // 0x1F
  { SCAN_NULL,      '3',                 '#'                   },  // 0x20
  { SCAN_NULL,      '4',                 '$'                   },  // 0x21
  { SCAN_NULL,      '5',                 '%'                   },  // 0x22
  { SCAN_NULL,      '6',                 '^'                   },  // 0x23
  { SCAN_NULL,      '7',                 '&'                   },  // 0x24
  { SCAN_NULL,      '8',                 '*'                   },  // 0x25
  { SCAN_NULL,      '9',                 '('                   },  // 0x26
  { SCAN_NULL,      '0',                 ')'                   },  // 0x27
  { SCAN_NULL,      CHAR_CARRIAGE_RETURN, CHAR_CARRIAGE_RETURN },  // 0x28
  { SCAN_ESC,       CHAR_NULL,            CHAR_NULL            },  // 0x29
  { SCAN_NULL,      CHAR_BACKSPACE,       CHAR_BACKSPACE       },  // 0x2A
  { SCAN_NULL,      CHAR_TAB,             CHAR_TAB             },  // 0x2B
  { SCAN_NULL,      ' ',                  ' '                  },  // 0x2C
  { SCAN_NULL,      '-',                  '_'                  },  // 0x2D
  { SCAN_NULL,      '=',                  '+'                  },  // 0x2E
  { SCAN_NULL,      '[',                  '{'                  },  // 0x2F
  { SCAN_NULL,      ']',                  '}'                  },  // 0x30
  { SCAN_NULL,      '\\',                 '|'                  },  // 0x31
  { SCAN_NULL,      '\\',                 '|'                  },  // 0x32
  { SCAN_NULL,      ';',                  ':'                  },  // 0x33
  { SCAN_NULL,      '\'',                 '"'                  },  // 0x34
  { SCAN_NULL,      '`',                  '~'                  },  // 0x35
  { SCAN_NULL,      ',',                  '<'                  },  // 0x36
  { SCAN_NULL,      '.',                  '>'                  },  // 0x37
  { SCAN_NULL,      '/',                  '?'                  },  // 0x38
  { SCAN_NULL,      CHAR_NULL,            CHAR_NULL            },  // 0x39  CapsLock
  { SCAN_F1,        CHAR_NULL,            CHAR_NULL            },  // 0x3A
  { SCAN_F2,        CHAR_NULL,            CHAR_NULL            },  // 0x3B
  { SCAN_F3,        CHAR_NULL,            CHAR_NULL            },  // 0x3C
  { SCAN_F4,        CHAR_NULL,            CHAR_NULL            },  // 0x3D
  { SCAN_F5,        CHAR_NULL,            CHAR_NULL            },  // 0x3E
  { SCAN_F6,        CHAR_NULL,            CHAR_NULL            },  // 0x3F
  { SCAN_F7,        CHAR_NULL,            CHAR_NULL            },  // 0x40
  { SCAN_F8,        CHAR_NULL,            CHAR_NULL            },  // 0x41
  { SCAN_F9,        CHAR_NULL,            CHAR_NULL            },  // 0x42
  { SCAN_F10,       CHAR_NULL,            CHAR_NULL            },  // 0x43
  { SCAN_F11,       CHAR_NULL,            CHAR_NULL            },  // 0x44
  { SCAN_F12,       CHAR_NULL,            CHAR_NULL            },  // 0x45
  { SCAN_NULL,      CHAR_NULL,            CHAR_NULL            },  // 0x46  PrintScreen
  { SCAN_NULL,      CHAR_NULL,            CHAR_NULL            },  // 0x47  Scroll Lock
  { SCAN_NULL,      CHAR_NULL,            CHAR_NULL            },  // 0x48  Pause
  { SCAN_INSERT,    CHAR_NULL,            CHAR_NULL            },  // 0x49
  { SCAN_HOME,      CHAR_NULL,            CHAR_NULL            },  // 0x4A
  { SCAN_PAGE_UP,   CHAR_NULL,            CHAR_NULL            },  // 0x4B
  { SCAN_DELETE,    CHAR_NULL,            CHAR_NULL            },  // 0x4C
  { SCAN_END,       CHAR_NULL,            CHAR_NULL            },  // 0x4D
  { SCAN_PAGE_DOWN, CHAR_NULL,            CHAR_NULL            },  // 0x4E
  { SCAN_RIGHT,     CHAR_NULL,            CHAR_NULL            },  // 0x4F
  { SCAN_LEFT,      CHAR_NULL,            CHAR_NULL            },  // 0x50
  { SCAN_DOWN,      CHAR_NULL,            CHAR_NULL            },  // 0x51
  { SCAN_UP,        CHAR_NULL,            CHAR_NULL            },  // 0x52
  { SCAN_NULL,      CHAR_NULL,            CHAR_NULL            },  // 0x53   NumLock
  { SCAN_NULL,      '/',                  '/'                  },  // 0x54
  { SCAN_NULL,      '*',                  '*'                  },  // 0x55
  { SCAN_NULL,      '-',                  '-'                  },  // 0x56
  { SCAN_NULL,      '+',                  '+'                  },  // 0x57
  { SCAN_NULL,      CHAR_CARRIAGE_RETURN, CHAR_CARRIAGE_RETURN },  // 0x58
  { SCAN_END,       '1',                  '1'                  },  // 0x59
  { SCAN_DOWN,      '2',                  '2'                  },  // 0x5A
  { SCAN_PAGE_DOWN, '3',                  '3'                  },  // 0x5B
  { SCAN_LEFT,      '4',                  '4'                  },  // 0x5C
  { SCAN_NULL,      '5',                  '5'                  },  // 0x5D
  { SCAN_RIGHT,     '6',                  '6'                  },  // 0x5E
  { SCAN_HOME,      '7',                  '7'                  },  // 0x5F
  { SCAN_UP,        '8',                  '8'                  },  // 0x60
  { SCAN_PAGE_UP,   '9',                  '9'                  },  // 0x61
  { SCAN_INSERT,    '0',                  '0'                  },  // 0x62
  { SCAN_DELETE,    '.',                  '.'                  },  // 0x63
  { SCAN_NULL,      '\\',                 '|'                  },  // 0x64
  { SCAN_NULL,      CHAR_NULL,            CHAR_NULL            },  // 0x65 Keyboard Application
  { SCAN_NULL,      CHAR_NULL,            CHAR_NULL            },  // 0x66 Keyboard Power
  { SCAN_NULL,      '=' ,                 '='                  }   // 0x67 Keypad =
};

// mKeyboardModifierMap
STATIC KB_MODIFIER mKeyboardModifierMap[] = {
  { USB_HID_KB_KP_MODIFIER_LEFT_CONTROL,  UsbHidUsageIdKbKpModifierKeyLeftControl  },
  { USB_HID_KB_KP_MODIFIER_RIGHT_CONTROL, UsbHidUsageIdKbKpModifierKeyRightControl },
  { USB_HID_KB_KP_MODIFIER_LEFT_SHIFT,    UsbHidUsageIdKbKpModifierKeyLeftShift    },
  { USB_HID_KB_KP_MODIFIER_RIGHT_SHIFT,   UsbHidUsageIdKbKpModifierKeyRightShift   },
  { USB_HID_KB_KP_MODIFIER_LEFT_ALT,      UsbHidUsageIdKbKpModifierKeyLeftAlt      },
  { USB_HID_KB_KP_MODIFIER_RIGHT_ALT,     UsbHidUsageIdKbKpModifierKeyRightAlt     },
  { USB_HID_KB_KP_MODIFIER_LEFT_GUI,      UsbHidUsageIdKbKpModifierKeyLeftGui      },
  { USB_HID_KB_KP_MODIFIER_RIGHT_GUI,     UsbHidUsageIdKbKpModifierKeyRightGui     },
};

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
  )
{
  EFI_STATUS Status;

  ASSERT (DevicePath != NULL);

  Status = ReportStatusCodeWithDevicePath (
             CodeType,
             Value,
             0,
             &gEfiUsbKeyboardDriverGuid,
             DevicePath
             );

  ASSERT_EFI_ERROR (Status);
}

// IsUSBKeyboard
/** Uses USB I/O to check whether the device is a USB Keyboard device.

  @param[in] UsbIo  Points to a USB I/O protocol instance.

  @return  Returned is whether the device is a USB Keyboard device.
**/
BOOLEAN
IsUsbKeyboard (
  IN EFI_USB_IO_PROTOCOL  *UsbIo
  )
{
  EFI_STATUS                   Status;
  EFI_USB_INTERFACE_DESCRIPTOR InterfaceDescriptor;

  ASSERT (UsbIo != NULL);

  // Get the Default interface descriptor, currently we
  // assume it is interface 1
  Status = UsbIo->UsbGetInterfaceDescriptor (
                    UsbIo,
                    &InterfaceDescriptor
                    );

  ASSERT_EFI_ERROR (Status);

  return (!EFI_ERROR (Status)
       && InterfaceDescriptor.InterfaceClass == CLASS_HID
       && InterfaceDescriptor.InterfaceSubClass == SUBCLASS_BOOT
       && InterfaceDescriptor.InterfaceProtocol == PROTOCOL_KEYBOARD);
}

// UsbKbCheckForKey
/** Check whether there is key pending.

  @param[in] UsbKbDev  The USB_KB_DEV instance.

  @retval EFI_SUCCESS    Success
  @retval EFI_NOT_READY  Device is not ready
**/
EFI_STATUS
UsbKbCheckForKey (
  IN USB_KB_DEV  *UsbKbDev
  )
{
  EFI_STATUS  Status;

  UINT8       KeyChar;

  ASSERT (UsbKbDev != NULL);
  ASSERT (UsbKbDev->Signature == USB_KB_DEV_SIGNATURE);

  // Fetch raw data from the USB keyboard input,
  // and translate it into ASCII data.
  Status = UsbParseKey (UsbKbDev, &KeyChar);

  if (!EFI_ERROR (Status)) {
    UsbKbDev->CurKeyChar = KeyChar;
    Status                        = EFI_SUCCESS;
  }

  return Status;
}

// UsbKbWaitForKey
/** Handler function for WaitForKey event.

  @param[in] Event    Event to be signaled when a key is pressed.
  @param[in] Context  Points to USB_KB_DEV instance.
**/
VOID
EFIAPI
UsbKbWaitForKey (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  USB_KB_DEV *UsbKbDev;
  EFI_STATUS Status;

  ASSERT (Event != NULL);
  ASSERT (Context != NULL);
  ASSERT (((USB_KB_DEV *)Context)->Signature == USB_KB_DEV_SIGNATURE);

  UsbKbDev = (USB_KB_DEV *)Context;

  if (UsbKbDev->CurKeyChar == 0) {
    Status = UsbKbCheckForKey (UsbKbDev);

    if (EFI_ERROR (Status)) {
      goto Return;
    }
  }

  // If has key pending, signal the event.
  Status = gBS->SignalEvent (Event);

  ASSERT_EFI_ERROR (Status);

Return:
  return;
}

// InitUSBKeyboard
/** Initialize USB Keyboard device and all private data structures.

  @param[in] UsbKbDev  The USB_KB_DEV instance.

  @retval EFI_SUCCESS       Success
  @retval EFI_DEVICE_ERROR  Hardware Error
**/
EFI_STATUS
InitUsbKeyboard (
  IN USB_KB_DEV  *UsbKbDev
  )
{
  EFI_STATUS Status;

  UINT8      ConfigValue;
  UINT8      Protocol;
  UINT8      ReportId;
  UINT8      Duration;
  UINT32     TransferResult;

  ASSERT (UsbKbDev != NULL);
  ASSERT (UsbKbDev->Signature == USB_KB_DEV_SIGNATURE);

  KbdReportStatusCode (
    UsbKbDev->DevicePath,
    EFI_PROGRESS_CODE,
    (EFI_PERIPHERAL_KEYBOARD | EFI_P_KEYBOARD_PC_SELF_TEST)
    );

  InitUsbKbBuffer (&UsbKbDev->KeyboardBuffer);

  // default configurations
  ConfigValue = 0x01;

  // Uses default configuration to configure the USB Keyboard device.
  Status      = UsbSetConfiguration (
                  UsbKbDev->UsbIo,
                  (UINT16)ConfigValue,
                  &TransferResult
                  );

  if (EFI_ERROR (Status)) {
    // If configuration could not be set here, it means
    // the keyboard interface has some errors and could
    // not be initialized
    KbdReportStatusCode (
      UsbKbDev->DevicePath,
      EFI_ERROR_CODE | EFI_ERROR_MINOR,
      (EFI_PERIPHERAL_KEYBOARD | EFI_P_EC_INTERFACE_ERROR)
      );

    Status = EFI_DEVICE_ERROR;
  } else {
    UsbGetProtocolRequest (
      UsbKbDev->UsbIo,
      UsbKbDev->InterfaceDescriptor.InterfaceNumber,
      &Protocol
      );

    // Sets boot protocol for the USB Keyboard.
    // This driver only supports boot protocol.
    // !!BugBug: How about the device that does not support boot protocol?

    if (Protocol != BOOT_PROTOCOL) {
      UsbSetProtocolRequest (
        UsbKbDev->UsbIo,
        UsbKbDev->InterfaceDescriptor.InterfaceNumber,
        BOOT_PROTOCOL
        );
    }

    // the duration is indefinite, so the endpoint will inhibit reporting
    // forever, and only reporting when a change is detected in the report
    // data.

    // idle value for all report ID
    ReportId = 0;

    // idle forever until there is a key pressed and released.
    Duration = 0;

    UsbSetIdleRequest (
      UsbKbDev->UsbIo,
      UsbKbDev->InterfaceDescriptor.InterfaceNumber,
      ReportId,
      Duration
      );

    UsbKbDev->CtrlOn    = 0;
    UsbKbDev->AltOn     = 0;
    UsbKbDev->ShiftOn   = 0;
    UsbKbDev->NumLockOn = 0;
    UsbKbDev->CapsOn    = 0;

    // Sync the initial state of lights
    SetKeyLed (UsbKbDev);
    EfiZeroMem (
      (VOID *)UsbKbDev->LastKeyCodeArray,
      sizeof (UsbKbDev->LastKeyCodeArray)
      );

    // Set a timer for repeat keys' generation.

    if (UsbKbDev->RepeatTimer != NULL) {
      Status = gBS->CloseEvent (UsbKbDev->RepeatTimer);

      ASSERT_EFI_ERROR (Status);

      UsbKbDev->RepeatTimer = 0;
    }

    Status = gBS->CreateEvent (
                    EFI_EVENT_TIMER | EFI_EVENT_NOTIFY_SIGNAL,
                    EFI_TPL_CALLBACK,
                    UsbKbRepeatHandler,
                    UsbKbDev,
                    &UsbKbDev->RepeatTimer
                    );

    ASSERT_EFI_ERROR (Status);

    if (UsbKbDev->DelayedRecoveryEvent != NULL) {
      Status = gBS->CloseEvent (UsbKbDev->DelayedRecoveryEvent);

      ASSERT_EFI_ERROR (Status);

      UsbKbDev->DelayedRecoveryEvent = 0;
    }

    Status = gBS->CreateEvent (
                    EFI_EVENT_TIMER | EFI_EVENT_NOTIFY_SIGNAL,
                    EFI_TPL_NOTIFY,
                    UsbKbRecoveryHandler,
                    UsbKbDev,
                    &UsbKbDev->DelayedRecoveryEvent
                    );

    ASSERT_EFI_ERROR (Status);

    Status = EFI_SUCCESS;
  }

  return Status;
}

// KeyboardHandler
/** Handler function for USB Keyboard's asynchronous interrupt transfer.

  @param[in] Data        A pointer to a Buffer that is filled with key data
                         which is retrieved via asynchronous interrupt
                         transfer.
  @param[in] DataLength  Indicates the size of the data Buffer.
  @param[in] Context     Pointing to USB_KB_DEV instance.
  @param[in] Result      Indicates the result of the asynchronous interrupt
                         transfer.

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
  )
{
  EFI_STATUS          Status;

  USB_KB_DEV          *UsbKbDev;
  EFI_USB_IO_PROTOCOL *UsbIo;
  UINT8               *CurKeyCodeBuffer;
  UINT8               *OldKeyCodeBuffer;
  UINT8               CurModifierMap;
  UINT8               OldModifierMap;
  UINT8               Index;
  UINT8               Index2;
  BOOLEAN             Down;
  UINT8               SavedTail;
  USB_KEY             UsbKey;
  UINT8               NewRepeatKey;
  UINT32              UsbStatus;
  UINTN               NumberOfKeys;
  APPLE_KEY           Keys[34];

  ASSERT (Data != NULL);
  ASSERT (DataLength > 0);
  ASSERT (Context != NULL);

  NewRepeatKey = 0;
  UsbKbDev     = (USB_KB_DEV *)Context;
  UsbIo        = UsbKbDev->UsbIo;

  // Analyzes the Result and performs corresponding action.
  if (Result != EFI_USB_NOERROR) {
    // Some errors happen during the process
    KbdReportStatusCode (
      UsbKbDev->DevicePath,
      EFI_ERROR_CODE | EFI_ERROR_MINOR,
      (EFI_PERIPHERAL_KEYBOARD | EFI_P_EC_INPUT_ERROR)
      );

    // stop the repeat key generation if any
    UsbKbDev->RepeatKey = 0;
    Status              = gBS->SetTimer (
                                 UsbKbDev->RepeatTimer,
                                 TimerCancel,
                                 USBKBD_REPEAT_RATE
                                 );

    ASSERT_EFI_ERROR (Status);

    if ((Result & EFI_USB_ERR_STALL) == EFI_USB_ERR_STALL) {
      UsbClearEndpointHalt (
        UsbIo,
        UsbKbDev->EndpointDescriptor.EndpointAddress,
        &UsbStatus
        );
    }

    // Delete & Submit this interrupt again

    Status = UsbIo->UsbAsyncInterruptTransfer (
                      UsbIo,
                      UsbKbDev->EndpointDescriptor.EndpointAddress,
                      FALSE,
                      0,
                      0,
                      NULL,
                      NULL
                      );

    ASSERT_EFI_ERROR (Status);

    Status = gBS->SetTimer (
                    UsbKbDev->DelayedRecoveryEvent,
                    TimerRelative,
                    EFI_USB_INTERRUPT_DELAY
                    );

    ASSERT_EFI_ERROR (Status);

    Status = EFI_DEVICE_ERROR;
  } else if ((DataLength == 0) || (Data == NULL)) {
    Status = EFI_SUCCESS;
  } else {
    CurKeyCodeBuffer = (UINT8 *)Data;
    OldKeyCodeBuffer = UsbKbDev->LastKeyCodeArray;

    // checks for new key stroke.
    // if no new key got, return immediately.
    for (Index = 0; Index < 8; Index++) {
      if (OldKeyCodeBuffer[Index] != CurKeyCodeBuffer[Index]) {
        break;
      }
    }

    Status = EFI_SUCCESS;

    if (Index != 8) {
      CurModifierMap = CurKeyCodeBuffer[0];
      NumberOfKeys   = 0;

      // Pass the data to the Apple protocol
      for (Index = 2; Index < 8; Index++) {
        if (USB_HID_KB_KP_VALID_KEYCODE (CurKeyCodeBuffer[Index])) {
          Keys[NumberOfKeys] = APPLE_HID_USB_KB_KP_USGAE (
                                 CurKeyCodeBuffer[Index]
                                 );

          ++NumberOfKeys;
        }
      }

      UsbKbDev->KeyMapDb->SetKeyStrokeBufferKeys (
                            UsbKbDev->KeyMapDb,
                            UsbKbDev->KeyMapDbIndex,
                            (APPLE_MODIFIER_MAP)CurModifierMap,
                            NumberOfKeys,
                            &Keys[0]
                            );

      // Parse the modifier key
      OldModifierMap = OldKeyCodeBuffer[0];

      // handle modifier key's pressing or releasing situation.
      for (Index = 0; Index < 8; ++Index) {
        if ((CurModifierMap & mKeyboardModifierMap[Index].Mask)
         != (OldModifierMap & mKeyboardModifierMap[Index].Mask)) {
          // if current modifier key is up, then
          // CurModifierMap & mKeyboardModifierMap[Index].Mask = 0;
          // otherwize it is a non-zero value.
          // Inserts the pressed modifier key into key Buffer.
          Down = (UINT8)(CurModifierMap & mKeyboardModifierMap[Index].Mask);

          InsertKeyCode (
            &(UsbKbDev->KeyboardBuffer),
            mKeyboardModifierMap[Index].Key,
            Down
            );
        }
      }

      // handle normal key's releasing situation

      for (Index = 2; Index < 8; Index++) {
        if (USB_HID_KB_KP_VALID_KEYCODE (OldKeyCodeBuffer[Index])) {
          for (Index2 = 2; Index2 < 8; ++Index2) {
            if (USB_HID_KB_KP_VALID_KEYCODE (CurKeyCodeBuffer[Index2])
             && (OldKeyCodeBuffer[Index] == CurKeyCodeBuffer[Index2])) {
              break;
            }

            InsertKeyCode (
              &(UsbKbDev->KeyboardBuffer),
              OldKeyCodeBuffer[Index],
              0
              );

            // the original reapeat key is released.

            if (OldKeyCodeBuffer[Index] == UsbKbDev->RepeatKey) {
              UsbKbDev->RepeatKey = 0;
            }
          }
        }
      }

      // original repeat key is released, cancel the repeat timer

      if (UsbKbDev->RepeatKey == 0) {
        Status = gBS->SetTimer (
                        UsbKbDev->RepeatTimer,
                        TimerCancel,
                        USBKBD_REPEAT_RATE
                        );

        ASSERT_EFI_ERROR (Status);
      }

      // handle normal key's pressing situation

      for (Index = 2; Index < 8; ++Index) {
        if (USB_HID_KB_KP_VALID_KEYCODE (CurKeyCodeBuffer[Index])) {
          for (Index2 = 2; Index2 < 8; ++Index2) {
            if (USB_HID_KB_KP_VALID_KEYCODE (OldKeyCodeBuffer[Index2])
             && (CurKeyCodeBuffer[Index] == OldKeyCodeBuffer[Index2])) {
              break;
            }
          }

          InsertKeyCode (
            &(UsbKbDev->KeyboardBuffer),
            CurKeyCodeBuffer[Index],
            1
            );

          // NumLock pressed or CapsLock pressed

          if ((CurKeyCodeBuffer[Index] == UsbHidUsageIdKbKpLockKeyNLock)
           || (CurKeyCodeBuffer[Index] == UsbHidUsageIdKbKpLockKeyCLock)) {
            UsbKbDev->RepeatKey = 0;
          } else {
            NewRepeatKey = CurKeyCodeBuffer[Index];

            // do not repeat the original repeated key
            UsbKbDev->RepeatKey = 0;
          }
        }
      }

      // Update LastKeycodeArray[] Buffer in the
      // Usb Keyboard Device data structure.
      for (Index = 0; Index < 8; ++Index) {
        UsbKbDev->LastKeyCodeArray[Index] = CurKeyCodeBuffer[Index];
      }

      // pre-process KeyboardBuffer, pop out the ctrl,alt,del key in sequence
      // and judge whether it will invoke reset event.
      SavedTail = UsbKbDev->KeyboardBuffer.Tail;
      Index     = UsbKbDev->KeyboardBuffer.Head;

      while (Index != SavedTail) {
        RemoveKeyCode (&(UsbKbDev->KeyboardBuffer), &UsbKey);

        switch (UsbKey.KeyCode) {
        case UsbHidUsageIdKbKpModifierKeyLeftControl:
        case UsbHidUsageIdKbKpModifierKeyRightControl:
        {
          if (UsbKey.Down != 0) {
            UsbKbDev->CtrlOn = 1;
          } else {
            UsbKbDev->CtrlOn = 0;
          }

          break;
        }

        case UsbHidUsageIdKbKpModifierKeyLeftAlt:
        case UsbHidUsageIdKbKpModifierKeyRightAlt:
        {
          if (UsbKey.Down != 0) {
            UsbKbDev->AltOn = 1;
          } else {
            UsbKbDev->AltOn = 0;
          }

          break;
        }

        default:
        {
          break;
        }
        }

        // insert the key back to the Buffer.
        // so the key sequence will not be destroyed.
        InsertKeyCode (
          &(UsbKbDev->KeyboardBuffer),
          UsbKey.KeyCode,
          UsbKey.Down
          );

        Index = UsbKbDev->KeyboardBuffer.Head;

      }

      // If have new key pressed, update the RepeatKey value, and set the
      // timer to repeate delay timer

      if (NewRepeatKey != 0) {
        // sets trigger time to "Repeat Delay Time",
        // to trigger the repeat timer when the key is hold long
        // enough time.
        Status = gBS->SetTimer (
                        UsbKbDev->RepeatTimer,
                        TimerRelative,
                        USBKBD_REPEAT_DELAY
                        );

        ASSERT_EFI_ERROR (Status);

        UsbKbDev->RepeatKey = NewRepeatKey;
      }

      Status = EFI_SUCCESS;
    }
  }

  return Status;
}

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
  )
{
  EFI_STATUS Status;

  USB_KEY    UsbKey;

  ASSERT (UsbKbDev != NULL);
  ASSERT (UsbKbDev->Signature == USB_KB_DEV_SIGNATURE);
  ASSERT (KeyChar != NULL);

  *KeyChar = 0;
  Status   = EFI_NOT_READY;

  while (!IsUsbKbBufferEmpty (UsbKbDev->KeyboardBuffer)) {
    // pops one raw data off.
    RemoveKeyCode (&(UsbKbDev->KeyboardBuffer), &UsbKey);

    if (UsbKey.Down == 0) {
      switch (UsbKey.KeyCode) {
      // CTRL release
      case UsbHidUsageIdKbKpModifierKeyLeftControl:
      case UsbHidUsageIdKbKpModifierKeyRightControl:
      {
        UsbKbDev->CtrlOn = 0;
        break;
      }

      // Shift release
      case UsbHidUsageIdKbKpModifierKeyLeftShift:
      case UsbHidUsageIdKbKpModifierKeyRightShift:
      {
        UsbKbDev->ShiftOn = 0;
        break;
      }

      // Alt release
      case UsbHidUsageIdKbKpModifierKeyLeftAlt:
      case UsbHidUsageIdKbKpModifierKeyRightAlt:
      {
        UsbKbDev->AltOn = 0;
        break;
      }

      default:
      {
        break;
      }
      }

      continue;
    }

    // Analyzes key pressing situation
    switch (UsbKey.KeyCode) {
    // CTRL press
    case UsbHidUsageIdKbKpModifierKeyLeftControl:
    case UsbHidUsageIdKbKpModifierKeyRightControl:
    {
      UsbKbDev->CtrlOn = 1;
      continue;
    }

    // Shift press
    case UsbHidUsageIdKbKpModifierKeyLeftShift:
    case UsbHidUsageIdKbKpModifierKeyRightShift:
    {
      UsbKbDev->ShiftOn = 1;
      continue;
    }

    // Alt press
    case UsbHidUsageIdKbKpModifierKeyLeftAlt:
    case UsbHidUsageIdKbKpModifierKeyRightAlt:
    {
      UsbKbDev->AltOn = 1;
      continue;
    }

    case UsbHidUsageIdKbKpModifierKeyLeftGui:
    case UsbHidUsageIdKbKpModifierKeyRightGui:
    {
      continue;
    }

    case UsbHidUsageIdKbKpPadKeyNLck:
    {
      UsbKbDev->NumLockOn ^= 1;

      // Turn on the NumLock light on KB
      SetKeyLed (UsbKbDev);
      continue;
    }

    case UsbHidUsageIdKbKpKeyCLock:
    {
      UsbKbDev->CapsOn ^= 1;

      // Turn on the CapsLock light on KB
      SetKeyLed (UsbKbDev);
      continue;
    }

    // PrintScreen, Pause, Application, Power
    // keys are not valid EFI key

    // fall through
    case UsbHidUsageIdKbKpKeyPrint:
    case UsbHidUsageIdKbKpKeyPause:
    case UsbHidUsageIdKbKpPadKeyApplication:
    case UsbHidUsageIdKbKpPadKeyPower:
    {
      continue;
    }

    default:
    {
      break;
    }
    }

    *KeyChar = UsbKey.KeyCode;
    Status   = EFI_SUCCESS;
  }

  DEBUG_CODE (
    if (Status != EFI_NOT_READY) {
      ASSERT_EFI_ERROR (Status);
    }
    );

  return Status;
}

// UsbKeyCodeToEfiScanCode
/** Converts USB Keyboard code to EFI Scan Code.

  @param[in]  UsbKbDev  The USB_KB_DEV instance.
  @param[in]  KeyChar   Indicates the key code that will be interpreted.
  @param[out] Key       A pointer to a Buffer that is filled in with the
                        keystroke information for the key that was pressed.

  @retval EFI_NOT_READY  Device is not ready
  @retval EFI_SUCCESS    Success
**/
EFI_STATUS
UsbKeyCodeToEfiScanCode (
  IN  USB_KB_DEV     *UsbKbDev,
  IN  UINT8          KeyChar,
  OUT EFI_INPUT_KEY  *Key
  )
{
  EFI_STATUS Status;

  UINT8      Index;

  ASSERT (UsbKbDev != NULL);
  ASSERT (UsbKbDev->Signature == USB_KB_DEV_SIGNATURE);
  ASSERT (USB_HID_KB_KP_VALID_KEYCODE (KeyChar));
  ASSERT (Key != NULL);

  Status = EFI_NOT_READY;

  if (USB_HID_KB_KP_VALID_KEYCODE (KeyChar)) {
    // valid USB Key Code starts from 4
    Index = (UINT8)(KeyChar - 4);

    ASSERT (Index < USB_KEYCODE_MAX_MAKE);

    if (Index < USB_KEYCODE_MAX_MAKE) {
      Key->ScanCode = mKeyConvertionTable[Index].ScanCode;

      if (UsbKbDev->ShiftOn != 0) {
        Key->UnicodeChar = mKeyConvertionTable[Index].ShiftedUnicode;
      } else {
        Key->UnicodeChar = mKeyConvertionTable[Index].Unicode;
      }

      if (UsbKbDev->CapsOn != 0) {
        if ((Key->UnicodeChar >= 'a') && (Key->UnicodeChar <= 'z')) {
          Key->UnicodeChar = mKeyConvertionTable[Index].ShiftedUnicode;
        } else if ((Key->UnicodeChar >= 'A') && (Key->UnicodeChar <= 'Z')) {
          Key->UnicodeChar = mKeyConvertionTable[Index].Unicode;
        }
      }

      // Translate the CTRL-Alpha characters to their corresponding control
      // value  (ctrl-a = 0x0001 through ctrl-Z = 0x001A)
      if (UsbKbDev->CtrlOn != 0) {
        if ((Key->UnicodeChar >= 'a') && (Key->UnicodeChar <= 'z')) {
          Key->UnicodeChar = (Key->UnicodeChar - 'a' + 1);
        } else if ((Key->UnicodeChar >= 'A') && (Key->UnicodeChar <= 'Z')) {
          Key->UnicodeChar = (Key->UnicodeChar - 'A' + 1);
        }
      }

      if ((KeyChar >= UsbHidUsageIdKbKpPadKeyOne)
       && (KeyChar <= UsbHidUsageIdKbKpPadKeyDel)) {
        if ((UsbKbDev->NumLockOn != 0) && (UsbKbDev->ShiftOn == 0)) {
          Key->ScanCode = SCAN_NULL;
        } else {
          Key->UnicodeChar = CHAR_NULL;
        }
      }

      Status = EFI_SUCCESS;

      if ((Key->UnicodeChar == 0) && (Key->ScanCode == SCAN_NULL)) {
        Status = EFI_NOT_READY;
      }
    }
  }

  return Status;
}

// InitUsbKbBuffer
/** Resets USB Keyboard Buffer.

  @param[in, out] KeyboardBuffer  Points to the USB Keyboard Buffer.

  @retval EFI_SUCCESS  Success
**/
EFI_STATUS
InitUsbKbBuffer (
  IN OUT USB_KB_BUFFER  *KeyboardBuffer
  )
{
  ASSERT (KeyboardBuffer != NULL);

  EfiZeroMem (KeyboardBuffer, sizeof (*KeyboardBuffer));

  KeyboardBuffer->Head = KeyboardBuffer->Tail;

  return EFI_SUCCESS;
}

// IsUsbKbBufferEmpty
/** Check whether USB Keyboard Buffer is empty.

  @param[in] KeyboardBuffer  USB Keyboard Buffer.

  @return  Returned is whether USB Keyboard Buffer is empty.
**/
BOOLEAN
IsUsbKbBufferEmpty (
  IN USB_KB_BUFFER  KeyboardBuffer
  )
{
  // meet FIFO empty condition
  return (BOOLEAN)(KeyboardBuffer.Head == KeyboardBuffer.Tail);
}

// IsUsbKbBufferFull
/** Check whether USB Keyboard Buffer is full.

  @param[in] KeyboardBuffer  USB Keyboard Buffer.

  @return  Returned is whether USB Keyboard Buffer is full.
**/
BOOLEAN
IsUsbKbBufferFull (
  IN USB_KB_BUFFER  KeyboardBuffer
  )
{
  return (((KeyboardBuffer.Tail + 1) % (MAX_KEY_ALLOWED + 1))
           == KeyboardBuffer.Head);
}

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
  )
{
  USB_KEY UsbKey;

  ASSERT (KeyboardBuffer != NULL);

  // if keyboard Buffer is full, throw the
  // first key out of the keyboard Buffer.
  if (IsUsbKbBufferFull (*KeyboardBuffer)) {
    RemoveKeyCode (KeyboardBuffer, &UsbKey);
  }

  KeyboardBuffer->Buffer[KeyboardBuffer->Tail].KeyCode = Key;
  KeyboardBuffer->Buffer[KeyboardBuffer->Tail].Down    = Down;

  // adjust the tail pointer of the FIFO keyboard Buffer.
  KeyboardBuffer->Tail = (UINT8)((KeyboardBuffer->Tail + 1)
                           % (MAX_KEY_ALLOWED + 1));

  return EFI_SUCCESS;
}

// RemoveKeyCode
/** Pops a key code off from keyboard Buffer.

  @param[in, out] KeyboardBuffer  Points to the USB Keyboard Buffer.
  @param[in]      UsbKey          Points to the Buffer that contains a usb key
                                  code.

  @retval EFI_SUCCESS       Success
  @retval EFI_DEVICE_ERROR  Hardware Error
**/
EFI_STATUS
RemoveKeyCode (
  IN OUT USB_KB_BUFFER  *KeyboardBuffer,
  OUT    USB_KEY        *UsbKey
  )
{
  EFI_STATUS Status;

  ASSERT (KeyboardBuffer != NULL);
  ASSERT (UsbKey != NULL);
  ASSERT (!IsUsbKbBufferEmpty (*KeyboardBuffer));

  Status = EFI_DEVICE_ERROR;

  if (!IsUsbKbBufferEmpty (*KeyboardBuffer)) {
    UsbKey->KeyCode = KeyboardBuffer->Buffer[KeyboardBuffer->Head].KeyCode;
    UsbKey->Down    = KeyboardBuffer->Buffer[KeyboardBuffer->Head].Down;

    // adjust the head pointer of the FIFO keyboard Buffer.
    KeyboardBuffer->Head = (UINT8)(
                             (KeyboardBuffer->Head + 1) % (MAX_KEY_ALLOWED + 1)
                             );

    Status = EFI_SUCCESS;
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}

// SetKeyLed
/** Sets USB Keyboard LED state.

  @param[in] UsbKbDev  The USB_KB_DEV instance.

  @retval EFI_SUCCESS  Success
**/
EFI_STATUS
SetKeyLed (
  IN USB_KB_DEV  *UsbKbDev
  )
{
  LED_MAP Led;
  UINT8   ReportId;

  ASSERT (UsbKbDev != NULL);
  ASSERT (UsbKbDev->Signature == USB_KB_DEV_SIGNATURE);

  // Set each field in Led map.
  Led.CapsLock   = (UINT8)UsbKbDev->CapsOn;
  Led.NumLock    = 0;
  Led.ScrollLock = 0;
  Led.Reserved   = 0;
  ReportId       = 0;

  // call Set Report Request to lighten the LED.
  UsbSetReportRequest (
    UsbKbDev->UsbIo,
    UsbKbDev->InterfaceDescriptor.InterfaceNumber,
    ReportId,
    HID_OUTPUT_REPORT,
    1,
    (UINT8 *)&Led
    );

  return EFI_SUCCESS;
}

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
  )
{
  USB_KB_DEV *UsbKbDev;

  EFI_STATUS Status;

  ASSERT (Event != NULL);
  ASSERT (Context != NULL);
  ASSERT (((USB_KB_DEV *)Context)->Signature == USB_KB_DEV_SIGNATURE);

  UsbKbDev = (USB_KB_DEV *)Context;

  // Do nothing when there is no repeat key.
  if (UsbKbDev->RepeatKey != 0) {
    // Inserts one Repeat key into keyboard Buffer.
    InsertKeyCode (
      &(UsbKbDev->KeyboardBuffer),
      UsbKbDev->RepeatKey,
      1
      );

    // set repeate rate for repeat key generation.
    Status = gBS->SetTimer (
                    UsbKbDev->RepeatTimer,
                    TimerRelative,
                    USBKBD_REPEAT_RATE
                    );

    ASSERT_EFI_ERROR (Status);
  }
}

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
  )
{
  USB_KB_DEV          *UsbKbDev;
  EFI_USB_IO_PROTOCOL *UsbIo;
  UINT8               PacketSize;
  EFI_STATUS          Status;

  ASSERT (Event != NULL);
  ASSERT (Context != NULL);
  ASSERT (((USB_KB_DEV *)Context)->Signature == USB_KB_DEV_SIGNATURE);

  UsbKbDev   = (USB_KB_DEV *)Context;
  UsbIo      = UsbKbDev->UsbIo;
  PacketSize = (UINT8)(UsbKbDev->EndpointDescriptor.MaxPacketSize);
  Status     = UsbIo->UsbAsyncInterruptTransfer (
                        UsbIo,
                        UsbKbDev->EndpointDescriptor.EndpointAddress,
                        TRUE,
                        UsbKbDev->EndpointDescriptor.Interval,
                        PacketSize,
                        KeyboardHandler,
                        UsbKbDev
                        );

  ASSERT_EFI_ERROR (Status);
}
