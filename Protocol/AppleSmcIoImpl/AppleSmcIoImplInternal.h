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
/// @file      Protocol/AppleSmcIoImpl/AppleSmcIoImplInternal.h
///
///            
///
/// @author    Download-Fritz
/// @date      12/12/2015: Initial version
/// @copyright Copyright (C) 2005 - 2015 Apple Inc. All rights reserved.
///

#ifndef __APPLE_SMC_IO_IMPL_INTERNAL_H__
#define __APPLE_SMC_IO_IMPL_INTERNAL_H__

#include <Protocol/AppleSmcIoImpl.h>

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

#endif // ifndef __APPLE_SMC_IO_IMPL_INTERNAL_H__
