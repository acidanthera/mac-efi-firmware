/** @file
  Copyright (C) 2004 - 2007, Intel Corporation<BR>
  All rights reserved.  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php/

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include <AppleEfi.h>

#include "UsbKbLib.h"

#include EFI_PROTOCOL_DEFINITION (SimpleTextInputEx)
#include "UsbKbSimpleTextInputImpl.h"

// UsbKbReset
/** Implements EFI_SIMPLE_TEXT_IN_PROTOCOL.Reset () function.

  @param[in] This                  The EFI_SIMPLE_TEXT_IN_PROTOCOL instance.
  @param[in] ExtendedVerification  Indicates that the driver may perform a more
                                   exhaustive verification operation of

  @retval EFI_SUCCESS       Success
  @retval EFI_DEVICE_ERROR  Hardware Error
**/
EFI_STATUS
EFIAPI
UsbKbReset (
  IN EFI_SIMPLE_TEXT_IN_PROTOCOL  *This,
  IN BOOLEAN                      ExtendedVerification
  )
{
  EFI_STATUS Status;

  USB_KB_DEV *UsbKbDev;

  ASSERT (This != NULL);

  UsbKbDev = USB_KB_DEV_FROM_THIS (This);

  KbdReportStatusCode (
    UsbKbDev->DevicePath,
    EFI_PROGRESS_CODE,
    (EFI_PERIPHERAL_KEYBOARD | EFI_P_PC_RESET)
    );

  // Non Exhaustive reset:
  // only reset private data structures.

  if (!ExtendedVerification) {
    // Clear the key Buffer of this Usb keyboard

    KbdReportStatusCode (
      UsbKbDev->DevicePath,
      EFI_PROGRESS_CODE,
      (EFI_PERIPHERAL_KEYBOARD | EFI_P_KEYBOARD_PC_CLEAR_BUFFER)
      );

    InitUsbKbBuffer (&(UsbKbDev->KeyboardBuffer));

    UsbKbDev->CurKeyChar = 0;
    Status               = EFI_SUCCESS;
  } else {
    // Exhaustive reset

    Status               = InitUsbKeyboard (UsbKbDev);
    UsbKbDev->CurKeyChar = 0;

    if (EFI_ERROR (Status)) {
      Status = EFI_DEVICE_ERROR;
    }
  }

  ASSERT (Status != EFI_DEVICE_ERROR);

  return Status;
}

// UsbKbReadKeyStrokeWorker
/** Reads the next keystroke from the input device.The WaitForKey Event can be
    used to test for existance of a keystroke via WaitForEvent () call.

  @param[in]  UsbKbDev  Usb keyboard private structure.
  @param[out] KeyData            A pointer to a Buffer that is filled in with
                                 the keystroke state data for the key that was
                                 pressed.

  @retval EFI_SUCCESS            The keystroke information was returned.
  @retval EFI_NOT_READY          There was no keystroke data availiable.
  @retval EFI_DEVICE_ERROR       The keystroke information was not returned due
                                 to hardware errors.
  @retval EFI_INVALID_PARAMETER  KeyData is NULL.
**/
EFI_STATUS
UsbKbReadKeyStrokeWorker (
  IN  USB_KB_DEV    *UsbKbDev,
  OUT EFI_KEY_DATA  *KeyData
  )
{

  EFI_STATUS Status;
  UINT8      KeyChar;

  ASSERT (UsbKbDev != NULL);
  ASSERT (UsbKbDev->Signature == USB_KB_DEV_SIGNATURE);
  ASSERT (KeyData != NULL);

  Status = EFI_INVALID_PARAMETER;

  if (KeyData != NULL) {
    // if there is no saved ASCII byte, fetch it
    // by calling UsbKbCheckForKey ().

    if (UsbKbDev->CurKeyChar == 0) {
      Status = UsbKbCheckForKey (UsbKbDev);

      if (EFI_ERROR (Status)) {
        goto Return;
      }
    }

    KeyData->Key.UnicodeChar = 0;
    KeyData->Key.ScanCode    = SCAN_NULL;
    KeyChar                  = UsbKbDev->CurKeyChar;
    UsbKbDev->CurKeyChar     = 0;

    // Translate saved ASCII byte into EFI_INPUT_KEY
    Status = UsbKeyCodeToEfiScanCode (UsbKbDev, KeyChar, &KeyData->Key);
  }

Return:
  ASSERT (Status != EFI_INVALID_PARAMETER);

  return Status;
}

// UsbKbReadKeyStroke
/** Implements EFI_SIMPLE_TEXT_IN_PROTOCOL.ReadKeyStroke () function.

  @param[in]  This  The EFI_SIMPLE_TEXT_IN_PROTOCOL instance.
  @param[out] Key   A pointer to a Buffer that is filled in with the keystroke
                    information for the key that was pressed.

  @retval EFI_SUCCESS  Success
**/
EFI_STATUS
EFIAPI
UsbKbReadKeyStroke (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL  *This,
  OUT EFI_INPUT_KEY                *Key
  )
{
  EFI_STATUS   Status;

  USB_KB_DEV   *UsbKbDev;
  EFI_KEY_DATA KeyData;

  ASSERT (This != NULL);
  ASSERT (Key != NULL);

  UsbKbDev = USB_KB_DEV_FROM_THIS (This);
  Status   = UsbKbReadKeyStrokeWorker (UsbKbDev, &KeyData);

  if (!EFI_ERROR (Status)) {
    EfiCopyMem (Key, &KeyData.Key, sizeof (KeyData.Key));

    Status = EFI_SUCCESS;
  }

  return Status;
}
