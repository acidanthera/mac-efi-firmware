/** @file
  Copyright (C) 2005 - 2015, Apple Inc.  All rights reserved.<BR>

  This program and the accompanying materials have not been licensed.
  Neither is its usage, its redistribution, in source or binary form,
  licensed, nor implicitely or explicitely permitted, except when
  required by applicable law.

  Unless required by applicable law or agreed to in writing, software
  distributed is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
  OR CONDITIONS OF ANY KIND, either express or implied.
**/

#include <AppleEfi.h>

#include EFI_GUID_DEFINITION (Hob)
#include APPLE_GUID_DEFINITION (AppleHob)

#include <Library/AppleDriverLib.h>
#include <EfiHobLib.h>

// mAppleDriverInitHobData
STATIC VOID *mAppleDriverInitHobData;

// SaveAppleDriverInitHobData 
VOID
SaveAppleDriverInitHobData (
  IN VOID  *Data
  )
{
  mAppleDriverInitHobData = Data;
}

// AppleInitializeDriverLib
/** Intialize Driver Lib if it has not yet been initialized.

  @param[in] ImageHandle  Standard EFI Image entry parameter
  @param[in] SystemTable  Standard EFI Image entry parameter
 
  @retval EFI_SUCCESS  Operation succeeded
**/
EFI_STATUS
AppleInitializeDriverLib (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  VOID                 *Table;
  EFI_PEI_HOB_POINTERS TableIterator;
  BOOLEAN              Match;

  EfiInitializeDriverLib (ImageHandle, SystemTable);
  EfiLibGetSystemConfigurationTable (&gEfiHobListGuid, &Table);

  TableIterator.Raw = Table;

  if (!END_OF_HOB_LIST (TableIterator)) {
    do {
      GetHob (EFI_HOB_TYPE_GUID_EXTENSION, (VOID *)TableIterator.Raw);

      if (GET_HOB_TYPE (TableIterator) == EFI_HOB_TYPE_GUID_EXTENSION) {
        Match = EfiCompareGuid (
                  &gAppleDriverInitHobGuid,
                  &TableIterator.Guid->Name
                  );

        if (Match) {
          Table = GET_NEXT_HOB (TableIterator);

          SaveAppleDriverInitHobData (
            (VOID *)((UINTN)TableIterator.Raw + sizeof (*TableIterator.Guid))
            );

          break;
        }
      }

      Table             = GET_NEXT_HOB (TableIterator);
      TableIterator.Raw = GET_NEXT_HOB (TableIterator);
    } while (!END_OF_HOB_LIST (TableIterator));
  }

  return EFI_SUCCESS;
}