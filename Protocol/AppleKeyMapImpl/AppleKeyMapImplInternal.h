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

#ifndef APPLE_KEY_MAP_IMPL_INTERNAL_H_
#define APPLE_KEY_MAP_IMPL_INTERNAL_H_

#include APPLE_PROTOCOL_PRODUCER (AppleKeyMapImpl)

/// @{
#define APPLE_KEY_STROKES_INFO_SIGNATURE  EFI_SIGNATURE_32 ('K', 'e', 'y', 'S')

#define APPLE_KEY_STROKES_INFO_FROM_LIST_ENTRY(Entry)  \
  ((APPLE_KEY_STROKES_INFO *)(                         \
    CR (                                               \
      (Entry),                                         \
      APPLE_KEY_STROKES_INFO_HDR,                      \
      This,                                            \
      APPLE_KEY_STROKES_INFO_SIGNATURE                 \
      )                                                \
    ))
/// @}

// APPLE_KEY_STROKES_INFO_HDR
typedef struct {
  UINTN              Signature;      ///< 
  EFI_LIST_ENTRY     This;           ///< 
  UINTN              Index;          ///< 
  UINTN              KeyBufferSize;  ///< 
  UINTN              NoKeys;         ///< 
  APPLE_MODIFIER_MAP Modifiers;      ///<
} APPLE_KEY_STROKES_INFO_HDR;

// APPLE_KEY_STROKES_INFO
typedef struct {
  APPLE_KEY_STROKES_INFO_HDR Hdr;   ///< 
  APPLE_KEY                  Keys;  ///< 
} APPLE_KEY_STROKES_INFO;

// KeyMapGetKeyStrokesByIndex
APPLE_KEY_STROKES_INFO *
KeyMapGetKeyStrokesByIndex (
  IN EFI_LIST  *List,
  IN UINTN     Index
  );

// KeyMapBubbleSort
VOID
KeyMapBubbleSort (
  IN OUT UINT16 *Operand,
  IN     UINTN  NoChilds
  );

#endif // APPLE_KEY_MAP_IMPL_INTERNAL_H_
