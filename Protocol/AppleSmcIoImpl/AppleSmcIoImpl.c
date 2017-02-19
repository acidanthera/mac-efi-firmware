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

#include <Library/AppleDriverLib.h>
#include <Library/AppleSmcMmioLib.h>

#include "AppleSmcIoImplInternal.h"

// mNumberOfKeysName
STATIC CHAR8 *mNumberOfKeysName = "#Key";

// SmcIoSmcReadValueImpl
EFI_STATUS
EFIAPI
SmcIoSmcReadValueImpl (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  IN  SMC_KEY                Key,
  IN  SMC_DATA_SIZE          Size,
  OUT SMC_DATA               *Value
  )
{
  EFI_STATUS Status;

  SMC_DEV    *SmcDev;
  SMC_RESULT Result;

  ASSERT (This != NULL);
  ASSERT (Size > 0);
  ASSERT (Value != NULL);

  Status = EFI_INVALID_PARAMETER;

  if ((Size > 0) && (Size <= SMC_MAX_DATA_SIZE) && (Value != NULL)) {
    SmcDev = SMC_DEV_FROM_THIS (This);
    Status = EfiAcquireLockOrFail (&SmcDev->Lock);

    if (!EFI_ERROR (Status)) {
      if (This->Mmio) {
        Status = SmcReadValueMmio (SMC_MMIO_BASE_ADDRESS, Key, &Size, Value);
      } else {
        Status = SmcIoSmcSmcInABadState (SmcDev);

        if (!EFI_ERROR (Status)) {
          Status = SmcIoSmcWriteCommand (SmcDev, SmcCmdReadValue);

          if (!EFI_ERROR (Status)) {
            Status = SmcIoSmcWriteData32 (SmcDev, (UINT32)Key);

            if (!EFI_ERROR (Status)) {
              Status = SmcIoSmcWriteData8 (SmcDev, (SMC_DATA)Size);

              if (!EFI_ERROR (Status)) {
                do {
                  Status = SmcIoSmcReadData8 (SmcDev, Value);
                  ++Value;

                  if (EFI_ERROR (Status)) {
                    break;
                  }

                  --Size;
                } while (Size > 0);

                if (Size == 0) {
                  Status = SmcIoSmcTimeoutWaitingForBusyClear (SmcDev);
                }
              }
            }
          }
        }

        Result = SmcIoSmcReadResult (SmcDev);

        if (Status == EFI_TIMEOUT) {
          Status = EFI_SMC_TIMEOUT_ERROR;
        } else if (Result == SmcSuccess) {
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

  ASSERT_EFI_ERROR (Status);

  return Status;
}

// SmcIoSmcWriteValueImpl
EFI_STATUS
EFIAPI
SmcIoSmcWriteValueImpl (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  IN  SMC_KEY                Key,
  IN  UINT32                 Size,
  OUT SMC_DATA               *Value
  )
{
  EFI_STATUS Status;

  SMC_DEV    *SmcDev;
  SMC_RESULT Result;

  ASSERT (This != NULL);
  ASSERT (Size > 0);
  ASSERT (Value != NULL);

  Status = EFI_INVALID_PARAMETER;

  if (((SMC_DATA_SIZE)Size > 0)
   && ((SMC_DATA_SIZE)Size <= SMC_MAX_DATA_SIZE)
   && (Value != NULL)) {
    SmcDev = SMC_DEV_FROM_THIS (This);
    Status = EfiAcquireLockOrFail (&SmcDev->Lock);

    if (!EFI_ERROR (Status)) {
      if (This->Mmio) {
        Status = SmcWriteValueMmio (
                   SMC_MMIO_BASE_ADDRESS,
                   Key,
                   (UINT32)(SMC_DATA_SIZE)Size,
                   Value
                   );
      } else {
        Status = SmcIoSmcSmcInABadState (SmcDev);

        if (!EFI_ERROR (Status)) {
          Status = SmcIoSmcWriteCommand (SmcDev, SmcCmdWriteValue);

          if (!EFI_ERROR (Status)) {
            Status = SmcIoSmcWriteData32 (SmcDev, (UINT32)Key);

            if (!EFI_ERROR (Status)) {
              Status = SmcIoSmcWriteData8 (SmcDev, (SMC_DATA)Size);

              if (!EFI_ERROR (Status)) {
                do {
                  Status = SmcIoSmcWriteData8 (SmcDev, *Value);
                  ++Value;

                  if (EFI_ERROR (Status)) {
                    break;
                  }

                  --Size;
                } while ((SMC_DATA_SIZE)Size > 0);

                if ((SMC_DATA_SIZE)Size == 0) {
                  Status = SmcIoSmcTimeoutWaitingForBusyClear (SmcDev);
                }
              }
            }
          }
        }

        Result = SmcIoSmcReadResult (SmcDev);
        Status = ((Status == EFI_TIMEOUT)
                   ? EFI_SMC_TIMEOUT_ERROR
                   : EFI_STATUS_FROM_SMC_RESULT (Result));

        EfiReleaseLock (&SmcDev->Lock);
      }
    }
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}

// SmcIoSmcMakeKeyImpl
EFI_STATUS
EFIAPI
SmcIoSmcMakeKeyImpl (
  IN  CHAR8    *Name,
  OUT SMC_KEY  *Key
  )
{
  EFI_STATUS Status;

  UINTN      Index;

  ASSERT (Name != NULL);
  ASSERT (Key != NULL);

  if ((Name != NULL) && (Key != NULL)) {
    *Key  = 0;
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
    goto Done;
  }

  Status = EFI_INVALID_PARAMETER;

Done:
  ASSERT_EFI_ERROR (Status);

  return Status;
}

// SmcGetyKeyCount
EFI_STATUS
EFIAPI
SmcIoSmcGetKeyCountImpl (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  OUT UINT32                 *Count
  )
{
  EFI_STATUS Status;

  SMC_KEY    Key;

  ASSERT (This != NULL);
  ASSERT (Count != NULL);

  Status = SmcIoSmcMakeKeyImpl (mNumberOfKeysName, &Key);

  if (!EFI_ERROR (Status)) {
    Status = SmcIoSmcReadValueImpl (This, Key, sizeof (*Count), (VOID *)Count);
  }

  return Status;
}

// SmcIoSmcGetKeyFromIndexImpl
EFI_STATUS
EFIAPI
SmcIoSmcGetKeyFromIndexImpl (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  IN  SMC_INDEX              Index,
  OUT SMC_KEY                *Key
  )
{
  EFI_STATUS Status;

  SMC_DEV    *SmcDev;
  SMC_RESULT Result;

  ASSERT (This != NULL);
  ASSERT (Key != NULL);

  Status = EFI_INVALID_PARAMETER;

  if (Key != NULL) {
    SmcDev = SMC_DEV_FROM_THIS (This);
    Status = EfiAcquireLockOrFail (&SmcDev->Lock);

    if (!EFI_ERROR (Status)) {
      if (This->Mmio) {
        Status = SmcGetKeyFromIndexMmio (SMC_MMIO_BASE_ADDRESS, Index, Key);
      } else {
        Status = SmcIoSmcSmcInABadState (SmcDev);

        if (!EFI_ERROR (Status)) {
          Status = SmcIoSmcWriteCommand (SmcDev, SmcCmdGetKeyFromIndex);

          if (!EFI_ERROR (Status)) {
            Status = SmcIoSmcWriteData32 (SmcDev, (UINT32)Index);

            if (!EFI_ERROR (Status)) {
              Status = SmcIoSmcReadData32 (SmcDev, (UINT32 *)Key);

              if (!EFI_ERROR (Status)) {
                Status = SmcIoSmcTimeoutWaitingForBusyClear (SmcDev);
              }
            }
          }
        }

        Result = SmcIoSmcReadResult (SmcDev);
        Status = ((Status == EFI_TIMEOUT)
                   ? EFI_SMC_TIMEOUT_ERROR
                   : EFI_STATUS_FROM_SMC_RESULT (Result));
      }

      EfiReleaseLock (&SmcDev->Lock);
    }
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}

// SmcIoSmcGetKeyInfoImpl
EFI_STATUS
EFIAPI
SmcIoSmcGetKeyInfoImpl (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  IN  SMC_KEY                Key,
  OUT SMC_DATA_SIZE          *Size,
  OUT SMC_KEY_TYPE           *Type,
  OUT SMC_KEY_ATTRIBUTES     *Attributes
  )
{
  EFI_STATUS Status;

  SMC_DEV    *SmcDev;
  SMC_RESULT Result;

  ASSERT (This != NULL);
  ASSERT (Size != NULL);
  ASSERT (Type != NULL);
  ASSERT (Attributes != NULL);

  Status = EFI_INVALID_PARAMETER;

  if ((Size != NULL) && (Type != NULL) && (Attributes != NULL)) {
    SmcDev = SMC_DEV_FROM_THIS (This);
    Status = EfiAcquireLockOrFail (&SmcDev->Lock);

    if (!EFI_ERROR (Status)) {
      if (This->Mmio) {
        Status = SmcGetKeyInfoMmio (
                   SMC_MMIO_BASE_ADDRESS,
                   Key,
                   Size,
                   Type,
                   Attributes
                   );
      } else {
        Status = SmcIoSmcSmcInABadState (SmcDev);

        if (!EFI_ERROR (Status)) {
          Status = SmcIoSmcWriteCommand (SmcDev, SmcCmdGetKeyInfo);

          if (!EFI_ERROR (Status)) {
            Status = SmcIoSmcWriteData32 (SmcDev, (UINT32)Key);

            if (!EFI_ERROR (Status)) {
              Status = SmcIoSmcReadData8 (SmcDev, (SMC_DATA *)Size);

              if (!EFI_ERROR (Status)) {
                Status = SmcIoSmcReadData32 (SmcDev, (UINT32 *)Type);

                if (!EFI_ERROR (Status)) {
                  Status = SmcIoSmcReadData8 (SmcDev, (SMC_DATA *)Attributes);

                  if (!EFI_ERROR (Status)) {
                    Status = SmcIoSmcTimeoutWaitingForBusyClear (SmcDev);
                  }
                }
              }
            }
          }
        }

        Result = SmcIoSmcReadResult (SmcDev);
        Status = ((Status == EFI_TIMEOUT)
                   ? EFI_SMC_TIMEOUT_ERROR
                   : EFI_STATUS_FROM_SMC_RESULT (Result));
      }

      EfiReleaseLock (&SmcDev->Lock);
    }
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}

// SmcIoSmcResetImpl
EFI_STATUS
EFIAPI
SmcIoSmcResetImpl (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINT32                 Mode
  )
{
  EFI_STATUS Status;

  SMC_DEV    *SmcDev;
  SMC_RESULT Result;

  ASSERT (This != NULL);

  SmcDev = SMC_DEV_FROM_THIS (This);
  Status = EfiAcquireLockOrFail (&SmcDev->Lock);

  if (!EFI_ERROR (Status)) {
    if (This->Mmio) {
      Status = SmcResetMmio (SMC_MMIO_BASE_ADDRESS, Mode);
    } else {
      Status = SmcIoSmcSmcInABadState (SmcDev);

      if (!EFI_ERROR (Status)) {
        Status = SmcIoSmcWriteCommand (SmcDev, SmcCmdReset);

        if (!EFI_ERROR (Status)) {
          SmcIoSmcWriteData8 (SmcDev, (SMC_DATA)Mode);

          if (!EFI_ERROR (Status)) {
            Status = SmcIoSmcTimeoutWaitingLongForBusyClear (SmcDev);
          }
        }
      }

      Result = SmcIoSmcReadResult (SmcDev);
      Status = ((Status == EFI_TIMEOUT)
                 ? EFI_SMC_TIMEOUT_ERROR
                 : EFI_STATUS_FROM_SMC_RESULT (Result));
    }

    EfiReleaseLock (&SmcDev->Lock);
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}

// SmcIoSmcFlashTypeImpl
EFI_STATUS
EFIAPI
SmcIoSmcFlashTypeImpl (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINT32                 Type
  )
{
  EFI_STATUS Status;

  SMC_DEV    *SmcDev;
  SMC_RESULT Result;

  ASSERT (This != NULL);

  SmcDev = SMC_DEV_FROM_THIS (This);
  Status = EfiAcquireLockOrFail (&SmcDev->Lock);

  if (!EFI_ERROR (Status)) {
    if (This->Mmio) {
      Status = SmcFlashTypeMmio (SMC_MMIO_BASE_ADDRESS, (SMC_FLASH_TYPE)Type);
    } else {
      Status = SmcIoSmcSmcInABadState (SmcDev);

      if (!EFI_ERROR (Status)) {
        Status = SmcIoSmcWriteCommand (SmcDev, SmcCmdFlashType);

        if (!EFI_ERROR (Status)) {
          Status = SmcIoSmcWriteData8 (SmcDev, (SMC_DATA)Type);

          if (!EFI_ERROR (Status)) {
            Status = SmcIoSmcTimeoutWaitingForBusyClear (SmcDev);
          }
        }
      }

      Result = SmcIoSmcReadResult (SmcDev);
      Status = ((Status == EFI_TIMEOUT)
                 ? EFI_SMC_TIMEOUT_ERROR
                 : EFI_STATUS_FROM_SMC_RESULT (Result));
    }

    EfiReleaseLock (&SmcDev->Lock);
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}

// SmcIoSmcFlashWriteImpl
EFI_STATUS
EFIAPI
SmcIoSmcFlashWriteImpl (
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

  ASSERT (This != NULL);
  ASSERT (Size > 0);
  ASSERT (Data != NULL);

  Status = EFI_INVALID_PARAMETER;

  if (((SMC_FLASH_SIZE)Size > 0)
   && ((SMC_FLASH_SIZE)Size <= SMC_FLASH_SIZE_MAX)
   && (Data != NULL)) {
    SmcDev = SMC_DEV_FROM_THIS (This);
    Status = EfiAcquireLockOrFail (&SmcDev->Lock);

    if (!EFI_ERROR (Status)) {
      if (This->Mmio) {
        Status = SmcFlashWriteMmio (
                   SMC_MMIO_BASE_ADDRESS,
                   Unknown,
                   Size,
                   Data
                   );
      } else {
        Status = SmcIoSmcSmcInABadState (SmcDev);

        if (!EFI_ERROR (Status)) {
          Status = SmcIoSmcWriteCommand (SmcDev, SmcCmdFlashWrite);

          if (!EFI_ERROR (Status)) {
            Status = SmcIoSmcWriteData32 (SmcDev, Unknown);

            if (!EFI_ERROR (Status)) {
              Status = SmcIoSmcWriteData16 (SmcDev, (UINT16)Size);

              if (!EFI_ERROR (Status)) {
                do {
                  Status = SmcIoSmcWriteData8 (SmcDev, *Data);

                  if (EFI_ERROR (Status)) {
                    break;
                  }

                  ++Data;
                  --Size;
                } while ((SMC_FLASH_SIZE)Size > 0);

                do {
                  Status = SmcIoSmcReadData8 (SmcDev, &Value);
                } while (Status == EFI_SUCCESS);
              }
            }
          }
        }

        Result = SmcIoSmcReadResult (SmcDev);
        Status = ((Status == EFI_TIMEOUT)
                   ? EFI_SMC_TIMEOUT_ERROR
                   : EFI_STATUS_FROM_SMC_RESULT (Result));
      }

      EfiReleaseLock (&SmcDev->Lock);
    }
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}

// SmcIoSmcFlashAuthImpl
EFI_STATUS
EFIAPI
SmcIoSmcFlashAuthImpl (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINT32                 Size,
  IN SMC_DATA               *Data
  )
{
  EFI_STATUS Status;

  SMC_DEV    *SmcDev;
  SMC_RESULT Result;
  SMC_DATA   Value;

  ASSERT (This != NULL);
  ASSERT (Size > 0);
  ASSERT (Data != NULL);

  Status = EFI_INVALID_PARAMETER;

  if (((SMC_FLASH_SIZE)Size > 0)
   && ((SMC_FLASH_SIZE)Size <= SMC_FLASH_SIZE_MAX)
   && (Data != NULL)) {
    SmcDev = SMC_DEV_FROM_THIS (This);
    Status = EfiAcquireLockOrFail (&SmcDev->Lock);

    if (!EFI_ERROR (Status)) {
      if (This->Mmio) {
        Status = SmcFlashAuthMmio (SMC_MMIO_BASE_ADDRESS, Size, Data);
      } else {
        Status = SmcIoSmcSmcInABadState (SmcDev);

        if (!EFI_ERROR (Status)) {
          Status = SmcIoSmcWriteCommand (SmcDev, SmcCmdFlashAuth);

          if (!EFI_ERROR (Status)) {
            Status = SmcIoSmcWriteData16 (SmcDev, (SMC_FLASH_SIZE)Size);

            if (!EFI_ERROR (Status)) {
              do {
                Status = SmcIoSmcWriteData8 (SmcDev, *Data);

                if (EFI_ERROR (Status)) {
                  break;
                }

                ++Data;
                --Size;
              } while ((SMC_FLASH_SIZE)Size > 0);

              do {
                Status = SmcIoSmcReadData8 (SmcDev, &Value);
              } while (Status == EFI_SUCCESS);
            }
          }
        }

        Result = SmcIoSmcReadResult (SmcDev);
        Status = ((Status == EFI_TIMEOUT)
                   ? EFI_SMC_TIMEOUT_ERROR
                   : EFI_STATUS_FROM_SMC_RESULT (Result));
      }

      EfiReleaseLock (&SmcDev->Lock);
    }
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}

// SmcIoSmcUnsupportedImpl
EFI_STATUS
EFIAPI
SmcIoSmcUnsupportedImpl (
  VOID
  )
{
  ASSERT (FALSE);

  return EFI_UNSUPPORTED;
}

// SmcIoSmcUnknown1Impl
EFI_STATUS
EFIAPI
SmcIoSmcUnknown1Impl (
  VOID
  )
{
  // TODO: implement

  return EFI_SUCCESS;
}

// SmcIoSmcUnknown2Impl
EFI_STATUS
EFIAPI
SmcIoSmcUnknown2Impl (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINTN                  Ukn1,
  IN UINTN                  Ukn2
  )
{
  EFI_STATUS Status;

  SMC_DEV    *SmcDev;

  ASSERT (This != NULL);

  SmcDev = SMC_DEV_FROM_THIS (This);
  Status = EfiAcquireLockOrFail (&SmcDev->Lock);

  if (!EFI_ERROR (Status)) {
    // Status = sub_10D4 (Ukn1, Ukn2);
    EfiReleaseLock (&SmcDev->Lock);
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}

// SmcIoSmcUnknown3Impl
EFI_STATUS
EFIAPI
SmcIoSmcUnknown3Impl (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINTN                  Ukn1,
  IN UINTN                  Ukn2
  )
{
  EFI_STATUS Status;

  SMC_DEV    *SmcDev;

  ASSERT (This != NULL);

  SmcDev = SMC_DEV_FROM_THIS (This);
  Status = EfiAcquireLockOrFail (&SmcDev->Lock);

  if (!EFI_ERROR (Status)) {
    // Status = sub_1125 (Ukn1, Ukn2);
    EfiReleaseLock (&SmcDev->Lock);
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}

// SmcIoSmcUnknown4Impl
EFI_STATUS
EFIAPI
SmcIoSmcUnknown4Impl (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINTN                  Ukn1
  )
{
  EFI_STATUS Status;

  SMC_DEV    *SmcDev;

  ASSERT (This != NULL);

  SmcDev = SMC_DEV_FROM_THIS (This);
  Status = EfiAcquireLockOrFail (&SmcDev->Lock);

  if (!EFI_ERROR (Status)) {
    // Status = sub_1181 (Ukn1);
    EfiReleaseLock (&SmcDev->Lock);
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}

// SmcIoSmcUnknown5Impl
EFI_STATUS
EFIAPI
SmcIoSmcUnknown5Impl (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINTN                  Ukn1
  )
{
  EFI_STATUS Status;

  SMC_DEV    *SmcDev;

  ASSERT (This != NULL);

  SmcDev = SMC_DEV_FROM_THIS (This);
  Status = EfiAcquireLockOrFail (&SmcDev->Lock);

  if (!EFI_ERROR (Status)) {
    // Status = sub_11BB (Ukn1);
    EfiReleaseLock (&SmcDev->Lock);
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}
