/** @file
  Copyright (C) 2005 - 2017, Apple Inc.  All rights reserved.<BR>

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

  KeyStrokesInfo = APPLE_KEY_STROKES_INFO_FROM_LIST_ENTRY (
                     GetFirstNode (List)
                     );

  while (KeyStrokesInfo->Hdr.Index != Index) {
    if (IsNull (List, &KeyStrokesInfo->Hdr.This)) {
      KeyStrokesInfo = NULL;

      break;
    }

    KeyStrokesInfo = APPLE_KEY_STROKES_INFO_FROM_LIST_ENTRY (
                       GetNextNode (List, &KeyStrokesInfo->Hdr.This)
                       );
  }

  return KeyStrokesInfo;
}

// KeyMapMinSort
VOID
KeyMapMinSort (
  IN OUT UINT16  *Operand,
  IN     UINTN   NumberOfChilds
  ) // sub_72C
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
