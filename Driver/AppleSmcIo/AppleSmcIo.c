// 22/10/2015

#include <AppleEfi.h>
#include <EfiDriverLib.h>

#include <IndustryStandard/AppleSmc.h>

#include EFI_PROTOCOL_CONSUMER (CpuIo)
#include <Protocol/AppleSmcIo.h>

#define IS_VALID_ASCII(x) (((x) >= 0x20) && ((x) <= 0x7f))

#define APPLE_SMC_IO_PROTOCOL_REVISION  0x33

#define SMC_DEV_SIGNATURE    EFI_SIGNATURE_64 ('A', 'p', 'p', 'l', 'e', 'S', 'm', 'c')
#define SMC_DEV_FROM_THIS(x) CR ((x), APPLE_SMC_DEV, SmcIo)

#pragma pack(1)

// _APPLE_SMC_DEV
typedef struct _APPLE_SMC_DEV {
  UINT64                Signature;
  EFI_HANDLE            Handle;
  EFI_LOCK              Lock;
  UINT32                __reserved[3];
  APPLE_SMC_IO_PROTOCOL SmcIo;
  EFI_CPU_IO_PROTOCOL   *CpuIo;
} APPLE_SMC_DEV;

#pragma pop()

SMC_STATUS
SmcReadStatus (
  IN APPLE_SMC_DEV  *SmcDev
  )
{
  SMC_STATUS Status;

  SmcDev->CpuIo->Io.Read (SmcDev->CpuIo, EfiCpuIoWidthUint8, (SmcDev->SmcIo.Address + SMC_PORT_STATUS), sizeof (Status), (VOID *)&Status);

  return Status;
}

SMC_RESULT
SmcReadResult (
  IN APPLE_SMC_DEV  *SmcDev
  )
{
  SMC_RESULT Result;

  SmcDev->CpuIo->Io.Read (SmcDev->CpuIo, EfiCpuIoWidthUint8, (SmcDev->SmcIo.Address + SMC_PORT_RESULT), sizeof (Result), (VOID *)&Result);

  return Result;
}

EFI_STATUS
SmcWriteCommand (
  IN  APPLE_SMC_DEV  *SmcDev,
  OUT UINT32         Command
  )
{
  EFI_STATUS  Status;

  UINTN       Index;
  SMC_STATUS  SmcStatus;

  Index     = 60000;
  SmcStatus = SmcReadStatus (SmcDev);

  if ((SmcStatus & SMC_STATUS_IB_CLOSED) != 0) {
    do {
      SmcStatus = SmcReadStatus (SmcDev);

      if (Index == 0) {
        Status = EFI_TIMEOUT;
        goto Return;
      }

      --Index;

      gBS->Stall (50);
    } while ((SmcStatus & SMC_STATUS_IB_CLOSED) != 0);
  }

  SmcDev->CpuIo->Io.Write (SmcDev->CpuIo, EfiCpuIoWidthUint8, (SmcDev->SmcIo.Address + SMC_PORT_COMMAND), sizeof(SMC_COMMAND), (VOID *)&(SMC_COMMAND)Command);

  Index = 20000;

  if ((SmcStatus & SMC_STATUS_BUSY) == 0) {
    do {
      SmcStatus = SmcReadStatus (SmcDev);

      if (Index == 0) {
        Status = EFI_TIMEOUT;
        goto Return;
      }

      --Index;

      gBS->Stall (50);
    } while ((SmcStatus & SMC_STATUS_BUSY) == 0);
  }

  Status = EFI_SUCCESS;

 Return:
  return Status;
}

EFI_STATUS
SmcReadData8 (
  IN  APPLE_SMC_DEV  *SmcDev,
  OUT SMC_DATA       *Data
  )
{
  EFI_STATUS Status;

  UINTN      Index;
  SMC_STATUS SmcStatus;
  SMC_DATA   Buffer;

  Index     = 60000;
  SmcStatus = SmcReadStatus (SmcDev);

  if ((SmcStatus & (SMC_STATUS_AWAITING_MORE_BYTES | SMC_STATUS_BUSY)) == SMC_STATUS_BUSY) {
    do {
      SmcStatus = SmcReadStatus (SmcDev);

      if (Index == 0) {
        Status = EFI_TIMEOUT;
        goto Return;
      }

      --Index;

      gBS->Stall (50);
    } while ((SmcStatus & (SMC_STATUS_AWAITING_MORE_BYTES | SMC_STATUS_BUSY)) == SMC_STATUS_BUSY);
  }

  if ((SmcStatus & SMC_STATUS_BUSY) != 0) {
    SmcDev->CpuIo->Io.Read (SmcDev->CpuIo, EfiCpuIoWidthUint8, (SmcDev->SmcIo.Address + SMC_PORT_DATA), sizeof (Buffer), (VOID *)&Buffer);

    Status = EFI_SUCCESS;
    *Data  = Buffer;
  } else {
    Status = EFI_NOT_READY;
  }

Return:
  return Status;
}

EFI_STATUS
SmcReadData16 (
  IN APPLE_SMC_DEV  *SmcDev,
  IN UINT16         *Data
  )
{
  EFI_STATUS Status;

  SMC_DATA   Buffer[2];

  Status = SmcReadData8 (SmcDev, &Buffer[1]);

  if (!EFI_ERROR (Status)) {
    Status = SmcReadData8 (SmcDev, &Buffer[0]);

    if (!EFI_ERROR (Status)) {
      gBS->CopyMem ((VOID *)Data, (VOID *)Buffer, sizeof (Buffer));
    }
  }

  return Status;
}

EFI_STATUS
SmcReadData32 (
  IN APPLE_SMC_DEV  *SmcDev,
  IN UINT32         *Data
  )
{
  EFI_STATUS Status;

  UINT16     Buffer[2];

  Status = SmcReadData16 (SmcDev, &Buffer[1]);

  if (!EFI_ERROR (Status)) {
    Status = SmcReadData16 (SmcDev, &Buffer[0]);

    if (!EFI_ERROR (Status)) {
      gBS->CopyMem ((VOID *)Data, (VOID *)Buffer, sizeof (Buffer));
    }
  }

  return Status;
}

EFI_STATUS
SmcWriteData8 (
  IN APPLE_SMC_DEV  *SmcDev,
  IN SMC_DATA       Data
  )
{
  EFI_STATUS Status;

  UINTN      Index;
  SMC_STATUS SmcStatus;

  Index     = 60000;
  SmcStatus = SmcReadStatus (SmcDev);

  if ((SmcStatus & SMC_STATUS_IB_CLOSED) != 0) {
    do {
      SmcStatus = SmcReadStatus (SmcDev);

      if (Index == 0) {
        Status = EFI_TIMEOUT;
        goto Return;
      }

      --Index;

      gBS->Stall (50);
    } while ((SmcStatus & SMC_STATUS_IB_CLOSED) != 0);
  }

  if ((SmcStatus & SMC_STATUS_BUSY) != 0) {
    SmcDev->CpuIo->Io.Write (SmcDev->CpuIo, EfiCpuIoWidthUint8, (SmcDev->SmcIo.Address + SMC_PORT_DATA), sizeof (Data), (VOID *)&Data);

    Status = EFI_SUCCESS;
  } else {
    Status = EFI_NOT_READY;
  }

Return:
  return Status;
}

EFI_STATUS
SmcWriteData16 (
  IN APPLE_SMC_DEV  *SmcDev,
  IN UINT16         Data
  )
{
  EFI_STATUS Status;

  if (!EFI_ERROR (Status)) {
    Status = SmcWriteData8 (SmcDev, (SMC_DATA)(Data >> 8));

    if (!EFI_ERROR (Status)) {
      Status = SmcWriteData8 (SmcDev, (SMC_DATA)Data);
    }
  }

  return Status;
}

EFI_STATUS
SmcWriteData32 (
  IN APPLE_SMC_DEV  *SmcDev,
  IN UINT32         Data
  )
{
  EFI_STATUS Status;

  Status = SmcWriteData8 (SmcDev, (SMC_DATA)(Data >> 24));

  if (!EFI_ERROR (Status)) {
    Status = SmcWriteData8 (SmcDev, (SMC_DATA)(Data >> 16));

    if (!EFI_ERROR (Status)) {
      Status = SmcWriteData8 (SmcDev, (SMC_DATA)(Data >> 8));

      if (!EFI_ERROR (Status)) {
        Status = SmcWriteData8 (SmcDev, (SMC_DATA)Data);
      }
    }
  }

  return Status;
}

EFI_STATUS
SmcTimeoutWaitingForBusyClear (
  IN APPLE_SMC_DEV  *SmcDev
  )
{
  EFI_STATUS Status;

  SMC_STATUS SmcStatus;
  UINTN      Index;

  Index     = 20000;
  SmcStatus = SmcReadStatus (SmcDev);

  if ((SmcStatus & SMC_STATUS_BUSY) != 0) {
    do {
      SmcStatus = SmcReadStatus (SmcDev);

      if (Index == 0) {
        Status = EFI_TIMEOUT;
        goto Return;
      }

      --Index;

      gBS->Stall (50);
    } while ((SmcStatus & SMC_STATUS_BUSY) != 0);
  }

  Status = EFI_SUCCESS;

Return:
  return Status;
}

EFI_STATUS
SmcTimeoutLongWaitingForBusyClear (
  IN APPLE_SMC_DEV  *SmcDev
  )
{
  EFI_STATUS Status;

  SMC_STATUS SmcStatus;
  UINTN      Index;

  Index     = 100000;
  SmcStatus = SmcReadStatus (SmcDev);

  if ((SmcStatus & SMC_STATUS_BUSY) != 0) {
    do {
      SmcStatus = SmcReadStatus (SmcDev);

      if (Index == 0) {
        Status = EFI_TIMEOUT;
        goto Return;
      }

      --Index;

      gBS->Stall (50);
    } while ((SmcStatus & SMC_STATUS_BUSY) != 0);
  }

  Status = EFI_SUCCESS;

Return:
  return Status;
}

EFI_STATUS
SmcSmcInABadState (
  IN APPLE_SMC_DEV  *SmcDev
  )
{
  EFI_STATUS Status;

  Status = SmcTimeoutWaitingForBusyClear (SmcDev);

  if (!EFI_ERROR (Status)) {
    Status = SmcWriteCommand (SmcDev, SmcCmdReadValue);

    if (!EFI_ERROR (Status)) {
      Status = SmcTimeoutWaitingForBusyClear (SmcDev);
    }
  }

  return Status;
}

EFI_STATUS
EFIAPI
SmcReadValue (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  IN  SMC_KEY                Key,
  IN  SMC_DATA_SIZE          Size,
  OUT VOID                   *Value
  )
{
  EFI_STATUS    Status;

  APPLE_SMC_DEV *SmcDev;
  SMC_RESULT    SmcResult;
  SMC_DATA      *Buffer;

  if (((Size - 1) <= 31) && (Value != NULL)) {
    SmcDev = SMC_DEV_FROM_THIS (This);
    Status = EfiAcquireLockOrFail (&SmcDev->Lock);

    if (!EFI_ERROR (Status)) {
      Status = SmcSmcInABadState (SmcDev);

      if (!EFI_ERROR (Status)) {
        Status = SmcWriteCommand (SmcDev, SmcCmdReadValue);

        if (!EFI_ERROR (Status)) {
          Status = SmcWriteData32 (SmcDev, (UINT32)Key);

          if (!EFI_ERROR (Status)) {
            Status = SmcWriteData8 (SmcDev, (SMC_DATA)Size);

            if (!EFI_ERROR (Status)) {
              Buffer = (SMC_DATA *)Value;

              do {
                Status = SmcReadData8 (SmcDev, Buffer);
                ++Buffer;

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

      SmcResult = SmcReadResult (SmcDev);

      if (Status == EFI_TIMEOUT) {
        Status = EFI_SMC_TIMEOUT_ERROR;
      } else if (SmcResult != SMC_SUCCESS) {
        Status = EFIERR (Status);
      }

      EfiReleaseLock (&SmcDev->Lock);
    }
  } else {
    Status = EFI_INVALID_PARAMETER;
  }

  return Status;
}

EFI_STATUS
EFIAPI
SmcWriteValue (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  IN  SMC_KEY                Key,
  IN  SMC_DATA_SIZE          Size,
  OUT VOID                   *Value
  )
{
  EFI_STATUS    Status;

  APPLE_SMC_DEV *SmcDev;
  SMC_RESULT    SmcResult;
  SMC_DATA      *Buffer;

  if (((Size - 1) <= 31) && (Value != NULL)) {
    SmcDev = SMC_DEV_FROM_THIS (This);
    Status = EfiAcquireLockOrFail (&SmcDev->Lock);

    if (!EFI_ERROR (Status)) {
      Status = SmcSmcInABadState (SmcDev);

      if (!EFI_ERROR (Status)) {
        Status = SmcWriteCommand (SmcDev, SmcCmdWriteValue);

        if (!EFI_ERROR (Status)) {
          Status = SmcWriteData32 (SmcDev, (UINT32)Key);

          if (!EFI_ERROR (Status)) {
            Status = SmcWriteData8 (SmcDev, (SMC_DATA)Size);

            if (!EFI_ERROR (Status)) {
              Buffer = (SMC_DATA *)Value;

              do {
                Status = SmcWriteData8 (SmcDev, *Buffer);
                ++Buffer;

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

      SmcResult = SmcReadResult (SmcDev);

      if (Status == EFI_TIMEOUT) {
        Status = EFI_SMC_TIMEOUT_ERROR;
      } else if (SmcResult != SMC_SUCCESS) {
        Status = EFIERR (Status);
      }

      EfiReleaseLock (&SmcDev->Lock);
    }
  } else {
    Status = EFI_INVALID_PARAMETER;
  }

  return Status;
}

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
      if (IS_VALID_ASCII (Name[Index])) {
        *Key <<= 8;
        *Key  |= Name[Index];
        ++Index;
      } else {
        *Key = 0;
        break;
      }
    } while (Index < 4);

    Status = EFI_SUCCESS;
    goto Return;
  }

  Status = EFI_INVALID_PARAMETER;

Return:
  return Status;
}

EFI_STATUS
EFIAPI
SmcGetKeyCount (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  OUT UINT32                *Count
  )
{
  EFI_STATUS Status;

  SMC_KEY    Key;

  Status = SmcMakeKey ("#Key", &Key);

  if (!EFI_ERROR (Status)) {
    Status = SmcReadValue (This, Key, sizeof (*Count), (VOID *)Count);
  }

  return Status;
}

EFI_STATUS
EFIAPI
SmcGetKeyFromIndex (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  IN  SMC_INDEX              Index,
  OUT SMC_KEY                *Key
  )
{
  EFI_STATUS    Status;

  APPLE_SMC_DEV *SmcDev;
  SMC_RESULT    SmcResult;

  if (Key != NULL) {
    SmcDev = SMC_DEV_FROM_THIS (This);
    Status = EfiAcquireLockOrFail (&SmcDev->Lock);

    if (!EFI_ERROR (Status)) {
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

      SmcResult = SmcReadResult (SmcDev);

      if (Status == EFI_TIMEOUT) {
        Status = EFI_SMC_TIMEOUT_ERROR;
      } else if (SmcResult != SMC_SUCCESS) {
        Status = EFIERR (Status);
      }

      EfiReleaseLock (&SmcDev->Lock);
    }
  } else {
    Status = EFI_INVALID_PARAMETER;
  }

  return Status;
}

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
  EFI_STATUS    Status;

  APPLE_SMC_DEV *SmcDev;
  SMC_RESULT    SmcResult;

  if ((Size != NULL) && (Attributes != NULL)) {
    SmcDev = SMC_DEV_FROM_THIS (This);
    Status = EfiAcquireLockOrFail (&SmcDev->Lock);

    if (!EFI_ERROR (Status)) {
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

      SmcResult = SmcReadResult (SmcDev);

      if (Status == EFI_TIMEOUT) {
        Status = EFI_SMC_TIMEOUT_ERROR;
      } else if (SmcResult != SMC_SUCCESS) {
        Status = EFIERR (Status);
      }

      EfiReleaseLock (&SmcDev->Lock);
    }
  } else {
    Status = EFI_INVALID_PARAMETER;
  }

  return Status;
}

EFI_STATUS
EFIAPI
SmcReset (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN SMC_RESET_MODE         Mode
  )
{
  EFI_STATUS    Status;

  APPLE_SMC_DEV *SmcDev;
  SMC_RESULT    SmcResult;

  SmcDev = SMC_DEV_FROM_THIS (This);
  Status = EfiAcquireLockOrFail (&SmcDev->Lock);

  if (!EFI_ERROR (Status)) {
    Status = SmcSmcInABadState (SmcDev);

    if (!EFI_ERROR (Status)) {
      Status = SmcWriteCommand (SmcDev, SmcCmdReset);

      if (!EFI_ERROR (Status)) {
        SmcWriteData8 (SmcDev, (SMC_DATA)Mode);

        if (!EFI_ERROR (Status)) {
          Status = SmcLongTimeoutWaitingForBusyClear (SmcDev);
        }
      }
    }

    SmcResult = SmcReadResult (SmcDev);

    if (Status == EFI_TIMEOUT) {
      Status = EFI_SMC_TIMEOUT_ERROR;
    } else if (SmcResult != SMC_SUCCESS) {
      Status = EFIERR (Status);
    }

    EfiReleaseLock (&SmcDev->Lock);
  }

  return Status;
}

EFI_STATUS
EFIAPI
SmcFlashType (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINT32                 Type
  )
{
  EFI_STATUS    Status;

  APPLE_SMC_DEV *SmcDev;
  SMC_RESULT    SmcResult;
  SMC_DATA      Value;

  SmcDev = SMC_DEV_FROM_THIS (This);
  Status = EfiAcquireLockOrFail (&SmcDev->Lock);

  if (!EFI_ERROR (Status)) {
    Status = SmcSmcInABadState (SmcDev);

    if (!EFI_ERROR (Status)) {
      Status = SmcWriteCommand (SmcDev, SmcCmdFlashType);

      if (!EFI_ERROR (Status)) {
        Status = SmcWriteData8 (SmcDev, (SMC_DATA)(SMC_FLASH_TYPE)Type);

        if (!EFI_ERROR (Status)) {
          Status = SmcTimeoutWaitingForBusyClear (SmcDev);
        }
      }
    }

    SmcResult = SmcReadResult (SmcDev);

    if (Status == EFI_TIMEOUT) {
      Status = EFI_SMC_TIMEOUT_ERROR;
    } else if (SmcResult != SMC_SUCCESS) {
      Status = EFIERR (Status);
    }

    EfiReleaseLock (&SmcDev->Lock);
  }

  return Status;
}

EFI_STATUS
EFIAPI
SmcFlashWrite (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINT32                 Unknown,
  IN UINT16                 Size,
  IN SMC_DATA               *Data
  )
{
  EFI_STATUS    Status;

  APPLE_SMC_DEV *SmcDev;
  SMC_RESULT    SmcResult;
  SMC_DATA      Value;

  if ((Size != 0) && (Size <= 0x800) && (Data != NULL)) {
    SmcDev = SMC_DEV_FROM_THIS (This);
    Status = EfiAcquireLockOrFail (&SmcDev->Lock);

    if (!EFI_ERROR (Status)) {
      Status = SmcSmcInABadState (SmcDev);

      if (!EFI_ERROR (Status)) {
        Status = SmcWriteCommand (SmcDev, SmcCmdFlashWrite);

        if (!EFI_ERROR (Status)) {
          Status = SmcWriteData32 (SmcDev, Unknown);

          if (!EFI_ERROR (Status)) {
            Status = SmcWriteData16 (SmcDev, Size);

            if (!EFI_ERROR (Status)) {
              do {
                Status = SmcWriteData8 (SmcDev, (SMC_DATA)*Data);

                if (EFI_ERROR (Status)) {
                  break;
                }

                ++Data;
                --Size;
              } while (Size != 0);

              do {
                Status = SmcReadData8 (SmcDev, &Value);
              } while (Status == EFI_SUCCESS);
            }
          }
        }
      }

      SmcResult = SmcReadResult (SmcDev);

      if (Status == EFI_TIMEOUT) {
        Status = EFI_SMC_TIMEOUT_ERROR;
      } else if (SmcResult != SMC_SUCCESS) {
        Status = EFIERR (Status);
      }

      EfiReleaseLock (&SmcDev->Lock);
    }
  }

  return Status;
}

EFI_STATUS
EFIAPI
SmcFlashAuth (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINT16                 Size,
  IN SMC_DATA               *Data
  )
{
  EFI_STATUS    Status;

  APPLE_SMC_DEV *SmcDev;
  SMC_RESULT    SmcResult;
  SMC_DATA      Value;

  if ((Size != 0) && (Size <= 0x800) && (Data != NULL)) {
    SmcDev = SMC_DEV_FROM_THIS (This);
    Status = EfiAcquireLockOrFail (&SmcDev->Lock);

    if (!EFI_ERROR (Status)) {
      Status = SmcSmcInABadState (SmcDev);

      if (!EFI_ERROR (Status)) {
        Status = SmcWriteCommand (SmcDev, SmcCmdFlashAuth);

        if (!EFI_ERROR (Status)) {
          Status = SmcWriteData16 (SmcDev, Size);

          if (!EFI_ERROR (Status)) {
            do {
              Status = SmcWriteData8 (SmcDev, (SMC_DATA)*Data);

              if (EFI_ERROR (Status)) {
                break;
              }

              ++Data;
              --Size;
            } while (Size != 0);

            do {
              Status = SmcReadData8 (SmcDev, &Value);
            } while (Status == EFI_SUCCESS);
          }
        }
      }

      SmcResult = SmcReadResult (SmcDev);

      if (Status == EFI_TIMEOUT) {
        Status = EFI_SMC_TIMEOUT_ERROR;
      } else if (SmcResult != SMC_SUCCESS) {
        Status = EFIERR (Status);
      }

      EfiReleaseLock (&SmcDev->Lock);
    }
  }

  return Status;
}

EFI_STATUS
EFIAPI
SmcUnsupported (
  VOID
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
AppleSmcIoMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  ) // start
{
  EFI_STATUS          Status;

  EFI_CPU_IO_PROTOCOL *CpuIo;
  APPLE_SMC_DEV       *SmcDev;
  UINT8               NoSmc;
  UINT8               Index;
  SMC_INDEX           SmcIndex;
  SMC_ADDRESS         SmcAddress;
  APPLE_SMC_DEV       *SmcDevChild;

  EfiInitializeDriverLib (ImageHandle, SystemTable);

  Status = gBS->LocateProtocol (&gEfiCpuIoProtocolGuid, NULL, (VOID **)&CpuIo);

  if (!EFI_ERROR (Status)) {
    SmcDev = (APPLE_SMC_DEV *)EfiLibAllocateZeroPool (sizeof (*SmcDev));

    if (SmcDev == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
    } else {
      SmcDev->Signature = SMC_DEV_SIGNATURE;
      SmcDev->CpuIo     = CpuIo;

      EfiInitializeLock (&SmcDev->Lock, EFI_TPL_NOTIFY);

      gBS->CopyMem ((VOID *)&SmcDev->SmcIo, (VOID *)&SmcIoProtocol, sizeof (SmcIoProtocol));

      Status = gBS->InstallProtocolInterface (&SmcDev->Handle, &gAppleSmcIoProtocolGuid, EFI_NATIVE_INTERFACE, (VOID *)&SmcDev->SmcIo);

      if (EFI_ERROR (Status)) {
        gBS->FreePool ((VOID *)SmcDev);
      } else {
        NoSmc = 1;

        SmcReadValue (&SmcDev->SmcIo, '$Num', sizeof (NoSmc), (VOID *)&NoSmc);

        Index    = 1;
        SmcIndex = Index;

        if (NoSmc <= Index) {
          Status = EFI_SUCCESS;
        } else {
          do {
            Status = SmcWriteValue (&SmcDev->SmcIo, '$Num', sizeof (SmcIndex), (VOID *)&SmcIndex);

            if (!EFI_ERROR (Status)) {
              Status = SmcReadValue (&SmcDev->SmcIo, '$Adr', sizeof (SmcAddress), (VOID *)&SmcAddress);

              if (!EFI_ERROR (Status)) {
                SmcDevChild = (APPLE_SMC_DEV *)EfiLibAllocateZeroPool (sizeof (*SmcDevChild));

                if (SmcDevChild != NULL) {
                  SmcDevChild->Signature = SMC_DEV_SIGNATURE;
                  SmcDevChild->CpuIo     = CpuIo;

                  EfiInitializeLock (&SmcDevChild->Lock, EFI_TPL_NOTIFY);

                  gBS->CopyMem ((VOID *)&SmcDevChild->SmcIo, (VOID *)&SmcIoProtocol, sizeof (SmcIoProtocol));

                  SmcDevChild->SmcIo.Index   = Index;
                  SmcDevChild->SmcIo.Address = ((((SmcAddress & 0xFF0000) | (SmcAddress >> 16)) >> 8) | (((SmcAddress & 0xFF00) | (SmcAddress << 16)) << 8));

                  Status = gBS->InstallProtocolInterface (&SmcDevChild->Handle, &gAppleSmcIoProtocolGuid, EFI_NATIVE_INTERFACE, (VOID *)&SmcDevChild->SmcIo);

                  if (EFI_ERROR (Status)) {
                    gBS->FreePool ((VOID *)SmcDevChild);
                  }
                }
              }
            }

            ++Index;
            SmcIndex = Index;
          } while (Index < NoSmc);
        }
      }
    }
  }

  return Status;
}

static APPLE_SMC_IO_PROTOCOL SmcIoProtocol = {
  APPLE_SMC_IO_PROTOCOL_REVISION,
  SmcReadValue,
  SmcWriteValue,
  SmcGetKeyCount,
  SmcMakeKey,
  SmcGetKeyFromIndex,
  SmcGetKeyInfo,
  SmcReset,
  SmcFlashType,
  SmcUnsupported,
  SmcFlashWrite,
  SmcFlashAuth,
  0,
  SMC_PORT_BASE,
  FALSE
};
