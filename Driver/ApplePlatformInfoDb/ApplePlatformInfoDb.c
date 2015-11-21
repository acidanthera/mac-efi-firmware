#include <AppleEfi.h>
#include <EfiDriverLib.h>
#include <EfiHobLib.h>

#include EFI_GUID_DEFINITION (Hob)

#include EFI_PROTOCOL_CONSUMER (FirmwareVolume)
#include <Protocol/ApplePlatformInfoDatabase.h>
#include <Protocol/ApplePlatformInfoDatabaseImpl.h>

#include <Driver/ApplePlatformInfoDB.h>

EFI_GUID gAppleFile1Guid = APPLE_FILE_1_GUID;

EFI_GUID gAppleHob1Guid = APPLE_HOB_1_GUID;

EFI_GUID gAppleHob2Guid = APPLE_HOB_2_GUID;

EFI_GUID gAppleHob3Guid = APPLE_HOB_3_GUID;

// mApplePlatformInfoDBProtocol
static APPLE_PLATFORM_INFO_DATABASE_PROTOCOL mApplePlatformInfoDBProtocol = {
  APPLE_PLATFORM_INFO_DATABASE_PROTOCOL_REVISION,
  ApplePlatformInfoDBGetFirstPlatformInfoDataImpl,
  ApplePlatformInfoDBGetFirstPlatformInfoDataSizeImpl,
  ApplePlatformInfoDBGetPlatformInfoDataImpl,
  ApplePlatformInfoDBGetPlatformInfoDataSizeImpl
};

// ApplePlatformInfoDBMain
///
///
/// @param[in] ImageHandle The firmware allocated handle for the EFI image.  
/// @param[in] SystemTable A pointer to the EFI System Table.
///
/// @retval EFI_SUCCESS         The entry point is executed successfully.
/// @retval EFI_ALREADY_STARTED The protocol has already been installed.
EFI_STATUS
EFIAPI
ApplePlatformInfoDBMain (
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
  EFI_FIRMWARE_VOLUME_PROTOCOL *FirmwareVolumeProtocol;
  EFI_DEVICE_PATH_PROTOCOL     *DevicePath;
  UINTN                        Index;
  BOOLEAN                      FirmwareVolumeFound;
  UINTN                        FileBufferSize;
  EFI_FV_FILETYPE              FoundType;
  EFI_FV_FILE_ATTRIBUTES       FileAttributes;
  UINT32                       AuthenticationStatus;

  APPLE_PLATFORM_INFO_DATABASE *PlatformInfoDatabase;

  EfiInitializeDriverLib (ImageHandle, SystemTable);

  FirmwareVolumeProtocol = NULL; /////
  HobListTable2          = NULL; /////////
  HobListTable           = NULL;
  Buffer                 = NULL;
  Status                 = EfiLibGetSystemConfigurationTable (&gEfiHobListGuid, &HobListTable);

  if (!EFI_ERROR (Status)) {
    Status = GetNextGuidHob (&HobListTable, &gAppleHob1Guid, &Buffer, &BufferSize);

    if (EFI_ERROR (Status)) {
      Buffer = NULL;
    }
  }

  Status = gBS->LocateHandleBuffer (ByProtocol, &gEfiFirmwareVolumeProtocolGuid, NULL, &NumberHandles, &HandleBuffer);

  if (!EFI_ERROR (Status)) {
    FirmwareVolumeProtocol = NULL;
    DevicePath             = NULL;

    if (Buffer != NULL) {
      FirmwareVolumeFound = TRUE;

      for (Index = 0; Index < NumberHandles; ++Index) {
        if (FirmwareVolumeFound) {

        }

        Status = gBS->HandleProtocol (HandleBuffer[Index], &gEfiDevicePathProtocolGuid, (VOID **)&DevicePath);

        if (!EFI_ERROR (Status)) {
          while (!IsDevicePathEnd (DevicePath)) {
            if ((DevicePathType (DevicePath) == HARDWARE_DEVICE_PATH) && (DevicePathSubType (DevicePath) == HW_MEMMAP_DP) && (((MEMMAP_DEVICE_PATH *)DevicePath)->StartingAddress == (EFI_PHYSICAL_ADDRESS)(UINTN)Buffer)) {
              Status = gBS->HandleProtocol (HandleBuffer[Index], &gEfiFirmwareVolumeProtocolGuid, (VOID **)&FirmwareVolumeProtocol);

              if (EFI_ERROR (Status)) {
                goto BreakBoth;
              }
            }

            DevicePath = NextDevicePathNode (DevicePath);
          }
        }

        FirmwareVolumeFound = (FirmwareVolumeProtocol == NULL);
      }
    }

  BreakBoth:
    for (Index = 0; Index < NumberHandles; ++Index) {
      Status = gBS->HandleProtocol (HandleBuffer[Index], &gEfiFirmwareVolumeProtocolGuid, (VOID **)&FirmwareVolumeProtocol);

      if (!EFI_ERROR (Status)) {
        Status = FirmwareVolumeProtocol->ReadFile (FirmwareVolumeProtocol, &gAppleFile1Guid, NULL, &FileBufferSize, &FoundType, &FileAttributes, &AuthenticationStatus);

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
    Status = EfiLibGetSystemConfigurationTable (&gEfiHobListGuid, HobListTable2);

    if (!EFI_ERROR (Status)) {

    }
  }

  PlatformInfoDatabase = (APPLE_PLATFORM_INFO_DATABASE *)EfiLibAllocatePool (sizeof (*PlatformInfoDatabase));
  Status               = EFI_OUT_OF_RESOURCES;

  if (PlatformInfoDatabase != NULL) {
    PlatformInfoDatabase->Signature              = APPLE_PLATFORM_INFO_DATABASE_SIGNATURE;
    PlatformInfoDatabase->FirmwareVolumeHandle   = NULL; ////
    PlatformInfoDatabase->FirmwareVolumeProtocol = FirmwareVolumeProtocol;

    EfiCopyMem ((VOID *)&PlatformInfoDatabase->Protocol, (VOID *)&mApplePlatformInfoDBProtocol, sizeof (mApplePlatformInfoDBProtocol));
    
    Status = gBS->InstallProtocolInterface (PlatformInfoDatabase->FirmwareVolumeHandle, &gApplePlatformInfoDatabaseProtocolGuid, EFI_NATIVE_INTERFACE, (VOID *)&PlatformInfoDatabase->Protocol);
  }

  return Status;
}
