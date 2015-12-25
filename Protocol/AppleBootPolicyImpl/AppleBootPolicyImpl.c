/** @file
  Apple protocol to get a volume's bootable file.

  Copyright (C) 2005 - 2015 Apple Inc.  All rights reserved.<BR>

  This program and the accompanying materials have not been licensed.
  Neither is its usage, its redistribution, in source or binary form,
  licensed, nor implicitely or explicitely permitted, except when
  required by applicable law.

  Unless required by applicable law or agreed to in writing, software
  distributed is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
  OR CONDITIONS OF ANY KIND, either express or implied.
**/

#include <AppleEfi.h>

#include <Library/AppleDriverLib.h>

#include <Guid/AppleBless.h>

#include EFI_PROTOCOL_CONSUMER (SimpleFileSystem)

#include "AppleBootPolicyImplInternal.h"

// mBootFilePaths
/// An array of file paths to search for in case no file is blessed.
STATIC CHAR16 *mBootFilePaths[4] = {
  APPLE_BOOTER_FILE_PATH,
  APPLE_REMOVABLE_MEDIA_FILE_NAME,
  EFI_REMOVABLE_MEDIA_FILE_NAME,
  APPLE_BOOTER_FILE_NAME
};

// BootPolicyGetBootFileImpl
/** Locates the bootable file of the given volume.  Prefered are the values blessed,
    though if unavailable, hard-coded names are being verified and used if existing.

  The blessed paths are to be determined by the HFS Driver via EFI_FILE_PROTOCOL.GetInfo().
  The related identifier definitions are to be found in AppleBless.h.

  @param[in]  Device        The Device's Handle to perform the search on.
  @param[out] BootFilePath  A pointer to the device path pointer to set to the file path of the boot file.

  @return                       The status of the operation is returned.
  @retval EFI_NOT_FOUND         A bootable file could not be found on the given volume.
  @retval EFI_OUT_OF_RESOURCES  The memory necessary to complete the operation could not be allocated.
  @retval EFI_SUCCESS           The operation completed successfully and the BootFilePath Buffer has been filled.
  @retval other                 The status of an operation used to complete this operation is returned.
**/
EFI_STATUS
EFIAPI
BootPolicyGetBootFileImpl (
  IN  EFI_HANDLE            Device,
  OUT FILEPATH_DEVICE_PATH  **BootFilePath
  )
{
  EFI_STATUS                      Status;

  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
  EFI_FILE_PROTOCOL               *Root;
  UINTN                           Size;
  EFI_DEV_PATH_PTR                FilePath;
  CHAR16                          *Path;
  CHAR16                          *FullPath;
  UINT8                           Index;

  ASSERT (Device != NULL);
  ASSERT (BootFilePath != NULL);

  Status = gBS->HandleProtocol (Device, &gEfiSimpleFileSystemProtocolGuid, (VOID **)&FileSystem);
  Root   = NULL;

  if (!EFI_ERROR (Status)) {
    Status = FileSystem->OpenVolume (FileSystem, &Root);

    if (!EFI_ERROR (Status)) {
      Size   = 0;
      Status = Root->GetInfo (Root, &gAppleBlessedFileInfoId, &Size, NULL);

      if (Status == EFI_BUFFER_TOO_SMALL) {
        FilePath.DevPath = EfiLibAllocateZeroPool (Size);

        if (FilePath.DevPath != NULL) {
          Status = Root->GetInfo (Root, &gAppleBlessedFileInfoId, &Size, FilePath.DevPath);

          if (!EFI_ERROR (Status)) {
            *BootFilePath = (FILEPATH_DEVICE_PATH *)EfiDuplicateDevicePath (FilePath.DevPath);

            gBS->FreePool ((VOID *)FilePath.DevPath);
            goto Done;
          }

          gBS->FreePool ((VOID *)FilePath.DevPath);
        }
      }

      Size   = 0;
      Status = Root->GetInfo (Root, &gAppleBlessedFolderInfoId, &Size, NULL);

      if (Status == EFI_BUFFER_TOO_SMALL) {
        FilePath.DevPath = EfiLibAllocateZeroPool (Size);

        if (FilePath.DevPath != NULL) {
          Status = Root->GetInfo (Root, &gAppleBlessedFolderInfoId, &Size, FilePath.DevPath);
          Path   = NULL;

          if (!EFI_ERROR (Status)) {
            while (!IsDevicePathEnd (FilePath.DevPath)) {
              if ((DevicePathType (FilePath.DevPath) == MEDIA_DEVICE_PATH)
               && (DevicePathSubType (FilePath.DevPath) == MEDIA_FILEPATH_DP)) {
                Size = EfiStrSize (FilePath.FilePath->PathName);
                Path = EfiLibAllocatePool (Size);

                if (Path != NULL) {
                  EfiStrCpy (Path, FilePath.FilePath->PathName);
                } else {
                  Status = EFI_OUT_OF_RESOURCES;
                }

                break;
              }

              FilePath.DevPath = NextDevicePathNode (FilePath.DevPath);
            }
          }

          gBS->FreePool ((VOID *)FilePath.DevPath);

          if (!EFI_ERROR (Status)) {
            Size     = (EfiStrSize (Path) + EfiStrSize (APPLE_BOOTER_FILE_NAME) - sizeof (*Path));
            FullPath = EfiLibAllocateZeroPool (Size);

            if (FullPath != NULL) {
              EfiStrCpy (FullPath, Path);
              EfiStrCat (FullPath, APPLE_BOOTER_FILE_NAME);

              if (BootPolicyFileExists (Root, FullPath)) {
                *BootFilePath = (FILEPATH_DEVICE_PATH *)EfiFileDevicePath (Device, FullPath);

                gBS->FreePool ((VOID *)FullPath);
                gBS->FreePool ((VOID *)Path);

                Status = EFI_SUCCESS;
                goto Return;
              }

              gBS->FreePool ((VOID *)FullPath);
            }

            gBS->FreePool ((VOID *)Path);
          }
        }
      }

      Status = EFI_NOT_FOUND;

      for (Index = 0; Index < ARRAY_LENGTH (mBootFilePaths); ++Index) {
        Path = mBootFilePaths[Index];

        if (BootPolicyFileExists (Root, Path)) {
          *BootFilePath = (FILEPATH_DEVICE_PATH *)EfiFileDevicePath (Device, Path);
          Status        = EFI_SUCCESS;

          break;
        }
      }
    }
  }

Done:
  if (Root != NULL) {
    Root->Close (Root);
  }

Return:
  ASSERT_EFI_ERROR (Status);

  return Status;
}
