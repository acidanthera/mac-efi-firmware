#include <AppleEfi.h>
#include <EfiDriverLib.h>
#include <EfiImageFormat.h>

#include EFI_PROTOCOL_CONSUMER (FirmwareVolume)
#include <Protocol/ApplePlatformInfoDatabase.h>
#include <Protocol/ApplePlatformInfoDatabaseImpl.h>

// ApplePlatformInfoDbGetFirstPlatformInfoDataSizeImpl
EFI_STATUS
EFIAPI
ApplePlatformInfoDbGetFirstPlatformInfoDataSizeImpl (
  IN     APPLE_PLATFORM_INFO_DATABASE_PROTOCOL  *This,
  IN     EFI_GUID                               *NameGuid,
  IN OUT UINTN                                  *Size
  ) // sub_7E6U2F
{
  return ApplePlatformInfoDbGetPlatformInfoDataImpl (This, NameGuid, 0, NULL, Size);
}

// ApplePlatformInfoDbGetPlatformInfoDataSizeImpl
EFI_STATUS
EFIAPI
ApplePlatformInfoDbGetPlatformInfoDataSizeImpl (
  IN     APPLE_PLATFORM_INFO_DATABASE_PROTOCOL  *This,
  IN     EFI_GUID                               *NameGuid,
  IN     UINTN                                  XorValue,
  IN OUT UINTN                                  *Size
  ) // sub_804
{
  return ApplePlatformInfoDbGetPlatformInfoDataImpl (This, NameGuid, XorValue, NULL, Size);
}

// ApplePlatformInfoDbGetFirstPlatformInfoDataImpl
EFI_STATUS
EFIAPI
ApplePlatformInfoDbGetFirstPlatformInfoDataImpl (
  IN     APPLE_PLATFORM_INFO_DATABASE_PROTOCOL  *This,
  IN     EFI_GUID                               *NameGuid,
  IN OUT VOID                                   *Data,
  IN OUT UINTN                                  *Size
  ) // sub_81F
{
  return ApplePlatformInfoDbGetPlatformInfoDataImpl (This, NameGuid, 0, Data, Size);
}

// ApplePlatformInfoDbGetPlatformInfoDataImpl
EFI_STATUS
EFIAPI
ApplePlatformInfoDbGetPlatformInfoDataImpl (
  IN     APPLE_PLATFORM_INFO_DATABASE_PROTOCOL  *This,
  IN     EFI_GUID                               *NameGuid,
  IN     UINTN                                  Index,
  IN OUT VOID                                   *Data,
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
        Result = EfiCompareMem ((VOID *)&BufferPtr->Hdr, (VOID *)&mD20Data, sizeof (BufferPtr->Hdr));

        if (Result != 0) {
          if ((BufferPtr->Hdr.CommonHeader.Size == 0) && (Buffer == NULL)) {
            Buffer = BufferPtr;

            continue;
          } else {
            Result = EfiCompareMem ((VOID *)&BufferPtr->Hdr, (VOID *)&mD30Data, sizeof (BufferPtr->Hdr));

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

  return Status;
}
