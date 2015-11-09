[Defines]
  PLATFORM_NAME             = AppleModulePkg
  PLATFORM_GUID             = 0adcc78e-2fdc-4491-998d-8684a7b602e9
  PLATFORM_VERSION          = 1.0
  DSC_SPECIFICATION         = 0x00010018
  OUTPUT_DIRECTORY          = Build/AppleModulePkg
  SUPPORTED_ARCHITECTURES   = X64|IA32
  BUILD_TARGETS             = RELEASE|DEBUG
  SKUID_IDENTIFIER          = DEFAULT
  EFI_SPECIFICATION_VERSION = 0x0001000A
  PI_SPECIFICATION_VERSION  = 0x00000009
  TIANO_RELEASE_VERSION     = 0x00080006

[Libraries]
!include AppleModulePkg/AppleModulePkgLibs.dsc
!include AppleModulePkg/Protocol/AppleBootPolicyImpl/AppleBootPolicyImplLibs.dsc
!include AppleModulePkg/Driver/AppleBootPolicy/AppleBootPolicyLibs.dsc
!include AppleModulePkg/Protocol/AppleEventImpl/AppleEventImplLibs.dsc
!include AppleModulePkg/Driver/AppleEvent/AppleEventLibs.dsc
!include AppleModulePkg/Protocol/AppleKeyMapImpl/AppleKeyMapImplLibs.dsc
!include AppleModulePkg/Driver/AppleKeyMapAggregator/AppleKeyMapAggregatorLibs.dsc
!include AppleModulePkg/Protocol/DevicePathPropertyDatabaseImpl/DevicePathPropertyDatabaseImplLibs.dsc
!include AppleModulePkg/Driver/EfiDevicePathPropertyDatabase/EfiDevicePathPropertyDatabaseLibs.dsc
!include AppleModulePkg/Protocol/OsIdentificationImpl/OsIdentificationImplLibs.dsc
!include AppleModulePkg/Driver/AppleOsIdentification/AppleOsIdentificationLibs.dsc
!include AppleModulePkg/Protocol/ApplePlatformInfoDatabaseImpl/ApplePlatformInfoDatabaseImplLibs.dsc
!include AppleModulePkg/Driver/ApplePlatformInfoDb/ApplePlatformInfoDbLibs.dsc
  ApplePkg/Library/UsbDxeLib/UsbDxeLib.inf
  EdkCompatibilityPkg/Foundation/Library/Dxe/UefiEfiIfrSupportLib/UefiEfiIfrSupportLib.inf

[LibraryClasses]
  #
  # EDK
  #
  EfiDriverLib|EdkCompatibilityPkg/Foundation/Library/Dxe/EfiDriverLib/EfiDriverLib_Edk2.inf
  EdkProtocolLib|EdkCompatibilityPkg/Foundation/Protocol/EdkProtocolLib.inf
  EdkFrameworkGuidLib|EdkCompatibilityPkg/Foundation/Framework/Guid/EdkFrameworkGuidLib.inf
  EfiCommonLib|EdkCompatibilityPkg/Foundation/Library/EfiCommonLib/EfiCommonLib_Edk2.inf
  EfiProtocolLib|EdkCompatibilityPkg/Foundation/Efi/Protocol/EfiProtocolLib.inf
  EdkGuidLib|EdkCompatibilityPkg/Foundation/Guid/EdkGuidLib.inf
  EdkFrameworkProtocolLib|EdkCompatibilityPkg/Foundation/Framework/Protocol/EdkFrameworkProtocolLib.inf
  EdkIIGlueBaseLib|EdkCompatibilityPkg/Foundation/Library/EdkIIGlueLib/Library/BaseLib/BaseLib.inf

  #
  # Apple
  #
  AppleKeyMapAggregatorLib|ApplePkg/Library/AppleKeyMapAggregatorLib/AppleKeyMapAggregatorLib.inf
  AppleKeyMapLib|ApplePkg/Library/AppleKeyMapLib/AppleKeyMapLib.inf
  UsbDxeLib|ApplePkg/Library/UsbDxeLib/UsbDxeLib.inf

  #
  # Apple protocol implementations
  #
  AppleEventImpl|AppleModulePkg/Protocol/AppleEventImpl/AppleEventImpl.inf
  AppleBootPolicyImpl|AppleModulePkg/Protocol/AppleBootPolicyImpl/AppleBootPolicyImpl.inf
  AppleKeyMapImpl|AppleModulePkg/Protocol/AppleKeyMapImpl/AppleKeyMapImpl.inf
  ApplePlatformInfoDatabaseImpl|AppleModulePkg/Protocol/ApplePlatformInfoDatabaseImpl/ApplePlatformInfoDatabaseImpl.inf
  EfiDevicePathPropertyDatabaseImpl|AppleModulePkg/Protocol/DevicePathPropertyDatabaseImpl/DevicePathPropertyDatabaseImpl.inf
  EfiOsIdentificationImpl|AppleModulePkg/Protocol/OsIdentificationImpl/OsIdentificationImpl.inf

[Components]
  AppleModulePkg/Driver/AppleBootPolicy/AppleBootPolicy.inf
  AppleModulePkg/Driver/AppleEvent/AppleEvent.inf
  AppleModulePkg/Driver/AppleKeyMapAggregator/AppleKeyMapAggregator.inf
  AppleModulePkg/Driver/AppleOsIdentification/AppleOsIdentification.inf
  AppleModulePkg/Driver/ApplePlatformInfoDb/ApplePlatformInfoDb.inf
  AppleModulePkg/Driver/EfiDevicePathPropertyDatabase/EfiDevicePathPropertyDatabase.inf
  AppleModulePkg/Driver/UsbKb/UsbKb.inf

!include AppleModulePkg/AppleModulePkgBuild.dsc
