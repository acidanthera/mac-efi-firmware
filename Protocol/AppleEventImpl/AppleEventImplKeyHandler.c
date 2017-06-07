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
#include <AppleMisc.h>

#include <IndustryStandard/AppleHid.h>

#include EFI_PROTOCOL_CONSUMER (ConsoleControl)
#include APPLE_PROTOCOL_CONSUMER (AppleKeyMapAggregator)

#include <Library/AppleDriverLib.h>
#include <Library/AppleEventLib.h>
#include <Library/AppleKeyMapLib.h>
#include <Library/AppleKeyMapAggregatorLib.h>

#include "AppleEventImplInternal.h"

// KEY_STROKE_DELAY
#define KEY_STROKE_DELAY  5

// mCLockOn
GLOBAL_REMOVE_IF_UNREFERENCED BOOLEAN mCLockOn = FALSE;

// mKeyStrokePollEvent
STATIC EFI_EVENT mKeyStrokePollEvent = NULL;

// mModifiers
STATIC APPLE_MODIFIER_MAP mModifiers = 0;

// mInitialized
STATIC BOOLEAN mInitialized = FALSE;

// mKeyInformation
STATIC KEY_STROKE_INFORMATION mKeyStrokeInfo[10];

// mPreviouslyCLockOn
STATIC BOOLEAN mPreviouslyCLockOn = FALSE;

// InternalAppleKeyEventDataFromInputKey
STATIC
EFI_STATUS
InternalAppleKeyEventDataFromInputKey (
  OUT APPLE_EVENT_DATA  *EventData,
  IN  APPLE_KEY_CODE    *AppleKeyCode,
  IN  EFI_INPUT_KEY     *InputKey
  ) // sub_1257
{
  EFI_STATUS           Status;

  APPLE_KEY_EVENT_DATA *KeyEventData;

  ASSERT (EventData != NULL);
  ASSERT (AppleKeyCode != NULL);
  ASSERT (InputKey != NULL);

  Status = EFI_INVALID_PARAMETER;

  if ((EventData != NULL) && (AppleKeyCode != NULL) && (InputKey != NULL)) {
    KeyEventData = EfiLibAllocateZeroPool (sizeof (*KeyEventData));
    Status       = EFI_OUT_OF_RESOURCES;

    if (KeyEventData != NULL) {
      KeyEventData->NumberOfKeyPairs = 1;
      KeyEventData->KeyPair.InputKey = *InputKey;

      EfiCopyMem (
        (VOID *)&KeyEventData->KeyPair.AppleKeyCode,
        (VOID *)AppleKeyCode,
        sizeof (*AppleKeyCode)
        );

      EventData->KeyData = KeyEventData;

      Status = EFI_SUCCESS;
    }
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}

// InternalGetAndRemoveReleasedKeys
STATIC
UINTN
InternalGetAndRemoveReleasedKeys (
  IN  UINTN           *NumberOfKeyCodes,
  IN  APPLE_KEY_CODE  *KeyCodes,
  OUT APPLE_KEY_CODE  **ReleasedKeys
  )
{
  UINTN          NumberOfReleasedKeys;

  UINTN          Index;
  UINTN          Index2;
  APPLE_KEY_CODE ReleasedKeysBuffer[12];
  UINTN          ReleasedKeysSize;

  NumberOfReleasedKeys = 0;

  for (Index = 0; Index < ARRAY_LENGTH (mKeyStrokeInfo); ++Index) {
    for (Index2 = 0; Index2 < *NumberOfKeyCodes; ++Index2) {
      if (mKeyStrokeInfo[Index].AppleKeyCode == KeyCodes[Index2]) {
        break;
      }
    }

    if (*NumberOfKeyCodes == Index2) {
      if (mKeyStrokeInfo[Index].AppleKeyCode != 0) {
        ReleasedKeysBuffer[NumberOfReleasedKeys] = mKeyStrokeInfo[Index].AppleKeyCode;
        ++NumberOfReleasedKeys;
      }

      EfiZeroMem (
        &mKeyStrokeInfo[Index],
        sizeof (mKeyStrokeInfo[Index])
        );
    }
  }

  // Add CLock to released keys if applicable and set bool to FALSE.

  if (mPreviouslyCLockOn) {
    for (Index = 0; Index < *NumberOfKeyCodes; ++Index) {
      if (KeyCodes[Index] == AppleHidUsbKbUsageKeyCLock) {
        break;
      }
    }

    if (*NumberOfKeyCodes == Index) {
      mPreviouslyCLockOn = FALSE;

      ReleasedKeysBuffer[NumberOfReleasedKeys] = AppleHidUsbKbUsageKeyCLock;
      ++NumberOfReleasedKeys;
    }
  }

  // Allocate a heap buffer to return.

  *ReleasedKeys = NULL;

  if (NumberOfReleasedKeys > 0) {
    ReleasedKeysSize = (sizeof (**ReleasedKeys) * NumberOfReleasedKeys);
    *ReleasedKeys    = EfiLibAllocatePool (ReleasedKeysSize);

    if (*ReleasedKeys != NULL) {
      EfiCopyMem (
        (VOID *)*ReleasedKeys,
        (VOID *)&ReleasedKeysBuffer[0],
        ReleasedKeysSize
        );
    } else {
      NumberOfReleasedKeys = 0;
    }
  }

  return NumberOfReleasedKeys;
}

// InternalIsCLockOn
STATIC
BOOLEAN
InternalIsCLockOn (
  IN UINTN           *NumberOfKeyCodes,
  IN APPLE_KEY_CODE  *KeyCodes
  )
{
  BOOLEAN                CLockOn;

  UINTN                  Index;
  KEY_STROKE_INFORMATION *KeyInfoWalker;
  UINTN                  Index2;
  KEY_STROKE_INFORMATION *KeyInfo;

  ASSERT (NumberOfKeyCodes != NULL);
  ASSERT ((((*NumberOfKeyCodes > 0) ? 1 : 0)
              ^ ((KeyCodes == NULL) ? 1 : 0)) != 0);

  CLockOn = FALSE;

  if ((NumberOfKeyCodes != NULL) && ((*NumberOfKeyCodes == 0) || (KeyCodes != NULL))) {
    CLockOn = mCLockOn;

    for (Index = 0; Index < *NumberOfKeyCodes; ++Index) {
      KeyInfo       = NULL;
      KeyInfoWalker = &mKeyStrokeInfo[0];

      for (Index2 = 0; Index2 < ARRAY_LENGTH (mKeyStrokeInfo); ++Index2) {
        KeyInfo = KeyInfoWalker;
        ++KeyInfoWalker;

        if (KeyInfoWalker->AppleKeyCode == KeyCodes[Index]) {
          KeyInfo = KeyInfoWalker;

          break;
        }
      }

      if ((Index2 >= ARRAY_LENGTH (mKeyStrokeInfo)) || (KeyInfo == NULL)) {
        if ((KeyCodes[Index] == AppleHidUsbKbUsageKeyCLock)
         && !mPreviouslyCLockOn) {
          CLockOn = !mCLockOn;
        }

        break;
      }
    }
  }

  return CLockOn;
}

// InternalGetCurrentStroke
STATIC
KEY_STROKE_INFORMATION *
InternalGetCurrentStroke (
  VOID
  )
{
  KEY_STROKE_INFORMATION *KeyInfo;

  KEY_STROKE_INFORMATION *KeyInfoWalker;
  UINTN                  Index;

  KeyInfo       = NULL;
  KeyInfoWalker = &mKeyStrokeInfo[0];

  for (Index = 0; Index < ARRAY_LENGTH (mKeyStrokeInfo); ++Index) {
    if (KeyInfo->CurrentStroke) {
      KeyInfo = KeyInfoWalker;

      break;
    }

    ++KeyInfoWalker;
  }

  return KeyInfo;
}

// InternalGetCurrentKeyStroke
STATIC
EFI_STATUS
InternalGetCurrentKeyStroke (
  IN     APPLE_MODIFIER_MAP  Modifiers,
  IN OUT UINTN               *NumberOfKeyCodes,
  IN OUT APPLE_KEY_CODE      *KeyCodes,
  IN OUT EFI_INPUT_KEY       *Key
  ) // sub_149D
{
  EFI_STATUS             Status;

  KEY_STROKE_INFORMATION *KeyInfo;
  UINTN                  Index;
  UINTN                  Index2;
  UINTN                  NumberOfReleasedKeys;
  APPLE_KEY_CODE         *ReleasedKeys;
  BOOLEAN                CLockOn;
  APPLE_MODIFIER_MAP     AppleModifiers;
  BOOLEAN                ShiftPressed;
  APPLE_KEY_CODE         *ReleasedKeyWalker;
  EFI_INPUT_KEY          InputKey;
  APPLE_EVENT_DATA       AppleEventData;
  KEY_STROKE_INFORMATION *KeyInfoWalker;
  UINTN                  NewKeyIndex;
  BOOLEAN                AcceptStroke;
  BOOLEAN                Shifted;

  ASSERT (NumberOfKeyCodes != NULL);

  if (NumberOfKeyCodes != NULL) {
    ASSERT ((((*NumberOfKeyCodes > 0) ? 1 : 0)
                ^ ((KeyCodes == NULL) ? 1 : 0)) != 0);
  }

  ASSERT (Key != NULL);

  if (mModifiers != Modifiers) {
    for (Index = 0; Index < ARRAY_LENGTH (mKeyStrokeInfo); ++Index) {
      mKeyStrokeInfo[Index].CurrentStroke = FALSE;
    }
  }

  NumberOfReleasedKeys = InternalGetAndRemoveReleasedKeys (
                           NumberOfKeyCodes,
                           KeyCodes,
                           &ReleasedKeys
                           );

  CLockOn = InternalIsCLockOn (NumberOfKeyCodes, KeyCodes);

  AppleModifiers = Modifiers;

  if (CLockOn) {
    AppleModifiers |= APPLE_MODIFIERS_SHIFT;
  }

  ShiftPressed      = (BOOLEAN)((AppleModifiers & APPLE_MODIFIERS_SHIFT) != 0);
  ReleasedKeyWalker = ReleasedKeys;

  for (Index = 0; Index < NumberOfReleasedKeys; ++Index) {
    KeyMapLibInputKeyFromAppleKeyCode (
      *ReleasedKeyWalker,
      &InputKey,
      ShiftPressed
      );

    AppleEventData.KeyData = NULL;
    Status                 = InternalAppleKeyEventDataFromInputKey (
                               &AppleEventData,
                               ReleasedKeyWalker,
                               &InputKey
                               );

    if (Status != EFI_SUCCESS) {
      gBS->FreePool ((VOID *)ReleasedKeys);

      ReleasedKeys = NULL;
    }

    EventCreateEventQuery (
      AppleEventData,
      APPLE_EVENT_TYPE_KEY_UP,
      AppleModifiers
      );

    ++ReleasedKeyWalker;
  }

  if (ReleasedKeys != NULL) {
    gBS->FreePool ((VOID *)ReleasedKeys);
  }

  if (CLockOn != mCLockOn) {
    mCLockOn           = CLockOn;
    mPreviouslyCLockOn = TRUE;
  }

  // increase number of strokes for all currently held keys

  for (NewKeyIndex = 0; NewKeyIndex < *NumberOfKeyCodes; ++NewKeyIndex) {
    KeyInfo       = NULL;
    KeyInfoWalker = mKeyStrokeInfo;

    for (Index2 = 0; Index2 < ARRAY_LENGTH (mKeyStrokeInfo); ++Index2) {
      KeyInfo = KeyInfoWalker;
      ++KeyInfoWalker;

      if (KeyInfo->AppleKeyCode == KeyCodes[NewKeyIndex]) {
        break;
      }
    }

    // Indicates a key has been pressed which is not part of mKeyInformation.
    if ((Index2 >= ARRAY_LENGTH (mKeyStrokeInfo)) || (KeyInfo == NULL)) {
      // if a new key is held down, cancel all previous inputs

      for (Index = 0; Index < ARRAY_LENGTH (mKeyStrokeInfo); ++Index) {
        mKeyStrokeInfo[Index].CurrentStroke = FALSE;
      }

      // Overwrite an empty key info with new key

      KeyInfo = mKeyStrokeInfo;

      for (Index = 0; Index < ARRAY_LENGTH (mKeyStrokeInfo); ++Index) {
        if (KeyInfo->AppleKeyCode == 0) {
          KeyInfo->AppleKeyCode        = KeyCodes[NewKeyIndex];
          KeyInfo->CurrentStroke   = TRUE;
          KeyInfo->NumberOfStrokes = 0;

          break;
        }

        ++KeyInfo;
      }

      break;
    }

    // increase number of strokes for all currently held keys

    ++KeyInfo->NumberOfStrokes;
  }

  KeyInfo = InternalGetCurrentStroke ();

  Status = EFI_NOT_READY;

  if ((KeyInfo != NULL) || (mModifiers != Modifiers)) {
    mModifiers = Modifiers;

    // verify the timeframe the key has been pressed

    if (KeyInfo != NULL) {
      AcceptStroke = (BOOLEAN)(
                       (KeyInfo->NumberOfStrokes < (KEY_STROKE_DELAY * 10))
                          ? (KeyInfo->NumberOfStrokes == 0)
                          : ((KeyInfo->NumberOfStrokes % KEY_STROKE_DELAY) == 0)
                       );

      if (AcceptStroke) {
        *NumberOfKeyCodes = 1;
        *KeyCodes         = KeyInfo->AppleKeyCode;

        Shifted = (BOOLEAN)(
                    (IS_APPLE_KEY_LETTER (KeyInfo->AppleKeyCode) && CLockOn)
                      != ((mModifiers & APPLE_MODIFIERS_SHIFT) != 0)
                    );

        KeyMapLibInputKeyFromAppleKeyCode (KeyInfo->AppleKeyCode, Key, Shifted);
      }
    } else {
      *NumberOfKeyCodes = 0;
    }

    Status = EFI_SUCCESS;
  }

  if (Status != EFI_NOT_READY) {
    ASSERT_EFI_ERROR (Status);
  }

  return Status;
}

// CreateAppleKeyCodeDescriptorsFromKeyStrokes
STATIC
EFI_STATUS
InternalAppleEventDataFromCurrentKeyStroke (
  IN OUT APPLE_EVENT_DATA    *EventData,
  IN OUT APPLE_MODIFIER_MAP  *Modifiers
  ) // sub_1119
{
  EFI_STATUS                      Status;

  EFI_INPUT_KEY                   InputKey;
  APPLE_KEY_CODE                  *KeyCodes;
  APPLE_MODIFIER_MAP              AppleModifiers;
  UINTN                           NumberOfKeyCodes;
  EFI_CONSOLE_CONTROL_PROTOCOL    *Interface;
  EFI_CONSOLE_CONTROL_SCREEN_MODE Mode;
  UINTN                           Index;

  ASSERT (EventData != NULL);
  ASSERT (Modifiers != NULL);
  ASSERT (mAppleKeyMapAggregator != NULL);

  EfiZeroMem (&InputKey, sizeof (InputKey));

  KeyCodes   = NULL;
  Status = EFI_UNSUPPORTED;

  if ((mAppleKeyMapAggregator != NULL)
   && (EventData != NULL)
   && (Modifiers != NULL)) {
    AppleModifiers = 0;
    NumberOfKeyCodes   = 0;

    KeyMapAggrLibGetAppleKeyStrokes (&AppleModifiers, &NumberOfKeyCodes, &KeyCodes);

    Mode   = EfiConsoleControlScreenGraphics;
    Status = gBS->LocateProtocol (
                    &gEfiConsoleControlProtocolGuid,
                    NULL,
                    (VOID *)&Interface
                    );

    if (!EFI_ERROR (Status)) {
      Interface->GetMode (Interface, &Mode, NULL, NULL);
    }

    if (Mode == EfiConsoleControlScreenGraphics) {
      for (Index = 0; Index < (NumberOfKeyCodes + 1); ++Index) {
        Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &InputKey);

        if (EFI_ERROR (Status)) {
          break;
        }
      }
    }

    *Modifiers = AppleModifiers;
    Status     = InternalGetCurrentKeyStroke (
                   AppleModifiers,
                   &NumberOfKeyCodes,
                   KeyCodes,
                   &InputKey
                   );

    if (!EFI_ERROR (Status) && (NumberOfKeyCodes > 0)) {
      InternalAppleKeyEventDataFromInputKey (EventData, KeyCodes, &InputKey);
    }
  }

  if (Status != EFI_NOT_READY) {
    ASSERT_EFI_ERROR (Status);
  }

  return Status;
}

// InternalKeyStrokePollNotifyFunction
STATIC
VOID
EFIAPI
InternalKeyStrokePollNotifyFunction (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  ) // sub_12CE
{
  EFI_STATUS         Status;

  APPLE_EVENT_DATA   EventData;
  APPLE_MODIFIER_MAP Modifiers;
  APPLE_MODIFIER_MAP PartialModifers;

  ASSERT (Event != NULL);

  EventData.KeyData = NULL;
  Modifiers         = 0;
  Status            = InternalAppleEventDataFromCurrentKeyStroke (
                        &EventData,
                        &Modifiers
                        );

  if (!EFI_ERROR (Status)) {
    if (EventData.KeyData != NULL) {
      EventCreateEventQuery (EventData, APPLE_EVENT_TYPE_KEY_DOWN, Modifiers);
    }

    if (mModifiers != Modifiers) {
      PartialModifers = ((mModifiers ^ Modifiers) & mModifiers);

      if (PartialModifers != 0) {
        EventData.KeyData = NULL;

        EventCreateEventQuery (
          EventData,
          APPLE_EVENT_TYPE_MODIFIER_UP,
          PartialModifers
          );
      }

      PartialModifers = (Modifiers & (mModifiers ^ Modifiers));

      if (PartialModifers != 0) {
        EventData.KeyData = NULL;

        EventCreateEventQuery (
          EventData,
          APPLE_EVENT_TYPE_MODIFIER_DOWN,
          PartialModifers
          );
      }

      mModifiers = Modifiers;
    }
  }
}

// InternalInitializeKeyHandler
STATIC
VOID
InternalInitializeKeyHandler (
  VOID
  ) // sub_143C
{
  ASSERT (!mInitialized);

  if (!mInitialized) {
    mInitialized = TRUE;

    EfiZeroMem ((VOID *)&mKeyStrokeInfo[0], sizeof (mKeyStrokeInfo));

    mModifiers         = 0;
    mCLockOn           = FALSE;
    mPreviouslyCLockOn = FALSE;
  }
}

// EventCreateKeyStrokePollEvent
EFI_STATUS
EventCreateKeyStrokePollEvent (
  VOID
  ) // sub_13AC
{
  EFI_STATUS Status;

  ASSERT (mKeyStrokePollEvent == NULL);

  Status = gBS->LocateProtocol (
                  &gAppleKeyMapAggregatorProtocolGuid,
                  NULL,
                  (VOID **)&mAppleKeyMapAggregator
                  );

  if (!EFI_ERROR (Status)) {
    InternalInitializeKeyHandler ();

    mKeyStrokePollEvent = EventLibCreateNotifyTimerEvent (
                            InternalKeyStrokePollNotifyFunction,
                            NULL,
                            EFI_TIMER_PERIOD_MILLISECONDS (10),
                            TRUE
                            );

    Status = ((mKeyStrokePollEvent == NULL)
               ? EFI_OUT_OF_RESOURCES
               : EFI_SUCCESS);
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}

// EventCancelKeyStrokePollEvent
VOID
EventCancelKeyStrokePollEvent (
  VOID
  ) // sub_1417
{
  ASSERT (mKeyStrokePollEvent != NULL);

  EventLibCancelEvent (mKeyStrokePollEvent);

  mKeyStrokePollEvent = NULL;
}
