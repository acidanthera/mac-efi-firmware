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

// APPLE_SMC_IO_PROTOCOL_REVISION
#define APPLE_SMC_IO_PROTOCOL_REVISION  0x33

/// @{
#define SMC_DEV_SIGNATURE    EFI_SIGNATURE_64 ('A', 'p', 'p', 'l', 'e', 'S', 'm', 'c')
#define SMC_DEV_FROM_THIS(x) CR ((x), SMC_DEV, SmcIo, SMC_DEV_SIGNATURE)
/// @}

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

// SmcIoSmcReadValueImpl
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
SmcIoSmcReadValueImpl (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  IN  SMC_KEY                Key,
  IN  SMC_DATA_SIZE          Size,
  OUT SMC_DATA               *Value
  );

// SmcIoSmcWriteValueImpl
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
SmcIoSmcWriteValueImpl (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  IN  SMC_KEY                Key,
  IN  UINT32                 Size,
  OUT SMC_DATA               *Value
  );

// SmcIoSmcMakeKeyImpl
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
SmcIoSmcMakeKeyImpl (
  IN  CHAR8    *Name,
  OUT SMC_KEY  *Key
  );

// SmcIoSmcGetKeyCountImpl
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
SmcIoSmcGetKeyCountImpl (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  OUT UINT32                 *Count
  );

// SmcIoSmcGetKeyFromIndexImpl
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
SmcIoSmcGetKeyFromIndexImpl (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  IN  SMC_INDEX              Index,
  OUT SMC_KEY                *Key
  );

// SmcIoSmcGetKeyInfoImpl
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
SmcIoSmcGetKeyInfoImpl (
  IN     APPLE_SMC_IO_PROTOCOL  *This,
  IN     SMC_KEY                Key,
  IN OUT SMC_DATA_SIZE          *Size,
  IN OUT SMC_KEY_TYPE           *Type,
  IN OUT SMC_KEY_ATTRIBUTES     *Attributes
  );

// SmcIoSmcResetImpl
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
SmcIoSmcResetImpl (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINT32                 Mode
  );

// SmcIoSmcFlashTypeImpl
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
SmcIoSmcFlashTypeImpl (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINT32                 Type
  );

// SmcIoSmcFlashWriteImpl
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
SmcIoSmcFlashWriteImpl (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINT32                 Unknown,
  IN UINT32                 Size,
  IN SMC_DATA               *Data
  );

// SmcIoSmcFlashAuthImpl
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
SmcIoSmcFlashAuthImpl (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINT32                 Size,
  IN SMC_DATA               *Data
  );

// SmcIoSmcUnsupportedImpl
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
SmcIoSmcUnsupportedImpl (
  VOID
  );

// SmcIoSmcUnknown1Impl
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
SmcIoSmcUnknown1Impl (
  VOID
  );

// SmcIoSmcUnknown2Impl
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
SmcIoSmcUnknown2Impl (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINTN                  Ukn1,
  IN UINTN                  Ukn2
  );

// SmcIoSmcUnknown3Impl
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
SmcIoSmcUnknown3Impl (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINTN                  Ukn1,
  IN UINTN                  Ukn2
  );

// SmcIoSmcUnknown4Impl
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
SmcIoSmcUnknown4Impl (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINTN                  Ukn1
  );

// SmcIoSmcUnknown5Impl
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
SmcIoSmcUnknown5Impl (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINTN                  Ukn1
  );

#endif // ifndef __APPLE_SMC_IO_IMPL_H__
