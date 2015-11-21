///
/// @file      AppleKeyMapImpl.h
///
///            Private data of the Apple protocols.
///
/// @author    Download-Fritz
/// @date      15/03/2015: Initial version
/// @copyright The decompilation is of an educational purpose to better understand the behavior of the
///            Apple EFI implementation and making use of it. In no way is the content's usage licensed
///            or allowed. All rights remain at Apple Inc. To be used under the terms of 'Fair use'.
///

//
// CREDITS:
//   Reversed from AppleKeyMapAggregator.efi and AppleEvent.efi, which are Apple Inc. property
//	 Key modifier and identifier list by tiamo (macosxbootloader, AppleKeyState.h)
//   Decompiled by Download-Fritz
//

#ifndef __APPLE_KEY_MAP_IMPL_H__
#define __APPLE_KEY_MAP_IMPL_H__

#define APPLE_KEY_MAP_PROTOCOLS_REVISION            0x010000
#define APPLE_KEY_MAP_DATABASE_PROTOCOL_REVISION    APPLE_KEY_MAP_PROTOCOLS_REVISION
#define APPLE_KEY_MAP_AGGREGATOR_PROTOCOL_REVISION  APPLE_KEY_MAP_PROTOCOLS_REVISION

#define APPLE_KEY_MAP_AGGREGATOR_SIGNATURE        EFI_SIGNATURE_32 ('K', 'e', 'y', 'A')
#define AGGREGATOR_FROM_AGGREGATOR_PROTOCOL(This) CR ((This), APPLE_KEY_MAP_AGGREGATOR, AggregatorProtocol, APPLE_KEY_MAP_AGGREGATOR_SIGNATURE)
#define AGGREGATOR_FROM_DATABASE_PROTOCOL(This)   CR ((This), APPLE_KEY_MAP_AGGREGATOR, DatabaseProtocol, APPLE_KEY_MAP_AGGREGATOR_SIGNATURE)

// _APPLE_KEY_MAP_AGGREGATOR
typedef struct _APPLE_KEY_MAP_AGGREGATOR {
  UINTN                             Signature;           ///< 
  UINTN                             NextKeyStrokeIndex;  ///< 
  APPLE_KEY                         *KeyBuffer;          ///< 
  UINTN                             KeyBuffersSize;      ///< 
  EFI_LIST                          KeyStrokesInfoList;  ///< 
  APPLE_KEY_MAP_DATABASE_PROTOCOL   DatabaseProtocol;    ///< 
  APPLE_KEY_MAP_AGGREGATOR_PROTOCOL AggregatorProtocol;  ///< 
} APPLE_KEY_MAP_AGGREGATOR;

#define APPLE_KEY_STROKES_INFO_SIGNATURE            EFI_SIGNATURE_32 ('K', 'e', 'y', 'S')
#define KEY_STROKES_INFO_FROM_LIST_ENTRY(ListEntry) _CR ((ListEntry), APPLE_KEY_STROKES_INFO, Hdr.This)

// _APPLE_KEY_STROKES_INFO
typedef struct _APPLE_KEY_STROKES_INFO {
  struct {
    UINTN              Signature;      ///< 
    EFI_LIST_ENTRY     This;           ///< 
    UINTN              Index;          ///< 
    UINTN              KeyBufferSize;  ///< 
    UINTN              NoKeys;         ///< 
    APPLE_MODIFIER_MAP Modifiers;      ///< 
  }         Hdr;                       ///< 
  APPLE_KEY Keys;                      ///< 
} APPLE_KEY_STROKES_INFO;

// AppleKeyMapCreateKeyStrokesBufferImpl
/// Creates a new key set with a given number of keys allocated. The index within the database is returned.
///
/// @param[in]  This            A pointer to the protocol instance.
/// @param[in]  KeyBufferLength The amount of keys to allocate for the key set.
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
  IN  UINTN                            KeyBufferLength,
  OUT UINTN                            *Index
  );

// AppleKeyMapRemoveKeyStrokesBufferImpl
/// Removes a key set specified by its index from the database.
///
/// @param[in] This  A pointer to the protocol instance.
/// @param[in] Index The index of the key set to remove.
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
  );

// AppleKeyMapSetKeyStrokeKeysImpl
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
  );

// AppleKeyMapGetKeyStrokesImpl
/// Returns all pressed keys and key modifiers into the appropiate buffers.
///
/// @param[in]  This      A pointer to the protocol instance.
/// @param[out] Modifiers The modifiers manipulating the given keys.
/// @param[out] NoKeys    On input the number of keys allocated.
///                       On output the number of keys returned into Keys.
/// @param[out] Keys      A Pointer to a caller-allocated the pressed keys get returned in.
///
/// @return
/// @retval EFI_SUCCESS          The pressed keys have been returned into Keys.
/// @retval EFI_BUFFER_TOO_SMALL The memory required to return the value exceeds the size of the allocated buffer.
///                              The required number of keys to allocate to complete the operation has been returned into NoKeys.
/// @retval other                An error returned by a sub-operation.
EFI_STATUS
EFIAPI
AppleKeyMapGetKeyStrokesImpl (
  IN  APPLE_KEY_MAP_AGGREGATOR_PROTOCOL  *This,
  OUT APPLE_MODIFIER_MAP                 *Modifiers,
  OUT UINTN                              *NoKeys,
  OUT APPLE_KEY                          *Keys
  );

// AppleKeyMapContainsKeyStrokesImpl
/// Returns whether or not a list of keys and their modifiers are part of the database of pressed keys.
///
/// @param[in]      This       A pointer to the protocol instance.
/// @param[in]      Modifiers  The modifiers manipulating the given keys.
/// @param[in]      NoKeys     The number of keys present in Keys.
/// @param[in, out] Keys       The list of keys to check for. The children may be sorted in the process.
/// @param[in]      ExactMatch Specifies whether Modifiers and Keys should be exact matches or just contained.
///
/// @return               Returns whether or not a list of keys and their modifiers are part of the database of pressed keys.
/// @retval EFI_SUCCESS   The queried keys are part of the database.
/// @retval EFI_NOT_FOUND The queried keys could not be found.
EFI_STATUS
EFIAPI
AppleKeyMapContainsKeyStrokesImpl (
  IN     APPLE_KEY_MAP_AGGREGATOR_PROTOCOL  *This,
  IN     APPLE_MODIFIER_MAP                 Modifiers,
  IN     UINTN                              NoKeys,
  IN OUT APPLE_KEY                          *Keys,
  IN     BOOLEAN                            ExactMatch
  );

#endif // ifndef __APPLE_KEY_MAP_IMPL_H__
