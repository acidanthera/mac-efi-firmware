/** @file
  Copyright (c) 2005 - 2017, Apple Inc.  All rights reserved.<BR>

  This program and the accompanying materials have not been licensed.
  Neither is its usage, its redistribution, in source or binary form,
  licensed, nor implicitely or explicitely permitted, except when
  required by applicable law.

  Unless required by applicable law or agreed to in writing, software
  distributed is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
  OR CONDITIONS OF ANY KIND, either express or implied.
**/

#include <AppleMacEfi.h>

#include <Guid/AppleBless.h>
#include <Guid/FileInfo.h>

#include <Protocol/AppleBootPolicy.h>
#include <Protocol/SimpleFileSystem.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>

// APPLE_BOOT_POLICY_PROTOCOL_REVISION
#define APPLE_BOOT_POLICY_PROTOCOL_REVISION  0x01

// mBootPathNames
/// An array of file paths to search for in case no file is blessed.
STATIC CONST CHAR16 *mBootPathNames[] = {
  APPLE_BOOTER_DEFAULT_FILE_NAME,
  APPLE_REMOVABLE_MEDIA_FILE_NAME,
  EFI_REMOVABLE_MEDIA_FILE_NAME,
  APPLE_BOOTER_ROOT_FILE_NAME
};

EFI_STATUS
EFIAPI
BootPolicyGetBootFileImpl (
  IN     EFI_HANDLE            Device,
  IN OUT FILEPATH_DEVICE_PATH  **FilePath
  );

// mAppleBootPolicyProtocol
/// The APPLE_BOOT_POLICY_PROTOCOL instance to get installed.
STATIC APPLE_BOOT_POLICY_PROTOCOL mAppleBootPolicyProtocol = {
  APPLE_BOOT_POLICY_PROTOCOL_REVISION,
  BootPolicyGetBootFileImpl
};

// InternalFileExists
/** Checks whether the given file exists or not.

  @param[in] Root      The volume's opened root.
  @param[in] FileName  The path of the file to check.

  @return  Returned is whether the specified file exists or not.
**/
STATIC
BOOLEAN
InternalFileExists (
  IN EFI_FILE_HANDLE  Root,
  IN CHAR16           *FileName
  )
{
  BOOLEAN         Exists;

  EFI_STATUS      Status;
  EFI_FILE_HANDLE FileHandle;

  ASSERT (Root != NULL);
  ASSERT (FileName != NULL);

  Status = Root->Open (
                   Root,
                   &FileHandle,
                   FileName,
                   EFI_FILE_MODE_READ,
                   0
                   );

  if (!EFI_ERROR (Status)) {
    FileHandle->Close (FileHandle);

    Exists = TRUE;
  } else {
    Exists = FALSE;
  }

  return Exists;
}

// InternalGetBlessedSystemFilePath
STATIC
EFI_STATUS
InternalGetBlessedSystemFilePath (
  IN  EFI_FILE_PROTOCOL     *Root,
  OUT FILEPATH_DEVICE_PATH  **FilePath
  )
{
  EFI_STATUS       Status;

  UINTN            Size;
  EFI_DEV_PATH_PTR DevPath;
  
  Size   = 0;
  Status = Root->GetInfo (
                   Root,
                   &gAppleBlessedSystemFileInfoGuid,
                   &Size,
                   NULL
                   );

  if (Status == EFI_BUFFER_TOO_SMALL) {
    DevPath.FilePath = AllocateZeroPool (Size);

    Status = EFI_OUT_OF_RESOURCES;

    if (DevPath.FilePath != NULL) {
      Status = Root->GetInfo (
                       Root,
                       &gAppleBlessedSystemFileInfoGuid,
                       &Size,
                       DevPath.FilePath
                       );

      if (!EFI_ERROR (Status)) {
        *FilePath = (FILEPATH_DEVICE_PATH *)DuplicateDevicePath (DevPath.DevPath);
      }

      gBS->FreePool ((VOID *)DevPath.DevPath);
    }
  } else {
    Status = EFI_NOT_FOUND;
  }

  return Status;
}

// InternalGetFileInfo
STATIC
VOID *
InternalGetFileInfo (
  IN EFI_FILE_PROTOCOL  *Root,
  IN EFI_GUID           *InformationType
  )
{
  VOID       *FileInfoBuffer;

  UINTN      FileInfoSize;
  EFI_STATUS Status;

  FileInfoSize   = 0;
  FileInfoBuffer = NULL;

  Status = Root->GetInfo (
                   Root,
                   InformationType,
                   &FileInfoSize,
                   NULL
                   );

  if (Status == EFI_BUFFER_TOO_SMALL) {
    FileInfoBuffer = AllocateZeroPool (FileInfoSize);

    if (FileInfoBuffer != NULL) {
      Status = Root->GetInfo (
                       Root,
                       InformationType,
                       &FileInfoSize,
                       FileInfoBuffer
                       );

      if (EFI_ERROR (Status)) {
        FreePool (FileInfoBuffer);

        FileInfoBuffer = NULL;
      }
    }
  }

  return FileInfoBuffer;
}

// InternalGetFilePathName
// NOTE: Memory leak. PathName is never used or deallocated.
STATIC
EFI_STATUS
InternalGetFilePathName (
  IN EFI_FILE_PROTOCOL  *Root
  )
{
  EFI_STATUS           Status;

  CONST VOID           *DevPathNode;
  FILEPATH_DEVICE_PATH *FilePath;
  UINTN                PathNameSize;
  CHAR16               *PathName;

  Status = EFI_NOT_FOUND;

  DevPathNode = InternalGetFileInfo (
                  Root,
                  &gAppleBlessedSystemFolderInfoGuid
                  );

  if (DevPathNode != NULL) {
    while (!IsDevicePathEnd (DevPathNode)) {
      if ((DevicePathType (DevPathNode) == MEDIA_DEVICE_PATH)
       && (DevicePathSubType (DevPathNode) == MEDIA_FILEPATH_DP)) {
        FilePath     = (FILEPATH_DEVICE_PATH *)DevPathNode;
        PathNameSize = StrSize (FilePath->PathName);
        PathName     = AllocatePool (PathNameSize);

        if (PathName != NULL) {
          StrCpy (PathName, FilePath->PathName);
        } else {
          Status = EFI_OUT_OF_RESOURCES;
        }

        break;
      }

      DevPathNode = NextDevicePathNode (DevPathNode);
    }

    gBS->FreePool ((VOID *)DevPathNode);
  }

  return Status;
}

// InternalAppendBootPathName
STATIC
EFI_STATUS
InternalAppendBootPathName (
  IN  EFI_HANDLE            Device,
  IN  EFI_FILE_PROTOCOL     *Root,
  OUT FILEPATH_DEVICE_PATH  **FilePath
  )
{
  EFI_STATUS   Status;

  UINTN        Index;
  CONST CHAR16 *PathName;

  Status = EFI_NOT_FOUND;

  for (Index = 0; Index < ARRAY_SIZE (mBootPathNames); ++Index) {
    PathName = mBootPathNames[Index];

    if (InternalFileExists (Root, (CHAR16 *)PathName)) {
      *FilePath = (FILEPATH_DEVICE_PATH *)FileDevicePath (Device, PathName);

      Status = EFI_SUCCESS;

      break;
    }
  }

  return Status;
}

// BootPolicyGetBootFileImpl
/** Locates the bootable file of the given volume.  Prefered are the values
    blessed, though if unavailable, hard-coded names are being verified and
    used if existing.

  The blessed paths are to be determined by the HFS Driver via
  EFI_FILE_PROTOCOL.GetInfo().  The related identifier definitions are to be
  found in AppleBless.h.

  @param[in]  Device    The Device's Handle to perform the search on.
  @param[out] PathName  A pointer to the device path pointer to set to the file
                        path of the boot file.

  @return                       The status of the operation is returned.
  @retval EFI_NOT_FOUND         A bootable file could not be found on the given
                                volume.
  @retval EFI_OUT_OF_RESOURCES  The memory necessary to complete the operation
                                could not be allocated.
  @retval EFI_SUCCESS           The operation completed successfully and the
                                PathName Buffer has been filled.
  @retval other                 The status of an operation used to complete
                                this operation is returned.
**/
EFI_STATUS
EFIAPI
BootPolicyGetBootFileImpl (
  IN     EFI_HANDLE            Device,
  IN OUT FILEPATH_DEVICE_PATH  **FilePath
  )
{
  EFI_STATUS                      Status;

  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
  EFI_FILE_PROTOCOL               *Root;

  ASSERT (Device != NULL);
  ASSERT (FilePath != NULL);

  Root = NULL;

  Status = gBS->HandleProtocol (
                  Device,
                  &gEfiSimpleFileSystemProtocolGuid,
                  (VOID **)&FileSystem
                  );

  if (!EFI_ERROR (Status)) {
    Status = FileSystem->OpenVolume (FileSystem, &Root);

    if (!EFI_ERROR (Status)) {
      Status = InternalGetBlessedSystemFilePath (Root, FilePath);

      if (EFI_ERROR (Status)) {
        Status = InternalGetFilePathName (Root);

        if (EFI_ERROR (Status)) {
          Status = InternalAppendBootPathName (Device, Root, FilePath);
        }
      }

      // BUG: Root should be closed here, it cannot be open before.
    }
  }

  if (Root != NULL) {
    Root->Close (Root);
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}

// AppleBootPolicyMain
/** The Entry Point installing the APPLE_BOOT_POLICY_PROTOCOL.

  @param[in] ImageHandle  The firmware allocated handle for the EFI image.
  @param[in] SystemTable  A pointer to the EFI System Table.

  @retval EFI_SUCCESS          The entry point is executed successfully.
  @retval EFI_ALREADY_STARTED  The protocol has already been installed.
**/
EFI_STATUS
EFIAPI
AppleBootPolicyMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS Status;

  VOID       *Interface;
  EFI_HANDLE Handle;

  ASSERT_PROTOCOL_ALREADY_INSTALLED (NULL, &gAppleBootPolicyProtocolGuid);

  Status = gBS->LocateProtocol (
                  &gAppleBootPolicyProtocolGuid,
                  NULL,
                  &Interface
                  );

  if (EFI_ERROR (Status)) {
    Status = gBS->InstallProtocolInterface (
                    &Handle,
                    &gAppleBootPolicyProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    (VOID **)&mAppleBootPolicyProtocol
                    );

    ASSERT_EFI_ERROR (Status);
  } else {
    Status = EFI_ALREADY_STARTED;
  }

  return Status;
}
