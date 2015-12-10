//
// Copyright (C) 2005 - 2015 Apple Inc. All rights reserved.
//
// This program and the accompanying materials have not been licensed.
// Neither is its usage, its redistribution, in source or binary form,
// licensed, nor implicitely or explicitely permitted, except when
// required by applicable law.
//
// Unless required by applicable law or agreed to in writing, software
// distributed is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
// OR CONDITIONS OF ANY KIND, either express or implied.
//

///
/// @file      Protocol/AppleSmcIoImpl/AppleSmcIoImpl.c
///
///            
///
/// @author    Download-Fritz
/// @date      22/10/2015: Initial version
/// @copyright Copyright (C) 2005 - 2015 Apple Inc. All rights reserved.
///

#include <AppleEfi.h>

#include <EfiDriverLib.h>

#include EFI_PROTOCOL_CONSUMER (CpuIo)
#include <Protocol/AppleSmcIoImpl.h>

#include <Library/AppleSmcMmioLib.h>

static CHAR8 *mNoKeysName = "#Key";

// SmcReadValue
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
SmcReadValue (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  IN  SMC_KEY                Key,
  IN  SMC_DATA_SIZE          Size,
  OUT SMC_DATA               *Value
  )
{
  EFI_STATUS Status;

  SMC_DEV    *SmcDev;
  SMC_RESULT Result;

  Status = EFI_INVALID_PARAMETER;

  if ((Size > 0) && (Size <= SMC_MAX_DATA_SIZE) && (Value != NULL)) {
    SmcDev = SMC_DEV_FROM_THIS (This);
    Status = EfiAcquireLockOrFail (&SmcDev->Lock);

    if (!EFI_ERROR (Status)) {
      if (This->Mmio) {
        Status = SmcReadValueMmio (SMC_MMIO_BASE_ADDRESS, Key, &Size, Value);
      } else {
        Status = SmcSmcInABadState (SmcDev);

        if (!EFI_ERROR (Status)) {
          Status = SmcWriteCommand (SmcDev, SmcCmdReadValue);

          if (!EFI_ERROR (Status)) {
            Status = SmcWriteData32 (SmcDev, (UINT32)Key);

            if (!EFI_ERROR (Status)) {
              Status = SmcWriteData8 (SmcDev, (SMC_DATA)Size);

              if (!EFI_ERROR (Status)) {
                do {
                  Status = SmcReadData8 (SmcDev, Value);
                  ++Value;

                  if (EFI_ERROR (Status)) {
                    break;
                  }

                  --Size;
                } while (Size > 0);

                if (Size == 0) {
                  Status = SmcTimeoutWaitingForBusyClear (SmcDev);
                }
              }
            }
          }
        }

        Result = SmcReadResult (SmcDev);

        if (Status == EFI_TIMEOUT) {
          Status = EFI_SMC_TIMEOUT_ERROR;
        } else if (Result == SMC_SUCCESS) {
          Status = EFI_SUCCESS;

          if (Key == SMC_MAKE_KEY ('R', 'P', 'L', 'T')) {
            if (*(UINTN *)Value == SMC_MAKE_KEY ('5', '0', '5', 'j')) {
              BYTE_PTR (Value, 2) = 0;
            }
          }
        } else {
          Status = EFIERR (Result);
        }
      }

      EfiReleaseLock (&SmcDev->Lock);
    }
  }

  return Status;
}

// SmcWriteValue
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
SmcWriteValue (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  IN  SMC_KEY                Key,
  IN  UINT32                 Size,
  OUT SMC_DATA               *Value
  )
{
  EFI_STATUS Status;

  SMC_DEV    *SmcDev;
  SMC_RESULT Result;

  Status = EFI_INVALID_PARAMETER;

  if (((SMC_DATA_SIZE)Size > 0) && ((SMC_DATA_SIZE)Size <= SMC_MAX_DATA_SIZE) && (Value != NULL)) {
    SmcDev = SMC_DEV_FROM_THIS (This);
    Status = EfiAcquireLockOrFail (&SmcDev->Lock);

    if (!EFI_ERROR (Status)) {
      if (This->Mmio) {
        Status = SmcWriteValueMmio (SMC_MMIO_BASE_ADDRESS, Key, (UINT32)(SMC_DATA_SIZE)Size, Value);
      } else {
        Status = SmcSmcInABadState (SmcDev);

        if (!EFI_ERROR (Status)) {
          Status = SmcWriteCommand (SmcDev, SmcCmdWriteValue);

          if (!EFI_ERROR (Status)) {
            Status = SmcWriteData32 (SmcDev, (UINT32)Key);

            if (!EFI_ERROR (Status)) {
              Status = SmcWriteData8 (SmcDev, (SMC_DATA)Size);

              if (!EFI_ERROR (Status)) {
                do {
                  Status = SmcWriteData8 (SmcDev, *Value);
                  ++Value;

                  if (EFI_ERROR (Status)) {
                    break;
                  }

                  --Size;
                } while ((SMC_DATA_SIZE)Size > 0);

                if ((SMC_DATA_SIZE)Size == 0) {
                  Status = SmcTimeoutWaitingForBusyClear (SmcDev);
                }
              }
            }
          }
        }

        Result = SmcReadResult (SmcDev);
        Status = ((Status == EFI_TIMEOUT) ? EFI_SMC_TIMEOUT_ERROR : EFI_STATUS_FROM_SMC_RESULT (Result));

        EfiReleaseLock (&SmcDev->Lock);
      }
    }
  }

  return Status;
}

// SmcMakeKey
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
SmcMakeKey (
  IN  CHAR8    *Name,
  OUT SMC_KEY  *Key
  )
{
  EFI_STATUS Status;

  UINTN      Index;

  if ((Name != NULL) && (Key != NULL)) {
    *Key   = 0;
    Index = 0;

    do {
      if (SMC_KEY_IS_VALID_CHAR (Name[Index])) {
        *Key <<= 8;
        *Key  |= Name[Index];
        ++Index;
      } else {
        *Key = 0;
        break;
      }
    } while (Index < (sizeof (*Key) / sizeof (Name[0])));

    Status = EFI_SUCCESS;
    goto Return;
  }

  Status = EFI_INVALID_PARAMETER;

Return:
  return Status;
}

// SmcGetyKeyCount
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
SmcGetKeyCount (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  OUT UINT32                 *Count
  )
{
  EFI_STATUS Status;

  SMC_KEY    Key;

  Status = SmcMakeKey (mNoKeysName, &Key);

  if (!EFI_ERROR (Status)) {
    Status = SmcReadValue (This, Key, sizeof (*Count), (VOID *)Count);
  }

  return Status;
}

// SmcGetKeyFromIndex
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
SmcGetKeyFromIndex (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  IN  SMC_INDEX              Index,
  OUT SMC_KEY                *Key
  )
{
  EFI_STATUS Status;

  SMC_DEV    *SmcDev;
  SMC_RESULT Result;

  Status = EFI_INVALID_PARAMETER;

  if (Key != NULL) {
    SmcDev = SMC_DEV_FROM_THIS (This);
    Status = EfiAcquireLockOrFail (&SmcDev->Lock);

    if (!EFI_ERROR (Status)) {
      if (This->Mmio) {
        Status = SmcGetKeyFromIndexMmio (SMC_MMIO_BASE_ADDRESS, Index, Key);
      } else {
        Status = SmcSmcInABadState (SmcDev);

        if (!EFI_ERROR (Status)) {
          Status = SmcWriteCommand (SmcDev, SmcCmdGetKeyFromIndex);

          if (!EFI_ERROR (Status)) {
            Status = SmcWriteData32 (SmcDev, (UINT32)Index);

            if (!EFI_ERROR (Status)) {
              Status = SmcReadData32 (SmcDev, (UINT32 *)Key);

              if (!EFI_ERROR (Status)) {
                Status = SmcTimeoutWaitingForBusyClear (SmcDev);
              }
            }
          }
        }

        Result = SmcReadResult (SmcDev);
        Status = ((Status == EFI_TIMEOUT) ? EFI_SMC_TIMEOUT_ERROR : EFI_STATUS_FROM_SMC_RESULT (Result));
      }

      EfiReleaseLock (&SmcDev->Lock);
    }
  }

  return Status;
}

// SmcGetKeyInfo
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
SmcGetKeyInfo (
  IN     APPLE_SMC_IO_PROTOCOL  *This,
  IN     SMC_KEY                Key,
  IN OUT SMC_DATA_SIZE          *Size,
  IN OUT SMC_KEY_TYPE           *Type,
  IN OUT SMC_KEY_ATTRIBUTES     *Attributes
  )
{
  EFI_STATUS Status;

  SMC_DEV    *SmcDev;
  SMC_RESULT Result;

  Status = EFI_INVALID_PARAMETER;

  if ((Size != NULL) && (Type != NULL) && (Attributes != NULL)) {
    SmcDev = SMC_DEV_FROM_THIS (This);
    Status = EfiAcquireLockOrFail (&SmcDev->Lock);

    if (!EFI_ERROR (Status)) {
      if (This->Mmio) {
        Status = SmcGetKeyInfoMmio (SMC_MMIO_BASE_ADDRESS, Key, Size, Type, Attributes);
      } else {
        Status = SmcSmcInABadState (SmcDev);

        if (!EFI_ERROR (Status)) {
          Status = SmcWriteCommand (SmcDev, SmcCmdGetKeyInfo);

          if (!EFI_ERROR (Status)) {
            Status = SmcWriteData32 (SmcDev, (UINT32)Key);

            if (!EFI_ERROR (Status)) {
              Status = SmcReadData8 (SmcDev, (SMC_DATA *)Size);

              if (!EFI_ERROR (Status)) {
                Status = SmcReadData32 (SmcDev, (UINT32 *)Type);

                if (!EFI_ERROR (Status)) {
                  Status = SmcReadData8 (SmcDev, (SMC_DATA *)Attributes);

                  if (!EFI_ERROR (Status)) {
                    Status = SmcTimeoutWaitingForBusyClear (SmcDev);
                  }
                }
              }
            }
          }
        }

        Result = SmcReadResult (SmcDev);
        Status = ((Status == EFI_TIMEOUT) ? EFI_SMC_TIMEOUT_ERROR : EFI_STATUS_FROM_SMC_RESULT (Result));
      }

      EfiReleaseLock (&SmcDev->Lock);
    }
  }

  return Status;
}

// SmcReset
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
SmcReset (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINT32                 Mode
  )
{
  EFI_STATUS Status;

  SMC_DEV    *SmcDev;
  SMC_RESULT Result;

  SmcDev = SMC_DEV_FROM_THIS (This);
  Status = EfiAcquireLockOrFail (&SmcDev->Lock);

  if (!EFI_ERROR (Status)) {
    if (This->Mmio) {
      Status = SmcResetMmio (SMC_MMIO_BASE_ADDRESS, Mode);
    } else {
      Status = SmcSmcInABadState (SmcDev);

      if (!EFI_ERROR (Status)) {
        Status = SmcWriteCommand (SmcDev, SmcCmdReset);

        if (!EFI_ERROR (Status)) {
          SmcWriteData8 (SmcDev, (SMC_DATA)Mode);

          if (!EFI_ERROR (Status)) {
            Status = SmcTimeoutWaitingLongForBusyClear (SmcDev);
          }
        }
      }

      Result = SmcReadResult (SmcDev);
      Status = ((Status == EFI_TIMEOUT) ? EFI_SMC_TIMEOUT_ERROR : EFI_STATUS_FROM_SMC_RESULT (Result));
    }

    EfiReleaseLock (&SmcDev->Lock);
  }

  return Status;
}

// SmcFlashType
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
SmcFlashType (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINT32                 Type
  )
{
  EFI_STATUS Status;

  SMC_DEV    *SmcDev;
  SMC_RESULT Result;

  SmcDev = SMC_DEV_FROM_THIS (This);
  Status = EfiAcquireLockOrFail (&SmcDev->Lock);

  if (!EFI_ERROR (Status)) {
    if (This->Mmio) {
      Status = SmcFlashTypeMmio (SMC_MMIO_BASE_ADDRESS, (SMC_FLASH_TYPE)Type);
    } else {
      Status = SmcSmcInABadState (SmcDev);

      if (!EFI_ERROR (Status)) {
        Status = SmcWriteCommand (SmcDev, SmcCmdFlashType);

        if (!EFI_ERROR (Status)) {
          Status = SmcWriteData8 (SmcDev, (SMC_DATA)Type);

          if (!EFI_ERROR (Status)) {
            Status = SmcTimeoutWaitingForBusyClear (SmcDev);
          }
        }
      }

      Result = SmcReadResult (SmcDev);
      Status = ((Status == EFI_TIMEOUT) ? EFI_SMC_TIMEOUT_ERROR : EFI_STATUS_FROM_SMC_RESULT (Result));
    }

    EfiReleaseLock (&SmcDev->Lock);
  }

  return Status;
}

// SmcFlashWrite
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
SmcFlashWrite (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINT32                 Unknown,
  IN UINT32                 Size,
  IN SMC_DATA               *Data
  )
{
  EFI_STATUS Status;

  SMC_DEV    *SmcDev;
  SMC_RESULT Result;
  SMC_DATA   Value;

  Status = EFI_INVALID_PARAMETER;

  if (((SMC_FLASH_SIZE)Size > 0) && ((SMC_FLASH_SIZE)Size <= SMC_FLASH_SIZE_MAX) && (Data != NULL)) {
    SmcDev = SMC_DEV_FROM_THIS (This);
    Status = EfiAcquireLockOrFail (&SmcDev->Lock);

    if (!EFI_ERROR (Status)) {
      if (This->Mmio) {
        Status = SmcFlashWriteMmio (SMC_MMIO_BASE_ADDRESS, Unknown, Size, Data);
      } else {
        Status = SmcSmcInABadState (SmcDev);

        if (!EFI_ERROR (Status)) {
          Status = SmcWriteCommand (SmcDev, SmcCmdFlashWrite);

          if (!EFI_ERROR (Status)) {
            Status = SmcWriteData32 (SmcDev, Unknown);

            if (!EFI_ERROR (Status)) {
              Status = SmcWriteData16 (SmcDev, (UINT16)Size);

              if (!EFI_ERROR (Status)) {
                do {
                  Status = SmcWriteData8 (SmcDev, *Data);

                  if (EFI_ERROR (Status)) {
                    break;
                  }

                  ++Data;
                  --Size;
                } while ((SMC_FLASH_SIZE)Size > 0);

                do {
                  Status = SmcReadData8 (SmcDev, &Value);
                } while (Status == EFI_SUCCESS);
              }
            }
          }
        }

        Result = SmcReadResult (SmcDev);
        Status = ((Status == EFI_TIMEOUT) ? EFI_SMC_TIMEOUT_ERROR : EFI_STATUS_FROM_SMC_RESULT (Result));
      }

      EfiReleaseLock (&SmcDev->Lock);
    }
  }

  return Status;
}

// SmcFlashAuth
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
SmcFlashAuth (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINT32                 Size,
  IN SMC_DATA               *Data
  )
{
  EFI_STATUS Status;

  SMC_DEV    *SmcDev;
  SMC_RESULT Result;
  SMC_DATA   Value;

  Status = EFI_INVALID_PARAMETER;

  if (((SMC_FLASH_SIZE)Size > 0) && ((SMC_FLASH_SIZE)Size <= SMC_FLASH_SIZE_MAX) && (Data != NULL)) {
    SmcDev = SMC_DEV_FROM_THIS (This);
    Status = EfiAcquireLockOrFail (&SmcDev->Lock);

    if (!EFI_ERROR (Status)) {
      if (This->Mmio) {
        Status = SmcFlashAuthMmio (SMC_MMIO_BASE_ADDRESS, Size, Data);
      } else {
        Status = SmcSmcInABadState (SmcDev);

        if (!EFI_ERROR (Status)) {
          Status = SmcWriteCommand (SmcDev, SmcCmdFlashAuth);

          if (!EFI_ERROR (Status)) {
            Status = SmcWriteData16 (SmcDev, (SMC_FLASH_SIZE)Size);

            if (!EFI_ERROR (Status)) {
              do {
                Status = SmcWriteData8 (SmcDev, *Data);

                if (EFI_ERROR (Status)) {
                  break;
                }

                ++Data;
                --Size;
              } while ((SMC_FLASH_SIZE)Size > 0);

              do {
                Status = SmcReadData8 (SmcDev, &Value);
              } while (Status == EFI_SUCCESS);
            }
          }
        }

        Result = SmcReadResult (SmcDev);
        Status = ((Status == EFI_TIMEOUT) ? EFI_SMC_TIMEOUT_ERROR : EFI_STATUS_FROM_SMC_RESULT (Result));
      }

      EfiReleaseLock (&SmcDev->Lock);
    }
  }

  return Status;
}

// SmcUnsupported
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
SmcUnsupported (
  VOID
  )
{
  return EFI_UNSUPPORTED;
}

//

// SmcUnknown2
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
SmcUnknown2 (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINTN                  Ukn1,
  IN UINTN                  Ukn2
  )
{
  EFI_STATUS Status;

  SMC_DEV    *SmcDev;

  SmcDev = SMC_DEV_FROM_THIS (This);
  Status = EfiAcquireLockOrFail (&SmcDev->Lock);

  if (!EFI_ERROR (Status)) {
    // Status = sub_10D4 (Ukn1, Ukn2);
    EfiReleaseLock (&SmcDev->Lock);
  }

  return Status;
}

// SmcUnknown3
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
SmcUnknown3 (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINTN                  Ukn1,
  IN UINTN                  Ukn2
  )
{
  EFI_STATUS Status;

  SMC_DEV    *SmcDev;

  SmcDev = SMC_DEV_FROM_THIS (This);
  Status = EfiAcquireLockOrFail (&SmcDev->Lock);

  if (!EFI_ERROR (Status)) {
    // Status = sub_1125 (Ukn1, Ukn2);
    EfiReleaseLock (&SmcDev->Lock);
  }

  return Status;
}

// SmcUnknown4
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
SmcUnknown4 (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINTN                  Ukn1
  )
{
  EFI_STATUS Status;

  SMC_DEV    *SmcDev;

  SmcDev = SMC_DEV_FROM_THIS (This);
  Status = EfiAcquireLockOrFail (&SmcDev->Lock);

  if (!EFI_ERROR (Status)) {
    // Status = sub_1181 (Ukn1);
    EfiReleaseLock (&SmcDev->Lock);
  }

  return Status;
}

// SmcUnknown5
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
SmcUnknown5 (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINTN                  Ukn1
  )
{
  EFI_STATUS Status;

  SMC_DEV    *SmcDev;

  SmcDev = SMC_DEV_FROM_THIS (This);
  Status = EfiAcquireLockOrFail (&SmcDev->Lock);

  if (!EFI_ERROR (Status)) {
    // Status = sub_11BB (Ukn1);
    EfiReleaseLock (&SmcDev->Lock);
  }

  return Status;
}
