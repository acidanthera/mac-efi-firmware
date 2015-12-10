//
// Copyright (C) 2005 - 2015 Apple Inc. All rights reserved.
//
// This program and the accompanying materials have not been licensed.
// Neither is its usage, its redistribution, in source or binary form,
// licensed, nor implicitely or explicitely permitted, except when
// required by applicable law.
//
// Unless required by applicable law or agreed to in writing, software
// distributed is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
// OR CONDITIONS OF ANY KIND, either express or implied.
//

///
/// @file      Protocol/AppleKeyMapDatabaseImpl/AppleKeyMapDatabaseImpl.c
///
///
///
/// @author    Download-Fritz
/// @date      15/03/2015: Initial version
/// @copyright Copyright (C) 2005 - 2015 Apple Inc. All rights reserved.
///

#include <AppleEfi.h>
#include <LinkedList.h>

#include <EfiDriverLib.h>

#include <IndustryStandard/AppleHid.h>

#include <Protocol/AppleKeyMapImpl.h>

#include <Library/AppleKeyMapLib.h>

// AppleKeyMapGetKeyStrokesImpl
/// 
/// @param
///
/// @return
/// @retval
APPLE_KEY_STROKES_INFO *
GetKeyStrokesByIndex (
  IN EFI_LIST  *List,
  IN UINTN     Index
  ) // sub_459
{
  APPLE_KEY_STROKES_INFO *KeyStrokesInfo;

  BOOLEAN                Result;

  for (
    KeyStrokesInfo = KEY_STROKES_INFO_FROM_LIST_ENTRY (GetFirstNode (List));
    KeyStrokesInfo->Hdr.Index != Index;
    KeyStrokesInfo = KEY_STROKES_INFO_FROM_LIST_ENTRY (GetNextNode (List, &KeyStrokesInfo->Hdr.This))
    ) {
    Result = IsNull (List, &KeyStrokesInfo->Hdr.This);

    if (Result) {
      KeyStrokesInfo = NULL;
      break;
    }
  }

  return KeyStrokesInfo;
}

// AppleKeyMapCreateKeyStrokesBufferImpl
/// Creates a new key set with a given number of keys allocated. The index within the database is returned.
///
/// @param[in]  This            A pointer to the protocol instance.
/// @param[in]  KeyBufferSize The amount of keys to allocate for the key set.
/// @param[out] Index           The assigned index of the created key set.
///
/// @return                      Returned is the status of the operation.
/// @retval EFI_SUCCESS          A key set with the given number of keys allocated has been created.
/// @retval EFI_OUT_OF_RESOURCES The memory necessary to complete the operation could not be allocated.
/// @retval other                An error returned by a sub-operation.
EFI_STATUS
EFIAPI
AppleKeyMapCreateKeyStrokesBufferImpl (
  IN  APPLE_KEY_MAP_DATABASE_PROTOCOL  *This,
  IN  UINTN                            KeyBufferSize,
  OUT UINTN                            *Index
  )
{
  EFI_STATUS               Status;

  APPLE_KEY_MAP_AGGREGATOR *Aggregator;
  UINTN                    BufferSize;
  APPLE_KEY                *Memory;
  APPLE_KEY_STROKES_INFO   *KeyStrokesInfo;

  Aggregator = AGGREGATOR_FROM_DATABASE_PROTOCOL (This);

  if (Aggregator->KeyBuffer != NULL) {
    gBS->FreePool ((VOID *)Aggregator->KeyBuffer);
  }

  BufferSize                 = (Aggregator->KeyBuffersSize + KeyBufferSize);
  Aggregator->KeyBuffersSize = BufferSize;
  Memory                     = (APPLE_KEY *)EfiLibAllocateZeroPool (BufferSize);
  Aggregator->KeyBuffer      = Memory;
  Status                     = EFI_OUT_OF_RESOURCES;

  if (Memory != NULL) {
    KeyStrokesInfo = (APPLE_KEY_STROKES_INFO *)EfiLibAllocateZeroPool (
                                                 (sizeof (*KeyStrokesInfo) + (KeyBufferSize * sizeof (APPLE_KEY)))
                                                 );
    Status         = EFI_OUT_OF_RESOURCES;

    if (KeyStrokesInfo != NULL) {
      KeyStrokesInfo->Hdr.Signature     = APPLE_KEY_STROKES_INFO_SIGNATURE;
      KeyStrokesInfo->Hdr.KeyBufferSize = KeyBufferSize;
      KeyStrokesInfo->Hdr.Index         = Aggregator->NextKeyStrokeIndex;
      ++Aggregator->NextKeyStrokeIndex;
      
      InsertTailList (&Aggregator->KeyStrokesInfoList, &KeyStrokesInfo->Hdr.This);

      Status = EFI_SUCCESS;
      *Index = KeyStrokesInfo->Hdr.Index;
    }
  }

  return Status;
}

// AppleKeyMapRemoveKeyStrokesBufferImpl
/// Removes a key set specified by its index from the database.
///
/// @param[in]  This  A pointer to the protocol instance.
/// @param[in]  Index The index of the key set to remove.
///
/// @return               Returned is the status of the operation.
/// @retval EFI_SUCCESS   The specified key set has been removed.
/// @retval EFI_NOT_FOUND No key set could be found for the given index.
/// @retval other         An error returned by a sub-operation.
EFI_STATUS
EFIAPI
AppleKeyMapRemoveKeyStrokesBufferImpl (
  IN APPLE_KEY_MAP_DATABASE_PROTOCOL  *This,
  IN UINTN                            Index
  )
{
  EFI_STATUS               Status;

  APPLE_KEY_MAP_AGGREGATOR *Aggregator;
  APPLE_KEY_STROKES_INFO   *KeyStrokesInfo;

  Aggregator     = AGGREGATOR_FROM_DATABASE_PROTOCOL (This);
  KeyStrokesInfo = GetKeyStrokesByIndex (&Aggregator->KeyStrokesInfoList, Index);
  Status         = EFI_NOT_FOUND;

  if (KeyStrokesInfo != NULL) {
    Aggregator->KeyBuffersSize -= KeyStrokesInfo->Hdr.KeyBufferSize;

    RemoveEntryList (&KeyStrokesInfo->Hdr.This);
    gBS->FreePool ((VOID *)KeyStrokesInfo);

    Status = EFI_SUCCESS;
  }

  return Status;
}

// AppleKeyMapSetKeyStrokeBufferKeysImpl
/// Sets the keys of a key set specified by its index to the given Keys buffer.
///
/// @param[in] This      A pointer to the protocol instance.
/// @param[in] Index     The index of the key set to edit.
/// @param[in] Modifiers The key modifiers manipulating the given keys.
/// @param[in] NoKeys    The number of keys contained in Keys.
/// @param[in] Keys      An array of keys to add to the specified key set.
///
/// @return                      Returned is the status of the operation.
/// @retval EFI_SUCCESS          The given keys were set for the specified key set.
/// @retval EFI_OUT_OF_RESOURCES The memory necessary to complete the operation could not be allocated.
/// @retval EFI_NOT_FOUND        No key set could be found for the given index.
/// @retval other                An error returned by a sub-operation.
EFI_STATUS
EFIAPI
AppleKeyMapSetKeyStrokeBufferKeysImpl (
  IN APPLE_KEY_MAP_DATABASE_PROTOCOL  *This,
  IN UINTN                            Index,
  IN APPLE_MODIFIER_MAP               Modifiers,
  IN UINTN                            NoKeys,
  IN APPLE_KEY                        *Keys
  )
{
  EFI_STATUS               Status;

  APPLE_KEY_MAP_AGGREGATOR *Aggregator;
  APPLE_KEY_STROKES_INFO   *KeyStrokesInfo;

  Aggregator     = AGGREGATOR_FROM_DATABASE_PROTOCOL (This);
  KeyStrokesInfo = GetKeyStrokesByIndex (&Aggregator->KeyStrokesInfoList, Index);
  Status         = EFI_NOT_FOUND;

  if (KeyStrokesInfo != NULL) {
    Status = EFI_OUT_OF_RESOURCES;

    if (KeyStrokesInfo->Hdr.KeyBufferSize >= NoKeys) {
      KeyStrokesInfo->Hdr.NoKeys    = NoKeys;
      KeyStrokesInfo->Hdr.Modifiers = Modifiers;

      gBS->CopyMem ((VOID *)&KeyStrokesInfo->Keys, (VOID *)Keys, (NoKeys * sizeof (*Keys)));

      Status = EFI_SUCCESS;
    }
  }

  return Status;
}
