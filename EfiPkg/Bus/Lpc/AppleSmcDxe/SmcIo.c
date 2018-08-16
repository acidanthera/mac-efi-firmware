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

#include <Guid/AppleHob.h>

#include <Protocol/AppleSmcIo.h>

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/HobLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "SmcIoInternal.h"

// KEY_PRESENT_MAP_UNITS
#define KEY_PRESENT_MAP_UNITS  256

// NEXT_SMC_ADDRESS
#define NEXT_SMC_ADDRESS(Address)                         \
  ((((Address) & 0xFF0000) | (((Address) >> 16) >> 8))    \
    | (((Address) & 0xFF00) | (((Address) << 16) << 8)))

#define NUMBER_OF_KEYS_KEY  "#Key"

// mSmcMmioAddress
STATIC SMC_ADDRESS mSmcMmioAddress = 0;

// mSoftwareSmc
STATIC BOOLEAN mSoftwareSmc = FALSE;

// InternalSmcGetKeyInfo
STATIC
EFI_STATUS
EFIAPI
InternalSmcGetKeyInfo (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  IN  SMC_KEY                Key,
  OUT SMC_DATA_SIZE          *Size,
  OUT SMC_KEY_TYPE           *Type,
  OUT SMC_KEY_ATTRIBUTES     *Attributes
  );

// TODO: Figure out and move.
STATIC CONST EFI_GUID gApplePhysicalSmcHobGuid =
  { 0xD1B58E22, 0x779B, 0x46AC,
    { 0x86, 0x7B, 0xF1, 0x59, 0x8D, 0x5E, 0xA0, 0x5A } };

// InternalIsKeyPresent
STATIC
BOOLEAN
InternalIsKeyPresent (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN SMC_KEY                Key
  )
{
  BOOLEAN              KeyExists;

  SMC_DEV              *SmcDev;
  UINTN                Index;
  SMC_KEY_PRESENCE_MAP *Entry;
  EFI_STATUS           Status;
  SMC_DATA_SIZE        Size;
  SMC_KEY_TYPE         Type;
  SMC_KEY_ATTRIBUTES   Attributes;
  SMC_KEY_PRESENCE_MAP *Buffer;

  SmcDev = SMC_DEV_FROM_THIS (This);

  Entry = &SmcDev->KeyPresenceMap[0];

  for (Index = 0; Index < SmcDev->KeyPresenceMapLength; ++Index) {
    if (Entry->Key == Key) {
      KeyExists = Entry->Present;

      goto Done;
    }

    ++Entry;
  }

  Status    = InternalSmcGetKeyInfo (This, Key, &Size, &Type, &Attributes);
  KeyExists = (BOOLEAN)!EFI_ERROR (Status);

  if (SmcDev->KeyPresenceMapLength >= SmcDev->MaxKeyPresenceMapLength) {
    Buffer = AllocateCopyPool (
               (SmcDev->MaxKeyPresenceMapLength + KEY_PRESENT_MAP_UNITS)
                  * sizeof (SmcDev->KeyPresenceMap),
               (CONST VOID *)SmcDev->KeyPresenceMap
               );

    if (Buffer == NULL) {
      goto Done;
    }

    FreePool ((VOID *)SmcDev->KeyPresenceMap);

    SmcDev->KeyPresenceMap           = Buffer;
    SmcDev->MaxKeyPresenceMapLength += KEY_PRESENT_MAP_UNITS;

    Entry = &SmcDev->KeyPresenceMap[SmcDev->KeyPresenceMapLength];
  }

  ++SmcDev->KeyPresenceMapLength;

  Entry->Key     = Key;
  Entry->Present = KeyExists;

Done:
  return KeyExists;
}

// InternalSmcReadValue
STATIC
EFI_STATUS
EFIAPI
InternalSmcReadValue (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  IN  SMC_KEY                Key,
  IN  SMC_DATA_SIZE          Size,
  OUT SMC_DATA               *Value
  )
{
  EFI_STATUS Status;

  BOOLEAN    KeyPresent;
  SMC_DEV    *SmcDev;
  SMC_RESULT Result;
  SMC_DATA   *ValueWalker;

  Status = EFI_INVALID_PARAMETER;

  if (mSoftwareSmc) {
    Status = SmcIoVirtualSmcReadValue (This, Key, Size, Value);
  } else {
    // BUG: This is used unchecked.

    if ((Size > 0) && (Size <= SMC_MAX_DATA_SIZE) && (Value != NULL)) {
      KeyPresent = InternalIsKeyPresent (This, Key);

      Status = EFI_NOT_FOUND;

      if (KeyPresent) {
        SmcDev = SMC_DEV_FROM_THIS (This);
        Status = EfiAcquireLockOrFail (&SmcDev->Lock);

        if (!EFI_ERROR (Status)) {
          if (This->Mmio) {
            Status = SmcReadValueMmio (mSmcMmioAddress, Key, &Size, Value);
          } else {
            Status = SmcIoSmcSmcInABadState (SmcDev);

            if (!EFI_ERROR (Status)) {
              Status = SmcIoSmcWriteCommand (SmcDev, SmcCmdReadValue);

              if (!EFI_ERROR (Status)) {
                Status = SmcIoSmcWriteData32 (SmcDev, (UINT32)Key);

                if (!EFI_ERROR (Status)) {
                  Status = SmcIoSmcWriteData8 (SmcDev, (SMC_DATA)Size);

                  if (!EFI_ERROR (Status)) {
                    ValueWalker = Value;

                    do {
                      Status = SmcIoSmcReadData8 (SmcDev, ValueWalker);
                      ++ValueWalker;

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

              if ((Key == SMC_MAKE_KEY ('R', 'P', 'l', 't'))
               && (*(UINT64 *)Value == SMC_MAKE_KEY ('5', '0', '5', 'j'))) {
                ((CHAR8 *)Value)[2] = '\0';
              }
            } else {
              Status = EFIERR (Result);
            }
          }

          EfiReleaseLock (&SmcDev->Lock);
        }
      }
    }
  }

  return Status;
}

// InternalSmcWriteValue
STATIC
EFI_STATUS
EFIAPI
InternalSmcWriteValue (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  IN  SMC_KEY                Key,
  IN  SMC_DATA_SIZE          Size,
  OUT SMC_DATA               *Value
  )
{
  EFI_STATUS Status;

  BOOLEAN    KeyPresent;
  SMC_DEV    *SmcDev;
  SMC_RESULT Result;

  if (mSoftwareSmc) {
    Status = SmcIoVirtualSmcWriteValue (This, Key, Size, Value);
  } else {
    Status = EFI_INVALID_PARAMETER;

    // BUG: This is used unchecked.

    if ((Size > 0) && (Size <= SMC_MAX_DATA_SIZE) && (Value != NULL)) {
      KeyPresent = InternalIsKeyPresent (This, Key);

      Status = EFI_NOT_FOUND;

      if (KeyPresent) {
        SmcDev = SMC_DEV_FROM_THIS (This);
        Status = EfiAcquireLockOrFail (&SmcDev->Lock);

        if (!EFI_ERROR (Status)) {
          if (This->Mmio) {
            Status = SmcWriteValueMmio (
                       mSmcMmioAddress,
                       Key,
                       (UINT32)Size,
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
                    } while (Size > 0);

                    if (Size == 0) {
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
    }
  }

  return Status;
}

// InternalSmcMakeKey
STATIC
EFI_STATUS
EFIAPI
InternalSmcMakeKey (
  IN  CHAR8    *Name,
  OUT SMC_KEY  *Key
  )
{
  EFI_STATUS Status;

  UINTN      Index;

  if (mSoftwareSmc) {
    Status = SmcIoVirtualSmcMakeKey (Name, Key);
  } else {
    Status = EFI_INVALID_PARAMETER;

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
      } while (Index < (sizeof (*Key) / sizeof (*Name)));

      Status = EFI_SUCCESS;
    }
  }

  return Status;
}

// SmcGetyKeyCount
STATIC
EFI_STATUS
EFIAPI
InternalSmcGetKeyCount (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  OUT UINT32                 *Count
  )
{
  EFI_STATUS Status;

  SMC_KEY    Key;

  Key = 0;

  if (mSoftwareSmc) {
    Status = SmcIoVirtualSmcGetKeyCount (This, Count);
  } else {
    Status = InternalSmcMakeKey (NUMBER_OF_KEYS_KEY, &Key);

    if (!EFI_ERROR (Status)) {
      Status = SmcIoVirtualSmcReadValue (
                 This,
                 Key,
                 sizeof (*Count),
                 (VOID *)Count
                 );
    }
  }

  return Status;
}

// InternalSmcGetKeyFromIndex
STATIC
EFI_STATUS
EFIAPI
InternalSmcGetKeyFromIndex (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  IN  SMC_KEY_INDEX          Index,
  OUT SMC_KEY                *Key
  )
{
  EFI_STATUS Status;

  SMC_DEV    *SmcDev;
  SMC_RESULT Result;

  if (mSoftwareSmc) {
    Status = SmcIoVirtualSmcGetKeyFromIndex (This, Index, Key);
  } else {
    Status = EFI_INVALID_PARAMETER;

    // BUG: This is used unchecked.

    if (Key != NULL) {
      SmcDev = SMC_DEV_FROM_THIS (This);

      Status = EfiAcquireLockOrFail (&SmcDev->Lock);

      if (!EFI_ERROR (Status)) {
        if (This->Mmio) {
          Status = SmcGetKeyFromIndexMmio (mSmcMmioAddress, Index, Key);
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
  }

  return Status;
}

// InternalSmcGetKeyInfo
STATIC
EFI_STATUS
EFIAPI
InternalSmcGetKeyInfo (
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

  if (mSoftwareSmc) {
    Status = SmcIoVirtualSmcGetKeyInfo (This, Key, Size, Type, Attributes);
  } else {
    Status = EFI_INVALID_PARAMETER;

    // BUG: This is used unchecked.

    if ((Size != NULL) && (Type != NULL) && (Attributes != NULL)) {
      SmcDev = SMC_DEV_FROM_THIS (This);

      Status = EfiAcquireLockOrFail (&SmcDev->Lock);

      if (!EFI_ERROR (Status)) {
        if (This->Mmio) {
          Status = SmcGetKeyInfoMmio (
                     mSmcMmioAddress,
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
  }

  return Status;
}

// InternalSmcReset
STATIC
EFI_STATUS
EFIAPI
InternalSmcReset (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINT32                 Mode
  )
{
  EFI_STATUS Status;

  SMC_DEV    *SmcDev;
  SMC_RESULT Result;
  BOOLEAN    Mmio;

  if (mSoftwareSmc) {
    Status = SmcIoVirtualSmcReset (This, Mode);
  } else {
    // BUG: This is used unchecked.

    SmcDev = SMC_DEV_FROM_THIS (This);

    Status = EfiAcquireLockOrFail (&SmcDev->Lock);

    if (!EFI_ERROR (Status)) {
      if (This->Mmio) {
        Status = SmcResetMmio (mSmcMmioAddress, Mode);

        Mmio = SmcMmioInterface (mSmcMmioAddress);

        if (!Mmio) {
          This->Mmio = FALSE;
        }
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
  }

  return Status;
}

// InternalSmcFlashType
STATIC
EFI_STATUS
EFIAPI
InternalSmcFlashType (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN SMC_FLASH_TYPE         Type
  )
{
  EFI_STATUS Status;

  SMC_DEV    *SmcDev;
  SMC_RESULT Result;

  if (mSoftwareSmc) {
    Status = SmcIoVirtualSmcFlashType (This, Type);
  } else {
    // BUG: This is used unchecked.

    SmcDev = SMC_DEV_FROM_THIS (This);

    Status = EfiAcquireLockOrFail (&SmcDev->Lock);

    if (!EFI_ERROR (Status)) {
      if (This->Mmio) {
        Status = SmcFlashTypeMmio (mSmcMmioAddress, Type);
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
  }

  return Status;
}

// InternalSmcFlashWrite
STATIC
EFI_STATUS
EFIAPI
InternalSmcFlashWrite (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINT32                 Unknown,
  IN SMC_FLASH_SIZE         Size,
  IN SMC_DATA               *Data
  )
{
  EFI_STATUS Status;

  SMC_DEV    *SmcDev;
  SMC_RESULT Result;
  SMC_DATA   Value;

  if (mSoftwareSmc) {
    Status = SmcIoVirtualSmcFlashWrite (This, Unknown, Size, Data);
  } else {
    Status = EFI_INVALID_PARAMETER;

    // BUG: This is used unchecked.

    if ((Size > 0) && (Size <= SMC_FLASH_SIZE_MAX)  && (Data != NULL)) {
      SmcDev = SMC_DEV_FROM_THIS (This);

      Status = EfiAcquireLockOrFail (&SmcDev->Lock);

      if (!EFI_ERROR (Status)) {
        if (This->Mmio) {
          Status = SmcFlashWriteMmio (
                     mSmcMmioAddress,
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
                  } while (Size > 0);

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
  }

  return Status;
}

// InternalSmcFlashAuth
STATIC
EFI_STATUS
EFIAPI
InternalSmcFlashAuth (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN SMC_FLASH_SIZE         Size,
  IN SMC_DATA               *Data
  )
{
  EFI_STATUS Status;

  SMC_DEV    *SmcDev;
  SMC_RESULT Result;
  SMC_DATA   Value;

  if (mSoftwareSmc) {
    Status = InternalSmcFlashAuth (This, Size, Data);
  } else {
    Status = EFI_INVALID_PARAMETER;

    // BUG: This is used unchecked.

    if ((Size > 0) && (Size <= SMC_FLASH_SIZE_MAX) && (Data != NULL)) {
      SmcDev = SMC_DEV_FROM_THIS (This);

      Status = EfiAcquireLockOrFail (&SmcDev->Lock);

      if (!EFI_ERROR (Status)) {
        if (This->Mmio) {
          Status = SmcFlashAuthMmio (mSmcMmioAddress, Size, Data);
        } else {
          Status = SmcIoSmcSmcInABadState (SmcDev);

          if (!EFI_ERROR (Status)) {
            Status = SmcIoSmcWriteCommand (SmcDev, SmcCmdFlashAuth);

            if (!EFI_ERROR (Status)) {
              Status = SmcIoSmcWriteData16 (SmcDev, Size);

              if (!EFI_ERROR (Status)) {
                do {
                  Status = SmcIoSmcWriteData8 (SmcDev, *Data);

                  if (EFI_ERROR (Status)) {
                    break;
                  }

                  ++Data;
                  --Size;
                } while (Size > 0);

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
  }

  return Status;
}

// InternalSmcUnsupported
STATIC
EFI_STATUS
EFIAPI
InternalSmcUnsupported (
  VOID
  )
{
  return EFI_UNSUPPORTED;
}

// InternalSmcUnknown1
STATIC
EFI_STATUS
EFIAPI
InternalSmcUnknown1 (
  VOID
  )
{
  // TODO: implement

  return EFI_SUCCESS;
}

// InternalSmcUnknown2
STATIC
EFI_STATUS
EFIAPI
InternalSmcUnknown2 (
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

// InternalSmcUnknown3
STATIC
EFI_STATUS
EFIAPI
InternalSmcUnknown3 (
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

// InternalSmcUnknown4
STATIC
EFI_STATUS
EFIAPI
InternalSmcUnknown4 (
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

// InternalSmcUnknown5
STATIC
EFI_STATUS
EFIAPI
InternalSmcUnknown5 (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINT8                  *Data
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

// AppleSmcIoMain
EFI_STATUS
EFIAPI
AppleSmcMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  STATIC APPLE_SMC_IO_PROTOCOL AppleSmcIoProtocolTemplate = {
    APPLE_SMC_IO_PROTOCOL_REVISION,
    InternalSmcReadValue,
    InternalSmcWriteValue,
    InternalSmcGetKeyCount,
    InternalSmcMakeKey,
    InternalSmcGetKeyFromIndex,
    InternalSmcGetKeyInfo,
    InternalSmcReset,
    InternalSmcFlashType,
    InternalSmcUnsupported,
    InternalSmcFlashWrite,
    InternalSmcFlashAuth,
    0,
    SMC_PORT_BASE,
    FALSE,
    InternalSmcUnknown1,
    InternalSmcUnknown2,
    InternalSmcUnknown3,
    InternalSmcUnknown4,
    InternalSmcUnknown5
  };

  EFI_STATUS       Status;

  SMC_DEV          *SmcDev;
  UINT8            NumberOfSmcDevices;
  UINT8            Index;
  SMC_DEVICE_INDEX SmcIndex;
  SMC_ADDRESS      SmcAddress;
  VOID             *SmcHob;
  UINT16           Value;
  SMC_DEV          *SmcDevChild;

  SmcDev = AllocateZeroPool (sizeof (*SmcDev));

  Status = EFI_OUT_OF_RESOURCES;

  if (SmcDev != NULL) {
    SmcDev->KeyPresenceMap = AllocateZeroPool (
                              KEY_PRESENT_MAP_UNITS
                                * sizeof (*SmcDev->KeyPresenceMap)
                              );

    if (SmcDev->KeyPresenceMap != NULL) {
      SmcDev->MaxKeyPresenceMapLength = KEY_PRESENT_MAP_UNITS;
      SmcDev->Signature               = SMC_DEV_SIGNATURE;

      EfiInitializeLock (&SmcDev->Lock, TPL_NOTIFY);

      CopyMem (
        (VOID *)&SmcDev->SmcIo,
        (VOID *)&AppleSmcIoProtocolTemplate,
        sizeof (AppleSmcIoProtocolTemplate)
        );

      Status = gBS->InstallProtocolInterface (
                      &SmcDev->Handle,
                      &gAppleSmcIoProtocolGuid,
                      EFI_NATIVE_INTERFACE,
                      (VOID *)&SmcDev->SmcIo
                      );

      if (!EFI_ERROR (Status)) {
        SmcAddress = SMC_MMIO_BASE_ADDRESS;

        SmcHob = GetFirstGuidHob (&gAppleSmcMmioAddressHobGuid);

        if (SmcHob != NULL) {
          SmcAddress = *(SMC_ADDRESS *)GET_GUID_HOB_DATA (SmcHob);
        }

        mSmcMmioAddress = SmcAddress;

        SmcDev->SmcIo.Mmio = SmcMmioInterface (SmcAddress);

        SmcHob = GetFirstGuidHob (&gApplePhysicalSmcHobGuid);

        if (SmcHob == NULL) {
          Status = InternalSmcReadValue (
                     &SmcDev->SmcIo,
                     SMC_MAKE_KEY ('M', 'S', 'P', 'S'),
                     sizeof (Value),
                     (SMC_DATA *)&Value
                     );

          if (EFI_ERROR (Status)) {
            mSoftwareSmc = TRUE;
            Status       = EFI_SUCCESS;

            goto Done;
          }
        }

        NumberOfSmcDevices = 1;

        InternalSmcReadValue (
          &SmcDev->SmcIo,
          SMC_KEY_NUM,
          sizeof (NumberOfSmcDevices),
          (VOID *)&NumberOfSmcDevices
          );

        Status = EFI_SUCCESS;

        if (NumberOfSmcDevices < 2) {
          goto Done;
        }

        for (Index = 1; Index < NumberOfSmcDevices; ++Index) {
          SmcIndex = Index;
          Status   = InternalSmcWriteValue (
                       &SmcDev->SmcIo,
                       SMC_KEY_NUM,
                       sizeof (SmcIndex),
                       (VOID *)&SmcIndex
                       );

          if (!EFI_ERROR (Status)) {
            Status = InternalSmcReadValue (
                       &SmcDev->SmcIo,
                       SMC_KEY_ADR,
                       sizeof (SmcAddress),
                       (VOID *)&SmcAddress
                       );

            if (!EFI_ERROR (Status)) {
              // BUG: If one child cannot be allocated, the driver exits
              // without freeing the others.

              SmcDevChild = AllocateZeroPool (sizeof (*SmcDevChild));

              if (SmcDevChild != NULL) {
                SmcDevChild->KeyPresenceMap = AllocateZeroPool (
                                                KEY_PRESENT_MAP_UNITS
                                                  * sizeof (*SmcDev->KeyPresenceMap)
                                                );
              
                if (SmcDev->KeyPresenceMap != NULL) {
                  SmcDevChild->MaxKeyPresenceMapLength = KEY_PRESENT_MAP_UNITS;
                  SmcDevChild->Signature               = SMC_DEV_SIGNATURE;

                  EfiInitializeLock (&SmcDevChild->Lock, TPL_NOTIFY);

                  CopyMem (
                    (VOID *)&SmcDevChild->SmcIo,
                    (VOID *)&AppleSmcIoProtocolTemplate,
                    sizeof (AppleSmcIoProtocolTemplate)
                    );

                  SmcDevChild->SmcIo.Index   = Index;
                  SmcDevChild->SmcIo.Address = NEXT_SMC_ADDRESS (SmcAddress);

                  Status = gBS->InstallProtocolInterface (
                                  &SmcDevChild->Handle,
                                  &gAppleSmcIoProtocolGuid,
                                  EFI_NATIVE_INTERFACE,
                                  (VOID *)&SmcDevChild->SmcIo
                                  );

                  if (!EFI_ERROR (Status)) {
                    goto Done;
                  }

                  gBS->FreePool ((VOID *)SmcDevChild->KeyPresenceMap);
                }
                
                gBS->FreePool ((VOID *)SmcDevChild);
              }
            }
          }
        }
      }

      gBS->FreePool ((VOID *)SmcDev->KeyPresenceMap);
    }

    // BUG: SmcIo is not uninstalled.
    gBS->FreePool ((VOID *)SmcDev);
  }

Done:
  return Status;
}
