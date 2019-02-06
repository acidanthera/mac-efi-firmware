#include <AppleMacEfi.h>
#include <FrameworkDxe.h>

#include <IndustryStandard/AppleSmBios.h>

#include <Guid/AppleDataHub.h>
#include <Guid/AppleHob.h>
#include <Guid/ApplePlatformInfo.h>
#include <Guid/AppleVariable.h>
#include <Guid/BiosId.h>

#include <Protocol/AppleDiagAccess.h>
#include <Protocol/ApplePlatformInfoDatabase.h>
#include <Protocol/AppleSmcIo.h>
#include <Protocol/Smbios.h>

#include <Library/AppleDataHubLib.h>
#include <Library/AppleSmbiosLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BiosIdLib.h>
#include <Library/HobLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include "AppleSmbiosInternal.h"

// mPlatformInfo
STATIC APPLE_PLATFORM_INFO_DATABASE_PROTOCOL *mPlatformInfo;

// mRomRecord
STATIC APPLE_ROM_RECORD mRomRecord = {
  {
    APPLE_SUBCLASS_VERSION,
    sizeof (mRomRecord.Header),
    APPLE_SUBCLASS_INSTANCE,
    APPLE_SUBCLASS_INSTANCE,
    APPLE_SUBCLASS_INSTANCE
  },
  3,
  0,
  { 0, 0, 0 }
};

// InternalGetPlatformInfoProtocol
STATIC
EFI_STATUS
InternalGetPlatformInfoProtocol (
  VOID
  )
{
  EFI_STATUS Status;

  Status = EFI_SUCCESS;

  if (mPlatformInfo == NULL) {
    Status = gBS->LocateProtocol (
                    &gApplePlatformInfoDatabaseProtocolGuid,
                    NULL,
                    (VOID **)&mPlatformInfo
                    );
  }

  return Status;
}

// InternalGetProductInfo
STATIC
VOID
InternalGetProductInfo (
  OUT APPLE_PRODUCT_INFO  **ProductInfo
  )
{
  APPLE_PRODUCT_INFO *ProductInfoData;
  EFI_STATUS         Status;
  UINT32             Size;

  ProductInfoData = NULL;

  Status = InternalGetPlatformInfoProtocol ();

  if (!EFI_ERROR (Status)) {
    Size   = sizeof (*ProductInfoData);
    ProductInfoData = AllocateZeroPool (sizeof (*ProductInfoData));

    if (ProductInfoData != NULL) {
      Status = mPlatformInfo->GetFirstData (
                                mPlatformInfo,
                                &gAppleProductInfoPlatformInfoGuid,
                                ProductInfoData,
                                &Size
                                );

      if (EFI_ERROR (Status)) {
        FreePool ((VOID *)ProductInfoData);

        ProductInfoData = NULL;
      }
    }
  }

  *ProductInfo = ProductInfoData;
}

// InternalGetBoardId
STATIC
VOID
InternalGetBoardId (
  OUT CHAR8  **BoardId
  )
{
  UINTN      DataSize;
  CHAR8      BoardIdBuffer[32];
  CHAR8      *BoardIdString;
  EFI_STATUS Status;

  DataSize = sizeof (BoardIdBuffer);

  ZeroMem ((VOID *)&BoardIdBuffer[0], sizeof (BoardIdBuffer));

  BoardIdString = NULL;

  Status = gRT->GetVariable (
                  L"HW_BID",
                  &gAppleVendorVariableGuid,
                  NULL,
                  &DataSize,
                  (VOID *)&BoardIdBuffer[0]
                  );

  if (!EFI_ERROR (Status)) {
    DataSize = AsciiStrSize (&BoardIdBuffer[0]);

    BoardIdString = AllocateZeroPool (DataSize);

    if (BoardIdString != NULL) {
      AsciiStrCpy (BoardIdString, &BoardIdBuffer[0]);
    }
  }

  *BoardId = BoardIdString;
}

// InternalGetProductSerialNumber
STATIC
VOID
InternalGetProductSerialNumber (
  OUT CHAR8   **SerialNumber,
  OUT CHAR16  **UnicodeSerialNumber
  )
{
  CHAR16                *UnicodeProductSerial;
  CHAR8                 *ProductSerial;
  EFI_STATUS            Status;
  APPLE_SMC_IO_PROTOCOL *SmcIo;
  SMC_DATA_SIZE         KeySize;
  SMC_KEY_TYPE          KeyType;
  SMC_KEY_ATTRIBUTES    KeyAttributes;
  UINTN                 DataSize;

  UnicodeProductSerial = NULL;
  ProductSerial        = NULL;

  Status = gBS->LocateProtocol (
                  &gAppleSmcIoProtocolGuid,
                  NULL,
                  (VOID **)&SmcIo
                  );

  if (!EFI_ERROR (Status)) {
    Status = SmcIo->SmcGetKeyInfo (
                      SmcIo,
                      SMC_MAKE_KEY ('R', 'S', 'S', 'N'),
                      &KeySize,
                      &KeyType,
                      &KeyAttributes
                      );

    ProductSerial = AllocateZeroPool (KeySize);

    if (ProductSerial != NULL) {
      Status = SmcIo->SmcReadValue (
                        SmcIo,
                        SMC_MAKE_KEY ('R', 'S', 'S', 'N'),
                        KeySize,
                        (SMC_DATA *)ProductSerial
                        );

      if (!EFI_ERROR (Status)) {
        DataSize = (KeySize / sizeof (ProductSerial[0])) * sizeof (UnicodeProductSerial[0]);

        UnicodeProductSerial = AllocateZeroPool (DataSize);

        if (UnicodeProductSerial != NULL) {
          // BUG: Use AsciiStrToUnicodeStr().
          UnicodeSPrint (UnicodeProductSerial, DataSize, L"%a", ProductSerial);
        } else {
          FreePool ((VOID *)ProductSerial);

          ProductSerial = NULL;
        }
      } else {
        FreePool ((VOID *)ProductSerial);

        ProductSerial = NULL;
      }
    }
  }

  *UnicodeSerialNumber = UnicodeProductSerial;
  *SerialNumber        = ProductSerial;
}

// InternalGetBoardSerialNumber
STATIC
VOID
InternalGetBoardSerialNumber (
  OUT CHAR8  **SerialNumber
  )
{
  CHAR8                 *BoardSerialNumber;

  EFI_STATUS            Status;
  APPLE_SMC_IO_PROTOCOL *SmcIo;
  SMC_DATA_SIZE         KeySize;
  SMC_KEY_TYPE          KeyType;
  SMC_KEY_ATTRIBUTES    KeyAttributes;

  BoardSerialNumber = NULL;

  Status = gBS->LocateProtocol (
                  &gAppleSmcIoProtocolGuid,
                  NULL,
                  (VOID **)&SmcIo
                  );

  if (!EFI_ERROR (Status)) {
    Status = SmcIo->SmcGetKeyInfo (
                      SmcIo,
                      SMC_MAKE_KEY ('R', 'M', 'S', 'N'),
                      &KeySize,
                      &KeyType,
                      &KeyAttributes
                      );

    BoardSerialNumber = AllocateZeroPool (0x80);

    if (BoardSerialNumber != NULL) {
      Status = SmcIo->SmcReadValue (
                        SmcIo,
                        SMC_MAKE_KEY ('R', 'M', 'S', 'N'),
                        KeySize,
                        (SMC_DATA *)BoardSerialNumber
                        );

      if (EFI_ERROR (Status)) {
        FreePool ((VOID *)BoardSerialNumber);

        BoardSerialNumber = NULL;
      }
    }
  }

  *SerialNumber = BoardSerialNumber;
}

// InternalGetSystemId
STATIC
VOID
InternalGetSystemId (
  OUT EFI_GUID  *SystemId
  )
{
  UINTN DataSize;

  ZeroMem ((VOID *)SystemId, sizeof (*SystemId));

  gRT->GetVariable (
          L"system-id",
          &gAppleVendorVariableGuid,
          NULL,
          &DataSize,
          (VOID **)SystemId
          );
}

// InternalGetFirmwareInfo
STATIC
VOID
InternalGetFirmwareInfo (
  OUT APPLE_SMBIOS_TABLE_TYPE128  *FirmwareInfo
  )
{
  UINTN                   DataSize;
  EFI_STATUS              Status;
  UINT32                  Attributes;
  UINT64                  FirmwareFeaturesMask;
  UINT64                  FirmwareFeatures;
  UINT64                  ExtendedFirmwareFeaturesMask;
  UINT64                  ExtendedFirmwareFeatures;
  EFI_HOB_FIRMWARE_VOLUME *FvHob;
  UINTN                   Index;

  ZeroMem ((VOID *)FirmwareInfo, sizeof (*FirmwareInfo));

  FirmwareInfo->Hdr.Type   = APPLE_SMBIOS_TYPE_FIRMWARE_INFORMATION;
  FirmwareInfo->Hdr.Length = sizeof (*FirmwareInfo);

  DataSize = sizeof (FirmwareFeaturesMask);
  Status   = gRT->GetVariable (
                    L"FirmwareFeaturesMask",
                    &gAppleVendorVariableGuid,
                    &Attributes,
                    &DataSize,
                    (VOID **)&FirmwareFeaturesMask
                    );

  if (!EFI_ERROR (Status)) {
    FirmwareInfo->FirmwareFeaturesMask = (UINT32)FirmwareFeaturesMask;
  }

  DataSize = sizeof (FirmwareFeaturesMask);
  Status   = gRT->GetVariable (
                    L"FirmwareFeatures",
                    &gAppleVendorVariableGuid,
                    &Attributes,
                    &DataSize,
                    (VOID **)&FirmwareFeatures
                    );

  if (!EFI_ERROR (Status)) {
    FirmwareInfo->FirmwareFeatures = (UINT32)FirmwareFeatures;
  }

  if (((FirmwareFeatures & FirmwareFeaturesMask) & BIT25) != 0) {
    DataSize = sizeof (ExtendedFirmwareFeaturesMask);
    Status   = gRT->GetVariable (
                      L"ExtendedFirmwareFeaturesMask",
                      &gAppleVendorVariableGuid,
                      &Attributes,
                      &DataSize,
                      (VOID **)&ExtendedFirmwareFeaturesMask
                      );

    if (!EFI_ERROR (Status)) {
      FirmwareInfo->ExtendedFirmwareFeaturesMask = (UINT32)RShiftU64 (ExtendedFirmwareFeaturesMask, 32);
    }

    DataSize = sizeof (ExtendedFirmwareFeaturesMask);
    Status   = gRT->GetVariable (
                      L"ExtendedFirmwareFeatures",
                      &gAppleVendorVariableGuid,
                      &Attributes,
                      &DataSize,
                      (VOID **)&ExtendedFirmwareFeatures
                      );

    if (!EFI_ERROR (Status)) {
      FirmwareInfo->ExtendedFirmwareFeatures = (UINT32)RShiftU64 (ExtendedFirmwareFeatures, 32);
    }
  }

  FvHob = GetFirstHob (EFI_HOB_TYPE_FV);

  for (Index = 0; Index < APPLE_NUMBER_OF_FLASHMAP_ENTRIES; ++Index) {
    switch (FvHob->BaseAddress) {
      case 0xFF139000:
      {
        FirmwareInfo->RegionTypeMap[Index] = AppleRegionTypeNvram;
        break;
      }

      case 0xFF169000:
      {
        FirmwareInfo->RegionTypeMap[Index] = AppleRegionTypeMain;
        break;
      }

      case 0xFFC80000:
      {
        FirmwareInfo->RegionTypeMap[Index] = AppleRegionTypeRecovery;
        break;
      }

      default:
      {
        FirmwareInfo->RegionTypeMap[Index] = AppleRegionTypeReserved;
        break;
      }
    }

    FirmwareInfo->FlashMap[Index].StartAddress = (UINT32)FvHob->BaseAddress;
    FirmwareInfo->FlashMap[Index].EndAddress  = ((UINT32)FvHob->BaseAddress + (UINT32)FvHob->Length - 1);

    FirmwareInfo->NumberOfRegions = (UINT8)(Index + 1);

    FvHob = GetNextHob (EFI_HOB_TYPE_FV, GET_NEXT_HOB (FvHob));

    if (FvHob == NULL) {
      break;
    }
  }
}

// InternalGetProductSerialNumber
STATIC
VOID
InternalGetSmcVersion (
  OUT UINT8  *SmcRevision
  )
{
  EFI_STATUS            Status;
  APPLE_SMC_IO_PROTOCOL *SmcIo;
  UINT8                 Rev[6];
  UINT8                 Temp;
  UINT8                 Index;

  SmcIo  = NULL;
  Status = gBS->LocateProtocol (
                  &gAppleSmcIoProtocolGuid,
                  NULL,
                  (VOID **)&SmcIo
                  );

  if (!EFI_ERROR (Status)) {
    Status = SmcIo->SmcReadValue (
                      SmcIo,
                      SMC_MAKE_KEY ('R', 'e', 'v', ' '),
                      sizeof (Rev),
                      (SMC_DATA *)&Rev[0]
                      );

    if (!EFI_ERROR (Status)) {
      Temp  = Rev[0];
      Index = 0;

      if (Temp < 0x10) {
        SmcRevision[Index] = (Temp + 0x30);
        ++Index;
      } else {
        SmcRevision[Index] = ((Temp >> 4) | 0x30);
        ++Index;

        SmcRevision[Index + 1] = ((Temp & 0x0F) | 0x30);
        ++Index;
      }

      SmcRevision[Index] = 0x2E;
      ++Index;

      Temp = Rev[1];

      if (Temp < 0x10) {
        SmcRevision[Index] = (Temp + 0x30);
        ++Index;
      } else {
        SmcRevision[Index] = ((Temp >> 4) | 0x30);
        ++Index;

        SmcRevision[Index] = ((Temp & 0x0F) | 0x30);
        ++Index;
      }

      Temp = Rev[2];

      if ((Temp & 0xF0) != 0) {
        if (Temp >= 0xA0) {
          SmcRevision[Index] = ((Temp >> 4) + 0x37);
          ++Index;
        } else {
          SmcRevision[Index] = ((Temp >> 4) | 0x30);
          ++Index;
        }

        Temp &= 0x0F;

        if (Temp >= 0x0A) {
          SmcRevision[Index] = (Temp + 0x37);
          ++Index;
        } else {
          SmcRevision[Index] = (Temp | 0x30);
          ++Index;
        }
      } else {
        if (Temp >= 0x0A) {
          SmcRevision[Index] = (Temp + 0x37);
          ++Index;
        } else {
          SmcRevision[Index] = (Temp + 0x30);
          ++Index;
        }
      }

      Temp = Rev[4];

      if (Temp < 0x10) {
        SmcRevision[Index] = (Temp + 0x30);
        ++Index;
      } else {
        SmcRevision[Index] = ((Temp >> 4) | 0x30);
        ++Index;

        SmcRevision[Index] = ((Temp & 0x0F) | 0x30);
        ++Index;
      }

      Temp = Rev[4];

      if (Temp < 0x10) {
        SmcRevision[Index] = (Temp + 0x30);
        ++Index;
      } else {
        SmcRevision[Index] = ((Temp >> 4) | 0x30);
        ++Index;

        SmcRevision[Index] = ((Temp & 0x0F) | 0x30);
        ++Index;
      }

      Temp = Rev[5];

      if (Temp < 0x10) {
        SmcRevision[Index] = (Temp + 0x30);
        ++Index;
      } else {
        SmcRevision[Index] = ((Temp >> 4) | 0x30);
        ++Index;

        SmcRevision[Index] = ((Temp & 0x0F) | 0x30);
        ++Index;
      }
    }
  }
}

// InternalGetMachinePersonality
STATIC
EFI_STATUS
InternalGetMachinePersonality (
  OUT CHAR16  **MachinePersonality,
  OUT UINTN   *DataSize
  )
{
  EFI_STATUS                 Status;

  APPLE_DIAG_ACCESS_PROTOCOL *DiagAccess;
  UINTN                      Size;
  CHAR16                     *Personality;

  Personality = NULL;

  Status = gBS->LocateProtocol (
                  &gAppleDiagAccessProtocolGuid,
                  NULL,
                  (VOID **)&DiagAccess
                  );

  if (!EFI_ERROR (Status)) {
    Size   = 0;
    Status = DiagAccess->Get (L"personality", NULL, &Size);

    if (Status == EFI_BUFFER_TOO_SMALL) {
      Personality = AllocatePool (Size);

      if (Personality != NULL) {
        Status = DiagAccess->Get (L"personality", (VOID *)Personality, &Size);

        if (EFI_ERROR (Status)) {
          FreePool ((VOID *)Personality);
        }
      }
    }

    // BUG: If the DiagAccess protocol call will return EFI_SUCCESS with
    // DataSize being 0, NULL will be passed to StrLen ().  The behavior will
    // be undefined.

    if (!EFI_ERROR (Status)) {
      *DataSize = (StrLen (Personality) * sizeof (CHAR16));
    }
  }

  *MachinePersonality = Personality;

  return Status;
}

// InternalGetFsbFrequency
STATIC
EFI_STATUS
InternalGetFsbFrequency (
  OUT VOID    **FsbFrequency,
  OUT UINT32  *DataSize
  )
{
  EFI_STATUS Status;

  UINT32     Size;
  VOID       *FsbHob;
  VOID       *FsbFrequencyData;

  FsbFrequencyData = NULL;

  Size   = 0;
  Status = mPlatformInfo->GetFirstDataSize (
                            mPlatformInfo,
                            &gAppleFsbFrequencyPlatformInfoGuid,
                            &Size
                            );

  if (EFI_ERROR (Status)) {
    FsbHob = GetFirstGuidHob (NULL);

    if (FsbHob != NULL) {
      Status = mPlatformInfo->GetDataSize (
                                mPlatformInfo,
                                &gAppleFsbFrequencyPlatformInfoIndexHobGuid,
                                *(UINT8 *)GET_GUID_HOB_DATA (FsbHob),
                                &Size
                                );

      if (!EFI_ERROR (Status)) {
        FsbFrequencyData = AllocateZeroPool (Size);

        if (FsbFrequencyData != NULL) {
          Status = mPlatformInfo->GetData (
                                    mPlatformInfo,
                                    &gAppleFsbFrequencyPlatformInfoIndexHobGuid,
                                    *(UINT8 *)GET_GUID_HOB_DATA (FsbHob),
                                    FsbFrequencyData,
                                    &Size
                                    );
        }
      } else {
        Status = EFI_SUCCESS;
      }
    }
  } else {
    FsbFrequencyData = AllocateZeroPool (Size);

    if (FsbFrequencyData != NULL) {
      Status = mPlatformInfo->GetFirstData (
                                mPlatformInfo,
                                &gAppleFsbFrequencyPlatformInfoIndexHobGuid,
                                FsbFrequencyData,
                                &Size
                                );
    }
  }

  *FsbFrequency = FsbFrequencyData;
  *DataSize     = Size;

  return Status;
}

// InternalGetBoardRevision
STATIC
EFI_STATUS
InternalGetBoardRevision (
  OUT UINT8  *BoardRevision
  )
{
  UINTN      DataSize;

  DataSize = sizeof (*BoardRevision);

  return gRT->GetVariable (
                L"HW_BREV",
                &gAppleVendorVariableGuid,
                NULL,
                &DataSize,
                (VOID *)BoardRevision
                );
}

// InternalGetDevicePathsSupported
STATIC
VOID
InternalGetDevicePathsSupported (
  OUT UINT32  *DevicePathsSupported
  )
{
  *DevicePathsSupported = 1;
}

// InternalGetStartupPowerEvents
STATIC
VOID
InternalGetStartupPowerEvents (
  OUT UINT64  *StartupPowerEvents
  )
{
  // TODO: Implement
  *StartupPowerEvents = 0;
}

// InternalGetInitialTsc
STATIC
VOID
InternalGetInitialTsc (
  OUT UINT64  *InitialTsc
  )
{
  *InitialTsc = 0;
}

// InternalGetDevicePathsSupported
STATIC
VOID
InternalGetCoprocessorVersion (
  OUT UINT32  *CoprocessorVersion
)
{
  *CoprocessorVersion = 0x020000;
}

// AppleSmbiosMain
EFI_STATUS
EFIAPI
AppleSmbiosMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                     Status;

  APPLE_PRODUCT_INFO             *ProductInfo;
  CHAR8                          ProductName[64];
  CHAR8                          Family[64];
  UINTN                          DataSize;
  CHAR8                          *BoardId;
  CHAR8                          *AsciiSerialNumber;
  CHAR16                         *SerialNumber;
  CHAR8                          *BoardSerialNumber;
  EFI_GUID                       SystemId;
  CHAR8                          *BiosVersion;
  CHAR8                          *BiosReleaseDate;
  CHAR8                          *RomInfo;
  EFI_SMBIOS_HANDLE              SmbiosHandle;
  EFI_SMBIOS_HANDLE              ChassisSmbiosHandle;
  APPLE_SMBIOS_STRUCTURE_POINTER SmbiosTable;
  APPLE_SMBIOS_TABLE_TYPE128     FirmwareInfo;
  APPLE_SMBIOS_TABLE_TYPE134     SmcInfo;
  CHAR16                         *MachinePersonality;
  VOID                           *FsbFrequency;
  UINT32                         FsbSize;
  UINT32                         DevicePathsSupported;
  CHAR8                          *BoardIdCopy;
  UINT8                          BoardRevision;
  UINT64                         InitialTsc;
  UINT32                         CoprocessorVersion;
  UINT64                         StartupPowerEvents;

  Status = SmbiosInstallTables ();

  if (!EFI_ERROR (Status)) {
    InternalGetProductInfo (&ProductInfo);

    if (ProductInfo != NULL) {
      UnicodeStrToAsciiStr (
        &ProductInfo->Model[0],
        ProductName
        );

      UnicodeStrToAsciiStr (
        &ProductInfo->Family[0],
        Family
        );
    } else {
      ZeroMem ((VOID *)&ProductName[0], sizeof (ProductName));
      ZeroMem ((VOID *)&Family[0], sizeof (Family));
    }

    InternalGetBoardId (&BoardId);
    InternalGetProductSerialNumber (&AsciiSerialNumber, &SerialNumber);

    if (AsciiSerialNumber != NULL) {
      DataSize = AsciiStrSize (AsciiSerialNumber);

      gRT->SetVariable (
             L"SSN",
             &gAppleVendorVariableGuid,
             (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS),
             DataSize,
             (VOID *)AsciiSerialNumber
             );
    }
    //
    // TODO: HW_MLB is read on other firmwares than IMP apparently.  Verify.
    //
    InternalGetBoardSerialNumber (&BoardSerialNumber);

    if (BoardSerialNumber != NULL) {
      DataSize = AsciiStrSize (BoardSerialNumber);

      gRT->SetVariable (
             L"HW_MLB",
             &gAppleVendorVariableGuid,
             (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS),
             DataSize,
             (VOID *)BoardSerialNumber
             );
    }

    InternalGetSystemId (&SystemId);
    GetBiosVersionDateTime (&BiosVersion, &BiosReleaseDate);
    GetRomInfo (&RomInfo);

    SmbiosTable.Standard.Hdr = SmbiosGetFirstHandle (
                        EFI_SMBIOS_TYPE_BIOS_INFORMATION,
                        &SmbiosHandle
                        );

    if (SmbiosTable.Standard.Hdr != NULL) {
      SmbiosTable.Standard.Type0->BiosSize = 0xFF;

      SmbiosUpdateString (
        &SmbiosHandle,
        SmbiosTable.Standard.Type0->BiosVersion,
        BiosVersion
        );

      SmbiosGetRecord (SmbiosHandle);

      SmbiosUpdateString (
        &SmbiosHandle,
        SmbiosTable.Standard.Type0->BiosReleaseDate,
        BiosReleaseDate
        );
    }

    SmbiosTable.Standard.Hdr = SmbiosGetFirstHandle (
                        EFI_SMBIOS_TYPE_SYSTEM_INFORMATION,
                        &SmbiosHandle
                        );

    if (SmbiosTable.Standard.Hdr != NULL) {
      SmbiosUpdateString (
        &SmbiosHandle,
        SmbiosTable.Standard.Type1->ProductName,
        &ProductName[0]
        );

      SmbiosGetRecord (SmbiosHandle);

      SmbiosUpdateString (
        &SmbiosHandle,
        SmbiosTable.Standard.Type1->Family,
        &Family[0]
        );

      SmbiosGetRecord (SmbiosHandle);

      SmbiosUpdateString (
        &SmbiosHandle,
        SmbiosTable.Standard.Type1->SerialNumber,
        AsciiSerialNumber
        );

      SmbiosGetRecord (SmbiosHandle);

      SmbiosTable.Standard.Type1->Uuid = SystemId;
    }

    SmbiosTable.Standard.Hdr = SmbiosGetFirstHandle (
                        EFI_SMBIOS_TYPE_SYSTEM_ENCLOSURE,
                        &ChassisSmbiosHandle
                        );

    if (SmbiosTable.Standard.Hdr != NULL) {
      SmbiosUpdateString (
        &ChassisSmbiosHandle,
        SmbiosTable.Standard.Type3->SerialNumber,
        AsciiSerialNumber
        );

      SmbiosGetRecord (ChassisSmbiosHandle);

      SmbiosUpdateString (
        &ChassisSmbiosHandle,
        SmbiosTable.Standard.Type3->Version,
        BoardId
        );
    }

    SmbiosTable.Standard.Hdr = SmbiosGetFirstHandle (
                        EFI_SMBIOS_TYPE_BASEBOARD_INFORMATION,
                        &SmbiosHandle
                        );

    if (SmbiosTable.Standard.Hdr != NULL) {
      SmbiosTable.Standard.Type2->ChassisHandle = ChassisSmbiosHandle;

      SmbiosUpdateString (
        &SmbiosHandle,
        SmbiosTable.Standard.Type2->Version,
        &ProductName[0]
        );

      SmbiosGetRecord (SmbiosHandle);

      SmbiosUpdateString (
        &SmbiosHandle,
        SmbiosTable.Standard.Type2->ProductName,
        BoardId
        );

      SmbiosUpdateString (
        &SmbiosHandle,
        SmbiosTable.Standard.Type2->SerialNumber,
        BoardSerialNumber
        );
    }

    if (RomInfo != NULL) {
      SmbiosTable.Standard.Hdr = SmbiosGetFirstHandle (
                          APPLE_SMBIOS_TYPE_FIRMWARE_INFORMATION,
                          &SmbiosHandle
                          );

      if (SmbiosTable.Standard.Hdr != NULL) {
        SmbiosUpdateString (
          &SmbiosHandle,
          1,
          RomInfo
          );
      }
    }

    InternalGetFirmwareInfo (&FirmwareInfo);
    SmbiosAdd (&FirmwareInfo.Hdr);

    SmbiosTable.Standard.Hdr = SmbiosGetFirstHandle (
                        APPLE_SMBIOS_TYPE_PROCESSOR_TYPE,
                        &SmbiosHandle
                        );

   if (SmbiosTable.Standard.Hdr != NULL) {
      InternalGetProcessorClass (
        &SmbiosTable.Type131->ProcessorType.Detail.MajorType
        );
    }

    ZeroMem ((VOID *)&SmcInfo, sizeof (SmcInfo));

    SmcInfo.Hdr.Type   = APPLE_SMBIOS_TYPE_SMC_INFORMATION;
    SmcInfo.Hdr.Length = sizeof (SmcInfo);

    InternalGetSmcVersion (&SmcInfo.SmcVersion[0]);

    SmbiosAdd (&SmcInfo.Hdr);

    DataSize = sizeof (mRomRecord.Rom);
    Status   = gRT->GetVariable (
                      L"HW_ROM",
                      &gAppleVendorVariableGuid,
                      NULL,
                      &DataSize,
                      (VOID *)&mRomRecord.Rom
                      );

    DataHubLogData (
      &gAppleRomDataRecordGuid,
      &gAppleRomProducerNameGuid,
      (VOID *)&mRomRecord.Header,
      sizeof (mRomRecord)
      );

    DataSize = (StrLen (SerialNumber) * sizeof (*SerialNumber));

    DataHubLogApplePlatformData (
      L"SystemSerialNumber",
      &gAppleSystemSerialNumbrDataRecordGuid,
      (VOID *)SerialNumber,
      DataSize
      );

    DataHubLogApplePlatformData (
      L"system-id",
      &gAppleSystemIdDataRecordGuid,
      (VOID *)&SystemId,
      sizeof (SystemId)
      );

    Status = InternalGetMachinePersonality (&MachinePersonality, &DataSize);

    if (!EFI_ERROR (Status)) {
      DataHubLogApplePlatformData (
        L"personality",
        &gAppleMachinePersonalityDataRecordGuid,
        (VOID *)&MachinePersonality[0],
        DataSize
        );
    }

    Status = InternalGetFsbFrequency (&FsbFrequency, &FsbSize);

    if (!EFI_ERROR (Status)) {
      DataHubLogApplePlatformData (
        L"FSBFrequency",
        &gAppleFsbFrequencyDataRecordGuid,
        FsbFrequency,
        (UINTN)FsbSize
        );
    }

    if (ProductInfo != NULL) {
      DataSize = (StrLen (ProductInfo->Model) * sizeof (CHAR16));

      DataHubLogApplePlatformData (
        L"Model",
        &gAppleModelDataRecordGuid,
        (VOID *)ProductInfo->Model,
        DataSize
        );
    }

    InternalGetDevicePathsSupported (&DevicePathsSupported);

    DataHubLogApplePlatformData (
      L"DevicePathsSupported",
      &gAppleDevicePathsSupportedDataRecordGuid,
      (VOID *)&DevicePathsSupported,
      sizeof (DevicePathsSupported)
      );

    if (BoardId != NULL) {
      DataSize    = AsciiStrSize (BoardId);
      BoardIdCopy = AllocateCopyPool (DataSize, (CONST VOID *)BoardId);

      if (BoardIdCopy != NULL) {
        // BUG: DataSize has already been calculated.
        DataHubLogApplePlatformData (
          L"board-id",
          &gAppleBoardIdDataRecordGuid,
          (VOID *)BoardIdCopy,
          AsciiStrSize (BoardIdCopy)
          );
      }
    }

    Status = InternalGetBoardRevision (&BoardRevision);

    if (!EFI_ERROR (Status)) {
      DataHubLogApplePlatformData (
        L"board-rev",
        &gAppleBoardRevisionDataRecordGuid,
        (VOID *)&BoardRevision,
        sizeof (BoardRevision)
        );
    }

    InternalGetStartupPowerEvents (&StartupPowerEvents);

    DataHubLogApplePlatformData (
      L"StartupPowerEvents",
      &gAppleStartupPowerEventsDataRecordGuid,
      (VOID *)&StartupPowerEvents,
      sizeof (StartupPowerEvents)
      );

    InternalGetInitialTsc (&InitialTsc);

    DataHubLogApplePlatformData (
      L"InitialTSC",
      &gAppleInitialTscDataRecordGuid,
      (VOID *)&InitialTsc,
      sizeof (InitialTsc)
      );

    // BUG: BoardIdCopy is not freed.

    if (MachinePersonality != NULL) {
      FreePool ((VOID *)MachinePersonality);
    }

    if (FsbFrequency != NULL) {
      FreePool ((VOID *)FsbFrequency);
    }

    if (RomInfo != NULL) {
      FreePool ((VOID *)RomInfo);
    }

    if (ProductInfo != NULL) {
      FreePool ((VOID *)ProductInfo);
    }

    if (BoardId != NULL) {
      FreePool ((VOID *)BoardId);
    }

    if (AsciiSerialNumber != NULL) {
      FreePool ((VOID *)AsciiSerialNumber);
    }

    if (SerialNumber != NULL) {
      FreePool ((VOID *)SerialNumber);
    }

    if (BoardSerialNumber != NULL) {
      FreePool ((VOID *)BoardSerialNumber);
    }

    if (BiosVersion != NULL) {
      FreePool ((VOID *)BiosVersion);
    }

    if (BiosReleaseDate != NULL) {
      FreePool ((VOID *)BiosReleaseDate);
    }

    InternalGetCoprocessorVersion (&CoprocessorVersion);

    DataHubLogApplePlatformData (
      L"apple-coprocessor-version",
      &gAppleCoprocessorVersionDataRecordGuid,
      (VOID *)&CoprocessorVersion,
      sizeof (CoprocessorVersion)
      );

    Status = EFI_SUCCESS;
  }

  return Status;
}
