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
BOOLEAN mCLockOn = FALSE;

// mKeyStrokePollEvent
STATIC EFI_EVENT mKeyStrokePollEvent = NULL;

// mModifiers
STATIC APPLE_MODIFIER_MAP mModifiers = 0;

// mInitialized
STATIC BOOLEAN mInitialized = FALSE;

// mKeyInformation
STATIC KEY_STROKE_INFORMATION mKeyInformation[10];

// mPreviouslyCLockOn
STATIC BOOLEAN mPreviouslyCLockOn = FALSE;

// AppleKeyDescriptorFromScanCode
EFI_STATUS
AppleKeyEventDataFromInputKey (
  OUT APPLE_EVENT_DATA  *EventData,
  IN  APPLE_KEY         *AppleKey,
  IN  EFI_INPUT_KEY     *InputKey
  ) // sub_1257
{
  EFI_STATUS           Status;

  APPLE_KEY_EVENT_DATA *KeyEventData;

  ASSERT (EventData != NULL);
  ASSERT (AppleKey != NULL);
  ASSERT (InputKey != NULL);

  Status = EFI_INVALID_PARAMETER;

  if ((EventData != NULL) && (AppleKey != NULL) && (InputKey != NULL)) {
    KeyEventData = EfiLibAllocateZeroPool (sizeof (*KeyEventData));
    Status       = EFI_OUT_OF_RESOURCES;

    if (KeyEventData != NULL) {
      KeyEventData->NumberOfKeyPairs = 1;
      KeyEventData->KeyPair.InputKey = *InputKey;

      EfiCommonLibCopyMem (
        (VOID *)&KeyEventData->KeyPair.AppleKey,
        (VOID *)AppleKey,
        sizeof (*AppleKey)
        );

      EventData->KeyData = KeyEventData;

      Status = EFI_SUCCESS;
    }
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}

inline
STATIC
UINTN
RemoveReleasedKeys (
  IN  UINTN      *NumberOfKeys,
  IN  APPLE_KEY  *Keys,
  OUT APPLE_KEY  *ReleasedKeys
  )
{
  UINTN NumberOfReleasedKeys;

  UINTN Index;
  UINTN Index2;

  NumberOfReleasedKeys = 0;

  for (Index = 0; Index < ARRAY_LENGTH (mKeyInformation); ++Index) {
    for (Index2 = 0; Index2 < *NumberOfKeys; ++Index2) {
      if (mKeyInformation[Index].AppleKey == Keys[Index2]) {
        break;
      }
    }

    if (*NumberOfKeys == Index2) {
      if (mKeyInformation[Index].AppleKey != 0) {
        ReleasedKeys[NumberOfReleasedKeys] = mKeyInformation[Index].AppleKey;
        ++NumberOfReleasedKeys;
      }

      EfiCommonLibZeroMem (
        &mKeyInformation[Index],
        sizeof (mKeyInformation[Index])
        );
    }
  }

  // add CLock to released keys if applicable and set bool to FALSE

  if (mPreviouslyCLockOn) {
    for (Index = 0; Index < *NumberOfKeys; ++Index) {
      if (Keys[Index] == AppleHidUsbKbUsageKeyCLock) {
        break;
      }
    }

    if (*NumberOfKeys == Index) {
      mPreviouslyCLockOn                 = FALSE;
      ReleasedKeys[NumberOfReleasedKeys] = AppleHidUsbKbUsageKeyCLock;
      ++NumberOfReleasedKeys;
    }
  }

  return NumberOfReleasedKeys;
}

// GetCurrentKeyStroke
EFI_STATUS
GetCurrentKeyStroke (
  IN     APPLE_MODIFIER_MAP  Modifiers,
  IN OUT UINTN               *NumberOfKeys,
  IN OUT APPLE_KEY           *Keys,
  IN OUT EFI_INPUT_KEY       *Key
  ) // sub_149D
{
  EFI_STATUS             Status;

  KEY_STROKE_INFORMATION *KeyInfo;
  UINTN                  Index;
  UINTN                  Index2;
  UINTN                  NoReleasedKeys;
  APPLE_KEY              ReleasedKeys[12];
  APPLE_KEY              *ReleasedKeysPtr;
  UINTN                  ReleasedKeysSize;
  BOOLEAN                TempCLockOn;
  BOOLEAN                CLockOn;
  APPLE_MODIFIER_MAP     AppleModifiers;
  BOOLEAN                ShiftPressed;
  APPLE_KEY              *ReleasedKeyWalker;
  EFI_INPUT_KEY          InputKey;
  APPLE_EVENT_DATA       AppleEventData;
  KEY_STROKE_INFORMATION *KeyInfoWalker;
  UINTN                  NewKeyIndex;
  BOOLEAN                Shifted;

  ASSERT (NumberOfKeys != NULL);
  ASSERT ((((*NumberOfKeys > 0) ? 1 : 0) ^ ((Keys == NULL) ? 1 : 0)) != 0);
  ASSERT (Key != NULL);

  if (mModifiers != Modifiers) {
    KeyInfo = mKeyInformation;

    for (Index = 0; Index < ARRAY_LENGTH (mKeyInformation); ++Index) {
      mKeyInformation[Index].CurrentStroke = FALSE;
    }
  }

  TempCLockOn = FALSE;

  NoReleasedKeys = RemoveReleasedKeys (NumberOfKeys, Keys, &ReleasedKeys[0]);

  ReleasedKeysPtr = NULL;

  if (NoReleasedKeys > 0) {
    ReleasedKeysSize = (sizeof (*ReleasedKeys) * NoReleasedKeys);
    ReleasedKeysPtr  = EfiLibAllocatePool (ReleasedKeysSize);

    if (ReleasedKeysPtr != NULL) {
      EfiCommonLibCopyMem (
        (VOID *)ReleasedKeysPtr,
        (VOID *)&ReleasedKeys,
        ReleasedKeysSize
        );
    } else {
      NoReleasedKeys = 0;
    }
  }

  if ((NumberOfKeys != NULL) && ((*NumberOfKeys == 0) || (Keys != NULL))) {
    TempCLockOn = mCLockOn;

    for (Index = 0; Index < *NumberOfKeys; ++Index) {
      KeyInfoWalker = mKeyInformation;

      for (Index2 = 0; Index2 < ARRAY_LENGTH (mKeyInformation); ++Index2) {
        KeyInfo = KeyInfoWalker;
        ++KeyInfoWalker;

        if (KeyInfo->AppleKey == Keys[Index]) {
          break;
        }
      }

      if ((Index2 >= ARRAY_LENGTH (mKeyInformation)) || (KeyInfo == NULL)) {
        if ((Keys[Index] == AppleHidUsbKbUsageKeyCLock) && !mPreviouslyCLockOn) {
          TempCLockOn = !mCLockOn;
        }

        break;
      }
    }
  }

  AppleModifiers = (Modifiers | APPLE_MODIFIERS_SHIFT);

  if (!TempCLockOn) {
    AppleModifiers = Modifiers;
  }

  ShiftPressed      = (BOOLEAN)((AppleModifiers & APPLE_MODIFIERS_SHIFT) != 0);
  ReleasedKeyWalker = ReleasedKeysPtr;

  for (Index = 0; Index < NoReleasedKeys; ++Index) {
    InputKeyFromAppleKey (*ReleasedKeyWalker, &InputKey, ShiftPressed);

    AppleEventData.KeyData = NULL;
    Status                 = AppleKeyEventDataFromInputKey (
                               &AppleEventData,
                               ReleasedKeyWalker,
                               &InputKey
                               );

    if (Status != EFI_SUCCESS) {
      gBS->FreePool ((VOID *)ReleasedKeysPtr);

      ReleasedKeysPtr = NULL;
    }

    EventCreateEventQuery (
      AppleEventData,
      APPLE_EVENT_TYPE_KEY_UP,
      AppleModifiers
      );

    ++ReleasedKeyWalker;
  }

  if (ReleasedKeysPtr != NULL) {
    gBS->FreePool ((VOID *)ReleasedKeysPtr);
  }

  CLockOn = mCLockOn;

  if (TempCLockOn != mCLockOn) {
    mCLockOn           = TempCLockOn;
    mPreviouslyCLockOn = TRUE;
    CLockOn            = TempCLockOn;
  }

  // increase number of strokes for all currently held keys

  for (NewKeyIndex = 0; NewKeyIndex < *NumberOfKeys; ++NewKeyIndex) {
    KeyInfoWalker = mKeyInformation;

    for (Index2 = 0; Index2 < ARRAY_LENGTH (mKeyInformation); ++Index2) {
      KeyInfo = KeyInfoWalker;
      ++KeyInfoWalker;

      if (KeyInfo->AppleKey == Keys[NewKeyIndex]) {
        break;
      }
    }

    // NOTE: (KeyInfo == NULL) makes no sense.
    // Indicates a key has been pressed which is not part of mKeyInformation.
    if ((Index2 >= ARRAY_LENGTH (mKeyInformation)) || (KeyInfo == NULL)) {
      // if a new key is held down, cancel all previous inputs

      for (Index = 0; Index < ARRAY_LENGTH (mKeyInformation); ++Index) {
        mKeyInformation[Index].CurrentStroke = FALSE;
      }

      // Overwrite an empty key info with new key

      KeyInfo = mKeyInformation;

      for (Index = 0; Index < ARRAY_LENGTH (mKeyInformation); ++Index) {
        if (KeyInfo->AppleKey == 0) {
          KeyInfo->AppleKey = Keys[NewKeyIndex];
          KeyInfo->CurrentStroke = TRUE;
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

  KeyInfo       = NULL;
  KeyInfoWalker = mKeyInformation;

  for (Index = 0; Index < ARRAY_LENGTH (mKeyInformation); ++Index) {
    KeyInfo = KeyInfoWalker;
    ++KeyInfoWalker;

    if (KeyInfo->CurrentStroke) {
      break;
    }

    KeyInfo = NULL;
  }

  Status = EFI_NOT_READY;

  if ((KeyInfo != NULL) || (mModifiers != Modifiers)) {
    mModifiers = Modifiers;

    // verify the timeframe the key has been pressed

    if (KeyInfo != NULL) {
      if (KeyInfo->NumberOfStrokes < (KEY_STROKE_DELAY * 10)) {
        if (KeyInfo->NumberOfStrokes > 0) {
          goto Return;
        }
      } else if ((KeyInfo->NumberOfStrokes % KEY_STROKE_DELAY) > 0) {
        goto Return;
      }

      *NumberOfKeys = 1;
      *Keys         = KeyInfo->AppleKey;

      Shifted = (BOOLEAN)(
                  (IS_APPLE_KEY_LETTER (KeyInfo->AppleKey) && CLockOn)
                    != ((mModifiers & APPLE_MODIFIERS_SHIFT) != 0)
                  );

      InputKeyFromAppleKey (KeyInfo->AppleKey, Key, Shifted);
    } else {
      *NumberOfKeys = 0;
    }

    Status = EFI_SUCCESS;
  }

Return:
  if (Status != EFI_NOT_READY) {
    ASSERT_EFI_ERROR (Status);
  }

  return Status;
}

// CreateAppleKeyDescriptorsFromKeyStrokes
EFI_STATUS
AppleEventDataFromCurrentKeyStroke (
  IN OUT APPLE_EVENT_DATA    *EventData,
  IN OUT APPLE_MODIFIER_MAP  *Modifiers
  ) // sub_1119
{
  EFI_STATUS                      Status;

  EFI_INPUT_KEY                   InputKey;
  APPLE_KEY                       *Keys;
  APPLE_MODIFIER_MAP              AppleModifiers;
  UINTN                           NumberOfKeys;
  EFI_CONSOLE_CONTROL_PROTOCOL    *Interface;
  EFI_CONSOLE_CONTROL_SCREEN_MODE Mode;
  UINTN                           Index;

  ASSERT (mAppleKeyMapAggregator != NULL);
  ASSERT (EventData != NULL);
  ASSERT (Modifiers != NULL);

  EfiZeroMem (&InputKey, sizeof (InputKey));

  Keys   = NULL;
  Status = EFI_UNSUPPORTED;

  if ((mAppleKeyMapAggregator != NULL)
   && (EventData != NULL)
   && (Modifiers != NULL)) {
    AppleModifiers = 0;
    NumberOfKeys   = 0;

    GetAppleKeyStrokes (&AppleModifiers, &NumberOfKeys, &Keys);

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
      for (Index = 0; Index < (NumberOfKeys + 1); ++Index) {
        Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &InputKey);

        if (EFI_ERROR (Status)) {
          break;
        }
      }
    }

    *Modifiers = AppleModifiers;
    Status     = GetCurrentKeyStroke (
                   AppleModifiers,
                   &NumberOfKeys,
                   Keys,
                   &InputKey
                   );

    if (!EFI_ERROR (Status) && (NumberOfKeys > 0)) {
      AppleKeyEventDataFromInputKey (EventData, Keys, &InputKey);
    }
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}

// KeyStrokePollNotifyFunction
VOID
EFIAPI
KeyStrokePollNotifyFunction (
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
  Status            = AppleEventDataFromCurrentKeyStroke (
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

// Initialize
VOID
Initialize (
  VOID
  ) // sub_143C
{
  ASSERT (!mInitialized);

  if (!mInitialized) {
    mInitialized = TRUE;

    EfiZeroMem ((VOID *)&mKeyInformation[0], sizeof (mKeyInformation));

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

  Status = gBS->LocateProtocol (
                  &gAppleKeyMapAggregatorProtocolGuid,
                  NULL,
                  (VOID **)&mAppleKeyMapAggregator
                  );

  if (!EFI_ERROR (Status)) {
    Initialize ();

    mKeyStrokePollEvent = CreateNotifyEvent (
                            KeyStrokePollNotifyFunction,
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

  CancelEvent (mKeyStrokePollEvent);

  mKeyStrokePollEvent = NULL;
}
