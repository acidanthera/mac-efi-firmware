#include <AppleMacEfi.h>
#include <FrameworkDxe.h>

#include <Guid/AppleDataHub.h>

#include <Protocol/DataHub.h>

#include <Library/BaseLib.h>
#include <Library/AppleDataHubLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

// mDataHub
STATIC EFI_DATA_HUB_PROTOCOL *mDataHub = NULL;

// InternalGetDataHubProtocol
STATIC
EFI_STATUS
InternalGetDataHubProtocol (
  VOID
  )
{
  EFI_STATUS Status;

  Status = EFI_SUCCESS;

  if (mDataHub == NULL) {
    Status = gBS->LocateProtocol (
                    &gEfiDataHubProtocolGuid,
                    NULL,
                    (VOID **)&mDataHub
                    );
  }

  return Status;
}

// DataHubLogData
VOID
DataHubLogData (
  IN  EFI_GUID  *DataRecordGuid,
  IN  EFI_GUID  *ProducerName,
  IN  VOID      *RawData,
  IN  UINT32    RawDataSize
  )
{
  EFI_STATUS Status;

  Status = InternalGetDataHubProtocol ();

  if (!EFI_ERROR (Status)) {
    mDataHub->LogData (
                mDataHub,
                DataRecordGuid,
                ProducerName,
                EFI_DATA_RECORD_CLASS_DATA,
                RawData,
                RawDataSize
                );
  }
}

// DataHubLogApplePlatformData
VOID
DataHubLogApplePlatformData (
  IN CHAR16    *Key,
  IN EFI_GUID  *DataRecordGuid,
  IN VOID      *Value,
  IN UINTN     ValueSize
  )
{
  UINTN                      KeySize;
  APPLE_PLATFORM_DATA_RECORD *Record;

  KeySize = (StrLen (Key) * sizeof (*Key));
  Record  = AllocatePool (sizeof (Record->Hdr) + KeySize + ValueSize);

  if (Record != NULL) {
    Record->Hdr.Header.Version     = APPLE_SUBCLASS_VERSION;
    Record->Hdr.Header.HeaderSize  = sizeof (Record->Hdr);
    Record->Hdr.Header.Instance    = APPLE_SUBCLASS_INSTANCE;
    Record->Hdr.Header.SubInstance = APPLE_SUBCLASS_INSTANCE;
    Record->Hdr.Header.RecordType  = APPLE_SUBCLASS_INSTANCE;
    Record->Hdr.KeySize            = (UINT32)KeySize;
    Record->Hdr.ValueSize          = (UINT32)ValueSize;

    CopyMem ((VOID *)&(&Record->Key)[0], (VOID *)Key, KeySize);
    CopyMem ((VOID *)((UINTN)&(&Record->Key)[0] + KeySize), Value, ValueSize);

    DataHubLogData (
      DataRecordGuid,
      &gApplePlatformProducerNameGuid,
      (VOID *)&Record->Hdr,
      sizeof (Record->Hdr)
      );

    FreePool ((VOID *)Record);
  } else {
    ASSERT (FALSE);
  }
}
