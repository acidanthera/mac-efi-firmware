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
/// @date      09/12/2015: Reorganize
/// @copyright Copyright (C) 2005 - 2015 Apple Inc. All rights reserved.
///

#include <AppleEfi.h>

#include <EfiDriverLib.h>

#include EFI_PROTOCOL_CONSUMER (CpuIo)
#include <Protocol/AppleSmcIoImpl.h>

// SmcReadStatus
/// 
///
/// @param 
///
/// @return 
/// @retval 
SMC_STATUS
SmcReadStatus (
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

// SmcReadResult
/// 
///
/// @param 
///
/// @return 
/// @retval 
SMC_RESULT
SmcReadResult (
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

// SmcWriteCommand
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
SmcWriteCommand (
  IN  SMC_DEV  *SmcDev,
  OUT UINT32   Command
  )
{
  EFI_STATUS  Status;

  UINTN       Index;
  SMC_STATUS  SmcStatus;

  Index     = 60000;
  SmcStatus = SmcReadStatus (SmcDev);

  while ((SmcStatus & SMC_STATUS_IB_CLOSED) != 0) {
    SmcStatus = SmcReadStatus (SmcDev);

    if (Index == 0) {
      Status = EFI_TIMEOUT;
      goto Return;
    }

    --Index;

    gBS->Stall (50);
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
    SmcStatus = SmcReadStatus (SmcDev);

    if (Index == 0) {
      Status = EFI_TIMEOUT;
      goto Return;
    }

    --Index;

    gBS->Stall (50);
  }

  Status = EFI_SUCCESS;

Return:
  return Status;
}

// SmcReadData8
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
SmcReadData8 (
  IN  SMC_DEV   *SmcDev,
  OUT SMC_DATA  *Data
  )
{
  EFI_STATUS Status;

  UINTN      Index;
  SMC_STATUS SmcStatus;
  SMC_DATA   Buffer;

  Index     = 60000;
  SmcStatus = SmcReadStatus (SmcDev);

  while ((SmcStatus & (SMC_STATUS_AWAITING_MORE_BYTES | SMC_STATUS_BUSY)) == SMC_STATUS_BUSY) {
    SmcStatus = SmcReadStatus (SmcDev);

    if (Index == 0) {
      Status = EFI_TIMEOUT;
      goto Return;
    }

    --Index;

    gBS->Stall (50);
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

// SmcReadData16
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
SmcReadData16 (
  IN SMC_DEV  *SmcDev,
  IN UINT16   *Data
  )
{
  EFI_STATUS Status;

  UINT16     Value;

  Status = SmcReadData8 (SmcDev, OFFSET (Value, 1, UINT8));

  if (!EFI_ERROR (Status)) {
    Status = SmcReadData8 (SmcDev, OFFSET (Value, 0, UINT8));

    if (!EFI_ERROR (Status)) {
      gBS->CopyMem ((VOID *)Data, (VOID *)&Value, sizeof (Value));
    }
  }

  return Status;
}

// SmcReadData32
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
SmcReadData32 (
  IN SMC_DEV  *SmcDev,
  IN UINT32   *Data
  )
{
  EFI_STATUS Status;

  UINT32     Value;

  Status = SmcReadData16 (SmcDev, OFFSET (Value, 1, UINT16));

  if (!EFI_ERROR (Status)) {
    Status = SmcReadData16 (SmcDev, OFFSET (Value, 1, UINT16));

    if (!EFI_ERROR (Status)) {
      gBS->CopyMem ((VOID *)Data, (VOID *)&Value, sizeof (Value));
    }
  }

  return Status;
}

// SmcWriteData8
EFI_STATUS
SmcWriteData8 (
  IN SMC_DEV   *SmcDev,
  IN SMC_DATA  Data
  )
{
  EFI_STATUS Status;

  UINTN      RemainingIterations;
  SMC_STATUS SmcStatus;

  RemainingIterations = 60000;
  SmcStatus           = SmcReadStatus (SmcDev);

  while ((SmcStatus & SMC_STATUS_IB_CLOSED) != 0) {
    SmcStatus = SmcReadStatus (SmcDev);

    if (RemainingIterations == 0) {
      Status = EFI_TIMEOUT;
      goto Return;
    }

    --RemainingIterations;

    gBS->Stall (50);
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

// SmcWriteData16
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
SmcWriteData16 (
  IN SMC_DEV  *SmcDev,
  IN UINT16   Data
  )
{
  EFI_STATUS Status;

  Status = SmcWriteData8 (SmcDev, (SMC_DATA)(Data >> 8));

  if (!EFI_ERROR (Status)) {
    Status = SmcWriteData8 (SmcDev, (SMC_DATA)Data);
  }

  return Status;
}

// SmcWriteData32
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
SmcWriteData32 (
  IN SMC_DEV  *SmcDev,
  IN UINT32   Data
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

// SmcTimeoutWaitingForBusyClear
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
SmcTimeoutWaitingForBusyClear (
  IN SMC_DEV  *SmcDev
  )
{
  EFI_STATUS Status;

  SMC_STATUS SmcStatus;
  UINTN      Index;

  Index     = 20000;
  SmcStatus = SmcReadStatus (SmcDev);

  while ((SmcStatus & SMC_STATUS_BUSY) != 0) {
    SmcStatus = SmcReadStatus (SmcDev);

    if (Index == 0) {
      Status = EFI_TIMEOUT;
      goto Return;
    }

    --Index;

    gBS->Stall (50);
  }

  Status = EFI_SUCCESS;

Return:
  return Status;
}

// SmcTimeoutWaitingLongForBusyClear
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
SmcTimeoutWaitingLongForBusyClear (
  IN SMC_DEV  *SmcDev
  )
{
  EFI_STATUS Status;

  SMC_STATUS SmcStatus;
  UINTN      Index;

  Index     = 100000;
  SmcStatus = SmcReadStatus (SmcDev);

  while ((SmcStatus & SMC_STATUS_BUSY) != 0) {
    SmcStatus = SmcReadStatus (SmcDev);

    if (Index == 0) {
      Status = EFI_TIMEOUT;
      goto Return;
    }

    --Index;

    gBS->Stall (50);
  }

  Status = EFI_SUCCESS;

Return:
  return Status;
}

// SmcSmcInABadState
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
SmcSmcInABadState (
  IN SMC_DEV  *SmcDev
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
