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

#ifndef APPLE_PLATFORM_INFO_DATABASE_IMPL_H_
#define APPLE_PLATFORM_INFO_DATABASE_IMPL_H_

#include EFI_PROTOCOL_CONSUMER (FirmwareVolume)
#include APPLE_PROTOCOL_PRODUCER (ApplePlatformInfoDatabase)

// APPLE_PLATFORM_INFO_DATABASE_PROTOCOL_REVISION
#define APPLE_PLATFORM_INFO_DATABASE_PROTOCOL_REVISION  0x01

/// @{
#define APPLE_PLATFORM_INFO_DATABASE_SIGNATURE  EFI_SIGNATURE_32 ('P', 'I', 'D', 'B')
#define PLATFORM_INFO_PROTOCOL_FROM_DATABASE(Database) \
  CR (Database, APPLE_PLATFORM_INFO_DATABASE, Protocol, APPLE_PLATFORM_INFO_DATABASE_SIGNATURE)
/// @}

// APPLE_PLATFORM_INFO_DATABASE
typedef struct {
  UINT32                                Signature;                ///< 
  EFI_HANDLE                            FirmwareVolumeHandle;     ///< 
  EFI_FIRMWARE_VOLUME_PROTOCOL          *FirmwareVolumeProtocol;  ///< 
  APPLE_PLATFORM_INFO_DATABASE_PROTOCOL Protocol;                 ///< 
} APPLE_PLATFORM_INFO_DATABASE;

// PlatformInfoDbGetFirstDataSizeImpl
EFI_STATUS
EFIAPI
PlatformInfoDbGetFirstDataSizeImpl (
  IN     APPLE_PLATFORM_INFO_DATABASE_PROTOCOL  *This,
  IN     EFI_GUID                               *NameGuid,
  IN OUT UINTN                                  *Size
  );

// PlatformInfoDbGetDataSizeImpl
EFI_STATUS
EFIAPI
PlatformInfoDbGetDataSizeImpl (
  IN     APPLE_PLATFORM_INFO_DATABASE_PROTOCOL  *This,
  IN     EFI_GUID                               *NameGuid,
  IN     UINTN                                  Index, OPTIONAL
  IN OUT UINTN                                  *Size
  );

// PlatformInfoDbGetFirstDataImpl
EFI_STATUS
EFIAPI
PlatformInfoDbGetFirstDataImpl (
  IN     APPLE_PLATFORM_INFO_DATABASE_PROTOCOL  *This,
  IN     EFI_GUID                               *NameGuid,
  IN OUT VOID                                   *Data, OPTIONAL
  IN OUT UINTN                                  *Size
  );

// PlatformInfoDbGetDataImpl
EFI_STATUS
EFIAPI
PlatformInfoDbGetDataImpl (
  IN     APPLE_PLATFORM_INFO_DATABASE_PROTOCOL  *This,
  IN     EFI_GUID                               *NameGuid,
  IN     UINTN                                  Index, OPTIONAL
  IN OUT VOID                                   *Data, OPTIONAL
  IN OUT UINTN                                  *Size
  );

#endif // APPLE_PLATFORM_INFO_DATABASE_IMPL_H_
