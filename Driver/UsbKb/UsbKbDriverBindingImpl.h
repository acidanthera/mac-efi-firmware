/** @file
  Copyright (C) 2004 - 2007, Intel Corporation<BR>
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php/

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#ifndef USB_KB_DRIVER_BINDING_IMPL_H_
#define USB_KB_DRIVER_BINDING_IMPL_H_

#include EFI_PROTOCOL_PRODUCER (DriverBinding)

// USB_KB_DRIVER_BINDING_VERSION
#define USB_KB_DRIVER_BINDING_VERSION  0x10

// UsbKbBindingSupported
/** Supported.

  @param[in] This                 EFI_DRIVER_BINDING_PROTOCOL
  @param[in] Controller           Controller handle
  @param[in] RemainingDevicePath  EFI_DEVICE_PATH_PROTOCOL

  @retval EFI_STATUS  Success.
**/
EFI_STATUS
EFIAPI
UsbKbBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

// UsbKbBindingStart
/** Start.

  @param[in] This                 EFI_DRIVER_BINDING_PROTOCOL
  @param[in] Controller           Controller handle
  @param[in] RemainingDevicePath  EFI_DEVICE_PATH_PROTOCOL

  @retval EFI_SUCCESS           Success
  @retval EFI_OUT_OF_RESOURCES  Can't allocate memory
  @retval EFI_UNSUPPORTED       The Start routine fail
**/
EFI_STATUS
EFIAPI
UsbKbBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

// UsbKbBindingStop
/** Stop.

  @param[in] This               EFI_DRIVER_BINDING_PROTOCOL
  @param[in] Controller         Controller handle
  @param[in] NumberOfChildren   Child handle number
  @param[in] ChildHandleBuffer  Child handle Buffer

  @retval EFI_SUCCESS      Success
  @retval EFI_UNSUPPORTED  Can't support
**/
EFI_STATUS
EFIAPI
UsbKbBindingStop (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN UINTN                        NumberOfChildren,
  IN EFI_HANDLE                   *ChildHandleBuffer
  );

// gUsbKbDriverBindingProtocol
extern EFI_DRIVER_BINDING_PROTOCOL gUsbKbDriverBindingProtocol;

#endif // USB_KB_DRIVER_BINDING_IMPL_H_
