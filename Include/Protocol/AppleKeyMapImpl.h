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

#ifndef APPLE_KEY_MAP_IMPL_H_
#define APPLE_KEY_MAP_IMPL_H_

#include <IndustryStandard/AppleHid.h>

#include APPLE_PROTOCOL_PRODUCER (AppleKeyMapAggregator)
#include APPLE_PROTOCOL_PRODUCER (AppleKeyMapDatabase)

/// @{
#define APPLE_KEY_MAP_PROTOCOLS_REVISION  0x010000

#define APPLE_KEY_MAP_DATABASE_PROTOCOL_REVISION  \
  APPLE_KEY_MAP_PROTOCOLS_REVISION

#define APPLE_KEY_MAP_AGGREGATOR_PROTOCOL_REVISION  \
  APPLE_KEY_MAP_PROTOCOLS_REVISION
/// @}

/// @{
#define APPLE_KEY_MAP_AGGREGATOR_SIGNATURE  \
          EFI_SIGNATURE_32 ('K', 'e', 'y', 'A')

#define APPLE_KEY_MAP_AGGREGATOR_FROM_AGGREGATOR_PROTOCOL(This)  \
  CR (                                                           \
    (This),                                                      \
    APPLE_KEY_MAP_AGGREGATOR,                                    \
    AggregatorProtocol,                                          \
    APPLE_KEY_MAP_AGGREGATOR_SIGNATURE                           \
    )

#define APPLE_KEY_MAP_AGGREGATOR_FROM_DATABASE_PROTOCOL(This)  \
  CR (                                                         \
    (This),                                                    \
    APPLE_KEY_MAP_AGGREGATOR,                                  \
    DatabaseProtocol,                                          \
    APPLE_KEY_MAP_AGGREGATOR_SIGNATURE                         \
    )
/// @}

// APPLE_KEY_MAP_AGGREGATOR
typedef struct {
  UINTN                             Signature;           ///< 
  UINTN                             NextKeyStrokeIndex;  ///< 
  APPLE_KEY                         *KeyBuffer;          ///< 
  UINTN                             KeyBuffersSize;      ///< 
  EFI_LIST                          KeyStrokesInfoList;  ///< 
  APPLE_KEY_MAP_DATABASE_PROTOCOL   DatabaseProtocol;    ///< 
  APPLE_KEY_MAP_AGGREGATOR_PROTOCOL AggregatorProtocol;  ///< 
} APPLE_KEY_MAP_AGGREGATOR;

// KeyMapCreateKeyStrokesBufferImpl
/** Creates a new key set with a given number of keys allocated.  The index
    within the database is returned.

  @param[in]  This             A pointer to the protocol instance.
  @param[in]  KeyBufferLength  The amount of keys to allocate for the key set.
  @param[out] Index            The assigned index of the created key set.

  @return                       Returned is the status of the operation.
  @retval EFI_SUCCESS           A key set with the given number of keys
                                allocated has been created.
  @retval EFI_OUT_OF_RESOURCES  The memory necessary to complete the operation
                                could not be allocated.
  @retval other                 An error returned by a sub-operation.
**/
EFI_STATUS
EFIAPI
KeyMapCreateKeyStrokesBufferImpl (
  IN  APPLE_KEY_MAP_DATABASE_PROTOCOL  *This,
  IN  UINTN                            KeyBufferLength,
  OUT UINTN                            *Index
  );

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
KeyMapRemoveKeyStrokesBufferImpl (
  IN APPLE_KEY_MAP_DATABASE_PROTOCOL  *This,
  IN UINTN                            Index
  );

// KeyMapSetKeyStrokeBufferKeysImpl
/** Sets the keys of a key set specified by its index to the given Keys Buffer.

  @param[in] This       A pointer to the protocol instance.
  @param[in] Index      The index of the key set to edit.
  @param[in] Modifiers  The key modifiers manipulating the given keys.
  @param[in] NoKeys     The number of keys contained in Keys.
  @param[in] Keys       An array of keys to add to the specified key set.

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
KeyMapSetKeyStrokeBufferKeysImpl (
  IN APPLE_KEY_MAP_DATABASE_PROTOCOL  *This,
  IN UINTN                            Index,
  IN APPLE_MODIFIER_MAP               Modifiers,
  IN UINTN                            NoKeys,
  IN APPLE_KEY                        *Keys
  );

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
  IN  APPLE_KEY_MAP_AGGREGATOR_PROTOCOL  *This,
  OUT APPLE_MODIFIER_MAP                 *Modifiers,
  OUT UINTN                              *NoKeys,
  OUT APPLE_KEY                          *Keys
  );

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
  );

#endif // APPLE_KEY_MAP_IMPL_H_
