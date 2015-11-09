[Defines]
  PLATFORM_NAME             = AppleOsIdentification
  PLATFORM_GUID             = 4391AA42-6644-4D8A-9A84-DDD405C312F3
  PLATFORM_VERSION          = 1.0
  DSC_SPECIFICATION         = 0x00010018
  OUTPUT_DIRECTORY          = Build/AppleModulePkg
  SUPPORTED_ARCHITECTURES   = X64|IA32
  BUILD_TARGETS             = RELEASE|DEBUG
  SKUID_IDENTIFIER          = DEFAULT
  EFI_SPECIFICATION_VERSION = 0x0001000A

[Libraries]
!include AppleModulePkg/AppleModulePkgLibs.dsc
!include AppleModulePkg/Driver/AppleOsIdentification/AppleOsIdentificationLibs.dsc
!include AppleModulePkg/Protocol/OsIdentificationImpl/OsIdentificationImplLibs.dsc
  
[Components]
  AppleModulePkg/Driver/AppleOsIdentification/AppleOsIdentification.inf

!include AppleModulePkg/AppleModulePkgBuild.dsc
