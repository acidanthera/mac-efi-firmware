/** @file
  Debug Print Error Level library instance that provide compatibility with the 
  "err" shell command.  This includes support for the Debug Mask Protocol
  supports for global debug print error level mask stored in an EFI Variable.
  This library instance only support DXE Phase modules.

  Copyright (c) 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiDxe.h>

#include <Guid/AppleHob.h>
#include <Guid/DebugMask.h>
#include <Guid/HobList.h>

#include <Library/DebugPrintErrorLevelLib.h>
#include <Library/PcdLib.h>
#include <Library/HobLib.h>
#include <Library/BaseMemoryLib.h>

///
/// Debug Mask Protocol function prototypes
///

// GetDebugMask
/** Retrieves the current debug print error level mask for a module are returns
    it in CurrentDebugMask.

  @param[in]      This              The protocol instance pointer.
  @param[in, out] CurrentDebugMask  Pointer to the debug print error level mask
                                    that is returned.

  @retval EFI_SUCCESS            The current debug print error level mask was
                                 returned in CurrentDebugMask.
  @retval EFI_INVALID_PARAMETER  CurrentDebugMask is NULL.
  @retval EFI_DEVICE_ERROR       The current debug print error level mask could
                                 not be retrieved.
**/
EFI_STATUS
EFIAPI
GetDebugMask (
  IN EFI_DEBUG_MASK_PROTOCOL  *This,             
  IN OUT UINTN                *CurrentDebugMask  
  );

// SetDebugMask
/** Sets the current debug print error level mask for a module to the value
    specified by NewDebugMask.

  @param[in] This          The protocol instance pointer.
  @param[in] NewDebugMask  The new debug print error level mask for this module.

  @retval EFI_SUCCESS       The current debug print error level mask was set to
                            the value specified by NewDebugMask.
  @retval EFI_DEVICE_ERROR  The current debug print error level mask could not
                            be set to the value specified by NewDebugMask.
**/
EFI_STATUS
EFIAPI
SetDebugMask (
  IN EFI_DEBUG_MASK_PROTOCOL  *This,
  IN UINTN                    NewDebugMask
  );

///
/// Debug Mask Protocol instance
///
EFI_DEBUG_MASK_PROTOCOL mDebugMaskProtocol = {
  EFI_DEBUG_MASK_REVISION,
  GetDebugMask,
  SetDebugMask
};

// mGlobalErrorLevelInitialized
/// Global variable that is set to TRUE after the first attempt is made to 
/// retrieve the global error level mask through the EFI Varibale Services.
/// This variable prevents the EFI Variable Services from being called fort
/// every DEBUG() macro.
BOOLEAN mGlobalErrorLevelInitialized = FALSE;

// mDebugPrintErrorLevel
/// Global variable that contains the current debug error level mask for the
/// module that is using this library instance.  This variable is initially
/// set to the PcdDebugPrintErrorLevel value.  If the EFI Variable exists that
/// contains the global debug print error level mask, then that overrides the
/// PcdDebugPrintErrorLevel value. The EFI Variable can optionally be 
/// discovered via a HOB so early DXE drivers can access the variable. If the
/// Debug Mask Protocol SetDebugMask() service is called, then that overrides 
/// the PcdDebugPrintErrorLevel and the EFI Variable setting.
UINT32 mDebugPrintErrorLevel = 0;

// mSystemTable
/// Global variable that is used to cache a pointer to the EFI System Table
/// that is required to access the EFI Variable Services to get and set
/// the global debug print error level mask value.  The
/// UefiBootServicesTableLib is not used to prevent a circular dependency
/// between these libraries.
EFI_SYSTEM_TABLE *mSystemTable = NULL;

// DxeDebugPrintErrorLevelLibConstructor
/** The constructor function caches the PCI Express Base Address and creates a 
    Set Virtual Address Map event to convert physical address to virtual
    addresses.
  
  @param[in] ImageHandle  The firmware allocated handle for the EFI image.
  @param[in] SystemTable  A pointer to the EFI System Table.
  
  @retval EFI_SUCCESS  The constructor completed successfully.
  @retval Other value  The constructor did not complete successfully.
**/
EFI_STATUS
EFIAPI
DxeDebugPrintErrorLevelLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  //
  // Initialize the error level mask from PCD setting.
  //
  mDebugPrintErrorLevel = PcdGet32 (PcdDebugPrintErrorLevel);

  mSystemTable = SystemTable;
    
  //
  // Install Debug Mask Protocol onto ImageHandle
  //
  SystemTable->BootServices->InstallMultipleProtocolInterfaces (
                               &ImageHandle,
                               &gEfiDebugMaskProtocolGuid,
                               &mDebugMaskProtocol,
                               NULL
                               );

  //
  // Attempt to retrieve the global debug print error level mask from the EFI Variable
  // If the EFI Variable can not be accessed when this module's library constructors are
  // executed a HOB can be used to set the global debug print error level. If no value 
  // was found then the EFI Variable access will be reattempted on every DEBUG() print
  // from this module until the EFI Variable services are available.
  //
  GetDebugPrintErrorLevel ();
  
  return EFI_SUCCESS;
}

/** The destructor function frees any allocated buffers and closes the Set
    Virtual Address Map event.
  
  @param[in] ImageHandle  The firmware allocated handle for the EFI image.
  @param[in] SystemTable  A pointer to the EFI System Table.
  
  @retval EFI_SUCCESS  The destructor completed successfully.
  @retval Other value  The destructor did not complete successfully.

**/
EFI_STATUS
EFIAPI
DxeDebugPrintErrorLevelLibDestructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  //
  // Uninstall the Debug Mask Protocol from ImageHandle
  //  
  SystemTable->BootServices->UninstallMultipleProtocolInterfaces (
                               ImageHandle,
                               &gEfiDebugMaskProtocolGuid,
                               &mDebugMaskProtocol,
                               NULL
                               );

  return EFI_SUCCESS;
}

/**
  Returns the next instance of a HOB type from the starting HOB.

  This function searches the first instance of a HOB type from the starting HOB pointer. 
  If there does not exist such HOB type from the starting HOB pointer, it will return NULL.
  In contrast with macro GET_NEXT_HOB(), this function does not skip the starting HOB pointer
  unconditionally: it returns HobStart back if HobStart itself meets the requirement;
  caller is required to use GET_NEXT_HOB() if it wishes to skip current HobStart.

  @param  Type          The HOB type to return.
  @param  HobStart      The starting HOB pointer to search from.

  @return The next instance of a HOB type from the starting HOB.

**/
STATIC
VOID *
InternalGetNextHob (
  IN UINT16      Type,
  IN CONST VOID  *HobStart
  )
{
  EFI_PEI_HOB_POINTERS  Hob;
   
  Hob.Raw = (UINT8 *) HobStart;
  //
  // Parse the HOB list until end of list or matching type is found.
  //
  while (TRUE) {
    if (Hob.Header->HobType == Type) {
      return Hob.Raw;
    }

    if (END_OF_HOB_LIST (Hob)) {
      break;
    }

    Hob.Raw = GET_NEXT_HOB (Hob);
  }
  return NULL;
}

/**
  Returns the next instance of the matched GUID HOB from the starting HOB.
  
  This function searches the first instance of a HOB from the starting HOB pointer. 
  Such HOB should satisfy two conditions: 
  its HOB type is EFI_HOB_TYPE_GUID_EXTENSION and its GUID Name equals to the input Guid. 
  If there does not exist such HOB from the starting HOB pointer, it will return NULL. 
  Caller is required to apply GET_GUID_HOB_DATA () and GET_GUID_HOB_DATA_SIZE ()
  to extract the data section and its size information, respectively.
  In contrast with macro GET_NEXT_HOB(), this function does not skip the starting HOB pointer
  unconditionally: it returns HobStart back if HobStart itself meets the requirement;
  caller is required to use GET_NEXT_HOB() if it wishes to skip current HobStart.

  @param  Guid          The GUID to match with in the HOB list.
  @param  HobStart      A pointer to a Guid.

  @return The next instance of the matched GUID HOB from the starting HOB.

**/
STATIC
VOID *
InternalGetNextGuidHob (
  IN CONST EFI_GUID  *Guid,
  IN CONST VOID      *HobStart
  )
{
  EFI_PEI_HOB_POINTERS  GuidHob;

  GuidHob.Raw = (UINT8 *) HobStart;
  while ((GuidHob.Raw = InternalGetNextHob (EFI_HOB_TYPE_GUID_EXTENSION, GuidHob.Raw)) != NULL) {
    if (CompareGuid (Guid, &GuidHob.Guid->Name)) {
      break;
    }
    GuidHob.Raw = GET_NEXT_HOB (GuidHob);
  }
  return GuidHob.Raw;
}

// GetDebugPrintErrorLevel
/** Returns the debug print error level mask for the current module.

  @return  Debug print error level mask for the current module.
**/
UINT32
EFIAPI
GetDebugPrintErrorLevel (
  VOID
  )
{
  UINTN             Index;
  VOID              *HobList;
  EFI_HOB_GUID_TYPE *Hob;

  //
  // If the constructor has not been executed yet, then just return the default value.
  // This case should only occur if debug print is generated by a library
  // constructor for this module
  //
  if (mSystemTable != NULL) {
    //
    // Check to see if an attempt has been made to retrieve the global debug print 
    // error level mask.
    //  
    // TODO: Should xor and test for BIT0 in the ASM.
    if (!mGlobalErrorLevelInitialized) {
      if (mSystemTable->NumberOfTableEntries > 0) {
        Index = 0;

        do {
          if (CompareGuid (&gEfiHobListGuid, &mSystemTable->ConfigurationTable[Index].VendorGuid)) {
            HobList = mSystemTable->ConfigurationTable[Index].VendorTable;

            while ((Hob = InternalGetNextGuidHob (&gAppleDebugMaskHobGuid, HobList)) != NULL) {
              if (GET_GUID_HOB_DATA_SIZE (Hob) != sizeof (mDebugPrintErrorLevel)) {
                break;
              }

              mDebugPrintErrorLevel = *(UINT32 *)GET_GUID_HOB_DATA (Hob);

              goto Done;
            }
          }

          ++Index;
        } while (Index < mSystemTable->NumberOfTableEntries);

      Done:
        mGlobalErrorLevelInitialized = TRUE;
      }
    }
  }

  //
  // Return the current mask value for this module.
  //
  return mDebugPrintErrorLevel;
}

// SetDebugPrintErrorLevel
/** Sets the global debug print error level mask fpr the entire platform.
  
  @param[in] ErrorLevel  Global debug print error level
  
  @retval TRUE   The debug print error level mask was sucessfully set.
  @retval FALSE  The debug print error level mask could not be set.
**/
BOOLEAN
EFIAPI
SetDebugPrintErrorLevel (
  IN UINT32  ErrorLevel
  )
{
  //
  // Make sure the constructor has been executed
  //
  if (mSystemTable != NULL) {
    //
    // If the EFI Variable was updated, then update the mask value for this 
    // module and return TRUE.
    //
    mGlobalErrorLevelInitialized = TRUE;    
    mDebugPrintErrorLevel = ErrorLevel;
    return TRUE;
  }
  //
  // Return FALSE since the EFI Variable could not be updated.
  //
  return FALSE;
}

/**
  Retrieves the current debug print error level mask for a module are returns
  it in CurrentDebugMask.

  @param  This              The protocol instance pointer.
  @param  CurrentDebugMask  Pointer to the debug print error level mask that 
                            is returned.

  @retval EFI_SUCCESS            The current debug print error level mask was
                                 returned in CurrentDebugMask.
  @retval EFI_INVALID_PARAMETER  CurrentDebugMask is NULL.
  @retval EFI_DEVICE_ERROR       The current debug print error level mask could
                                 not be retrieved.
**/
EFI_STATUS
EFIAPI
GetDebugMask (
  IN EFI_DEBUG_MASK_PROTOCOL  *This,             
  IN OUT UINTN                *CurrentDebugMask  
  )
{
  if (CurrentDebugMask == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // Retrieve the current debug mask from mDebugPrintErrorLevel
  //
  *CurrentDebugMask = (UINTN)mDebugPrintErrorLevel;
  return EFI_SUCCESS;
}

/**
  Sets the current debug print error level mask for a module to the value
  specified by NewDebugMask.

  @param  This          The protocol instance pointer.
  @param  NewDebugMask  The new debug print error level mask for this module.

  @retval EFI_SUCCESS            The current debug print error level mask was
                                 set to the value specified by NewDebugMask.
  @retval EFI_DEVICE_ERROR       The current debug print error level mask could
                                 not be set to the value specified by NewDebugMask.

**/
EFI_STATUS
EFIAPI
SetDebugMask (
  IN EFI_DEBUG_MASK_PROTOCOL  *This,
  IN UINTN                    NewDebugMask
  )
{
  //
  // Store the new debug mask into mDebugPrintErrorLevel
  //
  mDebugPrintErrorLevel = (UINT32)NewDebugMask;
  return EFI_SUCCESS;
}
