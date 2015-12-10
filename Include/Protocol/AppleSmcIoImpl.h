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
/// @file      Include/Protocol/AppleSmcIoImpl.h
///
///            
///
/// @author    Download-Fritz
/// @date      05/12/2015: Initial version
/// @copyright Copyright (C) 2005 - 2015 Apple Inc. All rights reserved.
///

#ifndef __APPLE_SMC_IO_IMPL_H__
#define __APPLE_SMC_IO_IMPL_H__

#include <Protocol/AppleSmcIo.h>

#define APPLE_SMC_IO_PROTOCOL_REVISION  0x33

#define SMC_DEV_SIGNATURE    EFI_SIGNATURE_64 ('A', 'p', 'p', 'l', 'e', 'S', 'm', 'c')
#define SMC_DEV_FROM_THIS(x) CR ((x), SMC_DEV, SmcIo, SMC_DEV_SIGNATURE)

#pragma pack(1)

// _SMC_DEV
typedef PACKED struct _SMC_DEV {
  UINT64                Signature;  ///<
  EFI_HANDLE            Handle;     ///<
  EFI_LOCK              Lock;       ///<
  APPLE_SMC_IO_PROTOCOL SmcIo;      ///<
  EFI_CPU_IO_PROTOCOL   *CpuIo;     ///<
} SMC_DEV;

#pragma pack()

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

// SmcWriteData8
EFI_STATUS
SmcWriteData8 (
  IN SMC_DEV   *SmcDev,
  IN SMC_DATA  Data
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

// SmcGetKeyCount
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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

#endif // ifndef __APPLE_SMC_IO_IMPL_H__
