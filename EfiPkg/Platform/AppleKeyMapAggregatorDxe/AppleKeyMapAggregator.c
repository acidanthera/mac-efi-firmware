/** @file
  Copyright (c) 2005 - 2017, Apple Inc.  All rights reserved.<BR>

  This program and the accompanying materials have not been licensed.
  Neither is its usage, its redistribution, in source or binary form,
  licensed, nor implicitely or explicitely permitted, except when
  required by applicable law.

  Unless required by applicable law or agreed to in writing, software
  distributed is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
  OR CONDITIONS OF ANY KIND, either express or implied.
**/

#include <AppleMacEfi.h>

#include <IndustryStandard/AppleHid.h>

#include <Protocol/AppleKeyMapAggregator.h>
#include <Protocol/AppleKeyMapDatabase.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

// KEY_MAP_AGGREGATOR_DATA_SIGNATURE
#define KEY_MAP_AGGREGATOR_DATA_SIGNATURE  \
  SIGNATURE_32 ('K', 'e', 'y', 'A')

// KEY_MAP_AGGREGATOR_DATA_FROM_AGGREGATOR_THIS
#define KEY_MAP_AGGREGATOR_DATA_FROM_AGGREGATOR_THIS(This)  \
  CR (                                                      \
    (This),                                                 \
    KEY_MAP_AGGREGATOR_DATA,                                \
    Aggregator,                                             \
    KEY_MAP_AGGREGATOR_DATA_SIGNATURE                       \
    )

// KEY_MAP_AGGREGATOR_DATA_FROM_DATABASE_THIS
#define KEY_MAP_AGGREGATOR_DATA_FROM_DATABASE_THIS(This)  \
  CR (                                                    \
    (This),                                               \
    KEY_MAP_AGGREGATOR_DATA,                              \
    Database,                                             \
    KEY_MAP_AGGREGATOR_DATA_SIGNATURE                     \
    )

// KEY_MAP_AGGREGATOR_DATA
typedef struct {
  UINTN                             Signature;           ///< 
  UINTN                             NextKeyStrokeIndex;  ///< 
  APPLE_KEY_CODE                    *KeyCodeBuffer;      ///< 
  UINTN                             KeyBuffersSize;      ///< 
  LIST_ENTRY                        KeyStrokesInfoList;  ///< 
  APPLE_KEY_MAP_DATABASE_PROTOCOL   Database;            ///< 
  APPLE_KEY_MAP_AGGREGATOR_PROTOCOL Aggregator;          ///< 
} KEY_MAP_AGGREGATOR_DATA;

// APPLE_KEY_MAP_DATABASE_PROTOCOL_REVISION
#define APPLE_KEY_MAP_DATABASE_PROTOCOL_REVISION  0x010000

// APPLE_KEY_MAP_AGGREGATOR_PROTOCOL_REVISION
#define APPLE_KEY_MAP_AGGREGATOR_PROTOCOL_REVISION  0x010000

// APPLE_KEY_STROKES_INFO_SIGNATURE
#define APPLE_KEY_STROKES_INFO_SIGNATURE  SIGNATURE_32 ('K', 'e', 'y', 'S')

// APPLE_KEY_STROKES_INFO_FROM_LIST_ENTRY
#define APPLE_KEY_STROKES_INFO_FROM_LIST_ENTRY(Entry)  \
  ((APPLE_KEY_STROKES_INFO *)(                         \
    CR (                                               \
      (Entry),                                         \
      APPLE_KEY_STROKES_INFO_HDR,                      \
      Link,                                            \
      APPLE_KEY_STROKES_INFO_SIGNATURE                 \
      )                                                \
    ))

// APPLE_KEY_STROKES_INFO_HDR
typedef struct {
  UINTN              Signature;         ///< 
  LIST_ENTRY         Link;              ///< 
  UINTN              Index;             ///< 
  UINTN              KeyBufferSize;     ///< 
  UINTN              NumberOfKeyCodes;  ///< 
  APPLE_MODIFIER_MAP Modifiers;         ///<
} APPLE_KEY_STROKES_INFO_HDR;

// APPLE_KEY_STROKES_INFO
typedef struct {
  APPLE_KEY_STROKES_INFO_HDR Hdr;       ///< 
  APPLE_KEY_CODE             KeyCodes;  ///< 
} APPLE_KEY_STROKES_INFO;

// InternalGetKeyStrokesByIndex
STATIC
APPLE_KEY_STROKES_INFO *
InternalGetKeyStrokesByIndex (
  IN LIST_ENTRY  *List,
  IN UINTN       Index
  )
{
  APPLE_KEY_STROKES_INFO *KeyStrokesInfo;

  KeyStrokesInfo = APPLE_KEY_STROKES_INFO_FROM_LIST_ENTRY (
                     GetFirstNode (List)
                     );

  while (KeyStrokesInfo->Hdr.Index != Index) {
    if (IsNull (List, &KeyStrokesInfo->Hdr.Link)) {
      KeyStrokesInfo = NULL;

      break;
    }

    KeyStrokesInfo = APPLE_KEY_STROKES_INFO_FROM_LIST_ENTRY (
                       GetNextNode (List, &KeyStrokesInfo->Hdr.Link)
                       );
  }

  return KeyStrokesInfo;
}

// InternalMinSort
STATIC
VOID
InternalMinSort (
  IN OUT UINT16  *Operand,
  IN     UINTN   NumberOfChilds
  )
{
  UINTN  TotalNumberOfRemainingChilds;
  UINTN  Index;
  UINTN  NumberOfRemainingChilds;
  UINT16 *OperandPtr;
  UINT16 FirstChild;

  ASSERT (Operand != NULL);
  ASSERT (NumberOfChilds > 0);

  if (Operand != NULL) {
    ++Operand;

    TotalNumberOfRemainingChilds = (NumberOfChilds - 1);

    Index = 1;

    do {
      NumberOfRemainingChilds = TotalNumberOfRemainingChilds;

      OperandPtr = Operand;

      if (Index < NumberOfChilds) {
        do {
          FirstChild = Operand[-1];

          if (FirstChild > *OperandPtr) {
            Operand[-1] = *OperandPtr;
            *OperandPtr = FirstChild;
          }

          ++OperandPtr;
          --NumberOfRemainingChilds;
        } while (NumberOfRemainingChilds > 0);
      }

      ++Index;
      ++Operand;
    } while ((TotalNumberOfRemainingChilds--) > 0);
  }
}

// InternalGetKeyStrokes
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
STATIC
EFI_STATUS
EFIAPI
InternalGetKeyStrokes (
  IN     APPLE_KEY_MAP_AGGREGATOR_PROTOCOL  *This,
  OUT    APPLE_MODIFIER_MAP                 *Modifiers,
  OUT    UINTN                              *NumberOfKeyCodes,
  IN OUT APPLE_KEY_CODE                     *KeyCodes OPTIONAL
  )
{
  EFI_STATUS              Status;

  KEY_MAP_AGGREGATOR_DATA *KeyMapAggregatorData;
  APPLE_KEY_STROKES_INFO  *KeyStrokesInfo;
  BOOLEAN                 Result;
  APPLE_MODIFIER_MAP      DbModifiers;
  UINTN                   DbNumberOfKeyCodestrokes;
  UINTN                   Index;
  UINTN                   Index2;
  APPLE_KEY_CODE          Key;

  ASSERT (This != NULL);
  ASSERT (Modifiers != NULL);
  ASSERT (NumberOfKeyCodes != NULL);
  ASSERT ((((*NumberOfKeyCodes > 0) ? 1 : 0)
              ^ ((KeyCodes == NULL) ? 1 : 0)) != 0);

  KeyMapAggregatorData = KEY_MAP_AGGREGATOR_DATA_FROM_AGGREGATOR_THIS (This);

  KeyStrokesInfo = APPLE_KEY_STROKES_INFO_FROM_LIST_ENTRY (
                     GetFirstNode (&KeyMapAggregatorData->KeyStrokesInfoList)
                     );

  Result = IsNull (
             &KeyMapAggregatorData->KeyStrokesInfoList,
             &KeyStrokesInfo->Hdr.Link
             );

  if (Result) {
    *NumberOfKeyCodes        = 0;
    DbNumberOfKeyCodestrokes = 0;
    DbModifiers              = 0;
  } else {
    DbModifiers              = 0;
    DbNumberOfKeyCodestrokes = 0;

    do {
      DbModifiers |= KeyStrokesInfo->Hdr.Modifiers;

      if (KeyStrokesInfo->Hdr.NumberOfKeyCodes > 0) {
        Index = 0;

        do {
          Key = (&KeyStrokesInfo->KeyCodes)[Index];
          ++Index;

          for (Index2 = 0; Index2 < DbNumberOfKeyCodestrokes; ++Index2) {
            if (KeyMapAggregatorData->KeyCodeBuffer[Index2] == Key) {
              break;
            }
          }

          if (DbNumberOfKeyCodestrokes == Index2) {
            KeyMapAggregatorData->KeyCodeBuffer[DbNumberOfKeyCodestrokes] = Key;
            ++DbNumberOfKeyCodestrokes;
          }
        } while (Index < KeyStrokesInfo->Hdr.NumberOfKeyCodes);
      }

      KeyStrokesInfo = APPLE_KEY_STROKES_INFO_FROM_LIST_ENTRY (
                         GetNextNode (
                           &KeyMapAggregatorData->KeyStrokesInfoList,
                           &KeyStrokesInfo->Hdr.Link
                           )
                         );

      Result = !IsNull (
                  &KeyMapAggregatorData->KeyStrokesInfoList,
                  &KeyStrokesInfo->Hdr.Link
                  );
    } while (Result);

    Result = (BOOLEAN)(DbNumberOfKeyCodestrokes > *NumberOfKeyCodes);

    *NumberOfKeyCodes = DbNumberOfKeyCodestrokes;

    Status = EFI_BUFFER_TOO_SMALL;

    if (Result) {
      goto Done;
    }
  }

  *Modifiers        = DbModifiers;
  *NumberOfKeyCodes = DbNumberOfKeyCodestrokes;

  Status = EFI_SUCCESS;

  if (KeyCodes != NULL) {
    CopyMem (
      (VOID *)KeyCodes,
      (VOID *)KeyMapAggregatorData->KeyCodeBuffer,
      (DbNumberOfKeyCodestrokes * sizeof (*KeyCodes))
      );
  }

Done:
  ASSERT_EFI_ERROR (Status);

  return Status;
}

// InternalContainsKeyStrokes
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
STATIC
EFI_STATUS
EFIAPI
InternalContainsKeyStrokes (
  IN     APPLE_KEY_MAP_AGGREGATOR_PROTOCOL  *This,
  IN     APPLE_MODIFIER_MAP                 Modifiers,
  IN     UINTN                              NumberOfKeyCodes,
  IN OUT APPLE_KEY_CODE                     *KeyCodes,
  IN     BOOLEAN                            ExactMatch
  )
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

  DbNumberOfKeyCodes = ARRAY_SIZE (DbKeyCodes);
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
        InternalMinSort ((UINT16 *)KeyCodes, NumberOfKeyCodes);
        InternalMinSort ((UINT16 *)DbKeyCodes, DbNumberOfKeyCodes);

        Result = CompareMem (
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
STATIC
EFI_STATUS
EFIAPI
InternalCreateKeyStrokesBuffer (
  IN  APPLE_KEY_MAP_DATABASE_PROTOCOL  *This,
  IN  UINTN                            KeyBufferSize,
  OUT UINTN                            *Index
  )
{
  EFI_STATUS              Status;

  KEY_MAP_AGGREGATOR_DATA *KeyMapAggregatorData;
  UINTN                   BufferSize;
  APPLE_KEY_CODE          *Memory;
  APPLE_KEY_STROKES_INFO  *KeyStrokesInfo;

  ASSERT (This != NULL);
  ASSERT (KeyBufferSize > 0);
  ASSERT (Index != NULL);

  KeyMapAggregatorData = KEY_MAP_AGGREGATOR_DATA_FROM_DATABASE_THIS (This);

  if (KeyMapAggregatorData->KeyCodeBuffer != NULL) {
    gBS->FreePool ((VOID *)KeyMapAggregatorData->KeyCodeBuffer);
  }

  BufferSize = (KeyMapAggregatorData->KeyBuffersSize + KeyBufferSize);

  KeyMapAggregatorData->KeyBuffersSize = BufferSize;

  Memory = AllocateZeroPool (BufferSize);

  KeyMapAggregatorData->KeyCodeBuffer  = Memory;

  Status = EFI_OUT_OF_RESOURCES;

  if (Memory != NULL) {
    KeyStrokesInfo = AllocateZeroPool (
                       sizeof (*KeyStrokesInfo)
                         + (KeyBufferSize * sizeof (APPLE_KEY_CODE))
                       );

    Status = EFI_OUT_OF_RESOURCES;

    if (KeyStrokesInfo != NULL) {
      KeyStrokesInfo->Hdr.Signature     = APPLE_KEY_STROKES_INFO_SIGNATURE;
      KeyStrokesInfo->Hdr.KeyBufferSize = KeyBufferSize;
      KeyStrokesInfo->Hdr.Index         = KeyMapAggregatorData->NextKeyStrokeIndex;

      ++KeyMapAggregatorData->NextKeyStrokeIndex;
      
      InsertTailList (
        &KeyMapAggregatorData->KeyStrokesInfoList,
        &KeyStrokesInfo->Hdr.Link
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
STATIC
EFI_STATUS
EFIAPI
InternalRemoveKeyStrokesBuffer (
  IN APPLE_KEY_MAP_DATABASE_PROTOCOL  *This,
  IN UINTN                            Index
  )
{
  EFI_STATUS              Status;

  KEY_MAP_AGGREGATOR_DATA *KeyMapAggregatorData;
  APPLE_KEY_STROKES_INFO  *KeyStrokesInfo;

  ASSERT (This != NULL);

  KeyMapAggregatorData = KEY_MAP_AGGREGATOR_DATA_FROM_DATABASE_THIS (This);

  KeyStrokesInfo = InternalGetKeyStrokesByIndex (
                     &KeyMapAggregatorData->KeyStrokesInfoList,
                     Index
                     );

  Status = EFI_NOT_FOUND;

  if (KeyStrokesInfo != NULL) {
    KeyMapAggregatorData->KeyBuffersSize -= KeyStrokesInfo->Hdr.KeyBufferSize;

    RemoveEntryList (&KeyStrokesInfo->Hdr.Link);
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
STATIC
EFI_STATUS
EFIAPI
InternalSetKeyStrokeBufferKeys (
  IN APPLE_KEY_MAP_DATABASE_PROTOCOL  *This,
  IN UINTN                            Index,
  IN APPLE_MODIFIER_MAP               Modifiers,
  IN UINTN                            NumberOfKeyCodes,
  IN APPLE_KEY_CODE                   *KeyCodes
  )
{
  EFI_STATUS               Status;

  KEY_MAP_AGGREGATOR_DATA *KeyMapAggregatorData;
  APPLE_KEY_STROKES_INFO  *KeyStrokesInfo;

  ASSERT (This != NULL);
  ASSERT (NumberOfKeyCodes > 0);
  ASSERT (KeyCodes != NULL);

  KeyMapAggregatorData = KEY_MAP_AGGREGATOR_DATA_FROM_DATABASE_THIS (This);

  KeyStrokesInfo = InternalGetKeyStrokesByIndex (
                     &KeyMapAggregatorData->KeyStrokesInfoList,
                     Index
                     );

  Status = EFI_NOT_FOUND;

  if (KeyStrokesInfo != NULL) {
    Status = EFI_OUT_OF_RESOURCES;

    if (KeyStrokesInfo->Hdr.KeyBufferSize >= NumberOfKeyCodes) {
      KeyStrokesInfo->Hdr.NumberOfKeyCodes = NumberOfKeyCodes;
      KeyStrokesInfo->Hdr.Modifiers        = Modifiers;

      CopyMem (
        (VOID *)&KeyStrokesInfo->KeyCodes,
        (VOID *)KeyCodes, (NumberOfKeyCodes * sizeof (*KeyCodes))
        );

      Status = EFI_SUCCESS;
    }
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}

// AppleKeyMapAggregatorMain
/**

  @param[in] ImageHandle  The firmware allocated handle for the EFI image.
  @param[in] SystemTable  A pointer to the EFI System Table.

  @retval EFI_SUCCESS          The entry point is executed successfully.
  @retval EFI_ALREADY_STARTED  The protocol has already been installed.
**/
EFI_STATUS
EFIAPI
AppleKeyMapAggregatorMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS              Status;

  UINTN                   NumberOfHandles;
  EFI_HANDLE              *Buffer;
  KEY_MAP_AGGREGATOR_DATA *KeyMapAggregatorData;
  EFI_HANDLE              Handle;

  ASSERT_PROTOCOL_ALREADY_INSTALLED (NULL, &gAppleKeyMapDatabaseProtocolGuid);

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gAppleKeyMapDatabaseProtocolGuid,
                  NULL,
                  &NumberOfHandles,
                  &Buffer
                  );

  if (!EFI_ERROR (Status)) {
    Status = EFI_DEVICE_ERROR;

    if (Buffer != NULL) {
      gBS->FreePool ((VOID *)Buffer);
    }
  } else {
    KeyMapAggregatorData = AllocateZeroPool (sizeof (*KeyMapAggregatorData));

    // BUG: Compare to != NULL.
    ASSERT (KeyMapAggregatorData); // By Apple.

    KeyMapAggregatorData->Signature          = KEY_MAP_AGGREGATOR_DATA_SIGNATURE;
    KeyMapAggregatorData->NextKeyStrokeIndex = 3000;

    KeyMapAggregatorData->Database.Revision               = APPLE_KEY_MAP_DATABASE_PROTOCOL_REVISION;
    KeyMapAggregatorData->Database.CreateKeyStrokesBuffer = InternalCreateKeyStrokesBuffer;
    KeyMapAggregatorData->Database.RemoveKeyStrokesBuffer = InternalRemoveKeyStrokesBuffer;
    KeyMapAggregatorData->Database.SetKeyStrokeBufferKeys = InternalSetKeyStrokeBufferKeys;

    KeyMapAggregatorData->Aggregator.Revision           = APPLE_KEY_MAP_AGGREGATOR_PROTOCOL_REVISION;
    KeyMapAggregatorData->Aggregator.GetKeyStrokes      = InternalGetKeyStrokes;
    KeyMapAggregatorData->Aggregator.ContainsKeyStrokes = InternalContainsKeyStrokes;

    InitializeListHead (&KeyMapAggregatorData->KeyStrokesInfoList);

    // BUG: Protocols are never uninstalled.

    Handle = NULL;
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &Handle,
                    &gAppleKeyMapDatabaseProtocolGuid,
                    (VOID *)&KeyMapAggregatorData->Database,
                    &gAppleKeyMapAggregatorProtocolGuid,
                    (VOID *)&KeyMapAggregatorData->Aggregator,
                    NULL
                    );

    ASSERT_EFI_ERROR (Status); // By Apple.
  }

  return Status;
}
