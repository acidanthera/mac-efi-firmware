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

#ifndef APPLE_SMC_IO_IMPL_H_
#define APPLE_SMC_IO_IMPL_H_

#include EFI_PROTOCOL_DEFINITION (CpuIo)
#include APPLE_PROTOCOL_PRODUCER (AppleSmcIo)

#include <Library/AppleDriverLib.h>

// APPLE_SMC_IO_PROTOCOL_REVISION
#define APPLE_SMC_IO_PROTOCOL_REVISION  0x33

/// @{
#define SMC_DEV_SIGNATURE    EFI_SIGNATURE_64 ('A', 'p', 'p', 'l', 'e', 'S', 'm', 'c')
#define SMC_DEV_FROM_THIS(x) CR ((x), SMC_DEV, SmcIo, SMC_DEV_SIGNATURE)
/// @}

#pragma pack (1)

// SMC_DEV
typedef PACKED struct SMC_DEV {
  UINT64                Signature;  ///<
  EFI_HANDLE            Handle;     ///<
  EFI_LOCK              Lock;       ///<
  APPLE_SMC_IO_PROTOCOL SmcIo;      ///<
  EFI_CPU_IO_PROTOCOL   *CpuIo;     ///<
} SMC_DEV;

#pragma pack ()

// SmcIoSmcReadValueImpl
EFI_STATUS
EFIAPI
SmcIoSmcReadValueImpl (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  IN  SMC_KEY                Key,
  IN  SMC_DATA_SIZE          Size,
  OUT SMC_DATA               *Value
  );

// SmcIoSmcWriteValueImpl
EFI_STATUS
EFIAPI
SmcIoSmcWriteValueImpl (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  IN  SMC_KEY                Key,
  IN  UINT32                 Size,
  OUT SMC_DATA               *Value
  );

// SmcIoSmcMakeKeyImpl
EFI_STATUS
EFIAPI
SmcIoSmcMakeKeyImpl (
  IN  CHAR8    *Name,
  OUT SMC_KEY  *Key
  );

// SmcIoSmcGetKeyCountImpl
EFI_STATUS
EFIAPI
SmcIoSmcGetKeyCountImpl (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  OUT UINT32                 *Count
  );

// SmcIoSmcGetKeyFromIndexImpl
EFI_STATUS
EFIAPI
SmcIoSmcGetKeyFromIndexImpl (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  IN  SMC_INDEX              Index,
  OUT SMC_KEY                *Key
  );

// SmcIoSmcGetKeyInfoImpl
EFI_STATUS
EFIAPI
SmcIoSmcGetKeyInfoImpl (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  IN  SMC_KEY                Key,
  OUT SMC_DATA_SIZE          *Size,
  OUT SMC_KEY_TYPE           *Type,
  OUT SMC_KEY_ATTRIBUTES     *Attributes
  );

// SmcIoSmcResetImpl
EFI_STATUS
EFIAPI
SmcIoSmcResetImpl (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINT32                 Mode
  );

// SmcIoSmcFlashTypeImpl
EFI_STATUS
EFIAPI
SmcIoSmcFlashTypeImpl (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINT32                 Type
  );

// SmcIoSmcFlashWriteImpl
EFI_STATUS
EFIAPI
SmcIoSmcFlashWriteImpl (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINT32                 Unknown,
  IN UINT32                 Size,
  IN SMC_DATA               *Data
  );

// SmcIoSmcFlashAuthImpl
EFI_STATUS
EFIAPI
SmcIoSmcFlashAuthImpl (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINT32                 Size,
  IN SMC_DATA               *Data
  );

// SmcIoSmcUnsupportedImpl
EFI_STATUS
EFIAPI
SmcIoSmcUnsupportedImpl (
  VOID
  );

// SmcIoSmcUnknown1Impl
EFI_STATUS
EFIAPI
SmcIoSmcUnknown1Impl (
  VOID
  );

// SmcIoSmcUnknown2Impl
EFI_STATUS
EFIAPI
SmcIoSmcUnknown2Impl (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINTN                  Ukn1,
  IN UINTN                  Ukn2
  );

// SmcIoSmcUnknown3Impl
EFI_STATUS
EFIAPI
SmcIoSmcUnknown3Impl (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINTN                  Ukn1,
  IN UINTN                  Ukn2
  );

// SmcIoSmcUnknown4Impl
EFI_STATUS
EFIAPI
SmcIoSmcUnknown4Impl (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINTN                  Ukn1
  );

// SmcIoSmcUnknown5Impl
EFI_STATUS
EFIAPI
SmcIoSmcUnknown5Impl (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINTN                  Ukn1
  );

#endif // APPLE_SMC_IO_IMPL_H_
