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

#include <IndustryStandard/AppleSmc.h>

#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>

// SmcReadKeyStatusMmio 
SMC_STATUS
SmcReadKeyStatusMmio (
  IN UINTN  BaseAddress
  )
{
  return (SMC_STATUS)MmioRead8 (BaseAddress + SMC_MMIO_READ_KEY_STATUS);
}

// SmcReadResultMmio
SMC_RESULT
SmcReadResultMmio (
  IN UINTN  BaseAddress
  )
{
  return (SMC_RESULT)MmioRead8 (BaseAddress + SMC_MMIO_READ_RESULT);
}

// SmcWriteCommandMmio
UINT8
SmcWriteCommandMmio (
  IN UINTN   BaseAddress,
  IN UINT32  Command
  )
{
  return MmioWrite8 ((BaseAddress + SMC_MMIO_WRITE_COMMAND), (UINT8)Command);
}

// SmcWriteAttributesMmio
UINT8
SmcWriteAttributesMmio (
  IN UINTN   BaseAddress,
  IN UINT32  Attributes
  )
{
  return MmioWrite8 (
           (BaseAddress + SMC_MMIO_WRITE_KEY_ATTRIBUTES),
           (UINT8)Attributes
           );
}

// SmcReadDataSizeMmio
SMC_DATA_SIZE
SmcReadDataSizeMmio (
  IN SMC_ADDRESS  BaseAddress
  )
{
  return (SMC_DATA_SIZE)MmioRead8 (
                          (UINTN)(BaseAddress + SMC_MMIO_READ_DATA_SIZE)
                          );
}

// SmcWriteDataSizeMmio
UINT8
SmcWriteDataSizeMmio (
  IN SMC_ADDRESS  BaseAddress,
  IN UINT32       Size
  )
{

  MmioWrite8 ((UINTN)(BaseAddress + SMC_MMIO_WRITE_DATA_SIZE), (UINT8)Size);

  return 0;
}

// SmcReadData8Mmio
SMC_DATA
SmcReadData8Mmio (
  IN SMC_ADDRESS  Address
  )
{
  return (SMC_DATA)MmioRead8 ((UINTN)Address);
}

// SmcWriteData8Mmio
UINT8
SmcWriteData8Mmio (
  IN UINTN     Address,
  IN SMC_DATA  Data
  )
{
  return MmioWrite8 (Address, (UINT8)Data);
}

// SmcWriteData32Mmio
EFI_STATUS
SmcWriteData32Mmio (
  IN SMC_ADDRESS  Address,
  IN UINT32       Data
  )
{
  SmcWriteData8Mmio (
    (UINTN)(Address + SMC_MMIO_DATA_FIXED),
    (SMC_DATA)(Data >> 24)
    );

  SmcWriteData8Mmio (
    (UINTN)(Address + SMC_MMIO_DATA_FIXED + (1 * sizeof (SMC_DATA))),
    (SMC_DATA)(Data >> 16)
    );

  SmcWriteData8Mmio (
    (UINTN)(Address + SMC_MMIO_DATA_FIXED + (2 * sizeof (SMC_DATA))),
    (SMC_DATA)(Data >> 8)
    );

  SmcWriteData8Mmio (
    (UINTN)(Address + SMC_MMIO_DATA_FIXED + (3 * sizeof (SMC_DATA))),
    (SMC_DATA)Data
    );

  return EFI_SUCCESS;
}

// SmcReadData32Mmio
UINT32
SmcReadData32Mmio (
  IN UINTN  Address
  )
{
  UINT32 Data;

  Data  = (MmioRead8 (Address + SMC_MMIO_DATA_VARIABLE) << 24);

  Data |= (MmioRead8 (
             Address + SMC_MMIO_DATA_VARIABLE + (1 * sizeof (UINT8))
             ) << 16);

  Data |= (MmioRead8 (
             Address + SMC_MMIO_DATA_VARIABLE + (2 * sizeof (UINT8))
             ) << 8);

  Data |= (MmioRead8 (
             Address + SMC_MMIO_DATA_VARIABLE + (3 * sizeof (UINT8))
             ));

  return Data;
}

// TimeoutWaitingForStatusFlagClearMmio
EFI_STATUS
TimeoutWaitingForStatusFlagClearMmio (
  IN SMC_ADDRESS  BaseAddress,
  IN UINT32       Flag,
  IN UINTN        Iterations
  )
{
  EFI_STATUS Status;

  SMC_STATUS SmcStatus;

  SmcStatus = SmcReadKeyStatusMmio ((UINTN)BaseAddress);

  while (TRUE) {
    Status = EFI_SUCCESS;

    if ((Flag & SmcStatus) == 0) {
      break;
    }

    Status = EFI_TIMEOUT;
    --Iterations;

    if (Iterations <= 0) {
      break;
    }

    MicroSecondDelay (100);

    SmcStatus = SmcReadKeyStatusMmio ((UINTN)BaseAddress);
  }

  return Status;
}

// TimeoutWaitingForStatusFlagSetMmio
EFI_STATUS
TimeoutWaitingForStatusFlagSetMmio (
  IN SMC_ADDRESS  BaseAddress,
  IN UINT32       Flag,
  IN UINTN        Iterations
  )
{
  EFI_STATUS Status;

  SMC_STATUS SmcStatus;

  SmcStatus = SmcReadKeyStatusMmio ((UINTN)BaseAddress);

  while (TRUE) {
    Status = EFI_SUCCESS;

    if ((SmcStatus & Flag) != 0) {
      break;
    }

    Status = EFI_TIMEOUT;
    --Iterations;

    if (Iterations <= 0) {
      break;
    }

    MicroSecondDelay (100);

    SmcStatus = SmcReadKeyStatusMmio ((UINTN)BaseAddress);
  }

  return Status;
}

// sub_5FB
EFI_STATUS
sub_5FB (
  IN SMC_ADDRESS  BaseAddress,
  IN UINTN        Iterations
  )
{
  EFI_STATUS Status;

  SMC_STATUS SmcStatus;
  UINT32     MaskedStatus;

  SmcStatus = SmcReadKeyStatusMmio ((UINTN)BaseAddress);

  if (SmcStatus >= SMC_STATUS_UKN_0x40) {
    SmcWriteCommandMmio ((UINTN)BaseAddress, SmcCmdUnknown1);

    Status = EFI_TIMEOUT;

    while ((SmcStatus & SMC_STATUS_KEY_DONE) == 0) {
      MaskedStatus = (SmcStatus & (SMC_STATUS_UKN_0x40 | SMC_STATUS_UKN_0x80));

      while (Iterations == 0) {
        if (MaskedStatus != 0) {
          goto Done;
        } else {
          CpuDeadLoop ();
        }
      }

      MicroSecondDelay (100);

      --Iterations;
      SmcStatus = SmcReadKeyStatusMmio ((UINTN)BaseAddress);
    }
  }

  SmcStatus = SmcReadKeyStatusMmio ((UINTN)BaseAddress);

  Status = EFI_SUCCESS;

  if ((SmcStatus & SMC_STATUS_KEY_DONE) != 0) {
    SmcReadResultMmio ((UINTN)BaseAddress);
  }

Done:
  return Status;
}

// ClearArbitration
EFI_STATUS
ClearArbitration (
  IN SMC_ADDRESS  BaseAddress
  )
{
  EFI_STATUS Status;

  Status = TimeoutWaitingForStatusFlagClearMmio (
             BaseAddress,
             (SMC_STATUS_UKN_0x16
               | SMC_STATUS_KEY_DONE
               | SMC_STATUS_UKN_0x40
               | SMC_STATUS_UKN_0x80),
             1000
             );

  if (EFI_ERROR (Status)) {
    Status = sub_5FB (BaseAddress, 1000);

    if (!EFI_ERROR (Status)) {
      Status = TimeoutWaitingForStatusFlagClearMmio (
                 BaseAddress,
                 (SMC_STATUS_UKN_0x16
                   | SMC_STATUS_KEY_DONE
                   | SMC_STATUS_UKN_0x40
                   | SMC_STATUS_UKN_0x80),
                 1000
                 );
    }
  }

  return Status;
}

// WaitForKeyDone
EFI_STATUS
WaitForKeyDone (
  IN SMC_ADDRESS  BaseAddress
  )
{
  return TimeoutWaitingForStatusFlagSetMmio (
           BaseAddress,
           SMC_STATUS_KEY_DONE,
           1000
           );
}

// WaitLongForKeyDone
EFI_STATUS
WaitLongForKeyDone (
  IN SMC_ADDRESS  BaseAddress
  )
{
  return TimeoutWaitingForStatusFlagSetMmio (
           BaseAddress,
           SMC_STATUS_KEY_DONE,
           100000
           );
}

// SmcReadValueMmio
EFI_STATUS
SmcReadValueMmio (
  IN     SMC_ADDRESS    BaseAddress,
  IN     SMC_KEY        Key,
  IN OUT SMC_DATA_SIZE  *Size,
  OUT    SMC_DATA       *Value
  )
{
  EFI_STATUS    Status;

  SMC_RESULT    Result;
  SMC_DATA_SIZE KeySize;
  UINT8         Index;

  Status = ClearArbitration (BaseAddress);

  if (!EFI_ERROR (Status)) {
    SmcWriteData32Mmio (BaseAddress, (UINT32)Key);
    SmcWriteAttributesMmio ((UINTN)BaseAddress, 0);
    SmcWriteCommandMmio ((UINTN)BaseAddress, SmcCmdReadValue);

    Status = WaitForKeyDone (BaseAddress);

    if (!EFI_ERROR (Status)) {
      Result = SmcReadResultMmio ((UINTN)BaseAddress);

      if (!SMC_ERROR (Result)) {
        KeySize = SmcReadDataSizeMmio (BaseAddress);
        *Size   = KeySize;

        if ((KeySize > SMC_MAX_DATA_SIZE) || (KeySize <= 0)) {
          Status = EFI_SMC_INVALID_SIZE;

          goto Done;
        }

        Index = 0;

        do {
          Value[Index] = SmcReadData8Mmio (
                           BaseAddress + SMC_MMIO_DATA_VARIABLE + Index
                           );

          ++Index;
        } while (Index < *Size);
      }
    }
  }

  if (Status == EFI_TIMEOUT) {
    Status = EFI_SMC_TIMEOUT_ERROR;
  } else if (Status == SmcInvalidSize) {
    Status = EFI_SMC_INVALID_SIZE;
  } else {
    Status = EFI_STATUS_FROM_SMC_RESULT (Status);
  }

Done:
  return Status;
}

// SmcWriteValueMmio
EFI_STATUS
SmcWriteValueMmio (
  IN SMC_ADDRESS  BaseAddress,
  IN SMC_KEY      Key,
  IN UINT32       Size,
  IN SMC_DATA     *Value
  )
{
  EFI_STATUS Status;

  UINTN      Index;
  SMC_RESULT Result;

  Status = EFI_INVALID_PARAMETER;

  if (((SMC_DATA_SIZE)Size > 0)
   && ((SMC_DATA_SIZE)Size <= SMC_MAX_DATA_SIZE)
   && (Value != NULL)) {
    ClearArbitration (BaseAddress);

    Index  = 0;

    do {
      SmcWriteData8Mmio ((UINTN)BaseAddress, Value[Index]);

      ++Index;
    } while (Index < (SMC_DATA_SIZE)Size);

    SmcWriteData32Mmio (BaseAddress, (UINT32)Key);
    SmcWriteAttributesMmio (BaseAddress, 0);
    SmcWriteDataSizeMmio (BaseAddress, (UINT32)(SMC_DATA_SIZE)Size);
    SmcWriteCommandMmio ((UINTN)BaseAddress, SmcCmdWriteValue);

    Status = WaitForKeyDone (BaseAddress);
    Result = SmcReadResultMmio ((UINTN)BaseAddress);
    Status = ((Status == EFI_TIMEOUT)
               ? EFI_SMC_TIMEOUT_ERROR
               : EFI_STATUS_FROM_SMC_RESULT (Result));
  }

  return Status;
}

// SmcGetKeyFromIndexMmio
EFI_STATUS
SmcGetKeyFromIndexMmio (
  IN SMC_ADDRESS    BaseAddress,
  IN SMC_KEY_INDEX  Index,
  IN SMC_KEY        *Key
  )
{
  EFI_STATUS Status;

  SMC_RESULT Result;

  Status = EFI_INVALID_PARAMETER;

  if (Index > 0) {
    Status = ClearArbitration (BaseAddress);

    if (!EFI_ERROR (Status)) {
      SmcWriteData32Mmio (BaseAddress, (UINT32)Index);
      SmcWriteCommandMmio ((UINTN)BaseAddress, SmcCmdGetKeyFromIndex);
      
      Status = WaitForKeyDone (BaseAddress);

      if (!EFI_ERROR (Status)) {
        Result = SmcReadResultMmio ((UINTN)BaseAddress);

        if (!SMC_ERROR (Result)) {
          *Key = (SMC_KEY)SmcReadData32Mmio (
                            (UINTN)BaseAddress + SMC_MMIO_READ_KEY
                            );
        }

        goto ReturnResult;
      }
    }

    Result = SmcSuccess;

    if (Status == EFI_TIMEOUT) {
      Status = EFI_SMC_TIMEOUT_ERROR;
    } else {
    ReturnResult:
      Status = EFI_STATUS_FROM_SMC_RESULT (Result);
    }
  }

  return Status;
}

// SmcGetkeyInfoMmio
EFI_STATUS
SmcGetKeyInfoMmio (
  IN SMC_ADDRESS         BaseAddress,
  IN SMC_KEY             Key,
  IN SMC_DATA_SIZE       *Size,
  IN SMC_KEY_TYPE        *Type,
  IN SMC_KEY_ATTRIBUTES  *Attributes
  )
{
  EFI_STATUS Status;

  SMC_RESULT Result;

  Status = EFI_INVALID_PARAMETER;

  if ((Size != NULL) && (Type != NULL) && (Attributes != NULL)) {
    Status = ClearArbitration (BaseAddress);

    if (!EFI_ERROR (Status)) {
      SmcWriteData32Mmio (BaseAddress, (UINT32)Key);
      SmcWriteCommandMmio (BaseAddress, SmcCmdGetKeyInfo);

      Status = WaitForKeyDone (BaseAddress);

      if (!EFI_ERROR (Status)) {
        Result = SmcReadResultMmio (BaseAddress);

        if (!SMC_ERROR (Result)) {
          *Type = (SMC_KEY_TYPE)SmcReadData32Mmio (
                                  (UINTN)(BaseAddress + SMC_MMIO_READ_KEY_TYPE)
                                  );

          *Size = (SMC_DATA_SIZE)SmcReadData8Mmio (
                                   (SMC_ADDRESS)(
                                     BaseAddress + SMC_MMIO_READ_DATA_SIZE
                                     )
                                   );

          *Attributes = (SMC_KEY_ATTRIBUTES)SmcReadData8Mmio (
                                              (SMC_ADDRESS)(
                                                BaseAddress
                                                  + SMC_MMIO_READ_KEY_ATTRIBUTES
                                                )
                                              );
        }

        goto ReturnResult;
      }
    }

    Result = SmcSuccess;

    if (Status == EFI_TIMEOUT) {
      Status = EFI_SMC_TIMEOUT_ERROR;
    } else {
    ReturnResult:
      Status = EFI_STATUS_FROM_SMC_RESULT (Result);
    }
  }

  return Status;
}

// SmcFlashTypeMmio
EFI_STATUS
SmcFlashTypeMmio (
  IN SMC_ADDRESS     BaseAddress,
  IN SMC_FLASH_TYPE  Type
  )
{
  EFI_STATUS Status;

  SMC_RESULT Result;

  Status = ClearArbitration (BaseAddress);

  if (!EFI_ERROR (Status)) {
    SmcWriteData8Mmio (
      (UINTN)(BaseAddress + SMC_MMIO_DATA_VARIABLE),
      (SMC_DATA)Type
      );

    SmcWriteCommandMmio (BaseAddress, SmcCmdFlashType);
  }

  WaitForKeyDone (BaseAddress);

  Result = SmcReadResultMmio ((UINTN)BaseAddress);

  Status = ((Status == EFI_TIMEOUT)
             ? EFI_SMC_TIMEOUT_ERROR
             : EFI_STATUS_FROM_SMC_RESULT (Result));

  return Status;
}

// MmioWriteSwapped32
UINT32
MmioWriteSwapped32 (
  IN UINTN   Address,
  IN UINT32  Value
  )
{
  return MmioWrite32 (Address, SwapBytes32 (Value));
}

// SmcFlashWriteMmio
EFI_STATUS
SmcFlashWriteMmio (
  IN SMC_ADDRESS  BaseAddress,
  IN UINT32       Unknown,
  IN UINT32       Size,
  IN SMC_DATA     *Data
  )
{
  EFI_STATUS Status;

  UINT8      *SizePtr;
  UINT32     Offset;
  UINT32     TotalSize;
  UINT32     BytesWritten;
  UINT32     SizeWritten;
  SMC_RESULT Result;
  UINT32     RemainingSize;
  UINT32     IterartionDataSize;

  Status = EFI_INVALID_PARAMETER;

  if (((SMC_FLASH_SIZE)Size > 0)
   && ((SMC_FLASH_SIZE)Size <= SMC_FLASH_SIZE_MAX)
   && (Data != NULL)) {
    Status = ClearArbitration (BaseAddress);

    if (!EFI_ERROR (Status)) {
      SizePtr = (UINT8 *)&Size;

      MmioWriteSwapped32 ((UINTN)BaseAddress, Unknown);
      MmioWrite8 (((UINTN)BaseAddress + sizeof (Unknown)), SizePtr[1]);
      MmioWrite8 (
        ((UINTN)BaseAddress + sizeof (Unknown) + sizeof (SMC_FLASH_SIZE)),
        SizePtr[0]
        );

      Offset       = (sizeof (Unknown) + sizeof (Size));
      TotalSize    = (UINT32)(UINT16)(Size + sizeof (Unknown) + sizeof (Size));
      BytesWritten = 0;
      SizeWritten  = 0;

      Result = SmcSuccess;

      while ((UINT16)BytesWritten < (SMC_FLASH_SIZE)Size) {
        RemainingSize      = (TotalSize - SizeWritten);
        IterartionDataSize = SMC_MAX_DATA_SIZE;

        if (RemainingSize < SMC_MAX_DATA_SIZE) {
          IterartionDataSize = RemainingSize;
        }

        while (Offset < IterartionDataSize) {
          if (((Offset + sizeof (UINT32)) <= IterartionDataSize)
           && ((UINT32)((UINT16)BytesWritten + sizeof (UINT32)) <= Size)) {
            MmioWrite32 (
              (UINTN)(BaseAddress + SMC_MMIO_DATA_VARIABLE + Offset),
              *(UINT32 *)((UINTN)Data + BytesWritten)
              );

            BytesWritten = (UINT32)((UINT16)BytesWritten + sizeof (UINT32));
            Offset      += sizeof (UINT32);
          } else {
            MmioWrite8 (
              (UINTN)(BaseAddress + SMC_MMIO_DATA_VARIABLE + Offset),
              *(UINT8 *)((UINTN)Data + BytesWritten)
              );

            BytesWritten += sizeof (UINT8);
            Offset       += sizeof (UINT8);
          }

          Offset = (UINT32)(UINT16)Offset;
        }

        SmcWriteDataSizeMmio (BaseAddress, IterartionDataSize);
        SmcWriteCommandMmio (
          (UINTN)BaseAddress,
          ((BytesWritten <= SMC_MAX_DATA_SIZE)
            ? SmcCmdFlashWrite
            : SmcCmdFlashWriteMoreData)
          );

        Status = WaitLongForKeyDone (BaseAddress);

        if (EFI_ERROR (Status)) {
          break;
        }

        Result = SmcReadResultMmio (BaseAddress);

        if (Result != SmcSuccess) {
          break; // This jumps to the else-branch, though is likely a break.
        }

        SizeWritten += (UINT32)(UINT16)IterartionDataSize;
        Offset       = 0;
      }

      if (Status == EFI_TIMEOUT) {
        Status = EFI_SMC_TIMEOUT_ERROR;
      } else {
        Status = EFI_STATUS_FROM_SMC_RESULT (Result);
      }
    }
  }

  return Status;
}

// SmcFlashAuthMmio 
EFI_STATUS
SmcFlashAuthMmio (
  IN SMC_ADDRESS  BaseAddress,
  IN UINT32       Size,
  IN SMC_DATA     *Data
  )
{
  EFI_STATUS Status;

  UINT8      *SizePtr;
  UINT32     Offset;
  UINT32     TotalSize;
  UINT32     BytesWritten;
  UINT32     SizeWritten;
  SMC_RESULT Result;
  UINT32     RemainingSize;
  UINT32     IterartionDataSize;

  Status = EFI_INVALID_PARAMETER;

  if (((SMC_FLASH_SIZE)Size > 0)
   && ((SMC_FLASH_SIZE)Size <= SMC_FLASH_SIZE_MAX)
   && (Data != NULL)) {
    Status = ClearArbitration (BaseAddress);

    if (!EFI_ERROR (Status)) {
      SizePtr = (UINT8 *)&Size;

      MmioWrite8 ((UINTN)BaseAddress, SizePtr[1]);
      MmioWrite8 (
        ((UINTN)BaseAddress + sizeof (SMC_FLASH_SIZE)),
        SizePtr[0]
        );

      Offset       = sizeof (Size);
      TotalSize    = (UINT32)(UINT16)(Size + sizeof (Size));
      BytesWritten = 0;
      SizeWritten  = 0;

      Result = SmcSuccess;

      while (BytesWritten < Size) {
        RemainingSize      = (TotalSize - SizeWritten);
        IterartionDataSize = SMC_MAX_DATA_SIZE;

        if (RemainingSize < SMC_MAX_DATA_SIZE) {
          IterartionDataSize = RemainingSize;
        }

        while (Offset < IterartionDataSize) {
          if (((Offset + sizeof (UINT32)) <= IterartionDataSize)
            && ((UINT32)((UINT16)BytesWritten + sizeof (UINT32)) <= Size)) {
            MmioWrite32 (
              (UINTN)(BaseAddress + SMC_MMIO_DATA_VARIABLE + Offset),
              *(UINT32 *)((UINTN)Data + BytesWritten)
              );

            BytesWritten = (UINT32)((UINT16)BytesWritten + sizeof (UINT32));
            Offset      += sizeof (UINT32);
          } else {
            MmioWrite8 (
              (UINTN)(BaseAddress + SMC_MMIO_DATA_VARIABLE + Offset),
              *(UINT8 *)((UINTN)Data + BytesWritten)
              );

            BytesWritten += sizeof (UINT8);
            Offset       += sizeof (UINT8);
          }

          Offset = (UINT32)(UINT16)Offset;
        }

        SmcWriteDataSizeMmio (BaseAddress, IterartionDataSize);
        SmcWriteCommandMmio (
          (UINTN)BaseAddress,
          ((BytesWritten <= SMC_MAX_DATA_SIZE)
            ? SmcCmdFlashAuth
            : SmcCmdFlashAuthMoreData)
          );

        Status = WaitLongForKeyDone (BaseAddress);

        if (EFI_ERROR (Status)) {
          break;
        }

        Result = SmcReadResultMmio (BaseAddress);

        if (Result != SmcSuccess) {
          goto ReturnResult;
        }

        SizeWritten += (UINT32)(UINT16)IterartionDataSize;
        Offset       = 0;
      }

      if (Status == EFI_TIMEOUT) {
        Status = EFI_SMC_TIMEOUT_ERROR;
      } else {
      ReturnResult:
        Status = EFI_STATUS_FROM_SMC_RESULT (Result);
      }
    }
  }

  return Status;
}

// SmcMmioInterface
BOOLEAN
SmcMmioInterface (
  IN SMC_ADDRESS  BaseAddress
  )
{
  BOOLEAN       Mmio;

  SMC_STATUS    Status;
  SMC_DATA_SIZE Size;
  SMC_DATA      Value;
  EFI_STATUS    EfiStatus;

  Mmio = FALSE;

  Status = SmcReadKeyStatusMmio ((UINTN)BaseAddress);

  if (Status != 0xFF) {
    Size      = sizeof (Value);
    EfiStatus = SmcReadValueMmio (BaseAddress, SMC_KEY_LDKN, &Size, &Value);

    if (!EFI_ERROR (EfiStatus)) {
      Mmio = (Value > 1);
    }
  }

  return Mmio;
}

// SmcResetMmio
EFI_STATUS
SmcResetMmio (
  IN SMC_ADDRESS  BaseAddress,
  IN UINT32       Mode
  )
{
  EFI_STATUS Status;

  SMC_RESULT Result;
  BOOLEAN    Mmio;

  SmcReadKeyStatusMmio ((UINTN)BaseAddress);

  Status = ClearArbitration (BaseAddress);

  if (!EFI_ERROR (Status)) {
    SmcWriteData8Mmio (
      (UINTN)(BaseAddress + SMC_MMIO_DATA_VARIABLE),
      (SMC_DATA)Mode
      );

    SmcWriteCommandMmio ((UINTN)BaseAddress, SmcCmdReset);
    WaitForKeyDone (BaseAddress);
  }

  Result = SmcReadResultMmio ((UINTN)BaseAddress);

  Mmio = SmcMmioInterface (BaseAddress);

  Status = (Mmio ? EFI_STATUS_FROM_SMC_RESULT (Result) : EFI_SUCCESS);

  return Status;
}
