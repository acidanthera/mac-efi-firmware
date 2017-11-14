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

EFI_GUID gAppleApfsVolumeGuid;
EFI_GUID gAppleApfsContainerGuid;

///
/// An array of file paths to search for in case no file is blessed.
///
STATIC CONST CHAR16 *mBootPathNames[] = {
  APPLE_BOOTER_DEFAULT_FILE_NAME,
  APPLE_REMOVABLE_MEDIA_FILE_NAME,
  EFI_REMOVABLE_MEDIA_FILE_NAME,
  APPLE_BOOTER_ROOT_FILE_NAME
};

EFI_STATUS
EFIAPI
BootPolicyGetBootFile (
  IN     EFI_HANDLE            Device,
  IN OUT FILEPATH_DEVICE_PATH  **FilePath
  );

///
/// The APPLE_BOOT_POLICY_PROTOCOL instance to get installed.
///
STATIC APPLE_BOOT_POLICY_PROTOCOL mAppleBootPolicyProtocol = {
  APPLE_BOOT_POLICY_PROTOCOL_REVISION,
  BootPolicyGetBootFile
};

/**
  Checks whether the given file exists or not.

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
        *FilePath = (FILEPATH_DEVICE_PATH *)DuplicateDevicePath (
                                              DevPath.DevPath
                                              );
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

// BUG: Memory leak. PathName is never used or deallocated.
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
        PathNameSize = StrSize (&FilePath->PathName[0]);
        PathName     = AllocatePool (PathNameSize);

        if (PathName != NULL) {
          StrCpy (PathName, &FilePath->PathName[0]);
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

STATIC
EFI_STATUS
InternalGetApfsVolumeInfo (
  IN  EFI_HANDLE  Device,
  OUT EFI_GUID    *Guid,
  OUT EFI_GUID    *Guid2,
  OUT UINT32      *Unknown
)
{
  EFI_STATUS                      Status;

  EFI_FILE_PROTOCOL               *Root;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
  VOID                            *FileInfo;

  Root = NULL;

  Status = gBS->HandleProtocol (
                  Device,
                  &gEfiSimpleFileSystemProtocolGuid,
                  (VOID **)&FileSystem
                  );

  if (!EFI_ERROR (Status)) {
    Status = FileSystem->OpenVolume (FileSystem, &Root);

    if (!EFI_ERROR (Status)) {
      Status = EFI_NOT_FOUND;

      FileInfo = InternalGetFileInfo (Root, &gAppleApfsContainerGuid);

      if (FileInfo != NULL) {
        CopyGuid (
          Guid,
          (EFI_GUID *)((UINTN)FileInfo + 4)
          );

        FreePool (FileInfo);
        
        Status = EFI_SUCCESS;
      }

      FileInfo = InternalGetFileInfo (Root, &gAppleApfsVolumeGuid);

      if (FileInfo != NULL) {
        CopyGuid (
          Guid2,
          (EFI_GUID *)((UINTN)FileInfo + 4)
          );

        *Unknown = (UINT32)((UINTN)FileInfo + 4 + sizeof (EFI_GUID));

        FreePool (FileInfo);
      }

      Root->Close (Root);
    }
  }

  return Status;
}

STATIC
EFI_STATUS
Internalsub_27DD (
  IN  EFI_HANDLE                      Device,
  IN  EFI_FILE_PROTOCOL               *Root,
  IN  EFI_GUID                        *Guid,
  IN  CHAR16                          *VolumeUuidString,
  OUT CONST EFI_DEVICE_PATH_PROTOCOL  **DevicePath,
  OUT EFI_HANDLE                      *VolumeHandle
  )
{
  EFI_STATUS                      Status;

  UINTN                           NumberOfHandles;
  EFI_HANDLE                      *HandleBuffer;
  UINTN                           Index;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
  EFI_FILE_PROTOCOL               *HandleRoot;
  VOID                            *ContainerInfo;
  BOOLEAN                         Result;
  VOID                            *VolumeInfo;
  CHAR16                          StartOfBuffer[38];
  EFI_FILE_PROTOCOL               *NewHandle;
  EFI_FILE_INFO                   *VolumeDirectoryInfo;
  FILEPATH_DEVICE_PATH            *FilePath;

  Status =  gBS->LocateHandleBuffer (
                   ByProtocol,
                   &gEfiSimpleFileSystemProtocolGuid,
                   NULL,
                   &NumberOfHandles,
                   &HandleBuffer
                   );

  if (!EFI_ERROR (Status)) {
    if (NumberOfHandles == 0) {
      Status = EFI_NOT_FOUND;
    } else {
      for (Index = 0; Index < NumberOfHandles; ++Index) {
        Status = gBS->HandleProtocol (
                        HandleBuffer[Index],
                        &gEfiSimpleFileSystemProtocolGuid,
                        (VOID **)&FileSystem
                        );

        if (!EFI_ERROR (Status)) {
          Status = FileSystem->OpenVolume (FileSystem, &HandleRoot);

          if (!EFI_ERROR (Status)) {
            ContainerInfo = InternalGetFileInfo (HandleRoot, &gAppleApfsContainerGuid);

            if (ContainerInfo != NULL) {
              Result = CompareGuid ((EFI_GUID *)((UINTN)ContainerInfo + 4), Guid);

              if (Result) {
                VolumeInfo = InternalGetFileInfo (HandleRoot, &gAppleApfsVolumeGuid);

                if (VolumeInfo != NULL) {
                  UnicodeSPrint (
                    &StartOfBuffer[0],
                    sizeof (StartOfBuffer),
                    L"%g", 
                    (EFI_GUID *)((UINTN)VolumeInfo + 4)
                    );

                  if (StrStr (VolumeUuidString, &StartOfBuffer[0]) != NULL) {
                    *VolumeHandle = HandleBuffer[Index];
                  }

                  Status = Root->Open (
                                   Root,
                                   &NewHandle,
                                   &StartOfBuffer[0],
                                   EFI_FILE_MODE_READ,
                                   0
                                   );

                  if (!EFI_ERROR (Status)) {
                    VolumeDirectoryInfo = InternalGetFileInfo (
                                            NewHandle,
                                            &gEfiFileInfoGuid
                                            );

                    if (VolumeDirectoryInfo != NULL) {
                      if ((VolumeDirectoryInfo->Attribute & EFI_FILE_DIRECTORY) != 0) {
                        FilePath = NULL;

                        Status = InternalAppendBootPathName (
                                   Device,
                                   NewHandle,
                                   &FilePath
                                   );

                        if (!EFI_ERROR (Status)) {
                          *DevicePath = AppendDevicePathInstance (
                                          *DevicePath,
                                          (CONST EFI_DEVICE_PATH_PROTOCOL *)FilePath
                                          );
                        }
                      }

                      FreePool (VolumeDirectoryInfo);
                    }
                  }
                }

                if (VolumeInfo != NULL) {
                  FreePool ((VOID *)VolumeInfo);
                }
              }

              FreePool (ContainerInfo);
            }

            HandleRoot->Close (HandleRoot);
          }
        }
      }

      FreePool (HandleBuffer);
    }
  }

  return Status;
}

/**
  Locates the bootable file of the given volume.  Prefered are the values
  blessed, though if unavailable, hard-coded names are being verified and used
  if existing.

  The blessed paths are to be determined by the HFS Driver via
  EFI_FILE_PROTOCOL.GetInfo().  The related identifier definitions are to be
  found in AppleBless.h.

  @param[in]  Device    The Device's Handle to perform the search on.
  @param[out] FilePath  A pointer to the device path pointer to set to the file
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
BootPolicyGetBootFile (
  IN     EFI_HANDLE            Device,
  IN OUT FILEPATH_DEVICE_PATH  **FilePath
  )
{
  EFI_STATUS                      Status;

  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
  EFI_FILE_PROTOCOL               *Root;

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

  return Status;
}

EFI_STATUS
EFIAPI
Policy2 (
  IN  EFI_HANDLE            Device,
      VOID                  *Unused,
  OUT FILEPATH_DEVICE_PATH  **FilePath
  )
{
  EFI_STATUS                      Status;

  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
  EFI_FILE_PROTOCOL               *Root;
  VOID                            *FileInfo;
  VOID                            *FileInfo2;

  *FilePath = NULL;

  Status = gBS->HandleProtocol (
                  Device,
                  &gEfiSimpleFileSystemProtocolGuid,
                  (VOID **)&FileSystem
                  );

  if (!EFI_ERROR (Status)) {
    Status = FileSystem->OpenVolume (FileSystem, &Root);

    if (!EFI_ERROR (Status)) {
      FileInfo = InternalGetFileInfo (Root, &gAppleApfsVolumeGuid);

      if (FileInfo != NULL) {
        FileInfo2 = InternalGetFileInfo (Root, &gAppleApfsContainerGuid);

        if (FileInfo2 != NULL) {
          Status = InternalGetBlessedSystemFilePath (Root, FilePath);

          if (EFI_ERROR (Status)) {
            Status = InternalGetFilePathName (Root);
          }

          Internalsub_27DD (
            Device,
            Root,
            (GUID *)((UINTN)FileInfo2 + 4),
            NULL,
            (CONST EFI_DEVICE_PATH_PROTOCOL **)FilePath,
            NULL
            );

          FreePool ((VOID *)FileInfo2);
        } else {
          Status = EFI_NOT_FOUND;
        }

        FreePool ((VOID *)FileInfo);

        Root->Close (Root);
      } else {
        // BUG: Root should be closed here too.

        Status = BootPolicyGetBootFile (Device, FilePath);
      }
    }
  }

  return Status;
}

EFI_STATUS
EFIAPI
Policy3 (
  IN  EFI_DEVICE_PATH_PROTOCOL  *DevicePath,
  OUT CHAR16                    **PathName,
  OUT EFI_HANDLE                *Device,
  OUT EFI_HANDLE                *Handle
  )
{
  EFI_STATUS Status;

  EFI_HANDLE lDevice;
  UINTN      Size;
  CHAR16       *Buffer;
  FILEPATH_DEVICE_PATH *FilePath;
  CHAR16 *Slash;
  UINTN Len;
  CHAR16 *BufferWalker;
  UINTN Index;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
  EFI_FILE_PROTOCOL *Root;
  VOID *FileInfo;

  *PathName = NULL;
  *Device   = NULL;
  *Handle   = NULL;

  Status = gBS->LocateDevicePath (
                  &gEfiSimpleFileSystemProtocolGuid,
                  &DevicePath,
                  &lDevice
                  );

  if (!EFI_ERROR (Status)) {
    *Device = lDevice;

    if ((DevicePathType (DevicePath) == MEDIA_DEVICE_PATH)
     && (DevicePathSubType (DevicePath) == MEDIA_FILEPATH_DP)) {
      FilePath = (FILEPATH_DEVICE_PATH *)DevicePath;

      Size = DevicePathNodeLength (DevicePath);

      Buffer = AllocateZeroPool ((Size - SIZE_OF_FILEPATH_DEVICE_PATH) + 1);

      Status = EFI_OUT_OF_RESOURCES;

      if (Buffer != NULL) {
        CopyMem (
          (VOID *)Buffer,
          (VOID *)FilePath->PathName,
          (Size - SIZE_OF_FILEPATH_DEVICE_PATH)
          );

        Slash = StrStr (FilePath->PathName, L"\\");

        if (Slash != NULL) {
          Len = StrLen (Buffer);

          if (Buffer[Len - 1] != L'\\') {
            BufferWalker = &Buffer[Len - 1];

            do {
              BufferWalker[0] = L'\0';
              --BufferWalker;

            } while (*BufferWalker != L'\\');
          }
        } else {
          StrCpy (Buffer, L"\\");
        }
      } else {
        return EFI_OUT_OF_RESOURCES;
      }
    } else {
      Size = StrSize (L"\\");

      Buffer = AllocateZeroPool (Size + 1);

      // BUG: Buffer should be checked.

      StrCpy (Buffer, L"\\");
    }

    BufferWalker = &Buffer[1];

    if (Buffer[0] != L'\\') {
      BufferWalker = Buffer;
    }

    // BUG: Check for NULL-terminator, not pointer?
    if (BufferWalker != NULL) {
      Len = StrLen (BufferWalker);

      if (Len >= 36) {
        for (Index = 0; Index < 36; ++Index) {
          if (
            ((Index == 8) || (Index == 12) || (Index == 16) || (Index == 20))
            ) {
            if (BufferWalker[Index] != '-') {
              goto Done;
            }
          } else {
            if (!(BufferWalker[Index] >= L'0') && (BufferWalker[Index] <= L'9')
             && !(BufferWalker[Index] >= L'A') && (BufferWalker[Index] <= L'F')
             ) {
              goto Done;
            }
          }
        }

        Status = gBS->HandleProtocol (
                        lDevice,
                        &gEfiSimpleFileSystemProtocolGuid,
                        (VOID **)&FileSystem
                        );

        if (!EFI_ERROR (Status)) {
          Status = FileSystem->OpenVolume (FileSystem, &Root);

          if (!EFI_ERROR (Status)) {
            FileInfo = InternalGetFileInfo (Root, &gAppleApfsContainerGuid);

            if (FileInfo != NULL) {
              FilePath = NULL;

              Internalsub_27DD (
                lDevice,
                Root,
                (GUID *)((UINTN)FileInfo + 4),
                BufferWalker,
                (CONST EFI_DEVICE_PATH_PROTOCOL **)&FilePath,
                Handle
                );

              if (FilePath != NULL) {
                FreePool (FilePath);
              }

              FreePool (FileInfo);
            }
          }
        }
      }
    }

  Done:
    *PathName = Buffer;
  }

  return Status;
}

// Policy4
EFI_STATUS
EFIAPI
Policy4 (

)
{
  
}

typedef struct {
  EFI_HANDLE Handle;
  EFI_GUID   DiskGuid;
  EFI_GUID   VolumeGuid;
  UINT32     Uint32;
} APFS_VOLUME_INFO;

typedef struct {
  EFI_HANDLE        Handle;
  CHAR16            *VolumeDirName;
  EFI_FILE_PROTOCOL *Root;
} APFS_VOLUME_ROOT;

// Policy5
EFI_STATUS
EFIAPI
Policy5 (
  IN  EFI_HANDLE  Handle,
  OUT VOID        **Sth,
  OUT UINTN       *NumberOfEntries
  )
{
  EFI_STATUS       Status;

  UINTN            NoHandles;
  EFI_HANDLE       *HandleBuffer;
  APFS_VOLUME_INFO *VolumeInfo;
  GUID             **DiskGuids;
  GUID             UnusedGuid; // BUG: This variable is never used.
  UINTN            Index;
  EFI_GUID         Guid1;
  EFI_GUID         Guid2;
  UINT32           Unknown;
  UINTN            Index2;
  UINTN            NumberOfGuids;
  UINTN            NumberOfBuffer1;
  APFS_VOLUME_ROOT **Memory;
  UINTN            Index3;
  UINTN            Index4;
  BOOLEAN          GuidPresent;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
  EFI_FILE_PROTOCOL *Root;
  EFI_FILE_PROTOCOL *NewHandle;
  CHAR16            String[80];
  EFI_FILE_INFO     *FileInfo;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiSimpleFileSystemProtocolGuid,
                  NULL,
                  &NoHandles,
                  &HandleBuffer
                  );

  if (!EFI_ERROR (Status)) {
    Status = EFI_NOT_FOUND;

    if (NoHandles > 0) {
      VolumeInfo = AllocateZeroPool (NoHandles * sizeof (*VolumeInfo));

      Status = EFI_OUT_OF_RESOURCES;

      if (VolumeInfo != NULL) {
        // BUG: The previous allocation is verified, this isn't.
        DiskGuids = AllocateZeroPool (NoHandles * sizeof (**DiskGuids));

        ZeroMem ((VOID *)&UnusedGuid, sizeof (UnusedGuid));

        NumberOfBuffer1 = 0;
        NumberOfGuids   = 0;

        Status = EFI_NOT_FOUND;

        for (Index = 0; Index < NoHandles; ++Index) {
          Status = InternalGetApfsVolumeInfo (
                     HandleBuffer[Index],
                     &Guid1,
                     &Guid2,
                     &Unknown
                     );

          if (!EFI_ERROR (Status)) {
            VolumeInfo[NumberOfBuffer1].Handle = HandleBuffer[Index];

            CopyGuid (
              &VolumeInfo[NumberOfBuffer1].DiskGuid,
              &Guid1
              );

            CopyGuid (
              &VolumeInfo[NumberOfBuffer1].VolumeGuid,
              &Guid2
              );

            VolumeInfo[NumberOfBuffer1].Uint32 = Unknown;

            GuidPresent = FALSE;

            for (Index2 = 0; Index2 < NumberOfGuids; ++Index2) {
              if (CompareGuid (DiskGuids[Index2], &Guid1)) {
                GuidPresent = TRUE;
                break;
              }
            }

            if (!GuidPresent) {
              CopyGuid (
                DiskGuids[NumberOfGuids],
                &VolumeInfo[NumberOfBuffer1].DiskGuid
                );

              // BUG: This may overwrite the GUID at [0]?
              if ((Index2 != 0) && (HandleBuffer[Index] == Handle)) {
                CopyGuid (DiskGuids[0], &VolumeInfo[NumberOfBuffer1].DiskGuid);
              }

              ++NumberOfGuids;
            }

            ++NumberOfBuffer1;
          }
        }

        Status = EFI_NOT_FOUND;

        if (NumberOfBuffer1 != 0) {
          Memory = AllocateZeroPool (NumberOfBuffer1 * 8 * 3);

          Status = EFI_OUT_OF_RESOURCES;

          if (Memory != NULL) {
            *Sth = Memory;
            *NumberOfEntries = 0;

            for (Index2 = 0; Index2 < NumberOfGuids; ++Index2) {
              for (Index3 = 0; Index3 < NumberOfBuffer1; ++Index3) {
                if (CompareGuid (DiskGuids[Index2], &VolumeInfo[Index3].DiskGuid)
                  && ((VolumeInfo[Index3].Uint32 & 0x04) != 0)
                  ) {
                  Status = gBS->HandleProtocol (
                                  VolumeInfo[Index3].Handle,
                                  &gEfiSimpleFileSystemProtocolGuid,
                                  (VOID **)&FileSystem
                                  );

                  if (!EFI_ERROR (Status)) {
                    Status = FileSystem->OpenVolume (FileSystem, &Root);
                    
                    if (!EFI_ERROR (Status)) {
                      for (Index4 = 0; Index4 < NumberOfBuffer1; ++Index4) {
                        if (
                          CompareGuid (DiskGuids[Index2], &VolumeInfo[Index4].DiskGuid)
                          ) {
                          UnicodeSPrint (
                            &String[0],
                            sizeof (String),
                            L"%g",
                            &VolumeInfo[Index4].VolumeGuid
                            );

                          Status = Root->Open (
                                            Root,
                                            &NewHandle,
                                            String,
                                            EFI_FILE_MODE_READ,
                                            0
                                            );

                          if (!EFI_ERROR (Status)) {
                            FileInfo = InternalGetFileInfo (
                                          NewHandle,
                                          &gEfiFileInfoGuid
                                          );

                            if ((FileInfo != NULL)
                              && ((FileInfo->Attribute & EFI_FILE_DIRECTORY) != 0)
                              ) {
                              Memory[*NumberOfEntries]->Handle = VolumeInfo[Index3].Handle;

                              Memory[*NumberOfEntries]->VolumeDirName = AllocateCopyPool (
                                                               StrSize (String),
                                                               (VOID *)&String[0]
                                                               );

                              Memory[*NumberOfEntries]->Root = Root;

                              ++(*NumberOfEntries);
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }

            FreePool (VolumeInfo);
            FreePool (DiskGuids);
            FreePool (HandleBuffer);
          }
        }
      }
    }
  }

  return Status;
}

/**
  The Entry Point installing the APPLE_BOOT_POLICY_PROTOCOL.

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
  } else {
    Status = EFI_ALREADY_STARTED;
  }

  return Status;
}
