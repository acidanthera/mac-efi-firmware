#ifndef _APPLE_PLATFORM_INFO_DATABASE_IMPL_H_
#define _APPLE_PLATFORM_INFO_DATABASE_IMPL_H_

#include EFI_PROTOCOL_CONSUMER (FirmwareVolume)
#include <Protocol/ApplePlatformInfoDatabase.h>

// APPLE_PLATFORM_INFO_DATABASE_PROTOCOL_REVISION
#define APPLE_PLATFORM_INFO_DATABASE_PROTOCOL_REVISION  0x01

// APPLE_PLATFORM_INFO_DATABASE_SIGNATURE
#define APPLE_PLATFORM_INFO_DATABASE_SIGNATURE  EFI_SIGNATURE_32 ('P', 'I', 'D', 'B')

// PLATFORM_INFO_PROTOCOL_FROM_DATABASE
#define PLATFORM_INFO_PROTOCOL_FROM_DATABASE(Database) CR (Database, APPLE_PLATFORM_INFO_DATABASE, Protocol, APPLE_PLATFORM_INFO_DATABASE_SIGNATURE)

// OWN HEADER?

// APPLE_HOB_1_GUID
#define APPLE_HOB_1_GUID \
  { 0x908B63A8, 0xC7C8, 0x493A, { 0x80, 0x72, 0x9D, 0x58, 0xDB, 0xCF, 0x72, 0x4D } }

// APPLE_HOB_2_GUID
#define APPLE_HOB_2_GUID \
  { 0xC78F061E, 0x0290, 0x4E4F, { 0x8D, 0xDC, 0x5B, 0xDA, 0xAC, 0x83, 0x7D, 0xE5 } }

// APPLE_HOB_3_GUID
#define APPLE_HOB_3_GUID \
  { 0xB8E65062, 0xFB30, 0x4078, { 0xAB, 0xD3, 0xA9, 0x4E, 0x09, 0xCA, 0x9D, 0xE6 } }

// APPLE_HOB_4_GUID
#define APPLE_FILE_1_GUID \
  { 0x95C8C131, 0x4467, 0x4447, { 0x8A, 0x71, 0xF0, 0x87, 0xAF, 0xCA, 0x07, 0xA5 } }

// _APPLE_PLATFORM_INFO_DATABASE
typedef struct _APPLE_PLATFORM_INFO_DATABASE {
  UINT32                                Signature;                ///< 
  EFI_HANDLE                            FirmwareVolumeHandle;     ///< 
  EFI_FIRMWARE_VOLUME_PROTOCOL          *FirmwareVolumeProtocol;  ///< 
  APPLE_PLATFORM_INFO_DATABASE_PROTOCOL Protocol;                 ///< 
} APPLE_PLATFORM_INFO_DATABASE;

// _APPLE_SECTION
typedef struct _EFI_APPLE_SECTION {
  EFI_RAW_SECTION Hdr;    ///< 
  UINT64          int_8;  ///< 
  UINT32          Size;   ///< 
  UINT8           Data;   ///< 
} EFI_APPLE_SECTION;

// mD20Data
EFI_APPLE_SECTION mD20Data;

// mD30Data
EFI_APPLE_SECTION mD30Data;

// ApplePlatformInfoDbGetFirstPlatformInfoDataSizeImpl
EFI_STATUS
EFIAPI
ApplePlatformInfoDbGetFirstPlatformInfoDataSizeImpl (
  IN     APPLE_PLATFORM_INFO_DATABASE_PROTOCOL  *This,
  IN     EFI_GUID                               *NameGuid,
  IN OUT UINTN                                  *Size
  );

// ApplePlatformInfoDbGetPlatformInfoDataSizeImpl
EFI_STATUS
EFIAPI
ApplePlatformInfoDbGetPlatformInfoDataSizeImpl (
  IN     APPLE_PLATFORM_INFO_DATABASE_PROTOCOL  *This,
  IN     EFI_GUID                               *NameGuid,
  IN     UINTN                                  Index,
  IN OUT UINTN                                  *Size
  );

// ApplePlatformInfoDbGetFirstPlatformInfoDataImpl
EFI_STATUS
EFIAPI
ApplePlatformInfoDbGetFirstPlatformInfoDataImpl (
  IN     APPLE_PLATFORM_INFO_DATABASE_PROTOCOL  *This,
  IN     EFI_GUID                               *NameGuid,
  IN OUT VOID                                   *Data,
  IN OUT UINTN                                  *Size
  );

// ApplePlatformInfoDbGetPlatformInfoDataImpl
EFI_STATUS
EFIAPI
ApplePlatformInfoDbGetPlatformInfoDataImpl (
  IN     APPLE_PLATFORM_INFO_DATABASE_PROTOCOL  *This,
  IN     EFI_GUID                               *NameGuid,
  IN     UINTN                                  Index,
  IN OUT VOID                                   *Data,
  IN OUT UINTN                                  *Size
  );

// gAppleHob1Guid
extern EFI_GUID gAppleHob1Guid;

// gAppleHob1Guid
extern EFI_GUID gAppleHob2Guid;

// gAppleHob1Guid
extern EFI_GUID gAppleHob3Guid;

// gAppleFile1Guid
extern EFI_GUID gAppleFile1Guid;

#endif // ifndef _APPLE_PLATFORM_INFO_DATABASE_IMPL_H_
