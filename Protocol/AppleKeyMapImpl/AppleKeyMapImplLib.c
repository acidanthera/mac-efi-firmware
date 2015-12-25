/** @file
  Copyright (C) 2005 - 2015 Apple Inc.  All rights reserved.<BR>

  This program and the accompanying materials have not been licensed.
  Neither is its usage, its redistribution, in source or binary form,
  licensed, nor implicitely or explicitely permitted, except when
  required by applicable law.

  Unless required by applicable law or agreed to in writing, software
  distributed is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
  OR CONDITIONS OF ANY KIND, either express or implied.
**/

#include <AppleEfi.h>
#include <LinkedList.h>

#include "AppleKeyMapImplInternal.h"

// KeyMapGetKeyStrokesByIndex
APPLE_KEY_STROKES_INFO *
KeyMapGetKeyStrokesByIndex (
  IN EFI_LIST  *List,
  IN UINTN     Index
  ) // sub_459
{
  APPLE_KEY_STROKES_INFO *KeyStrokesInfo;

  BOOLEAN                Result;

  for (
    KeyStrokesInfo = APPLE_KEY_STROKES_INFO_FROM_LIST_ENTRY (GetFirstNode (List));
    KeyStrokesInfo->Hdr.Index != Index;
    KeyStrokesInfo = APPLE_KEY_STROKES_INFO_FROM_LIST_ENTRY (GetNextNode (List, &KeyStrokesInfo->Hdr.This))
    ) {
    Result = IsNull (List, &KeyStrokesInfo->Hdr.This);

    if (Result) {
      KeyStrokesInfo = NULL;
      break;
    }
  }

  return KeyStrokesInfo;
}

// KeyMapBubbleSort
VOID
KeyMapBubbleSort (
  IN OUT UINT16 *Operand,
  IN     UINTN  NoChilds
  ) // sub_72C
{
  UINTN  NoRemainingChilds;
  UINTN  Index;
  UINTN  NoRemainingChilds2;
  UINT16 *OperandPtr;
  UINT16 FirstChild;

  ASSERT (Operand != NULL);
  ASSERT (NoChilds > 0);

  if (Operand != NULL) {
    ++Operand;
    NoRemainingChilds = (NoChilds - 1);
    Index             = 1;

    do {
      NoRemainingChilds2 = NoRemainingChilds;
      OperandPtr         = Operand;

      if (Index < NoChilds) {
        do {
          FirstChild = Operand[-1];

          if (FirstChild > *OperandPtr) {
            Operand[-1] = *OperandPtr;
            *OperandPtr = FirstChild;
          }

          ++OperandPtr;
          --NoRemainingChilds2;
        } while (NoRemainingChilds2 > 0);
      }

      ++Index;
      ++Operand;
    } while ((NoRemainingChilds--) > 0);
  }
}
