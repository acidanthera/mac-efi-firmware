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
#include <AppleMisc.h>

#include <Library/AppleDriverLib.h>

#include "AppleKeyMapImplInternal.h"

// KeyMapGetKeyStrokesImpl
/** Returns all pressed keys and key modifiers into the appropiate buffers.

  @param[in]  This              A pointer to the protocol instance.
  @param[out] Modifiers         The modifiers manipulating the given keys.
  @param[out] NumberOfKeyCodes  On input the number of keys allocated.
                                On output the number of keys returned into
                                KeyCodes.
  @param[out] KeyCodes          A Pointer to a caller-allocated the pressed
                                keys get returned in.

  @retval EFI_SUCCESS           The pressed keys have been returned into
                                KeyCodes.
  @retval EFI_BUFFER_TOO_SMALL  The memory required to return the value exceeds
                                the size of the allocated Buffer.
                                The required number of keys to allocate to
                                complete the operation has been returned into
                                NumberOfKeyCodes.
  @retval other                 An error returned by a sub-operation.
**/
EFI_STATUS
EFIAPI
KeyMapGetKeyStrokes (
  IN     APPLE_KEY_MAP_AGGREGATOR_PROTOCOL  *This,
  OUT    APPLE_MODIFIER_MAP                 *Modifiers,
  OUT    UINTN                              *NumberOfKeyCodes,
  IN OUT APPLE_KEY_CODE                     *KeyCodes OPTIONAL
  ) // sub_51B
{
  EFI_STATUS               Status;

  APPLE_KEY_MAP_AGGREGATOR_PRIVATE *Aggregator;
  APPLE_KEY_STROKES_INFO   *KeyStrokesInfo;
  APPLE_MODIFIER_MAP       DbModifiers;
  BOOLEAN                  Result;
  UINTN                    DbNumberOfKeyCodestrokes;
  UINTN                    Index;
  UINTN                    Index2;
  APPLE_KEY_CODE                Key;

  ASSERT (This != NULL);
  ASSERT (Modifiers != NULL);
  ASSERT (NumberOfKeyCodes != NULL);
  ASSERT ((((*NumberOfKeyCodes > 0) ? 1 : 0) ^ ((KeyCodes == NULL) ? 1 : 0)) != 0);

  Aggregator     = APPLE_KEY_MAP_AGGREGATOR_PRIVATE_FROM_AGGREGATOR_THIS (This);
  KeyStrokesInfo = APPLE_KEY_STROKES_INFO_FROM_LIST_ENTRY (
                     GetFirstNode (&Aggregator->KeyStrokesInfoList)
                     );

  if (IsNull (&Aggregator->KeyStrokesInfoList, &KeyStrokesInfo->Hdr.This)) {
    *NumberOfKeyCodes        = 0;
    DbNumberOfKeyCodestrokes = 0;
    DbModifiers          = 0;
  } else {
    DbModifiers          = 0;
    DbNumberOfKeyCodestrokes = 0;

    do {
      DbModifiers |= KeyStrokesInfo->Hdr.Modifiers;

      if (KeyStrokesInfo->Hdr.NumberOfKeyCodes > 0) {
        Index = 0;

        do {
          Key = (&KeyStrokesInfo->KeyCodes)[Index];
          ++Index;

          for (Index2 = 0; Index2 < DbNumberOfKeyCodestrokes; ++Index2) {
            if (Aggregator->KeyCodeBuffer[Index2] == Key) {
              break;
            }
          }

          if (DbNumberOfKeyCodestrokes == Index2) {
            Aggregator->KeyCodeBuffer[DbNumberOfKeyCodestrokes] = Key;
            ++DbNumberOfKeyCodestrokes;
          }
        } while (Index < KeyStrokesInfo->Hdr.NumberOfKeyCodes);
      }

      KeyStrokesInfo = APPLE_KEY_STROKES_INFO_FROM_LIST_ENTRY (
                         GetNextNode (
                           &Aggregator->KeyStrokesInfoList,
                           &KeyStrokesInfo->Hdr.This
                           )
                         );

      Result = !IsNull (
                  &Aggregator->KeyStrokesInfoList,
                  &KeyStrokesInfo->Hdr.This
                  );
    } while (Result);

    Result        = (BOOLEAN)(DbNumberOfKeyCodestrokes > *NumberOfKeyCodes);
    *NumberOfKeyCodes = DbNumberOfKeyCodestrokes;

    Status = EFI_BUFFER_TOO_SMALL;

    if (Result) {
      goto Done;
    }
  }

  *Modifiers    = DbModifiers;
  *NumberOfKeyCodes = DbNumberOfKeyCodestrokes;

  Status = EFI_SUCCESS;

  if (KeyCodes != NULL) {
    EfiCopyMem (
      (VOID *)KeyCodes,
      (VOID *)Aggregator->KeyCodeBuffer,
      (DbNumberOfKeyCodestrokes * sizeof (*KeyCodes))
      );
  }

Done:
  ASSERT_EFI_ERROR (Status);

  return Status;
}

// KeyMapContainsKeyStrokesImpl
/** Returns whether or not a list of keys and their modifiers are part of the
    database of pressed keys.

  @param[in]      This          A pointer to the protocol instance.
  @param[in]      Modifiers     The modifiers manipulating the given keys.
  @param[in]      NumberOfKeyCodes  The number of keys present in KeyCodes.
  @param[in, out] KeyCodes          The list of keys to check for.  The children
                                may be sorted in the process.
  @param[in]      ExactMatch    Specifies whether Modifiers and KeyCodes should be
                                exact matches or just contained.

  @return                Returns whether or not a list of keys and their
                         modifiers are part of the database of pressed keys.
  @retval EFI_SUCCESS    The queried keys are part of the database.
  @retval EFI_NOT_FOUND  The queried keys could not be found.
**/
EFI_STATUS
EFIAPI
KeyMapContainsKeyStrokes (
  IN     APPLE_KEY_MAP_AGGREGATOR_PROTOCOL  *This,
  IN     APPLE_MODIFIER_MAP                 Modifiers,
  IN     UINTN                              NumberOfKeyCodes,
  IN OUT APPLE_KEY_CODE                          *KeyCodes,
  IN     BOOLEAN                            ExactMatch
  ) // sub_638
{
  EFI_STATUS         Status;

  UINTN              DbNumberOfKeyCodes;
  APPLE_KEY_CODE     DbKeyCodes[8];
  APPLE_MODIFIER_MAP DbModifiers;
  INTN               Result;
  UINTN              Index;
  UINTN              DbIndex;

  ASSERT (This != NULL);
  ASSERT (NumberOfKeyCodes > 0);
  ASSERT (KeyCodes != NULL);

  DbNumberOfKeyCodes = ARRAY_LENGTH (DbKeyCodes);
  Status             = This->GetKeyStrokes (
                               This,
                               &DbModifiers,
                               &DbNumberOfKeyCodes,
                               DbKeyCodes
                               );

  if (!EFI_ERROR (Status)) {
    if (ExactMatch) {
      Status = EFI_NOT_FOUND;

      if ((DbModifiers == Modifiers)
       && (DbNumberOfKeyCodes == NumberOfKeyCodes)) {
        KeyMapMinSort ((UINT16 *)KeyCodes, NumberOfKeyCodes);
        KeyMapMinSort ((UINT16 *)DbKeyCodes, DbNumberOfKeyCodes);

        Result = EfiCompareMem (
                   (VOID *)KeyCodes,
                   (VOID *)DbKeyCodes,
                   (NumberOfKeyCodes * sizeof (*KeyCodes))
                   );

        if (Result == 0) {
          Status = EFI_SUCCESS;
        }
      }
    } else {
      Status = EFI_NOT_FOUND;

      if ((DbModifiers & Modifiers) != 0) {
        for (Index = 0; Index < NumberOfKeyCodes; ++Index) {
          for (DbIndex = 0; DbIndex < DbNumberOfKeyCodes; ++DbIndex) {
            if (KeyCodes[Index] == DbKeyCodes[DbIndex]) {
              break;
            }
          }

          if (DbNumberOfKeyCodes == DbIndex) {
            break;
          }

          Status = EFI_SUCCESS;
        }
      }
    }
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}
