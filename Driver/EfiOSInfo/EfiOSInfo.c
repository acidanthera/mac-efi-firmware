/** @file
  Copyright (C) 2005 - 2017, Apple Inc.  All rights reserved.<BR>

  This program and the accompanying materials have not been licensed.
  Neither is its usage, its redistribution, in source or binary form,
  licensed, nor implicitely or explicitely permitted, except when
  required by applicable law.

  Unless required by applicable law or agreed to in writing, software
  distributed is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
  OR CONDITIONS OF ANY KIND, either express or implied.
**/

#include <AppleEfi.h>

#include APPLE_PROTOCOL_PRODUCER (OSInfoImpl)

#include <Library/AppleDriverLib.h>

#include <Driver/EfiOSInfo.h>

// mEfiOSInfo
STATIC EFI_OS_INFO_PROTOCOL mOSInfoProtocol = {
  EFI_OS_INFO_PROTOCOL_REVISION,
  OSInfoOSVendor,
  OSInfoOSName
};

EFI_DRIVER_ENTRY_POINT (EfiOSInfoMain);

// EfiOSInfoMain
/**

  @param[in] ImageHandle  The firmware allocated handle for the EFI image.
  @param[in] SystemTable  A pointer to the EFI System Table.

  @retval EFI_SUCCESS          The entry point is executed successfully.
  @retval EFI_ALREADY_STARTED  The protocol has already been installed.
**/
EFI_STATUS
EFIAPI
EfiOSInfoMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  ) // start
{
  AppleInitializeDriverLib (ImageHandle, SystemTable);

  ASSERT_PROTOCOL_ALREADY_INSTALLED (NULL, &gEfiOSInfoProtocolGuid);

  return gBS->InstallProtocolInterface (
                ImageHandle,
                &gEfiOSInfoProtocolGuid,
                EFI_NATIVE_INTERFACE,
                (VOID *)&mOSInfoProtocol
                );
}
