[Defines]
  PLATFORM_NAME             = ApplePlatformInfoDb
  PLATFORM_GUID             = 4391AA92-66A4-4D8A-9A84-DDD405C312F3
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
!include AppleModulePkg/Driver/AppleBootPolicy/AppleBootPolicyLibs.dsc
!include AppleModulePkg/Protocol/AppleBootPolicyImpl/AppleBootPolicyImplLibs.dsc
  
[Components]
  AppleModulePkg/Driver/AppleBootPolicy/AppleBootPolicy.inf

!include AppleModulePkg/AppleModulePkgBuild.dsc
