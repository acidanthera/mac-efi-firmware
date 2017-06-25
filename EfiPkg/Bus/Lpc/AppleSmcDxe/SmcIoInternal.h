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

#ifndef SMC_IO_INTERNAL_H_
#define SMC_IO_INTERNAL_H_

#include <Library/UefiLib.h>

// SMC_DEV_SIGNATURE
#define SMC_DEV_SIGNATURE  \
  SIGNATURE_64 ('A', 'p', 'p', 'l', 'e', 'S', 'm', 'c')

// SMC_DEV_FROM_THIS
#define SMC_DEV_FROM_THIS(x) CR ((x), SMC_DEV, SmcIo, SMC_DEV_SIGNATURE)

// SMC_KEY_PRESENCE_MAP
typedef struct {
  SMC_KEY Key;
  BOOLEAN Present;
} SMC_KEY_PRESENCE_MAP;

// SMC_DEV
typedef struct SMC_DEV {
  UINT64                Signature;                ///<
  EFI_HANDLE            Handle;                   ///<
  EFI_LOCK              Lock;                     ///<
  APPLE_SMC_IO_PROTOCOL SmcIo;                    ///<
  UINT32                KeyPresenceMapLength;     ///<
  UINT32                MaxKeyPresenceMapLength;  ///<
  SMC_KEY_PRESENCE_MAP  *KeyPresenceMap;          ///<
} SMC_DEV;

// SmcIoSmcReadStatus
SMC_STATUS
SmcIoSmcReadStatus (
  IN SMC_DEV  *SmcDev
  );

// SmcIoSmcReadResult
SMC_RESULT
SmcIoSmcReadResult (
  IN SMC_DEV  *SmcDev
  );

// SmcIoSmcWriteCommand
EFI_STATUS
SmcIoSmcWriteCommand (
  IN  SMC_DEV  *SmcDev,
  OUT UINT32   Command
  );

// SmcIoSmcReadData8
EFI_STATUS
SmcIoSmcReadData8 (
  IN  SMC_DEV  *SmcDev,
  OUT UINT8    *Data
  );

// SmcIoSmcReadData16
EFI_STATUS
SmcIoSmcReadData16 (
  IN SMC_DEV  *SmcDev,
  IN UINT16   *Data
  );

// SmcIoSmcReadData32
EFI_STATUS
SmcIoSmcReadData32 (
  IN SMC_DEV  *SmcDev,
  IN UINT32   *Data
  );

// SmcIoSmcWriteData8
EFI_STATUS
SmcIoSmcWriteData8 (
  IN SMC_DEV   *SmcDev,
  IN SMC_DATA  Data
  );

// SmcIoSmcWriteData16
EFI_STATUS
SmcIoSmcWriteData16 (
  IN SMC_DEV  *SmcDev,
  IN UINT16   Data
  );

// SmcIoSmcWriteData32
EFI_STATUS
SmcIoSmcWriteData32 (
  IN SMC_DEV  *SmcDev,
  IN UINT32   Data
  );

// SmcIoSmcTimeoutWaitingForBusyClear
EFI_STATUS
SmcIoSmcTimeoutWaitingForBusyClear (
  IN SMC_DEV  *SmcDev
  );

// SmcIoSmcTimeoutWaitingLongForBusyClear
EFI_STATUS
SmcIoSmcTimeoutWaitingLongForBusyClear (
  IN SMC_DEV  *SmcDev
  );

// SmcIoSmcSmcInABadState
EFI_STATUS
SmcIoSmcSmcInABadState (
  IN SMC_DEV  *SmcDev
  );

// SMC MMIO

// SmcReadValueMmio
EFI_STATUS
SmcReadValueMmio (
  IN     SMC_ADDRESS    BaseAddress,
  IN     SMC_KEY        Key,
  IN OUT SMC_DATA_SIZE  *Size,
  OUT    SMC_DATA       *Value
  );

// SmcWriteValueMmio
EFI_STATUS
SmcWriteValueMmio (
  IN SMC_ADDRESS  BaseAddress,
  IN SMC_KEY      Key,
  IN UINT32       Size,
  IN SMC_DATA     *Value
  );

// SmcGetKeyFromIndexMmio
EFI_STATUS
SmcGetKeyFromIndexMmio (
  IN SMC_ADDRESS  BaseAddress,
  IN SMC_INDEX    Index,
  IN SMC_KEY      *Key
  );

// SmcGetKeyInfoMmio
EFI_STATUS
SmcGetKeyInfoMmio (
  IN SMC_ADDRESS         BaseAddress,
  IN SMC_KEY             Key,
  IN SMC_DATA_SIZE       *Size,
  IN SMC_KEY_TYPE        *Type,
  IN SMC_KEY_ATTRIBUTES  *Attributes
  );

// SmcFlashTypeMmio
EFI_STATUS
SmcFlashTypeMmio (
  IN SMC_ADDRESS     BaseAddress,
  IN SMC_FLASH_TYPE  Type
  );

// SmcFlashWriteMmio
EFI_STATUS
SmcFlashWriteMmio (
  IN SMC_ADDRESS  BaseAddress,
  IN UINT32       Unknown,
  IN UINT32       Size,
  IN SMC_DATA     *Data
  );

// SmcFlashAuthMmio
EFI_STATUS
SmcFlashAuthMmio (
  IN SMC_ADDRESS  BaseAddress,
  IN UINT32       Size,
  IN SMC_DATA     *Data
  );

// SmcMmioInterface
BOOLEAN
SmcMmioInterface (
  IN SMC_ADDRESS  BaseAddress
  );

// SmcResetMmio
EFI_STATUS
SmcResetMmio (
  IN SMC_ADDRESS  BaseAddress,
  IN UINT32       Mode
  );

// Virtual SMC

// SmcIoVirtualSmcReadValue
EFI_STATUS
SmcIoVirtualSmcReadValue (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  IN  SMC_KEY                Key,
  IN  SMC_DATA_SIZE          Size,
  OUT SMC_DATA               *Value
  );

// SmcIoVirtualSmcWriteValue
EFI_STATUS
SmcIoVirtualSmcWriteValue (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  IN  SMC_KEY                Key,
  IN  SMC_DATA_SIZE          Size,
  OUT SMC_DATA               *Value
  );

// SmcIoVirtualSmcMakeKey
EFI_STATUS
SmcIoVirtualSmcMakeKey (
  IN  CHAR8    *Name,
  OUT SMC_KEY  *Key
  );

// SmcGetyKeyCount
EFI_STATUS
SmcIoVirtualSmcGetKeyCount (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  OUT UINT32                 *Count
  );

// SmcIoVirtualSmcGetKeyFromIndex
EFI_STATUS
SmcIoVirtualSmcGetKeyFromIndex (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  IN  SMC_INDEX              Index,
  OUT SMC_KEY                *Key
  );

// SmcIoVirtualSmcGetKeyInfo
EFI_STATUS
SmcIoVirtualSmcGetKeyInfo (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  IN  SMC_KEY                Key,
  OUT SMC_DATA_SIZE          *Size,
  OUT SMC_KEY_TYPE           *Type,
  OUT SMC_KEY_ATTRIBUTES     *Attributes
  );

// SmcIoVirtualSmcReset
EFI_STATUS
SmcIoVirtualSmcReset (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINT32                 Mode
  );

// SmcIoVirtualSmcFlashType
EFI_STATUS
SmcIoVirtualSmcFlashType (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN SMC_FLASH_TYPE         Type
  );

// SmcIoVirtualSmcFlashWrite
EFI_STATUS
SmcIoVirtualSmcFlashWrite (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINT32                 Unknown,
  IN SMC_FLASH_SIZE         Size,
  IN SMC_DATA               *Data
  );

// SmcIoVirtualSmcFlashAuth
EFI_STATUS
SmcIoVirtualSmcFlashAuth (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN SMC_FLASH_SIZE         Size,
  IN SMC_DATA               *Data
  );

#endif // SMC_IO_INTERNAL_H_
