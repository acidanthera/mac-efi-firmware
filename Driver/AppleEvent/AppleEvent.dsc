[Defines]
  PLATFORM_NAME             = AppleEvent
  PLATFORM_GUID             = 4391AA92-6644-4D8A-9A84-DDD406C312F3
  PLATFORM_VERSION          = 1.0
  DSC_SPECIFICATION         = 0x00010018
  OUTPUT_DIRECTORY          = Build/ApplePkg
  SUPPORTED_ARCHITECTURES   = X64|IA32
  BUILD_TARGETS             = RELEASE|DEBUG
  SKUID_IDENTIFIER          = DEFAULT
  EFI_SPECIFICATION_VERSION = 0x0001000A
  PI_SPECIFICATION_VERSION  = 0x00000009
  TIANO_RELEASE_VERSION     = 0x00080006

[Libraries]
!include AppleModulePkg/Protocol/AppleEventImpl/AppleEventImplLibs.dsc
!include AppleModulePkg/Driver/AppleEvent/AppleEventLibs.dsc
  
[Components]
  AppleModulePkg/Driver/AppleEvent/AppleEvent.inf

!include AppleModulePkg/AppleModulePkgBuild.dsc
