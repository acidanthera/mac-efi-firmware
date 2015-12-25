/** @file
  Copyright (C) 2005 - 2015 Apple Inc.  All rights reserved.<BR>

  This program and the accompanying materials have not been licensed.
  Neither is its usage, its redistribution, in source or binary form,
  licensed, nor implicitely or explicitely permitted, except when
  required by applicable law.

  Unless required by applicable law or agreed to in writing, software
  distributed is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
  OR CONDITIONS OF ANY KIND, either express or implied.
**/

#ifndef APPLE_SMC_IO_IMPL_INTERNAL_H_
#define APPLE_SMC_IO_IMPL_INTERNAL_H_

#include <Protocol/AppleSmcIoImpl.h>

// SmcIoSmcReadStatus
SMC_STATUS
SmcIoSmcReadStatus (
  IN SMC_DEV  *SmcDev UNALIGNED
  );

// SmcIoSmcReadResult
SMC_RESULT
SmcIoSmcReadResult (
  IN SMC_DEV  *SmcDev UNALIGNED
  );

// SmcIoSmcWriteCommand
EFI_STATUS
SmcIoSmcWriteCommand (
  IN  SMC_DEV  *SmcDev, UNALIGNED
  OUT UINT32   Command
  );

// SmcIoSmcReadData8
EFI_STATUS
SmcIoSmcReadData8 (
  IN  SMC_DEV   *SmcDev, UNALIGNED
  OUT SMC_DATA  *Data
  );

// SmcIoSmcReadData16
EFI_STATUS
SmcIoSmcReadData16 (
  IN SMC_DEV  *SmcDev, UNALIGNED
  IN UINT16   *Data
  );

// SmcIoSmcReadData32
EFI_STATUS
SmcIoSmcReadData32 (
  IN SMC_DEV  *SmcDev, UNALIGNED
  IN UINT32   *Data
  );

// SmcIoSmcWriteData8
EFI_STATUS
SmcIoSmcWriteData8 (
  IN SMC_DEV   *SmcDev, UNALIGNED
  IN SMC_DATA  Data
  );

// SmcIoSmcWriteData16
EFI_STATUS
SmcIoSmcWriteData16 (
  IN SMC_DEV  *SmcDev, UNALIGNED
  IN UINT16   Data
  );

// SmcIoSmcWriteData32
EFI_STATUS
SmcIoSmcWriteData32 (
  IN SMC_DEV  *SmcDev, UNALIGNED
  IN UINT32   Data
  );

// SmcIoSmcTimeoutWaitingForBusyClear
EFI_STATUS
SmcIoSmcTimeoutWaitingForBusyClear (
  IN SMC_DEV  *SmcDev UNALIGNED
  );

// SmcIoSmcTimeoutWaitingLongForBusyClear
EFI_STATUS
SmcIoSmcTimeoutWaitingLongForBusyClear (
  IN SMC_DEV  *SmcDev UNALIGNED
  );

// SmcIoSmcSmcInABadState
EFI_STATUS
SmcIoSmcSmcInABadState (
  IN SMC_DEV  *SmcDev UNALIGNED
  );

#endif // APPLE_SMC_IO_IMPL_INTERNAL_H_
