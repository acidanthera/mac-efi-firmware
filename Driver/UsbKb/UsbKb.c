/** @file
  USB Keyboard Driver

  Copyright (C) 2004 - 2007, Intel Corporation<BR>
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php/

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include <AppleEfi.h>
#include <EfiCompNameSupport.h>

#include "UsbKbLib.h"

#include "UsbKbComponentNameImpl.h"
#include "UsbKbDriverBindingImpl.h"

// gEfiUsbKeyboardDriverGuid
GLOBAL_REMOVE_IF_UNREFERENCED EFI_GUID gEfiUsbKeyboardDriverGuid = EFI_USB_KEYBOARD_DRIVER_GUID;

// gUsbKbComponentNameProtocol
GLOBAL_REMOVE_IF_UNREFERENCED EFI_COMPONENT_NAME_PROTOCOL gUsbKbComponentNameProtocol = {
  UsbKbComponentNameGetDriverName,
  UsbKbComponentNameGetControllerName,
  LANGUAGE_CODE_ENGLISH
};

// gUsbKbDriverBindingProtocol
GLOBAL_REMOVE_IF_UNREFERENCED EFI_DRIVER_BINDING_PROTOCOL gUsbKbDriverBindingProtocol = {
  UsbKbBindingSupported,
  UsbKbBindingStart,
  UsbKbBindingStop,
  USB_KB_DRIVER_BINDING_VERSION,
  NULL,
  NULL
};

// gUsbKbKeyboardInformationProtocol
GLOBAL_REMOVE_IF_UNREFERENCED EFI_KEYBOARD_INFO_PROTOCOL gUsbKbKeyboardInformationProtocol = {
  KbInfoGetInfo
};

EFI_DRIVER_ENTRY_POINT (UsbKbMain);

// UsbKbMain
/** Driver Entry Point.

  @param[in] ImageHandle  EFI_HANDLE
  @param[in] SystemTable  EFI_SYSTEM_TABLE

  @retval EFI_STATUS  Success
**/
EFI_STATUS
EFIAPI
UsbKbMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )     
{
  return INSTALL_ALL_DRIVER_PROTOCOLS_OR_PROTOCOLS2 (
           ImageHandle,
           SystemTable,
           &gUsbKbDriverBindingProtocol,
           ImageHandle,
           &gUsbKbComponentNameProtocol,
           NULL,
           NULL
           );
}
