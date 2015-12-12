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
/// @file      Protocol/AppleSmcIoImpl/AppleSmcIoImplPmioLib.c
///
///            
///
/// @author    Download-Fritz
/// @date      22/10/2015: Initial version
/// @date      09/12/2015: Reorganization
/// @copyright Copyright (C) 2005 - 2015 Apple Inc. All rights reserved.
///

#include <AppleEfi.h>

#include <Library/AppleDriverLib.h>

#include EFI_PROTOCOL_CONSUMER (CpuIo)

#include "AppleSmcIoImplInternal.h"

// ITERATION_STALL
#define ITERATION_STALL  50

// SmcIoSmcReadStatus
/// 
///
/// @param 
///
/// @return 
/// @retval 
SMC_STATUS
SmcIoSmcReadStatus (
  IN SMC_DEV  *SmcDev
  )
{
  SMC_STATUS Status;

  SmcDev->CpuIo->Io.Read (
                      SmcDev->CpuIo,
                      EfiCpuIoWidthUint8,
                      (SmcDev->SmcIo.Address + SMC_PORT_STATUS),
                      sizeof (Status),
                      (VOID *)&Status
                      );

  return Status;
}

// SmcIoSmcReadResult
/// 
///
/// @param 
///
/// @return 
/// @retval 
SMC_RESULT
SmcIoSmcReadResult (
  IN SMC_DEV  *SmcDev
  )
{
  SMC_RESULT Result;

  SmcDev->CpuIo->Io.Read (
                      SmcDev->CpuIo,
                      EfiCpuIoWidthUint8,
                      (SmcDev->SmcIo.Address + SMC_PORT_RESULT),
                      sizeof (Result),
                      (VOID *)&Result
                      );

  return Result;
}

// SmcIoSmcWriteCommand
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
SmcIoSmcWriteCommand (
  IN  SMC_DEV  *SmcDev,
  OUT UINT32   Command
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
      goto Return;
    }

    --Index;

    gBS->Stall (ITERATION_STALL);
  }

  SmcDev->CpuIo->Io.Write (
                      SmcDev->CpuIo,
                      EfiCpuIoWidthUint8,
                      (SmcDev->SmcIo.Address + SMC_PORT_COMMAND),
                      sizeof (SMC_COMMAND),
                      (VOID *)&Command
                      );

  Index = 20000;

  while ((SmcStatus & SMC_STATUS_BUSY) == 0) {
    SmcStatus = SmcIoSmcReadStatus (SmcDev);

    if (Index == 0) {
      Status = EFI_TIMEOUT;
      goto Return;
    }

    --Index;

    gBS->Stall (ITERATION_STALL);
  }

  Status = EFI_SUCCESS;

Return:
  return Status;
}

// SmcIoSmcReadData8
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
SmcIoSmcReadData8 (
  IN  SMC_DEV   *SmcDev,
  OUT SMC_DATA  *Data
  )
{
  EFI_STATUS Status;

  UINTN      Index;
  SMC_STATUS SmcStatus;
  SMC_DATA   Buffer;

  Index     = 60000;
  SmcStatus = SmcIoSmcReadStatus (SmcDev);

  while ((SmcStatus & (SMC_STATUS_AWAITING_MORE_BYTES | SMC_STATUS_BUSY)) == SMC_STATUS_BUSY) {
    SmcStatus = SmcIoSmcReadStatus (SmcDev);

    if (Index == 0) {
      Status = EFI_TIMEOUT;
      goto Return;
    }

    --Index;

    gBS->Stall (ITERATION_STALL);
  }

  if ((SmcStatus & SMC_STATUS_BUSY) != 0) {
    SmcDev->CpuIo->Io.Read (
                        SmcDev->CpuIo,
                        EfiCpuIoWidthUint8,
                        (SmcDev->SmcIo.Address + SMC_PORT_DATA),
                        sizeof (Buffer),
                        (VOID *)&Buffer
                        );

    Status = EFI_SUCCESS;
    *Data  = Buffer;
  } else {
    Status = EFI_NOT_READY;
  }

Return:
  return Status;
}

// SmcIoSmcReadData16
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
SmcIoSmcReadData16 (
  IN SMC_DEV  *SmcDev,
  IN UINT16   *Data
  )
{
  EFI_STATUS Status;

  UINT16     Value;

  Status = SmcIoSmcReadData8 (SmcDev, OFFSET (Value, 1, UINT8));

  if (!EFI_ERROR (Status)) {
    Status = SmcIoSmcReadData8 (SmcDev, OFFSET (Value, 0, UINT8));

    if (!EFI_ERROR (Status)) {
      gBS->CopyMem ((VOID *)Data, (VOID *)&Value, sizeof (Value));
    }
  }

  return Status;
}

// SmcIoSmcReadData32
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
SmcIoSmcReadData32 (
  IN SMC_DEV  *SmcDev,
  IN UINT32   *Data
  )
{
  EFI_STATUS Status;

  UINT32     Value;

  Status = SmcIoSmcReadData16 (SmcDev, OFFSET (Value, 1, UINT16));

  if (!EFI_ERROR (Status)) {
    Status = SmcIoSmcReadData16 (SmcDev, OFFSET (Value, 1, UINT16));

    if (!EFI_ERROR (Status)) {
      gBS->CopyMem ((VOID *)Data, (VOID *)&Value, sizeof (Value));
    }
  }

  return Status;
}

// SmcIoSmcWriteData8
/// 
///
/// @param 
///
/// @return 
/// @retval 
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
      goto Return;
    }

    --RemainingIterations;

    gBS->Stall (ITERATION_STALL);
  }

  if ((SmcStatus & SMC_STATUS_BUSY) != 0) {
    SmcDev->CpuIo->Io.Write (
                        SmcDev->CpuIo,
                        EfiCpuIoWidthUint8,
                        (SmcDev->SmcIo.Address + SMC_PORT_DATA),
                        sizeof (Data),
                        (VOID *)&Data
                        );

    Status = EFI_SUCCESS;
  } else {
    Status = EFI_NOT_READY;
  }

Return:
  return Status;
}

// SmcIoSmcWriteData16
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
SmcIoSmcWriteData16 (
  IN SMC_DEV  *SmcDev,
  IN UINT16   Data
  )
{
  EFI_STATUS Status;

  Status = SmcIoSmcWriteData8 (SmcDev, (SMC_DATA)(Data >> 8));

  if (!EFI_ERROR (Status)) {
    Status = SmcIoSmcWriteData8 (SmcDev, (SMC_DATA)Data);
  }

  return Status;
}

// SmcIoSmcWriteData32
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
SmcIoSmcWriteData32 (
  IN SMC_DEV  *SmcDev,
  IN UINT32   Data
  )
{
  EFI_STATUS Status;

  Status = SmcIoSmcWriteData8 (SmcDev, (SMC_DATA)(Data >> 24));

  if (!EFI_ERROR (Status)) {
    Status = SmcIoSmcWriteData8 (SmcDev, (SMC_DATA)(Data >> 16));

    if (!EFI_ERROR (Status)) {
      Status = SmcIoSmcWriteData8 (SmcDev, (SMC_DATA)(Data >> 8));

      if (!EFI_ERROR (Status)) {
        Status = SmcIoSmcWriteData8 (SmcDev, (SMC_DATA)Data);
      }
    }
  }

  return Status;
}

// SmcIoSmcTimeoutWaitingForBusyClear
/// 
///
/// @param 
///
/// @return 
/// @retval 
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
      goto Return;
    }

    --Index;

    gBS->Stall (ITERATION_STALL);
  }

  Status = EFI_SUCCESS;

Return:
  return Status;
}

// SmcIoSmcTimeoutWaitingLongForBusyClear
/// 
///
/// @param 
///
/// @return 
/// @retval 
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
      goto Return;
    }

    --Index;

    gBS->Stall (ITERATION_STALL);
  }

  Status = EFI_SUCCESS;

Return:
  return Status;
}

// SmcIoSmcSmcInABadState
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
SmcIoSmcSmcInABadState (
  IN SMC_DEV  *SmcDev
  )
{
  EFI_STATUS Status;

  Status = SmcIoSmcTimeoutWaitingForBusyClear (SmcDev);

  if (!EFI_ERROR (Status)) {
    Status = SmcIoSmcWriteCommand (SmcDev, SmcCmdReadValue);

    if (!EFI_ERROR (Status)) {
      Status = SmcIoSmcTimeoutWaitingForBusyClear (SmcDev);
    }
  }

  return Status;
}
