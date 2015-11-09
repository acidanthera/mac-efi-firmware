///
/// @file      Include/Protocol/AppleBootPolicyImpl.h
///
///            Apple protocol to get a volume's bootable file.
///
/// @author    Download-Fritz
/// @date      19/12/2014: Initial version
/// @date      23/02/2015: Minor tweaks
/// @date      15/03/2015: Updated documentation and restructuring
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

#ifndef _APPLE_BOOT_POLICY_IMPL_H_
#define _APPLE_BOOT_POLICY_IMPL_H_

// APPLE_BOOT_POLICY_PROTOCOL_REVISION
#define APPLE_BOOT_POLICY_PROTOCOL_REVISION  0x01

// AppleBootPolicyGetBootFileImpl
/// Locates the bootable file of the given volume. Prefered are the values blessed,
/// though if unavailable, hard-coded names are being verified and used if existing.
///
/// The blessed paths are to be determined by the HFS Driver via EFI_FILE_PROTOCOL.GetInfo().
/// The related identifier definitions are to be found in AppleBless.h.
///
/// @param[in]  Device       The Device's Handle to perform the search on.
/// @param[out] BootFilePath A pointer to the device path pointer to set to the file path of the boot file.
///
/// @return                      The status of the operation is returned.
/// @retval EFI_NOT_FOUND        A bootable file could not be found on the given volume.
/// @retval EFI_OUT_OF_RESOURCES The memory necessary to complete the operation could not be allocated.
/// @retval EFI_SUCCESS          The operation completed successfully and the BootFilePath buffer has been filled.
/// @retval other                The status of an operation used to complete this operation is returned.
EFI_STATUS
EFIAPI
AppleBootPolicyGetBootFileImpl (
	IN  EFI_HANDLE            Device,
	OUT FILEPATH_DEVICE_PATH  **BootFilePath
	);

#endif // ifndef _APPLE_BOOT_POLICY_IMPL_H_
