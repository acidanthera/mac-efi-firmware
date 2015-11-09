///
/// @file      Driver/AppleBootPolicy.c
///
///            Apple's driver to get a volume's bootable file.
///
/// @author    Download-Fritz
/// @date      19/12/2014: Initial version
/// @date      23/02/2015: Minor tweaks
/// @date      15/03/2015: Updated documentation
/// @copyright The decompilation is of an educational purpose to better understand the behavior of the
///            Apple EFI implementation and making use of it. In no way is the content's usage licensed
///            or allowed. All rights remain at Apple Inc. To be used under the terms of 'Fair use'.
///

//
// CREDITS:
//   Reversed from AppleBootPolicy.efi, which is Apple Inc. property
//   Relies on HFSPlus.efi which is Apple Inc. property as well
//   Decompiled by Download-Fritz
//

#include <AppleEfi.h>
#include <EfiDriverLib.h>
#include <EfiCommon.h>

#include <Protocol/AppleBootPolicy.h>
#include <Protocol/AppleBootPolicyImpl.h>

#include <Driver/AppleBootPolicy.h>

// mAppleBootPolicyProtocol
/// The APPLE_BOOT_POLICY_PROTOCOL instance to get installed.
static APPLE_BOOT_POLICY_PROTOCOL mAppleBootPolicyProtocol = {
  APPLE_BOOT_POLICY_PROTOCOL_REVISION,
  AppleBootPolicyGetBootFileImpl
};

// AppleBootPolicyMain
/// The Entry Point installing the APPLE_BOOT_POLICY_PROTOCOL.
///
/// @param[in] ImageHandle The firmware allocated handle for the EFI image.  
/// @param[in] SystemTable A pointer to the EFI System Table.
///
/// @retval EFI_SUCCESS         The entry point is executed successfully.
/// @retval EFI_ALREADY_STARTED The protocol has already been installed.
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

  EfiInitializeDriverLib (ImageHandle, SystemTable);

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