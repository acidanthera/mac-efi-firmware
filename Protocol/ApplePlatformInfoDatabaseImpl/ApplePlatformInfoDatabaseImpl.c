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
#include <AppleMisc.h>
#include <EfiImageFormat.h>

#include <Library/AppleDriverLib.h>

#include "ApplePlatformInfoDatabaseImplInternal.h"

// mD20Data
EFI_APPLE_SECTION_IDENTIFIER mD20Data = { { { { 0, 0, 0 }, 0 } }, 0 };

// mD30Data
EFI_APPLE_SECTION_IDENTIFIER mD30Data = { { { { 0, 0, 0 }, 0 } }, 0 };

// PlatformInfoDbGetFirstDataSizeImpl
EFI_STATUS
EFIAPI
PlatformInfoDbGetFirstDataSizeImpl (
  IN     APPLE_PLATFORM_INFO_DATABASE_PROTOCOL  *This,
  IN     EFI_GUID                               *NameGuid, OPTIONAL
  IN OUT UINTN                                  *Size
  ) // sub_7E6U2F
{
  return PlatformInfoDbGetDataImpl (This, NameGuid, 0, NULL, Size);
}

// PlatformInfoDbGetDataSizeImpl
EFI_STATUS
EFIAPI
PlatformInfoDbGetDataSizeImpl (
  IN     APPLE_PLATFORM_INFO_DATABASE_PROTOCOL  *This,
  IN     EFI_GUID                               *NameGuid, OPTIONAL
  IN     UINTN                                  Index, OPTIONAL
  IN OUT UINTN                                  *Size
  ) // sub_804
{
  return PlatformInfoDbGetDataImpl (This, NameGuid, Index, NULL, Size);
}

// PlatformInfoDbGetFirstDataImpl
EFI_STATUS
EFIAPI
PlatformInfoDbGetFirstDataImpl (
  IN     APPLE_PLATFORM_INFO_DATABASE_PROTOCOL  *This,
  IN     EFI_GUID                               *NameGuid,
  IN OUT VOID                                   *Data, OPTIONAL
  IN OUT UINTN                                  *Size
  ) // sub_81F
{
  return PlatformInfoDbGetDataImpl (This, NameGuid, 0, Data, Size);
}

// PlatformInfoDbGetDataImpl
EFI_STATUS
EFIAPI
PlatformInfoDbGetDataImpl (
  IN     APPLE_PLATFORM_INFO_DATABASE_PROTOCOL  *This,
  IN     EFI_GUID                               *NameGuid,
  IN     UINTN                                  Index, OPTIONAL
  IN OUT VOID                                   *Data, OPTIONAL
  IN OUT UINTN                                  *Size
  ) // sub_840
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

  ASSERT (This != NULL);
  ASSERT (NameGuid != NULL);
  ASSERT (Size != NULL);
  ASSERT ((*Size == 0) || (Data != NULL));

  if ((This != NULL) && (NameGuid != NULL) && (Size != NULL)) {
    FirmwareVolume = (PLATFORM_INFO_PROTOCOL_FROM_DATABASE (This))->FirmwareVolumeProtocol;
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
      XoredValue   = DIFF_BITS (BufferPtr->int_8, Index);

      ++SectionInstance;

      if ((ShiftedValue & XoredValue) == 0) {
        Result = EfiCompareMem ((VOID *)&BufferPtr->Hdr, (VOID *)&mD20Data, sizeof (mD20Data));

        if (Result != 0) {
          if ((BufferPtr->Hdr.Hdr.CommonHeader.Size == 0) && (Buffer == NULL)) {
            Buffer = BufferPtr;

            continue;
          } else {
            Result = EfiCompareMem ((VOID *)&BufferPtr->Hdr, (VOID *)&mD30Data, sizeof (mD30Data));

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
      *Size    = (UINTN)Buffer->Size;

      if (Data != NULL) {
        if (TempLength < (UINTN)Buffer->Size) {
          Status = EFI_BUFFER_TOO_SMALL;
        } else {
          gBS->CopyMem (Data, (VOID *)&Buffer->Data, (UINTN)Buffer->Size);
        }
      }

      if (Buffer != NULL) {
        gBS->FreePool ((VOID *)Buffer);
      }
    }
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}
