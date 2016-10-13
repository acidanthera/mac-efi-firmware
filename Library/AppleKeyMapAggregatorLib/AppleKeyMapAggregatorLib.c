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

#include <IndustryStandard/AppleHid.h>

#include APPLE_PROTOCOL_PRODUCER (AppleKeyMapAggregator)

#include <Library/AppleDriverLib.h>
#include <Library/AppleKeyMapAggregatorLib.h>

// mAppleKeyMapAggregator
GLOBAL_REMOVE_IF_UNREFERENCED
APPLE_KEY_MAP_AGGREGATOR_PROTOCOL *mAppleKeyMapAggregator = NULL;

// GetAppleKeyStrokes
EFI_STATUS
GetAppleKeyStrokes (
  OUT APPLE_MODIFIER_MAP  *Modifiers,
  OUT UINTN               *NumberOfKeys,
  OUT APPLE_KEY           **Keys
  ) // sub_1015
{
  EFI_STATUS Status;

  ASSERT (Modifiers != NULL);
  ASSERT (NumberOfKeys != NULL);
  ASSERT (Keys != NULL);
  ASSERT (mAppleKeyMapAggregator != NULL);

  Status = EFI_UNSUPPORTED;

  if (mAppleKeyMapAggregator != NULL) {
    Status = EFI_INVALID_PARAMETER;

    if ((Modifiers != NULL) && (NumberOfKeys != NULL) && (Keys != NULL)) {
      *NumberOfKeys = 0;
      *Keys         = NULL;
      Status        = mAppleKeyMapAggregator->GetKeyStrokes (
                                                mAppleKeyMapAggregator,
                                                Modifiers,
                                                NumberOfKeys,
                                                NULL
                                                );

      ASSERT (!EFI_ERROR (Status) || Status == EFI_BUFFER_TOO_SMALL);

      if (Status == EFI_BUFFER_TOO_SMALL) {
        if (*NumberOfKeys == 0) {
          *Keys = NULL;
        } else {
          *Keys = EfiLibAllocatePool (*NumberOfKeys * sizeof (**Keys));

          if (*Keys == NULL) {
            *NumberOfKeys = 0;
            Status        = EFI_OUT_OF_RESOURCES;
          } else {
            Status = mAppleKeyMapAggregator->GetKeyStrokes (
                                               mAppleKeyMapAggregator,
                                               Modifiers,
                                               NumberOfKeys,
                                               *Keys
                                               );

            if (EFI_ERROR (Status)) {
              gBS->FreePool ((VOID *)*Keys);

              *Keys         = NULL;
              *NumberOfKeys = 0;
            }
          }
        }
      }
    }
  }

  return Status;
}

// GetModifierStrokes
APPLE_MODIFIER_MAP
GetModifierStrokes (
  VOID
  ) // sub_FDA
{
  APPLE_MODIFIER_MAP Modifiers;

  EFI_STATUS         Status;
  UINTN              NumberOfKeys;
  APPLE_KEY          *Keys;

  Status = GetAppleKeyStrokes (&Modifiers, &NumberOfKeys, &Keys);

  if (!EFI_ERROR (Status)) {
    if (Keys != NULL) {
      gBS->FreePool ((VOID *)Keys);
    }
  } else {
    Modifiers = 0;
  }

  return Modifiers;
}
