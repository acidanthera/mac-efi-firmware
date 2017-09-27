## @file
# Copyright (c) 2005 - 2017 Apple Inc.  All rights reserved.<BR>
#
# This program and the accompanying materials have not been licensed.
# Neither is its usage, its redistribution, in source or binary form,
# licensed, nor implicitely or explicitely permitted, except when
# required by applicable law.
#
# Unless required by applicable law or agreed to in writing, software
# distributed is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
# OR CONDITIONS OF ANY KIND, either express or implied.
#
#  
##

[Defines]
  PLATFORM_NAME           = Efi
  PLATFORM_GUID           = 0ADCC78E-2FDC-4491-998D-8684A7B602E9
  PLATFORM_VERSION        = 2017.001
  DSC_SPECIFICATION       = 0x00010006
  SUPPORTED_ARCHITECTURES = IA32|IPF|X64|EBC|ARM|AARCH64
  BUILD_TARGETS           = DEBUG|NOOPT|RELEASE
  SKUID_IDENTIFIER        = DEFAULT
  OUTPUT_DIRECTORY        = BuildResults/J137

  DEFINE STACK_PROTECTOR_ENABLE = TRUE

  DEFINE PEI_LTO_ENABLE = TRUE
  DEFINE DXE_LTO_ENABLE = TRUE

[LibraryClasses]
  #
  # Entry point
  #
  PeiCoreEntryPoint|MdePkg/Library/PeiCoreEntryPoint/PeiCoreEntryPoint.inf
  PeimEntryPoint|MdePkg/Library/PeimEntryPoint/PeimEntryPoint.inf
  DxeCoreEntryPoint|MdePkg/Library/DxeCoreEntryPoint/DxeCoreEntryPoint.inf
  UefiDriverEntryPoint|EfiPkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  UefiApplicationEntryPoint|EfiPkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  #
  # Basic
  #
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLibOptDxe/BaseMemoryLibOptDxe.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  #
  # UEFI & PI
  #
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  #
  # Misc
  #
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  DebugLib|EfiPkg/Library/AppleBaseDebugLibInterrupt/AppleBaseDebugLibInterrupt.inf
  #
  # Generic Modules
  #
  UefiUsbLib|MdePkg/Library/UefiUsbLib/UefiUsbLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  #
  # Apple
  #
  AppleCpuExtensionsLib|EfiPkg/Library/AppleCpuExtensionsLib/AppleCpuExtensionsLib.inf
  AppleDataHubLib|EfiPkg/Library/AppleDataHubLib/AppleDataHubLib.inf
  AppleEventLib|EfiPkg/Library/AppleEventLib/AppleEventLib.inf
  AppleInterruptLib|EfiPkg/Library/AppleInterruptLib/AppleInterruptLib.inf
  AppleSmbiosLib|EfiPkg/Library/AppleSmbiosLib/AppleSmbiosLib.inf
  BiosIdLib|EfiPkg/Library/BiosIdLib/BiosIdLib.inf

  #
  # Since software stack checking may be heuristically enabled by the compiler
  # include BaseStackCheckLib unconditionally.
  #
  NULL|EfiPkg/Library/AppleBaseStackCheckLib/AppleBaseStackCheckLib.inf

[LibraryClasses.common.PEI_CORE]
  HobLib|MdePkg/Library/PeiHobLib/PeiHobLib.inf
  MemoryAllocationLib|MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/PeiReportStatusCodeLib/PeiReportStatusCodeLib.inf

[LibraryClasses.common.PEIM]
  HobLib|MdePkg/Library/PeiHobLib/PeiHobLib.inf
  MemoryAllocationLib|MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/PeiReportStatusCodeLib/PeiReportStatusCodeLib.inf

[LibraryClasses.IA32.PEI_CORE, LibraryClasses.X64.PEI_CORE, LibraryClasses.IA32.PEIM, LibraryClasses.X64.PEIM]
  TimerLib|EfiPkg/Library/AcpiTscTimerLib/PeiTscTimerLib.inf

[LibraryClasses.IPF]
  TimerLib|UefiCpuPkg/Library/SecPeiDxeTimerLibUefiCpu/SecPeiDxeTimerLibUefiCpu.inf

[LibraryClasses.EBC]
  TimerLib|MdePkg/Library/BaseTimerLibNullTemplate/BaseTimerLibNullTemplate.inf

[LibraryClasses.common.DXE_CORE]
  HobLib|MdePkg/Library/DxeCoreHobLib/DxeCoreHobLib.inf
  MemoryAllocationLib|MdeModulePkg/Library/DxeCoreMemoryAllocationLib/DxeCoreMemoryAllocationLib.inf
  DebugPrintErrorLevelLib|EfiPkg/Library/AppleDxeDebugPrintErrorLevelLib/AppleDxeDebugPrintErrorLevelLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf

[LibraryClasses.IA32.DXE_CORE, LibraryClasses.X64.DXE_CORE, LibraryClasses.IA32.DXE_DRIVER, LibraryClasses.X64.DXE_DRIVER, LibraryClasses.IA32.DXE_RUNTIME_DRIVER, LibraryClasses.X64.DXE_RUNTIME_DRIVER, LibraryClasses.IA32.DXE_SMM_DRIVER, LibraryClasses.X64.DXE_SMM_DRIVER, LibraryClasses.IA32.UEFI_DRIVER, LibraryClasses.X64.UEFI_DRIVER, LibraryClasses.IA32.UEFI_APPLICATION, LibraryClasses.X64.UEFI_APPLICATION]
  TimerLib|EfiPkg/Library/AcpiTscTimerLib/DxeTscTimerLib.inf

[LibraryClasses.common.DXE_DRIVER]
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  DebugPrintErrorLevelLib|EfiPkg/Library/AppleDxeDebugPrintErrorLevelLib/AppleDxeDebugPrintErrorLevelLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf

[LibraryClasses.common.DXE_RUNTIME_DRIVER]
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  DebugPrintErrorLevelLib|EfiPkg/Library/AppleDxeDebugPrintErrorLevelLib/AppleDxeDebugPrintErrorLevelLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf

[LibraryClasses.common.SMM_CORE]
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  MemoryAllocationLib|MdeModulePkg/Library/PiSmmCoreMemoryAllocationLib/PiSmmCoreMemoryAllocationLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf

[LibraryClasses.common.DXE_SMM_DRIVER]
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  MemoryAllocationLib|MdePkg/Library/SmmMemoryAllocationLib/SmmMemoryAllocationLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf

[LibraryClasses.common.UEFI_DRIVER]
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  DebugPrintErrorLevelLib|EfiPkg/Library/AppleDxeDebugPrintErrorLevelLib/AppleDxeDebugPrintErrorLevelLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf

[LibraryClasses.common.UEFI_APPLICATION]
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  DebugPrintErrorLevelLib|EfiPkg/Library/AppleDxeDebugPrintErrorLevelLib/AppleDxeDebugPrintErrorLevelLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf

[LibraryClasses.ARM, LibraryClasses.AARCH64]
  #
  # It is not possible to prevent ARM compiler calls to generic intrinsic functions.
  # This library provides the instrinsic functions generated by a given compiler.
  # [LibraryClasses.ARM] and NULL mean link this library into all ARM images.
  #
  NULL|ArmPkg/Library/CompilerIntrinsicsLib/CompilerIntrinsicsLib.inf

  ArmLib|ArmPkg/Library/ArmLib/ArmBaseLib.inf
  TimerLib|ArmPkg/Library/ArmArchTimerLib/ArmArchTimerLib.inf
  ArmGenericTimerCounterLib|ArmPkg/Library/ArmGenericTimerPhyCounterLib/ArmGenericTimerPhyCounterLib.inf

[PcdsFixedAtBuild]
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x11

  ## Indicates the maximum node number of linked list.<BR><BR>
  #  0  - No node number check for linked list.<BR>
  #  >0 - Maximum node number of linked list.<BR>
  # @Prompt Maximum Length of Linked List.
  gEfiMdePkgTokenSpaceGuid.PcdMaximumLinkedListLength|0

[Components]
  EfiPkg/Bus/Lpc/AppleSmcDxe/AppleSmcDxe.inf

  EfiPkg/Platform/AppleBootPolicyDxe/AppleBootPolicyDxe.inf
  EfiPkg/Platform/AppleEventDxe/AppleEventDxe.inf
  EfiPkg/Platform/AppleKeyMapAggregatorDxe/AppleKeyMapAggregatorDxe.inf
  EfiPkg/Platform/ApplePlatformInfoDatabase/Dxe/ApplePlatformInfoDatabaseDxe.inf
  EfiPkg/Platform/EfiDevicePathPropertyDatabaseDxe/EfiDevicePathPropertyDatabaseDxe.inf
  EfiPkg/Platform/EfiOSInfoDxe/EfiOSInfoDxe.inf
  EfiPkg/Platform/UserInterfaceThemeDriverDxe/UserInterfaceThemeDriverDxe.inf

  EfiPkg/Universal/AppleSmbiosDxe/AppleSmbiosDxe.inf

  EfiPkg/Library/AppleBaseDebugLibInterrupt/AppleBaseDebugLibInterrupt.inf
  EfiPkg/Library/AppleCpuExtensionsLib/AppleCpuExtensionsLib.inf
  EfiPkg/Library/AppleDataHubLib/AppleDataHubLib.inf
  EfiPkg/Library/AppleEventLib/AppleEventLib.inf
  EfiPkg/Library/AppleInterruptLib/AppleInterruptLib.inf
  EfiPkg/Library/AppleSmbiosLib/AppleSmbiosLib.inf
  EfiPkg/Library/BiosIdLib/BiosIdLib.inf
  EfiPkg/Library/AppleDxeDebugPrintErrorLevelLib/AppleDxeDebugPrintErrorLevelLib.inf
  EfiPkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  EfiPkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf

  StoragePkg/PartitionDxe/PartitionDxe.inf

  UsbPkg/Bus/Usb/UsbKbDxe/UsbKbDxe.inf

[Components.IA32, Components.X64]
  EfiPkg/Library/AcpiTscTimerLib/BaseTscTimerLib.inf
  EfiPkg/Library/AcpiTscTimerLib/DxeTscTimerLib.inf
  EfiPkg/Library/AcpiTscTimerLib/PeiTscTimerLib.inf

[BuildOptions]
  XCODE:*_*_*_PLATFORM_FLAGS = -fstack-protector

!if $(PEI_LTO_ENABLE)
  XCODE:*_*_IA32_PLATFORM_FLAGS = -flto
  XCODE:*_*_*_DLINK_FLAGS       = -object_path_lto $(DEST_DIR_DEBUG)/$(BASE_NAME).lto
!endif

!if $(DXE_LTO_ENABLE)
  XCODE:*_*_X64_PLATFORM_FLAGS = -flto
  XCODE:*_*_*_DLINK_FLAGS      = -object_path_lto $(DEST_DIR_DEBUG)/$(BASE_NAME).lto
!endif
