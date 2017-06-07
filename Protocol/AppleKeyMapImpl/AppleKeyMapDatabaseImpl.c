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

#include <Library/AppleDriverLib.h>

#include "AppleKeyMapImplInternal.h"

// KeyMapCreateKeyStrokesBufferImpl
/** Creates a new key set with a given number of keys allocated.  The index
    within the database is returned.

  @param[in]  This           A pointer to the protocol instance.
  @param[in]  KeyBufferSize  The amount of keys to allocate for the key set.
  @param[out] Index          The assigned index of the created key set.

  @return                       Returned is the status of the operation.
  @retval EFI_SUCCESS           A key set with the given number of keys
                                allocated has been created.
  @retval EFI_OUT_OF_RESOURCES  The memory necessary to complete the operation
                                could not be allocated.
  @retval other                 An error returned by a sub-operation.
**/
EFI_STATUS
EFIAPI
KeyMapCreateKeyStrokesBuffer (
  IN  APPLE_KEY_MAP_DATABASE_PROTOCOL  *This,
  IN  UINTN                            KeyBufferSize,
  OUT UINTN                            *Index
  )
{
  EFI_STATUS                       Status;

  APPLE_KEY_MAP_AGGREGATOR_PRIVATE *Aggregator;
  UINTN                            BufferSize;
  APPLE_KEY_CODE                   *Memory;
  APPLE_KEY_STROKES_INFO           *KeyStrokesInfo;

  ASSERT (This != NULL);
  ASSERT (KeyBufferSize > 0);
  ASSERT (Index != NULL);

  Aggregator = APPLE_KEY_MAP_AGGREGATOR_PRIVATE_FROM_DATABASE_THIS (This);

  if (Aggregator->KeyCodeBuffer != NULL) {
    gBS->FreePool ((VOID *)Aggregator->KeyCodeBuffer);
  }

  BufferSize                 = (Aggregator->KeyBuffersSize + KeyBufferSize);
  Aggregator->KeyBuffersSize = BufferSize;
  Memory                     = EfiLibAllocateZeroPool (BufferSize);
  Aggregator->KeyCodeBuffer      = Memory;

  Status = EFI_OUT_OF_RESOURCES;

  if (Memory != NULL) {
    KeyStrokesInfo = EfiLibAllocateZeroPool (
                       sizeof (*KeyStrokesInfo)
                         + (KeyBufferSize * sizeof (APPLE_KEY_CODE))
                       );

    Status = EFI_OUT_OF_RESOURCES;

    if (KeyStrokesInfo != NULL) {
      KeyStrokesInfo->Hdr.Signature     = APPLE_KEY_STROKES_INFO_SIGNATURE;
      KeyStrokesInfo->Hdr.KeyBufferSize = KeyBufferSize;
      KeyStrokesInfo->Hdr.Index         = Aggregator->NextKeyStrokeIndex;
      ++Aggregator->NextKeyStrokeIndex;
      
      InsertTailList (
        &Aggregator->KeyStrokesInfoList,
        &KeyStrokesInfo->Hdr.This
        );

      Status = EFI_SUCCESS;
      *Index = KeyStrokesInfo->Hdr.Index;
    }
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}

// KeyMapRemoveKeyStrokesBufferImpl
/** Removes a key set specified by its index from the database.

  @param[in] This   A pointer to the protocol instance.
  @param[in] Index  The index of the key set to remove.

  @return                Returned is the status of the operation.
  @retval EFI_SUCCESS    The specified key set has been removed.
  @retval EFI_NOT_FOUND  No key set could be found for the given index.
  @retval other          An error returned by a sub-operation.
**/
EFI_STATUS
EFIAPI
KeyMapRemoveKeyStrokesBuffer (
  IN APPLE_KEY_MAP_DATABASE_PROTOCOL  *This,
  IN UINTN                            Index
  )
{
  EFI_STATUS                       Status;

  APPLE_KEY_MAP_AGGREGATOR_PRIVATE *Private;
  APPLE_KEY_STROKES_INFO           *KeyStrokesInfo;

  ASSERT (This != NULL);

  Private     = APPLE_KEY_MAP_AGGREGATOR_PRIVATE_FROM_DATABASE_THIS (This);
  KeyStrokesInfo = KeyMapGetKeyStrokesByIndex (
                     &Private->KeyStrokesInfoList,
                     Index
                     );

  Status         = EFI_NOT_FOUND;

  if (KeyStrokesInfo != NULL) {
    Private->KeyBuffersSize -= KeyStrokesInfo->Hdr.KeyBufferSize;

    RemoveEntryList (&KeyStrokesInfo->Hdr.This);
    gBS->FreePool ((VOID *)KeyStrokesInfo);

    Status = EFI_SUCCESS;
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}

// KeyMapSetKeyStrokeBufferKeysImpl
/** Sets the keys of a key set specified by its index to the given KeyCodes
    Buffer.

  @param[in] This              A pointer to the protocol instance.
  @param[in] Index             The index of the key set to edit.
  @param[in] Modifiers         The key modifiers manipulating the given keys.
  @param[in] NumberOfKeyCodes  The number of keys contained in KeyCodes.
  @param[in] KeyCodes          An array of keys to add to the specified key
                               set.

  @return                       Returned is the status of the operation.
  @retval EFI_SUCCESS           The given keys were set for the specified key
                                set.
  @retval EFI_OUT_OF_RESOURCES  The memory necessary to complete the operation
                                could not be allocated.
  @retval EFI_NOT_FOUND         No key set could be found for the given index.
  @retval other                 An error returned by a sub-operation.
**/
EFI_STATUS
EFIAPI
KeyMapSetKeyStrokeBufferKeys (
  IN APPLE_KEY_MAP_DATABASE_PROTOCOL  *This,
  IN UINTN                            Index,
  IN APPLE_MODIFIER_MAP               Modifiers,
  IN UINTN                            NumberOfKeyCodes,
  IN APPLE_KEY_CODE                   *KeyCodes
  )
{
  EFI_STATUS                       Status;

  APPLE_KEY_MAP_AGGREGATOR_PRIVATE *Private;
  APPLE_KEY_STROKES_INFO           *KeyStrokesInfo;

  ASSERT (This != NULL);
  ASSERT (NumberOfKeyCodes > 0);
  ASSERT (KeyCodes != NULL);

  Private        = APPLE_KEY_MAP_AGGREGATOR_PRIVATE_FROM_DATABASE_THIS (This);
  KeyStrokesInfo = KeyMapGetKeyStrokesByIndex (
                     &Private->KeyStrokesInfoList,
                     Index
                     );

  Status = EFI_NOT_FOUND;

  if (KeyStrokesInfo != NULL) {
    Status = EFI_OUT_OF_RESOURCES;

    if (KeyStrokesInfo->Hdr.KeyBufferSize >= NumberOfKeyCodes) {
      KeyStrokesInfo->Hdr.NumberOfKeyCodes = NumberOfKeyCodes;
      KeyStrokesInfo->Hdr.Modifiers    = Modifiers;

      EfiCopyMem (
        (VOID *)&KeyStrokesInfo->KeyCodes,
        (VOID *)KeyCodes, (NumberOfKeyCodes * sizeof (*KeyCodes))
        );

      Status = EFI_SUCCESS;
    }
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}
