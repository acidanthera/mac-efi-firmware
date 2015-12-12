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
/// @file      Protocol/AppleKeyMapImpl/AppleKeyMapImplInternal.h
///
///            
///
/// @author    Download-Fritz
/// @date      12/12/2015: Initial version
/// @copyright Copyright (C) 2005 - 2015 Apple Inc. All rights reserved.
///

#ifndef __APPLE_KEY_MAP_IMPL_INTERNAL_H__
#define __APPLE_KEY_MAP_IMPL_INTERNAL_H__

#include <Protocol/AppleKeyMapImpl.h>

/// @{
#define APPLE_KEY_STROKES_INFO_SIGNATURE                  EFI_SIGNATURE_32 ('K', 'e', 'y', 'S')
#define APPLE_KEY_STROKES_INFO_FROM_LIST_ENTRY(ListEntry) _CR ((ListEntry), APPLE_KEY_STROKES_INFO, Hdr.This)
/// @}

// _APPLE_KEY_STROKES_INFO
typedef struct _APPLE_KEY_STROKES_INFO {
  struct {
    UINTN              Signature;      ///< 
    EFI_LIST_ENTRY     This;           ///< 
    UINTN              Index;          ///< 
    UINTN              KeyBufferSize;  ///< 
    UINTN              NoKeys;         ///< 
    APPLE_MODIFIER_MAP Modifiers;      ///< 
  }         Hdr;   ///< 
  APPLE_KEY Keys;  ///< 
} APPLE_KEY_STROKES_INFO;

// KeyMapGetKeyStrokesByIndex
/// 
/// @param
///
/// @return
/// @retval
APPLE_KEY_STROKES_INFO *
KeyMapGetKeyStrokesByIndex (
  IN EFI_LIST  *List,
  IN UINTN     Index
  );

// KeyMapBubbleSort
/// 
/// @param
///
/// @return
/// @retval
VOID
KeyMapBubbleSort (
  IN OUT UINT16 *Operand,
  IN     UINTN  NoChilds
  );

#endif // ifndef __APPLE_KEY_MAP_IMPL_INTERNAL_H__
