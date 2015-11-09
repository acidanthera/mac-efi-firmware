[Defines]
  PLATFORM_NAME             = EfiDevicePathPropertyDatabase
  PLATFORM_GUID             = BDFDE060-7E41-4EAE-AD9B-E5BBA7A48A3A
  PLATFORM_VERSION          = 1.0
  DSC_SPECIFICATION         = 0x00010006
  OUTPUT_DIRECTORY          = Build/ApplePkg
  SUPPORTED_ARCHITECTURES   = X64|IA32
  BUILD_TARGETS             = RELEASE|DEBUG
  SKUID_IDENTIFIER          = DEFAULT
  EFI_SPECIFICATION_VERSION = 0x0001000A
  PI_SPECIFICATION_VERSION  = 0x00000009
  TIANO_RELEASE_VERSION     = 0x00080006

[Libraries]
  !include AppleModulePkg/Protocol/DevicePathPropertyDatabaseImpl/DevicePathPropertyDatabaseImplLibs.dsc
  !include AppleModulePkg/Driver/EfiDevicePathPropertyDatabase/EfiDevicePathPropertyDatabaseLibs.dsc
  
[Components]
  AppleModulePkg/Driver/EfiDevicePathPropertyDatabase/EfiDevicePathPropertyDatabase.inf

!include AppleModulePkg/AppleModulePkgBuild.dsc
