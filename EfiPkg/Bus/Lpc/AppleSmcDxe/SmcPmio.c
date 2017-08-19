/** @file
  Copyright (c) 2005 - 2015, Apple Inc.  All rights reserved.<BR>

  This program and the accompanying materials have not been licensed.
  Neither is its usage, its redistribution, in source or binary form,
  licensed, nor implicitely or explicitely permitted, except when
  required by applicable law.

  Unless required by applicable law or agreed to in writing, software
  distributed is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
  OR CONDITIONS OF ANY KIND, either express or implied.
**/

#include <AppleMacEfi.h>

#include <Protocol/AppleSmcIo.h>

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "SmcIoInternal.h"

// ITERATION_STALL
#define ITERATION_STALL  50

// SmcIoSmcReadStatus
SMC_STATUS
SmcIoSmcReadStatus (
  IN SMC_DEV  *SmcDev
  )
{
  return IoRead8 (SmcDev->SmcIo.Address + SMC_PORT_OFFSET_STATUS);
}

// SmcIoSmcReadResult
SMC_RESULT
SmcIoSmcReadResult (
  IN SMC_DEV  *SmcDev
  )
{
  return IoRead8 (SmcDev->SmcIo.Address + SMC_PORT_OFFSET_RESULT);
}

// SmcIoSmcWriteCommand
EFI_STATUS
SmcIoSmcWriteCommand (
  IN SMC_DEV      *SmcDev,
  IN SMC_COMMAND  Command
  )
{
  EFI_STATUS  Status;

  UINTN       Index;
  SMC_STATUS  SmcStatus;

  Index     = 60000;
  SmcStatus = SmcIoSmcReadStatus (SmcDev);

  while ((SmcStatus & SMC_STATUS_IB_CLOSED) != 0) {
    SmcStatus = SmcIoSmcReadStatus (SmcDev);

    if (Index == 0) {
      Status = EFI_TIMEOUT;
      goto Done;
    }

    --Index;

    gBS->Stall (ITERATION_STALL);
  }

  IoWrite8 ((SmcDev->SmcIo.Address + SMC_PORT_OFFSET_COMMAND), Command);

  Index = 20000;

  while ((SmcStatus & SMC_STATUS_BUSY) == 0) {
    SmcStatus = SmcIoSmcReadStatus (SmcDev);

    if (Index == 0) {
      Status = EFI_TIMEOUT;
      goto Done;
    }

    --Index;

    gBS->Stall (ITERATION_STALL);
  }

  Status = EFI_SUCCESS;

Done:
  return Status;
}

// SmcIoSmcReadData8
EFI_STATUS
SmcIoSmcReadData8 (
  IN  SMC_DEV  *SmcDev,
  OUT UINT8    *Data
  )
{
  EFI_STATUS Status;

  UINTN      Index;
  SMC_STATUS SmcStatus;
  SMC_DATA   Buffer;

  Index     = 60000;
  SmcStatus = SmcIoSmcReadStatus (SmcDev);

  while ((SmcStatus & (SMC_STATUS_AWAITING_DATA | SMC_STATUS_BUSY)) == SMC_STATUS_BUSY) {
    SmcStatus = SmcIoSmcReadStatus (SmcDev);

    if (Index == 0) {
      Status = EFI_TIMEOUT;
      goto Done;
    }

    --Index;

    gBS->Stall (ITERATION_STALL);
  }

  if ((SmcStatus & SMC_STATUS_BUSY) != 0) {
    Buffer = IoRead8 (SmcDev->SmcIo.Address + SMC_PORT_OFFSET_DATA);

    Status = EFI_SUCCESS;
    *Data  = Buffer;
  } else {
    Status = EFI_NOT_READY;
  }

Done:
  return Status;
}

// SmcIoSmcReadData16
EFI_STATUS
SmcIoSmcReadData16 (
  IN SMC_DEV  *SmcDev,
  IN UINT16   *Data
  )
{
  EFI_STATUS Status;

  UINT8      Value[2];

  Status = SmcIoSmcReadData8 (SmcDev, &Value[1]);

  if (!EFI_ERROR (Status)) {
    Status = SmcIoSmcReadData8 (SmcDev, &Value[0]);

    if (!EFI_ERROR (Status)) {
      CopyMem ((VOID *)Data, (VOID *)&Value[0], sizeof (Value));
    }
  }

  return Status;
}

// SmcIoSmcReadData32
EFI_STATUS
SmcIoSmcReadData32 (
  IN SMC_DEV  *SmcDev,
  IN UINT32   *Data
  )
{
  EFI_STATUS Status;

  UINT16     Value[2];

  Status = SmcIoSmcReadData16 (SmcDev, &Value[1]);

  if (!EFI_ERROR (Status)) {
    Status = SmcIoSmcReadData16 (SmcDev, &Value[0]);

    if (!EFI_ERROR (Status)) {
      CopyMem ((VOID *)Data, (VOID *)&Value[0], sizeof (Value));
    }
  }

  return Status;
}

// SmcIoSmcWriteData8
EFI_STATUS
SmcIoSmcWriteData8 (
  IN SMC_DEV   *SmcDev,
  IN SMC_DATA  Data
  )
{
  EFI_STATUS Status;

  UINTN      RemainingIterations;
  SMC_STATUS SmcStatus;

  RemainingIterations = 60000;
  SmcStatus           = SmcIoSmcReadStatus (SmcDev);

  while ((SmcStatus & SMC_STATUS_IB_CLOSED) != 0) {
    SmcStatus = SmcIoSmcReadStatus (SmcDev);

    if (RemainingIterations == 0) {
      Status = EFI_TIMEOUT;
      goto Done;
    }

    --RemainingIterations;

    gBS->Stall (ITERATION_STALL);
  }

  if ((SmcStatus & SMC_STATUS_BUSY) != 0) {
    IoWrite8 ((SmcDev->SmcIo.Address + SMC_PORT_OFFSET_DATA), Data);

    Status = EFI_SUCCESS;
  } else {
    Status = EFI_NOT_READY;
  }

Done:
  return Status;
}

// SmcIoSmcWriteData16
EFI_STATUS
SmcIoSmcWriteData16 (
  IN SMC_DEV  *SmcDev,
  IN UINT16   Data
  )
{
  EFI_STATUS Status;

  UINT8      *DataPtr;

  DataPtr = (UINT8 *)&Data;

  Status = SmcIoSmcWriteData8 (SmcDev, DataPtr[1]);

  if (!EFI_ERROR (Status)) {
    Status = SmcIoSmcWriteData8 (SmcDev, DataPtr[0]);
  }

  return Status;
}

// SmcIoSmcWriteData32
EFI_STATUS
SmcIoSmcWriteData32 (
  IN SMC_DEV  *SmcDev,
  IN UINT32   Data
  )
{
  EFI_STATUS Status;

  UINT8      *DataPtr;

  DataPtr = (UINT8 *)&Data;

  Status = SmcIoSmcWriteData8 (SmcDev, DataPtr[3]);

  if (!EFI_ERROR (Status)) {
    Status = SmcIoSmcWriteData8 (SmcDev, DataPtr[2]);

    if (!EFI_ERROR (Status)) {
      Status = SmcIoSmcWriteData8 (SmcDev, DataPtr[1]);

      if (!EFI_ERROR (Status)) {
        Status = SmcIoSmcWriteData8 (SmcDev, DataPtr[0]);
      }
    }
  }

  return Status;
}

// SmcIoSmcTimeoutWaitingForBusyClear
EFI_STATUS
SmcIoSmcTimeoutWaitingForBusyClear (
  IN SMC_DEV  *SmcDev
  )
{
  EFI_STATUS Status;

  SMC_STATUS SmcStatus;
  UINTN      Index;

  Index     = 20000;
  SmcStatus = SmcIoSmcReadStatus (SmcDev);

  while ((SmcStatus & SMC_STATUS_BUSY) != 0) {
    SmcStatus = SmcIoSmcReadStatus (SmcDev);

    if (Index == 0) {
      Status = EFI_TIMEOUT;
      goto Done;
    }

    --Index;

    gBS->Stall (ITERATION_STALL);
  }

  Status = EFI_SUCCESS;

Done:
  return Status;
}

// SmcIoSmcTimeoutWaitingLongForBusyClear
EFI_STATUS
SmcIoSmcTimeoutWaitingLongForBusyClear (
  IN SMC_DEV  *SmcDev
  )
{
  EFI_STATUS Status;

  SMC_STATUS SmcStatus;
  UINTN      Index;

  Index     = 100000;
  SmcStatus = SmcIoSmcReadStatus (SmcDev);

  while ((SmcStatus & SMC_STATUS_BUSY) != 0) {
    SmcStatus = SmcIoSmcReadStatus (SmcDev);

    if (Index == 0) {
      Status = EFI_TIMEOUT;
      goto Done;
    }

    --Index;

    gBS->Stall (ITERATION_STALL);
  }

  Status = EFI_SUCCESS;

Done:
  return Status;
}

// SmcIoSmcSmcInABadState
EFI_STATUS
SmcIoSmcSmcInABadState (
  IN SMC_DEV  *SmcDev
  )
{
  EFI_STATUS Status;

  Status = SmcIoSmcTimeoutWaitingForBusyClear (SmcDev);

  if (EFI_ERROR (Status)) {
    Status = SmcIoSmcWriteCommand (SmcDev, SmcCmdReadValue);

    if (!EFI_ERROR (Status)) {
      Status = SmcIoSmcTimeoutWaitingForBusyClear (SmcDev);
    }
  }

  return Status;
}
