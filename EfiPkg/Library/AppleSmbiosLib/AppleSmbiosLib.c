#include <AppleMacEfi.h>

#include <IndustryStandard/SmBios.h>

#include <Protocol/ApplePlatformInfoDatabase.h>
#include <Protocol/Smbios.h>

#include <Library/AppleSmbiosLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

// mSmbios
STATIC CONST EFI_SMBIOS_PROTOCOL *mSmbios = NULL;

// InternalGetSmbiosProtocol
STATIC
EFI_STATUS
InternalGetSmbiosProtocol (
  VOID
  )
{
  EFI_STATUS Status;

  Status = EFI_SUCCESS;

  if (mSmbios == NULL) {
    Status = gBS->LocateProtocol (
                    &gEfiSmbiosProtocolGuid,
                    NULL,
                    (VOID **)&mSmbios
                    );
  }

  return Status;
}

// SmbiosGetRecord
SMBIOS_STRUCTURE *
SmbiosGetRecord (
  IN EFI_SMBIOS_HANDLE  Handle
  )
{
  SMBIOS_STRUCTURE        *Record;

  EFI_STATUS              Status;
  EFI_SMBIOS_HANDLE       SmbiosHandle;
  EFI_SMBIOS_TABLE_HEADER *RecordWalker;
  EFI_HANDLE              ProducerHandle;

  Record = NULL;

  Status = InternalGetSmbiosProtocol ();

  if (!EFI_ERROR (Status)) {
    SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
    RecordWalker = NULL;

    do {
      ProducerHandle = NULL;
      Status         = mSmbios->GetNext (
                                  mSmbios,
                                  &SmbiosHandle,
                                  NULL,
                                  &RecordWalker,
                                  &ProducerHandle
                                  );

      if ((SmbiosHandle == Handle) && !EFI_ERROR (Status)) {
        Record = (SMBIOS_STRUCTURE *)RecordWalker;

        break;
      }
    } while (!EFI_ERROR (Status));
  }

  return Record;
}

// SmbiosAdd
VOID
SmbiosAdd (
  IN SMBIOS_STRUCTURE  *Record
  )
{
  EFI_STATUS              Status;

  EFI_SMBIOS_TABLE_HEADER *SmbiosRecord;
  EFI_SMBIOS_HANDLE       SmbiosHandle;

  Status = InternalGetSmbiosProtocol ();

  if (!EFI_ERROR (Status)) {
    // BUG: Use AllocatePool () and manually set the two zeros.
    SmbiosRecord = AllocateZeroPool (Record->Length + 2);

    if (SmbiosRecord != NULL) {
      CopyMem ((VOID *)SmbiosRecord, (VOID *)Record, (UINTN)Record->Length);

      SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
      Status       = mSmbios->Add (
                                mSmbios,
                                gImageHandle,
                                &SmbiosHandle,
                                SmbiosRecord
                                );

      ASSERT_EFI_ERROR (Status);

      FreePool ((VOID *)SmbiosRecord);

      SmbiosGetRecord (SmbiosHandle);
    }
  }
}

// SmbiosUpdateString
VOID
SmbiosUpdateString (
  IN EFI_SMBIOS_HANDLE  *Handle,
  IN UINTN              StringNumber,
  IN CHAR8              *String
  )
{
  EFI_STATUS Status;

  if (String != NULL && String[0] != L'\0') {
    Status = InternalGetSmbiosProtocol ();

    if (!EFI_ERROR (Status)) {
      mSmbios->UpdateString (mSmbios, Handle, &StringNumber, String);
    }
  }
}

// SmbiosGetFirstHandle
SMBIOS_STRUCTURE *
SmbiosGetFirstHandle (
  IN     EFI_SMBIOS_TYPE    Type,
  IN OUT EFI_SMBIOS_HANDLE  *Handle
  )
{
  SMBIOS_STRUCTURE        *SmbiosRecord;

  EFI_STATUS              Status;
  EFI_HANDLE              ProducerHandle;
  EFI_SMBIOS_TABLE_HEADER *TempRecord;

  SmbiosRecord = NULL;

  Status = InternalGetSmbiosProtocol ();

  if (!EFI_ERROR (Status)) {
    ProducerHandle = NULL;

    Status = mSmbios->GetNext (
                        mSmbios,
                        Handle,
                        &Type,
                        &TempRecord,
                        &ProducerHandle
                        );

    if (!EFI_ERROR (Status)) {
      SmbiosRecord = (SMBIOS_STRUCTURE *)TempRecord;
    }
  }

  return SmbiosRecord;
}

// SmbiosInstallTables
EFI_STATUS
SmbiosInstallTables (
  VOID
  )
{
  EFI_STATUS                            Status;

  APPLE_PLATFORM_INFO_DATABASE_PROTOCOL *PlatformInfoDb;
  UINT32                                SmbiosTemplateSize;
  EFI_SMBIOS_TABLE_HEADER               *SmbiosRecord;
  UINT32                                DataSize;
  EFI_SMBIOS_HANDLE                     SmbiosHandle;
  UINT8                                 *Walker;

  Status = gBS->LocateProtocol (
                  &gApplePlatformInfoDatabaseProtocolGuid,
                  NULL,
                  (VOID **)&PlatformInfoDb
                  );

  if (!EFI_ERROR (Status)) {
    SmbiosTemplateSize = 0;
    Status             = PlatformInfoDb->GetFirstDataSize (
                                           PlatformInfoDb,
                                           &gEfiSmbiosProtocolGuid,
                                           &SmbiosTemplateSize
                                           );

    if (!EFI_ERROR (Status) || (Status == EFI_BUFFER_TOO_SMALL)) {
      SmbiosRecord = AllocatePool (SmbiosTemplateSize);

      Status = EFI_OUT_OF_RESOURCES;

      if (SmbiosTemplateSize != 0) {
        Status = PlatformInfoDb->GetFirstData (
                                   PlatformInfoDb,
                                   &gEfiSmbiosProtocolGuid,
                                   (VOID *)SmbiosRecord,
                                   &SmbiosTemplateSize
                                   );

        if (!EFI_ERROR (Status)) {
          Status = InternalGetSmbiosProtocol ();

          if (EFI_ERROR (Status)) {
            Status = (EFI_INVALID_PARAMETER | (MAX_BIT - 2));
          } else if (SmbiosTemplateSize != 0) {
            DataSize = 0;

            do {
              SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
              Status       = mSmbios->Add (
                                        mSmbios,
                                        gImageHandle,
                                        &SmbiosHandle,
                                        SmbiosRecord
                                        );

              if (EFI_ERROR (Status)) {
                break;
              }

              SmbiosRecord = (EFI_SMBIOS_TABLE_HEADER *)(
                               (UINTN)SmbiosRecord + SmbiosRecord->Length
                               );

              DataSize += (SmbiosRecord->Length + 2);

              Walker = (UINT8 *)SmbiosRecord;

              do {
                if ((Walker[0] == 0) && (Walker[1] == 0)) {
                  break;
                }

                ++Walker;
                ++DataSize;
              } while ((DataSize - 2) < SmbiosTemplateSize);

              SmbiosRecord = (EFI_SMBIOS_TABLE_HEADER *)(Walker + 2);
            } while (ALIGN_VALUE (DataSize, 4) < SmbiosTemplateSize);

            if ((ALIGN_VALUE (DataSize, 4) >= SmbiosTemplateSize)) {
              ASSERT (DataSize <= SmbiosTemplateSize);
            }
          }
        }
      }
    }
  }

  return Status;
}
