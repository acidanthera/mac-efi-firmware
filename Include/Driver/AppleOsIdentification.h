#ifndef __APPLE_OS_IDENTIFICATION_H__
#define __APPLE_OS_IDENTIFICATION_H__

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
	);

#endif // ifndef __APPLE_OS_IDENTIFICATION_H__
