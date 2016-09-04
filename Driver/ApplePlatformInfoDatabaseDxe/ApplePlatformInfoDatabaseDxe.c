/** @file
  Copyright (C) 2005 - 2015, Apple Inc.  All rights reserved.<BR>

  This program and the accompanying materials have not been licensed.
  Neither is its usage, its redistribution, in source or binary form,
  licensed, nor implicitely or explicitely permitted, except when
  required by applicable law.

  Unless required by applicable law or agreed to in writing, software
  distributed is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
  OR CONDITIONS OF ANY KIND, either express or implied.
**/

#include <AppleEfi.h>

#include EFI_GUID_DEFINITION (Hob)
#include APPLE_GUID_DEFINITION (AppleFile)
#include APPLE_GUID_DEFINITION (AppleHob)

#include APPLE_PROTOCOL_PRODUCER (ApplePlatformInfoDatabaseImpl)

#include <Library/AppleDriverLib.h>
#include <EfiHobLib.h>

#include <Driver/ApplePlatformInfoDatabaseDxe.h>

// mApplePlatformInfoDatabaseDxe
STATIC APPLE_PLATFORM_INFO_DATABASE_PROTOCOL mApplePlatformInfoDatabase = {
  APPLE_PLATFORM_INFO_DATABASE_PROTOCOL_REVISION,
  PlatformInfoDbGetFirstDataImpl,
  PlatformInfoDbGetFirstDataSizeImpl,
  PlatformInfoDbGetDataImpl,
  PlatformInfoDbGetDataSizeImpl
};

EFI_DRIVER_ENTRY_POINT (ApplePlatformInfoDatabaseDxeMain);

// ApplePlatformInfoDatabaseDxeMain
/**

  @param[in] ImageHandle  The firmware allocated handle for the EFI image.
  @param[in] SystemTable  A pointer to the EFI System Table.

  @retval EFI_SUCCESS          The entry point is executed successfully.
  @retval EFI_ALREADY_STARTED  The protocol has already been installed.
**/
EFI_STATUS
EFIAPI
ApplePlatformInfoDatabaseDxeMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  ) // start
{
  EFI_STATUS                   Status;

  VOID                         *HobListTable;
  VOID                         *HobListTable2;
  VOID                         *Buffer;
  UINTN                        BufferSize;
  UINTN                        NumberHandles;
  EFI_HANDLE                   *HandleBuffer;
  EFI_FIRMWARE_VOLUME_PROTOCOL *FirmwareVolume;
  EFI_DEVICE_PATH_PROTOCOL     *DevicePath;
  UINTN                        Index;
  BOOLEAN                      FirmwareVolumeFound;
  UINTN                        FileBufferSize;
  EFI_FV_FILETYPE              FoundType;
  EFI_FV_FILE_ATTRIBUTES       FileAttributes;
  UINT32                       AuthenticationStatus;
  APPLE_PLATFORM_INFO_DATABASE *PlatformInfoDatabase;

  AppleInitializeDriverLib (ImageHandle, SystemTable);

  ASSERT_PROTOCOL_ALREADY_INSTALLED (
    NULL,
    &gApplePlatformInfoDatabaseProtocolGuid
    );

  FirmwareVolume = NULL; /////
  HobListTable2  = NULL; /////////
  HobListTable   = NULL;
  Buffer         = NULL;
  Status         = EfiLibGetSystemConfigurationTable (
                     &gEfiHobListGuid,
                     &HobListTable
                     );

  if (!EFI_ERROR (Status)) {
    Status = GetNextGuidHob (&HobListTable, &gAppleHob1Guid, &Buffer, &BufferSize);

    if (EFI_ERROR (Status)) {
      Buffer = NULL;
    }
  }

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiFirmwareVolumeProtocolGuid,
                  NULL,
                  &NumberHandles,
                  &HandleBuffer
                  );

  if (!EFI_ERROR (Status)) {
    FirmwareVolume = NULL;
    DevicePath     = NULL;

    if (Buffer != NULL) {
      FirmwareVolumeFound = TRUE;

      for (Index = 0; Index < NumberHandles; ++Index) {
        if (FirmwareVolumeFound) {

        }

        Status = gBS->HandleProtocol (
                        HandleBuffer[Index],
                        &gEfiDevicePathProtocolGuid,
                        (VOID **)&DevicePath
                        );

        if (!EFI_ERROR (Status)) {
          while (!IsDevicePathEnd (DevicePath)) {
            if ((DevicePathType (DevicePath) == HARDWARE_DEVICE_PATH)
             && (DevicePathSubType (DevicePath) == HW_MEMMAP_DP)
             && (((MEMMAP_DEVICE_PATH *)DevicePath)->StartingAddress == (EFI_PHYSICAL_ADDRESS)(UINTN)Buffer)) {
              Status = gBS->HandleProtocol (
                              HandleBuffer[Index],
                              &gEfiFirmwareVolumeProtocolGuid,
                              (VOID **)&FirmwareVolume
                              );

              if (EFI_ERROR (Status)) {
                goto BreakBoth;
              }
            }

            DevicePath = NextDevicePathNode (DevicePath);
          }
        }

        FirmwareVolumeFound = (BOOLEAN)(FirmwareVolume == NULL);
      }
    }

  BreakBoth:
    for (Index = 0; Index < NumberHandles; ++Index) {
      Status = gBS->HandleProtocol (
                      HandleBuffer[Index],
                      &gEfiFirmwareVolumeProtocolGuid,
                      (VOID **)&FirmwareVolume
                      );

      if (!EFI_ERROR (Status)) {
        Status = FirmwareVolume->ReadFile (
                                   FirmwareVolume,
                                   &gAppleFile1Guid,
                                   NULL,
                                   &FileBufferSize,
                                   &FoundType,
                                   &FileAttributes,
                                   &AuthenticationStatus
                                   );

        if (!EFI_ERROR (Status)) {
          break;
        }
      }
    }

    gBS->FreePool ((VOID *)HandleBuffer);

    if (!EFI_ERROR (Status)) {
      EfiLibGetSystemConfigurationTable (&gEfiHobListGuid, &HobListTable);
    }

    return Status;
  }

  ///

  gBS->FreePool ((VOID *)HandleBuffer);

  if (!EFI_ERROR (Status)) {
    Status = EfiLibGetSystemConfigurationTable (
               &gEfiHobListGuid,
               HobListTable2
               );

    if (!EFI_ERROR (Status)) {

    }
  }

  PlatformInfoDatabase = EfiLibAllocatePool (sizeof (*PlatformInfoDatabase));
  Status               = EFI_OUT_OF_RESOURCES;

  if (PlatformInfoDatabase != NULL) {
    PlatformInfoDatabase->Signature            = APPLE_PLATFORM_INFO_DATABASE_SIGNATURE;
    PlatformInfoDatabase->FirmwareVolumeHandle = NULL; ////
    PlatformInfoDatabase->FirmwareVolume       = FirmwareVolume;

    EfiCopyMem (
      (VOID *)&PlatformInfoDatabase->Protocol,
      (VOID *)&mApplePlatformInfoDatabase,
      sizeof (mApplePlatformInfoDatabase)
      );
    
    Status = gBS->InstallProtocolInterface (
                    PlatformInfoDatabase->FirmwareVolumeHandle,
                    &gApplePlatformInfoDatabaseProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    (VOID *)&PlatformInfoDatabase->Protocol
                    );
  }

  return Status;
}
