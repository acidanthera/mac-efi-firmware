//
// Copyright (C) 2005 - 2015 Apple Inc. All rights reserved.
//
// This program and the accompanying materials have not been licensed.
// Neither is its usage, its redistribution, in source or binary form,
// licensed, nor implicitely or explicitely permitted, except when
// required by applicable law.
//
// Unless required by applicable law or agreed to in writing, software
// distributed is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
// OR CONDITIONS OF ANY KIND, either express or implied.
//

///
/// @file      Driver/AppleBootPolicy/AppleBootPolicy.c
///
///            Apple's driver to get a volume's bootable file.
///
/// @author    Download-Fritz
/// @date      19/12/2014: Initial version
/// @date      23/02/2015: Minor tweaks
/// @date      15/03/2015: Updated documentation
/// @copyright Copyright (C) 2005 - 2015 Apple Inc. All rights reserved.
///

#include <AppleEfi.h>
#include <EfiCommon.h>

#include <Library/AppleDriverLib.h>

#include <Protocol/AppleBootPolicyImpl.h>

#include <Driver/AppleBootPolicy.h>

// mAppleBootPolicyProtocol
/// The APPLE_BOOT_POLICY_PROTOCOL instance to get installed.
static APPLE_BOOT_POLICY_PROTOCOL mAppleBootPolicyProtocol = {
  APPLE_BOOT_POLICY_PROTOCOL_REVISION,
  BootPolicyGetBootFileImpl
};

EFI_DRIVER_ENTRY_POINT (AppleBootPolicyMain);

// AppleBootPolicyMain
/// The Entry Point installing the APPLE_BOOT_POLICY_PROTOCOL.
///
/// @param[in] ImageHandle  The firmware allocated handle for the EFI image.
/// @param[in] SystemTable  A pointer to the EFI System Table.
///
/// @retval EFI_SUCCESS          The entry point is executed successfully.
/// @retval EFI_ALREADY_STARTED  The protocol has already been installed.
EFI_STATUS
EFIAPI
AppleBootPolicyMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  ) // start
{
  EFI_STATUS Status;

  VOID       *Interface;
  EFI_HANDLE Handle;

  AppleInitializeDriverLib (ImageHandle, SystemTable);

  Status = gBS->LocateProtocol (&gAppleBootPolicyProtocolGuid, NULL, &Interface);

  if (EFI_ERROR (Status)) {
    gBS->InstallProtocolInterface (
           &Handle,
           &gAppleBootPolicyProtocolGuid,
           EFI_NATIVE_INTERFACE,
           (VOID **)&mAppleBootPolicyProtocol
           );

    Status = EFI_SUCCESS;
  } else {
    Status = EFI_ALREADY_STARTED;
  }

  return Status;
}
