/** @file
  Copyright (C) 2005 - 2017, Apple Inc.  All rights reserved.<BR>

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

// KeyMapAggrLibGetAppleKeyStrokes
EFI_STATUS
KeyMapAggrLibGetAppleKeyStrokes (
  OUT APPLE_MODIFIER_MAP  *Modifiers,
  OUT UINTN               *NumberOfKeyCodes,
  OUT APPLE_KEY_CODE      **KeyCodes
  ) // sub_1015
{
  EFI_STATUS Status;

  ASSERT (Modifiers != NULL);
  ASSERT (NumberOfKeyCodes != NULL);
  ASSERT (KeyCodes != NULL);
  ASSERT (mAppleKeyMapAggregator != NULL);

  Status = EFI_UNSUPPORTED;

  if (mAppleKeyMapAggregator != NULL) {
    Status = EFI_INVALID_PARAMETER;

    if ((Modifiers != NULL)
     && (NumberOfKeyCodes != NULL)
     && (KeyCodes != NULL)) {
      *NumberOfKeyCodes = 0;
      *KeyCodes         = NULL;
      Status        = mAppleKeyMapAggregator->GetKeyStrokes (
                                                mAppleKeyMapAggregator,
                                                Modifiers,
                                                NumberOfKeyCodes,
                                                NULL
                                                );

      ASSERT (!EFI_ERROR (Status) || Status == EFI_BUFFER_TOO_SMALL);

      if (Status == EFI_BUFFER_TOO_SMALL) {
        if (*NumberOfKeyCodes == 0) {
          *KeyCodes = NULL;
        } else {
          *KeyCodes = EfiLibAllocatePool (
                        *NumberOfKeyCodes * sizeof (**KeyCodes)
                        );

          if (*KeyCodes == NULL) {
            *NumberOfKeyCodes = 0;
            Status        = EFI_OUT_OF_RESOURCES;
          } else {
            Status = mAppleKeyMapAggregator->GetKeyStrokes (
                                               mAppleKeyMapAggregator,
                                               Modifiers,
                                               NumberOfKeyCodes,
                                               *KeyCodes
                                               );

            if (EFI_ERROR (Status)) {
              gBS->FreePool ((VOID *)*KeyCodes);

              *KeyCodes         = NULL;
              *NumberOfKeyCodes = 0;
            }
          }
        }
      }
    }
  }

  return Status;
}

// KeyMapAggrLibGetModifierStrokes
APPLE_MODIFIER_MAP
KeyMapAggrLibGetModifierStrokes (
  VOID
  ) // sub_FDA
{
  APPLE_MODIFIER_MAP Modifiers;

  EFI_STATUS         Status;
  UINTN              NumberOfKeyCodes;
  APPLE_KEY_CODE     *KeyCodes;

  Status = KeyMapAggrLibGetAppleKeyStrokes (
             &Modifiers,
             &NumberOfKeyCodes,
             &KeyCodes
             );

  if (!EFI_ERROR (Status)) {
    if (KeyCodes != NULL) {
      gBS->FreePool ((VOID *)KeyCodes);
    }
  } else {
    Modifiers = 0;
  }

  return Modifiers;
}
