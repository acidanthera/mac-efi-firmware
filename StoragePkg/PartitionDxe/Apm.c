#include "Partition.h"
#include "Apm.h"

BOOLEAN
PartitionInstallAppleChildHandles (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Handle,
  IN  EFI_DISK_IO_PROTOCOL         *DiskIo,
  IN  EFI_BLOCK_IO_PROTOCOL        *BlockIo,
  IN  EFI_DEVICE_PATH_PROTOCOL     *DevicePath
  )
/*++

Routine Description:
  Install child handles if the Handle supports APM format.

Arguments:       
  This       - Calling context.
  Handle     - Parent Handle 
  DiskIo     - Parent DiskIo interface
  BlockIo    - Parent BlockIo interface
  DevicePath - Parent Device Path

Returns:
  TRUE       - If a child handle was added
  FALSE      - Not found APM partition.

--*/
{
  EFI_STATUS                     Status;
  UINT32                         BlockSize;
  APM_DRIVER_DESCRIPTOR_MAP      *Apm;
  APM_ENTRY                      *ApmEntry;
  UINTN                          NumberOfPartitionEntries;
  UINT64                         Offset;
  UINT64                         PartitionSize;
  UINT64                         PartitionStart;
  EFI_LBA                        StartingLBA;
  EFI_LBA                        EndingLBA;
  UINT64                         LBASize;
  UINT32                         Remainder;
  UINTN                          Index;
  BOOLEAN                        ApmValid;
  HARDDRIVE_DEVICE_PATH          HdDev;
  APPLE_PARTITION_INFO_PROTOCOL  PartitionInfo;

  //
  // Check whether a medium is present
  //
  if (BlockIo->Media == NULL) {
    return FALSE;
  }

  //
  // Cerify the partition is physical
  //
  if (BlockIo->Media->LogicalPartition) {
    return FALSE;
  }

  //
  // Allocate a buffer for the APM Driver Descriptor Map
  //
  Apm = AllocatePool (BlockIo->Media->BlockSize);
  if (Apm == NULL) {
    return FALSE;
  }

  ApmValid = FALSE;

  //
  // Read the APM Driver Descriptor Map from LBA #0
  //
  Status = BlockIo->ReadBlocks (
                      BlockIo,
                      BlockIo->Media->MediaId,
                      0,
                      BlockIo->Media->BlockSize,
                      Apm
                      );
  if (EFI_ERROR (Status)) {
    gBS->FreePool (Apm);
    return ApmValid;
  }
  //
  // Verify that the APM Driver Descriptor Map is valid
  //
  if (Apm->Signature != APM_DRIVER_DESCRIPTOR_MAP_SIGNATURE) {
    gBS->FreePool (Apm);
    return ApmValid;
  }

  BlockSize = SwapBytes16 (Apm->BlockSize);

  //
  // Allocate a buffer for an APM Entry
  //
  ApmEntry = AllocatePool (BlockSize);

  if (ApmEntry == NULL) {
    gBS->FreePool (Apm);
    return ApmValid;
  }

  //
  // Read the APM from LBA #1
  //
  Status = DiskIo->ReadDisk (
                     DiskIo,
                     BlockIo->Media->MediaId,
                     BlockSize,
                     BlockSize,
                     ApmEntry
                     );
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // Verify that the APM is valid
  //
  if (ApmEntry->Signature != APM_ENTRY_SIGNATURE ||
      CompareMem (ApmEntry->PartitionType, APM_ENTRY_TYPE_APM, sizeof (APM_ENTRY_TYPE_APM)) != 0
      ) {
    goto Done;
  }

  NumberOfPartitionEntries = SwapBytes32 (ApmEntry->NumberOfPartitionEntries);

  //
  // Check if there are Apple Partition Entries
  //
  if (NumberOfPartitionEntries < 2) {
    goto Done;
  }

  Offset = 2 * BlockSize;

  //
  // Read the Apple Partition Entries
  //
  for (Index = 1; Index < NumberOfPartitionEntries; ++Index, Offset += BlockSize) {
    Status = DiskIo->ReadDisk (
                       DiskIo,
                       BlockIo->Media->MediaId,
                       Offset,
                       BlockSize,
                       ApmEntry
                       );

    if (EFI_ERROR (Status) ||
        ApmEntry->Signature != APM_ENTRY_SIGNATURE
        ) {
      goto Done;
    }

    //
    // Verify that the Apple Partition Entry is valid
    //
    if (CompareMem (ApmEntry->PartitionType, APM_ENTRY_TYPE_FREE, sizeof (APM_ENTRY_TYPE_FREE)) == 0 ||
        SwapBytes32 (ApmEntry->PartitionSize) == 0
        ) {
      continue;
    }

    PartitionStart = SwapBytes32 (ApmEntry->PartitionStart);

    StartingLBA = DivU64x32Remainder (
                    MultU64x32 (PartitionStart, BlockSize),
                    BlockIo->Media->BlockSize,
                    &Remainder
                    );

    if (Remainder != 0) {
      continue;
    }

    // BUG: Already calculated above -> cache!
    PartitionSize = SwapBytes32 (ApmEntry->PartitionSize);

    LBASize = DivU64x32Remainder (
                MultU64x32 (PartitionSize, BlockSize),
                BlockIo->Media->BlockSize,
                &Remainder
                );

    if (Remainder != 0) {
      continue;
    }

    EndingLBA = StartingLBA + LBASize - 1;

    if (EndingLBA > BlockIo->Media->LastBlock) {
      continue;
    }

    ZeroMem (&HdDev, sizeof (HdDev));
    HdDev.Header.Type     = MEDIA_DEVICE_PATH;
    HdDev.Header.SubType  = MEDIA_HARDDRIVE_DP;
    SetDevicePathNodeLength (&HdDev.Header, sizeof (HdDev));

    HdDev.PartitionNumber = (UINT32) Index + 1;
    HdDev.MBRType         = MBR_TYPE_APPLE_PARTITION_TABLE_HEADER;
    HdDev.PartitionStart  = StartingLBA;
    HdDev.PartitionSize   = LBASize;

    DEBUG ((EFI_D_INFO, " Index : %d\n", Index));
    DEBUG ((EFI_D_INFO, " Start LBA : %x\n", HdDev.PartitionStart));
    DEBUG ((EFI_D_INFO, " End LBA: %x\n", EndingLBA));
    DEBUG ((EFI_D_INFO, " Partition size: %x\n", HdDev.PartitionSize));
    DEBUG ((EFI_D_INFO, " Start : %x", MultU64x32 (PartitionStart, BlockSize)));
    DEBUG ((EFI_D_INFO, " End : %x", MultU64x32 ((PartitionStart + PartitionSize - 1), BlockSize)));

    ZeroMem (&PartitionInfo, sizeof (APPLE_PARTITION_INFO_PROTOCOL));

    PartitionInfo.Revision        = 0x010000;
    PartitionInfo.PartitionNumber = HdDev.PartitionNumber;
    PartitionInfo.MBRType         = HdDev.MBRType;
    PartitionInfo.PartitionStart  = HdDev.PartitionStart;
    PartitionInfo.PartitionSize   = HdDev.PartitionSize;

    CopyMem (&PartitionInfo.PartitionType, ApmEntry->PartitionType, sizeof(EFI_GUID));

    Status = PartitionInstallChildHandle (
              This,
              Handle,
              DiskIo,
              BlockIo,
              DevicePath,
              (EFI_DEVICE_PATH_PROTOCOL *) &HdDev,
              StartingLBA,
              EndingLBA,
              BlockIo->Media->BlockSize,
              FALSE,
              &PartitionInfo
              );

    if (!EFI_ERROR (Status)) {
      ApmValid = TRUE;
    }
  }

Done:
  gBS->FreePool (ApmEntry);
  gBS->FreePool (Apm);

  return ApmValid;
}
