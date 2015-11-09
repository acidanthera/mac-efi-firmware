// 18/07/2015

#include <AppleEfi.h>
#include <EfiDriverLib.h>

#include <Protocol/OsIdentification.h>
#include <Protocol/OsIdentificationImpl.h>

#include <Driver/AppleOsIdentification.h>

// mAppleOsIdentification
static EFI_OS_IDENTIFICATION_PROTOCOL mAppleOsIdentification = {
  OS_IDENTIFICATION_PROTOCOL_REVISION,
  AppleOsIdentificationOSName,
  AppleOsIdentificationOSVendor
};

// AppleOsIdentificationMain
/// 
///
/// @param[in] ImageHandle The firmware allocated handle for the EFI image.  
/// @param[in] SystemTable A pointer to the EFI System Table.
///
/// @retval EFI_SUCCESS         The entry point is executed successfully.
/// @retval EFI_ALREADY_STARTED The protocol has already been installed.
EFI_STATUS
EFIAPI
AppleOsIdentificationMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  ) // start
{
  EfiInitializeDriverLib (ImageHandle, SystemTable);

  return gBS->InstallProtocolInterface (
                ImageHandle,
                &gEfiOsIdentificationProtocolGuid,
                EFI_NATIVE_INTERFACE,
                (VOID *)&mAppleOsIdentification
                );
}
