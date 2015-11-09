[Defines]
  PLATFORM_NAME             = AppleKeyMapAggregator
  PLATFORM_GUID             = AC4CE557-F5CD-439E-963C-40F09683DAC5
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
!include AppleModulePkg/Protocol/AppleKeyMapImpl/AppleKeyMapImplLibs.dsc
!include AppleModulePkg/Driver/AppleKeyMapAggregator/AppleKeyMapAggregatorLibs.dsc
  
[Components]
  AppleModulePkg/Driver/AppleKeyMapAggregator/AppleKeyMapAggregator.inf

!include AppleModulePkg/AppleModulePkgBuild.dsc
