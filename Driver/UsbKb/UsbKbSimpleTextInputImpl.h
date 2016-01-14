/** @file
  Copyright (C) 2004 - 2007, Intel Corporation<BR>
  All rights reserved.  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php/

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#ifndef USB_KB_SIMPLE_TEXT_INPUT_IMPL_H_
#define USB_KB_SIMPLE_TEXT_INPUT_IMPL_H_

#include EFI_PROTOCOL_PRODUCER (SimpleTextIn)

// UsbKbReset
/** Implements EFI_SIMPLE_TEXT_IN_PROTOCOL.Reset () function.

  @param[in] This                  The EFI_SIMPLE_TEXT_IN_PROTOCOL instance.
  @param[in] ExtendedVerification  Indicates that the driver may perform a more exhaustive verification operation of

  @retval EFI_SUCCESS       Success
  @retval EFI_DEVICE_ERROR  Hardware Error
**/
EFI_STATUS
EFIAPI
UsbKbReset (
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL  *This,
  IN  BOOLEAN                      ExtendedVerification
  );

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
  IN  EFI_SIMPLE_TEXT_IN_PROTOCOL   *This,
  OUT EFI_INPUT_KEY                 *Key
  );

// UsbKbWaitForKey
/** Handler function for WaitForKey event.

  @param[in] Event    Event to be signaled when a key is pressed.
  @param[in] Context  Points to USB_KB_DEV instance.
**/
VOID
EFIAPI
UsbKbWaitForKey (
  IN  EFI_EVENT               Event,
  IN  VOID                    *Context
  );

#endif // USB_KB_SIMPLE_TEXT_INPUT_IMPL_H_
