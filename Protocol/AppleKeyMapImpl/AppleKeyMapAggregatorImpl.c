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

  @param[in]  This       A pointer to the protocol instance.
  @param[out] Modifiers  The modifiers manipulating the given keys.
  @param[out] NoKeys     On input the number of keys allocated.
                         On output the number of keys returned into Keys.
  @param[out] Keys       A Pointer to a caller-allocated the pressed keys get
                         returned in.

  @retval EFI_SUCCESS           The pressed keys have been returned into Keys.
  @retval EFI_BUFFER_TOO_SMALL  The memory required to return the value exceeds
                                the size of the allocated Buffer.
                                The required number of keys to allocate to
                                complete the operation has been returned into
                                NoKeys.
  @retval other                 An error returned by a sub-operation.
**/
EFI_STATUS
EFIAPI
KeyMapGetKeyStrokesImpl (
  IN     APPLE_KEY_MAP_AGGREGATOR_PROTOCOL  *This,
  OUT    APPLE_MODIFIER_MAP                 *Modifiers,
  OUT    UINTN                              *NoKeys,
  IN OUT APPLE_KEY                          *Keys OPTIONAL
  ) // sub_51B
{
  EFI_STATUS               Status;

  APPLE_KEY_MAP_AGGREGATOR *Aggregator;
  APPLE_KEY_STROKES_INFO   *KeyStrokesInfo;
  APPLE_MODIFIER_MAP       DbModifiers;
  BOOLEAN                  Result;
  UINTN                    DbNoKeyStrokes;
  UINTN                    Index;
  UINTN                    Index2;
  APPLE_KEY                Key;

  ASSERT (This != NULL);
  ASSERT (Modifiers != NULL);
  ASSERT (NoKeys != NULL);
  ASSERT (((((*NoKeys == 0) ? 1 : 0)) ^ ((Keys != NULL) ? 0 : 1)) != 0);

  Aggregator     = APPLE_KEY_MAP_AGGREGATOR_FROM_AGGREGATOR_PROTOCOL (This);
  KeyStrokesInfo = APPLE_KEY_STROKES_INFO_FROM_LIST_ENTRY (
                     GetFirstNode (&Aggregator->KeyStrokesInfoList)
                     );

  if (IsNull (&Aggregator->KeyStrokesInfoList, &KeyStrokesInfo->Hdr.This)) {
    *NoKeys        = 0;
    DbNoKeyStrokes = 0;
    DbModifiers    = 0;
  } else {
    DbModifiers    = 0;
    DbNoKeyStrokes = 0;

    do {
      DbModifiers |= KeyStrokesInfo->Hdr.Modifiers;

      if (KeyStrokesInfo->Hdr.NoKeys > 0) {
        Index = 0;

        do {
          Key = (&KeyStrokesInfo->Keys)[Index];
          ++Index;

          for (Index2 = 0; Index2 < DbNoKeyStrokes; ++Index2) {
            if (Aggregator->KeyBuffer[Index2] == Key) {
              break;
            }
          }

          if (DbNoKeyStrokes == Index2) {
            Aggregator->KeyBuffer[DbNoKeyStrokes] = Key;
            ++DbNoKeyStrokes;
          }
        } while (Index < KeyStrokesInfo->Hdr.NoKeys);
      }

      KeyStrokesInfo = APPLE_KEY_STROKES_INFO_FROM_LIST_ENTRY (
                         GetNextNode (
                           &Aggregator->KeyStrokesInfoList,
                           &KeyStrokesInfo->Hdr.This
                           )
                         );
    } while (!IsNull (&Aggregator->KeyStrokesInfoList, &KeyStrokesInfo->Hdr.This));

    Result  = (DbNoKeyStrokes > *NoKeys);
    *NoKeys = DbNoKeyStrokes;
    Status  = EFI_BUFFER_TOO_SMALL;

    if (Result) {
      goto Return;
    }
  }

  *Modifiers = DbModifiers;
  *NoKeys    = DbNoKeyStrokes;
  Status     = EFI_SUCCESS;

  if (Keys != NULL) {
    gBS->CopyMem (
           (VOID *)Keys,
           (VOID *)Aggregator->KeyBuffer,
           (DbNoKeyStrokes * sizeof (*Keys))
           );
  }

Return:
  ASSERT_EFI_ERROR (Status);

  return Status;
}

// KeyMapContainsKeyStrokesImpl
/** Returns whether or not a list of keys and their modifiers are part of the
    database of pressed keys.

  @param[in]      This        A pointer to the protocol instance.
  @param[in]      Modifiers   The modifiers manipulating the given keys.
  @param[in]      NoKeys      The number of keys present in Keys.
  @param[in, out] Keys        The list of keys to check for.  The children may
                              be sorted in the process.
  @param[in]      ExactMatch  Specifies whether Modifiers and Keys should be
                             exact matches or just contained.

  @return                Returns whether or not a list of keys and their
                         modifiers are part of the database of pressed keys.
  @retval EFI_SUCCESS    The queried keys are part of the database.
  @retval EFI_NOT_FOUND  The queried keys could not be found.
**/
EFI_STATUS
EFIAPI
KeyMapContainsKeyStrokesImpl (
  IN     APPLE_KEY_MAP_AGGREGATOR_PROTOCOL  *This,
  IN     APPLE_MODIFIER_MAP                 Modifiers,
  IN     UINTN                              NoKeys,
  IN OUT APPLE_KEY                          *Keys,
  IN     BOOLEAN                            ExactMatch
  ) // sub_638
{
  EFI_STATUS         Status;

  UINTN              DbNoKeys;
  APPLE_KEY          DbKeys[8];
  APPLE_MODIFIER_MAP DbModifiers;
  INTN               Result;
  UINTN              Index;
  UINTN              DbIndex;

  ASSERT (This != NULL);
  ASSERT (NoKeys > 0);
  ASSERT (Keys != NULL);

  DbNoKeys = ARRAY_LENGTH (DbKeys);
  Status   = This->GetKeyStrokes (This, &DbModifiers, &DbNoKeys, DbKeys);

  if (!EFI_ERROR (Status)) {
    if (ExactMatch) {
      Status = EFI_NOT_FOUND;

      if ((DbModifiers == Modifiers) && (DbNoKeys == NoKeys)) {
        KeyMapBubbleSort ((UINT16 *)Keys, NoKeys);
        KeyMapBubbleSort ((UINT16 *)DbKeys, DbNoKeys);

        Result = EfiCompareMem (
          (VOID *)Keys,
          (VOID *)DbKeys,
          (NoKeys * sizeof (*Keys))
          );

        if (Result == 0) {
          Status = EFI_SUCCESS;
        }
      }
    } else {
      Status = EFI_NOT_FOUND;

      if ((DbModifiers & Modifiers) != 0) {
        for (Index = 0; Index < NoKeys; ++Index) {
          for (DbIndex = 0; DbIndex < DbNoKeys; ++DbIndex) {
            if (Keys[Index] == DbKeys[DbIndex]) {
              break;
            }
          }

          if (DbNoKeys == DbIndex) {
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
