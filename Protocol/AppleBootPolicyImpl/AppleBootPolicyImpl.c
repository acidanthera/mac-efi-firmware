///
/// @file      ProtocolImpl/AppleBootPolicy/AppleBootPolicy.c
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

#include <AppleEfi.h>
#include <EfiDriverLib.h>
#include <EfiDebug.h>

#include <Guid/AppleBless.h>

#include EFI_PROTOCOL_CONSUMER (SimpleFileSystem)
#include <Protocol/AppleBootPolicy.h>
#include <Protocol/AppleBootPolicyImpl.h>

// mBootFilePaths
/// An array of file paths to search for in case no file is blessed.
static CHAR16 *mBootFilePaths[4] = {
  APPLE_BOOTER_FILE_PATH,
  APPLE_REMOVABLE_MEDIA_FILE_NAME,
  EFI_REMOVABLE_MEDIA_FILE_NAME,
  APPLE_BOOTER_FILE_NAME
};

// AppleBootPolicyFileExists
/// Checks whether the given file exists or not.
///
/// @param[in] Root     The volume's opened root.
/// @param[in] FileName The path of the file to check.
///
/// @return Returned is whether the specified file exists or not.
BOOLEAN
AppleBootPolicyFileExists (
  IN EFI_FILE_HANDLE  Root,
  IN CHAR16           *FileName
  )
{
  BOOLEAN         Exists;

  EFI_STATUS      Status;
  EFI_FILE_HANDLE FileHandle;

  ASSERT (Root != NULL);
  ASSERT (FileName != NULL);

  Status = Root->Open (Root, &FileHandle, FileName, EFI_FILE_MODE_READ, 0);

  if (!EFI_ERROR (Status)) {
    FileHandle->Close (FileHandle);

    Exists = TRUE;
  } else {
    Exists = FALSE;
  }

  return Exists;
}

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
        FilePath.DevPath = (EFI_DEVICE_PATH_PROTOCOL *)EfiLibAllocateZeroPool (Size);

        if (FilePath.DevPath != NULL) {
          Status = Root->GetInfo (Root, &gAppleBlessedFileInfoId, &Size, FilePath.DevPath);

          if (!EFI_ERROR (Status)) {
            *BootFilePath = (FILEPATH_DEVICE_PATH *)EfiDuplicateDevicePath (FilePath.DevPath);

            gBS->FreePool ((VOID *)FilePath.DevPath);
            goto CloseRoot;
          }

          gBS->FreePool ((VOID *)FilePath.DevPath);
        }
      }

      Size   = 0;
      Status = Root->GetInfo (Root, &gAppleBlessedFolderInfoId, &Size, NULL);

      if (Status == EFI_BUFFER_TOO_SMALL) {
        FilePath.DevPath = (EFI_DEVICE_PATH_PROTOCOL *)EfiLibAllocateZeroPool (Size);

        if (FilePath.DevPath != NULL) {
          Status = Root->GetInfo (Root, &gAppleBlessedFolderInfoId, &Size, FilePath.DevPath);
          Path   = NULL;

          if (!EFI_ERROR (Status)) {
            while (!IsDevicePathEnd (FilePath.DevPath)) {
              if ((DevicePathType (FilePath.DevPath) == MEDIA_DEVICE_PATH)
               && (DevicePathSubType (FilePath.DevPath) == MEDIA_FILEPATH_DP)) {
                Size = EfiStrSize (FilePath.FilePath->PathName);
                Path = (CHAR16 *)EfiLibAllocatePool (Size);

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
            Size     = (EfiStrSize (Path) + EfiStrSize (mBootFilePaths[3]) - sizeof (*Path));
            FullPath = (CHAR16 *)EfiLibAllocateZeroPool (Size);

            if (FullPath != NULL) {
              EfiStrCpy (FullPath, Path);
              EfiStrCat (FullPath, mBootFilePaths[3]);

              if (AppleBootPolicyFileExists (Root, FullPath)) {
                *BootFilePath  = (FILEPATH_DEVICE_PATH *)EfiFileDevicePath (Device, FullPath);

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

        if (AppleBootPolicyFileExists (Root, Path)) {
          *BootFilePath = (FILEPATH_DEVICE_PATH *)EfiFileDevicePath (Device, Path);
          Status        = EFI_SUCCESS;

          break;
        }
      }
    }
  }

CloseRoot:
  if (Root != NULL) {
    Root->Close (Root);
  }

Return:
  return Status;
}