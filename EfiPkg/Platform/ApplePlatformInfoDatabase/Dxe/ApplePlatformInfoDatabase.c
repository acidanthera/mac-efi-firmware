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
#include <PiDxe.h>

#include <Guid/AppleFile.h>
#include <Guid/AppleHob.h>
#include <Guid/HobList.h>

#include <Protocol/ApplePlatformInfoDatabase.h>
#include <Protocol/FirmwareVolume.h>

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/HobLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#define APPLE_PLATFORM_INFO_DATABASE_SIGNATURE  \
  SIGNATURE_32 ('P', 'I', 'D', 'B')

#define PLATFORM_INFO_PROTOCOL_FROM_DATABASE(Database)  \
  CR (                                                  \
    Database,                                           \
    APPLE_PLATFORM_INFO_DATABASE,                       \
    Protocol,                                           \
    APPLE_PLATFORM_INFO_DATABASE_SIGNATURE              \
    )

// APPLE_PLATFORM_INFO_DATABASE
typedef struct {
  UINT32                                Signature;             ///< 
  EFI_HANDLE                            FirmwareVolumeHandle;  ///< 
  EFI_FIRMWARE_VOLUME_PROTOCOL          *FirmwareVolume;       ///< 
  APPLE_PLATFORM_INFO_DATABASE_PROTOCOL Protocol;              ///< 
} APPLE_PLATFORM_INFO_DATABASE;

// EFI_APPLE_SECTION_IDENTIFIER
typedef PACKED struct {
  EFI_RAW_SECTION Hdr;    ///< 
  UINT32          Ukn_4;  ///< 
} EFI_APPLE_SECTION_IDENTIFIER;

// EFI_APPLE_SECTION
typedef PACKED struct {
  EFI_APPLE_SECTION_IDENTIFIER Hdr;    ///< 
  UINT64                       int_8;  ///< 
  UINT32                       Size;   ///< 
  UINT8                        Data;   ///< 
} EFI_APPLE_SECTION;

#pragma pack ()

// mD20Data
STATIC EFI_APPLE_SECTION_IDENTIFIER mD20Data = { { { 0, 0, 0 }, 0 }, 0 };

// mD30Data
STATIC EFI_APPLE_SECTION_IDENTIFIER mD30Data = { { { 0, 0, 0 }, 0 }, 0 };

// PlatformInfoDbGetData
EFI_STATUS
EFIAPI
PlatformInfoDbGetData (
  IN     APPLE_PLATFORM_INFO_DATABASE_PROTOCOL  *This,
  IN     EFI_GUID                               *NameGuid,
  IN     UINTN                                  Index, OPTIONAL
  IN OUT VOID                                   *Data, OPTIONAL
  IN OUT UINT32                                 *Size
  )
{
  EFI_STATUS                   Status;

  EFI_FIRMWARE_VOLUME_PROTOCOL *FirmwareVolume;
  //EFI_SECTION_TYPE           SectionType;
  UINTN                        SectionInstance;
  EFI_APPLE_SECTION            *Buffer;
  EFI_APPLE_SECTION            *BufferPtr;
  UINTN                        BufferSize;
  UINT32                       AuthenticationStatus;
  UINT64                       ShiftedValue;
  UINT64                       XoredValue;
  INTN                         Result;
  UINTN                        TempLength;

  SectionInstance = 0;
  Status          = EFI_INVALID_PARAMETER;

  if ((This != NULL) && (NameGuid != NULL) && (Size != NULL)) {
    FirmwareVolume = (PLATFORM_INFO_PROTOCOL_FROM_DATABASE (This))->FirmwareVolume;
    Buffer         = NULL;

    do {
      BufferPtr = NULL;
      Status    = FirmwareVolume->ReadSection (
                                    FirmwareVolume,
                                    NameGuid,
                                    EFI_SECTION_RAW,
                                    SectionInstance,
                                    (VOID **)&BufferPtr,
                                    &BufferSize,
                                    &AuthenticationStatus
                                    );

      if (EFI_ERROR (Status)) {
        break;
      }

      ShiftedValue = RShiftU64 (BufferPtr->int_8, 32);
      XoredValue   = (BufferPtr->int_8 ^ Index);

      ++SectionInstance;

      if ((ShiftedValue & XoredValue) == 0) {
        Result = CompareMem (
                   (VOID *)&BufferPtr->Hdr,
                   (VOID *)&mD20Data,
                   sizeof (mD20Data)
                   );

        if (Result != 0) {
          if ((BufferPtr->Hdr.Hdr.Size == 0)
           && (Buffer == NULL)) {
            Buffer = BufferPtr;

            continue;
          } else {
            Result = CompareMem (
                       (VOID *)&BufferPtr->Hdr,
                       (VOID *)&mD30Data,
                       sizeof (mD30Data)
                       );

            if (Result == 0) {
              if (Buffer != NULL) {
                gBS->FreePool ((VOID *)Buffer);
              }

              Buffer    = BufferPtr;
              BufferPtr = NULL;

              continue;
            }
          }
        } else {
          if (Buffer != NULL) {
            gBS->FreePool ((VOID *)Buffer);
          }

          Buffer    = BufferPtr;
          BufferPtr = NULL;

          break;
        }
      }

      gBS->FreePool ((VOID *)BufferPtr);
    } while (!EFI_ERROR (Status));

    if (Buffer != NULL) {
      Status = EFI_SUCCESS;
    }

    if (!EFI_ERROR (Status)) {
      TempLength = *Size;
      *Size      = Buffer->Size;

      if (Data != NULL) {
        if (TempLength < (UINTN)Buffer->Size) {
          Status = EFI_BUFFER_TOO_SMALL;
        } else {
          CopyMem (Data, (VOID *)&Buffer->Data, (UINTN)Buffer->Size);
        }
      }

      if (Buffer != NULL) {
        gBS->FreePool ((VOID *)Buffer);
      }
    }
  }

  return Status;
}

// PlatformInfoDbGetFirstDataSize
EFI_STATUS
EFIAPI
PlatformInfoDbGetFirstDataSize (
  IN     APPLE_PLATFORM_INFO_DATABASE_PROTOCOL  *This,
  IN     EFI_GUID                               *NameGuid, OPTIONAL
  IN OUT UINT32                                 *Size
  )
{
  return PlatformInfoDbGetData (This, NameGuid, 0, NULL, Size);
}

// PlatformInfoDbGetDataSize
EFI_STATUS
EFIAPI
PlatformInfoDbGetDataSize (
  IN     APPLE_PLATFORM_INFO_DATABASE_PROTOCOL  *This,
  IN     EFI_GUID                               *NameGuid, OPTIONAL
  IN     UINTN                                  Index, OPTIONAL
  IN OUT UINT32                                 *Size
  )
{
  return PlatformInfoDbGetData (This, NameGuid, Index, NULL, Size);
}

// PlatformInfoDbGetFirstData
EFI_STATUS
EFIAPI
PlatformInfoDbGetFirstData (
  IN     APPLE_PLATFORM_INFO_DATABASE_PROTOCOL  *This,
  IN     EFI_GUID                               *NameGuid,
  IN OUT VOID                                   *Data, OPTIONAL
  IN OUT UINT32                                 *Size
  )
{
  return PlatformInfoDbGetData (This, NameGuid, 0, Data, Size);
}

// mApplePlatformInfoDatabaseDxe
STATIC
APPLE_PLATFORM_INFO_DATABASE_PROTOCOL mApplePlatformInfoDbProtocolTemplate = {
  APPLE_PLATFORM_INFO_DATABASE_PROTOCOL_REVISION,
  PlatformInfoDbGetFirstData,
  PlatformInfoDbGetFirstDataSize,
  PlatformInfoDbGetData,
  PlatformInfoDbGetDataSize
};

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
  )
{
  EFI_STATUS                   Status;

  VOID                         *HobListTable;
  VOID                         *HobListTable2;
  VOID                         *Buffer;
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
  APPLE_PLATFORM_INFO_DATABASE *PlatformInfoDb;

  FirmwareVolume = NULL; /////
  HobListTable2  = NULL; /////////
  HobListTable   = NULL;
  Buffer         = NULL;
  Status         = EfiGetSystemConfigurationTable (
                     &gEfiHobListGuid,
                     &HobListTable
                     );

  if (!EFI_ERROR (Status)) {
    Buffer = GetFirstGuidHob (&gAppleHob1Guid);
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
          //
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
      EfiGetSystemConfigurationTable (&gEfiHobListGuid, &HobListTable);
    }

    return Status;
  }

  ///

  gBS->FreePool ((VOID *)HandleBuffer);

  if (!EFI_ERROR (Status)) {
    Status = EfiGetSystemConfigurationTable (
               &gEfiHobListGuid,
               HobListTable2
               );

    if (!EFI_ERROR (Status)) {

    }
  }

  PlatformInfoDb = AllocatePool (sizeof (*PlatformInfoDb));

  Status = EFI_OUT_OF_RESOURCES;

  if (PlatformInfoDb != NULL) {
    PlatformInfoDb->Signature            = APPLE_PLATFORM_INFO_DATABASE_SIGNATURE;
    PlatformInfoDb->FirmwareVolumeHandle = NULL; ////
    PlatformInfoDb->FirmwareVolume       = FirmwareVolume;

    CopyMem (
      (VOID *)&PlatformInfoDb->Protocol,
      (VOID *)&mApplePlatformInfoDbProtocolTemplate,
      sizeof (mApplePlatformInfoDbProtocolTemplate)
      );
    
    Status = gBS->InstallProtocolInterface (
                    PlatformInfoDb->FirmwareVolumeHandle,
                    &gApplePlatformInfoDatabaseProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    (VOID *)&PlatformInfoDb->Protocol
                    );
  }

  return Status;
}
