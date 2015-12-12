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
/// @file      Protocol/AppleEventImpl/AppleEventImplKeyHandler.c
///
///            
///
/// @author    Download-Fritz
/// @date      31/02/2015: Initial version
/// @copyright Copyright (C) 2005 - 2015 Apple Inc. All rights reserved.
///

#include <AppleEfi.h>

#include <EfiDriverLib.h>

#include <IndustryStandard/AppleHid.h>

#include EFI_PROTOCOL_CONSUMER (ConsoleControl)
#include <Protocol/AppleKeyMapAggregator.h>

#include <Library/EfiEventLib.h>
#include <Library/AppleKeyMapLib.h>
#include <Library/AppleKeyMapAggregatorLib.h>
#include <Library/AppleEventLib.h>

#include "AppleEventImplInternal.h"

// KEY_STROKE_DELAY
#define KEY_STROKE_DELAY  5

// mCLockOn
BOOLEAN mCLockOn = FALSE;

// mKeyStrokePollEvent
static EFI_EVENT mKeyStrokePollEvent = NULL;

// mModifiers
static APPLE_MODIFIER_MAP mModifiers = 0;

// mInitialized
static BOOLEAN mInitialized = FALSE;

// mKeyInformation
static KEY_STROKE_INFORMATION mKeyInformation[10];

// mPreviouslyCLockOn
static BOOLEAN mPreviouslyCLockOn = FALSE;

// AppleKeyDescriptorFromScanCode
/// 
///
/// @param 
///
/// @return 
/// @retval 
static
EFI_STATUS
AppleKeyEventDataFromInputKey (
  OUT APPLE_EVENT_DATA  *EventData,
  IN  APPLE_KEY         *AppleKey,
  IN  EFI_INPUT_KEY    *InputKey
  ) // sub_1257
{
  EFI_STATUS           Status;

  APPLE_KEY_EVENT_DATA *KeyEventData;

  Status = EFI_INVALID_PARAMETER;

  if ((EventData != NULL) && (AppleKey != NULL) && (InputKey != NULL)) {
    KeyEventData = (APPLE_KEY_EVENT_DATA *)EfiLibAllocateZeroPool (sizeof (*KeyEventData));
    Status       = EFI_OUT_OF_RESOURCES;

    if (KeyEventData != NULL) {
      KeyEventData->NoKeyPairs       = 1;
      KeyEventData->KeyPair.InputKey = *InputKey;

      EfiCommonLibCopyMem ((VOID *)&KeyEventData->KeyPair.AppleKey, (VOID *)AppleKey, sizeof (*AppleKey));

      EventData->AppleKeyEventData = KeyEventData;
      Status                       = EFI_SUCCESS;
    }
  }

  return Status;
}

// GetCurrentKeyStroke
/// 
///
/// @param 
///
/// @return 
/// @retval 
static
EFI_STATUS
GetCurrentKeyStroke (
  IN     APPLE_MODIFIER_MAP  Modifiers,
  IN OUT UINTN               *NoKeys,
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
  APPLE_KEY              *ReleasedKeyPtr;
  EFI_INPUT_KEY          InputKey;
  APPLE_EVENT_DATA       AppleEventData;
  KEY_STROKE_INFORMATION *KeyInfo2;
  UINTN                  NewKeyIndex;
  BOOLEAN                Shifted;

  TempCLockOn = FALSE;
  NoReleasedKeys  = 0;
  ReleasedKeysPtr = NULL;

  if (mModifiers != Modifiers) {
    KeyInfo = mKeyInformation;

    for (Index = 0; Index < ARRAY_LENGTH (mKeyInformation); ++Index) {
      mKeyInformation[Index].CurrentStroke = FALSE;
    }
  }

  // clean previous keys - set no longer pressed key infos to 0s

  for (Index = 0; Index < ARRAY_LENGTH (mKeyInformation); ++Index) {
    for (Index2 = 0; Index < *NoKeys; ++Index) {
      if (mKeyInformation[Index].AppleKey == Keys[Index2]) {
        break;
      }
    }

    if (*NoKeys == Index2) {
      if (mKeyInformation[Index].AppleKey != 0) {
        ReleasedKeys[NoReleasedKeys] = mKeyInformation[Index].AppleKey;
        ++NoReleasedKeys;
      }

      EfiCommonLibZeroMem (&mKeyInformation[Index], sizeof (mKeyInformation[Index]));
    }
  }

  // add CAPS to previous keys if applicable (inactive now) and set bool to FALSE

  if (mPreviouslyCLockOn) {
    for (Index = 0; Index < *NoKeys; ++Index) {
      if (Keys[Index] == AppleHidUsbKbUsageKeyCLock) {
        break;
      }
    }

    if (*NoKeys == Index) {
      mPreviouslyCLockOn       = FALSE;
      ReleasedKeys[NoReleasedKeys] = AppleHidUsbKbUsageKeyCLock;
      ++NoReleasedKeys;
    }
  }

  if (NoReleasedKeys > 0) {
    ReleasedKeysSize = (sizeof (*ReleasedKeys) * NoReleasedKeys);
    ReleasedKeysPtr  = (APPLE_KEY *)EfiLibAllocatePool (ReleasedKeysSize);

    if (ReleasedKeysPtr != NULL) {
      EfiCommonLibCopyMem ((VOID *)ReleasedKeysPtr, (VOID *)&ReleasedKeys, ReleasedKeysSize);
    }
  }

  if ((NoKeys != NULL) && ((*NoKeys == 0) || (Keys != NULL))) {
    TempCLockOn = mCLockOn;

    for (Index = 0; Index < *NoKeys; ++Index) {
      Index2          = 0;
      KeyInfo2 = mKeyInformation;

      while (TRUE) {
        KeyInfo = KeyInfo2;

        if (Index2 < ARRAY_LENGTH (mKeyInformation)) {
          if (KeyInfo->AppleKey == Keys[Index]) {
            break;
          }

          ++KeyInfo2;
          ++Index2;
        } else {
          if ((Keys[Index] == AppleHidUsbKbUsageKeyCLock) && !mPreviouslyCLockOn) {
            TempCLockOn = !mCLockOn;
          }

          goto BreakBoth;
        }
      }

      if (KeyInfo == NULL) {
        break;
      }
    }
  }

BreakBoth:
  AppleModifiers = (Modifiers | APPLE_MODIFIERS_SHIFT);

  if (!TempCLockOn) {
    AppleModifiers = Modifiers;
  }

  ShiftPressed   = ((AppleModifiers & APPLE_MODIFIERS_SHIFT) != 0);
  ReleasedKeyPtr = ReleasedKeysPtr;

  for (Index = 0; Index < *NoKeys; ++Index) {
    InputKeyFromAppleKey (*ReleasedKeyPtr, &InputKey, ShiftPressed);

    Status = AppleKeyEventDataFromInputKey (&AppleEventData, ReleasedKeyPtr, &InputKey);

    if (Status != EFI_SUCCESS) {
      gBS->FreePool ((VOID *)ReleasedKeysPtr);
    } else {
      EventCreateEventQuery (AppleEventData, APPLE_EVENT_TYPE_KEY_UP, AppleModifiers);

      ++ReleasedKeyPtr;
    }
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

  for (NewKeyIndex = 0; NewKeyIndex < *NoKeys; ++NewKeyIndex) {
    Index2   = 0;
    KeyInfo2 = mKeyInformation;

    while (TRUE) {
      if (Index2 >= ARRAY_LENGTH (mKeyInformation)) {
        goto NoNewKey;
      }

      KeyInfo = KeyInfo2;

      if (mKeyInformation[Index2].AppleKey == Keys[NewKeyIndex]) {
        break;
      }

      ++KeyInfo2;
      ++NewKeyIndex;
    }

    if (KeyInfo == NULL) {
      break;
    }

    ++KeyInfo->NoStrokes;
  }

  // if a new key is hold down, cancel all previos inputs

  for (Index = 0; Index < ARRAY_LENGTH (mKeyInformation); ++Index) {
    mKeyInformation[Index].CurrentStroke = FALSE;
  }

  // Overwrite an empty key info with new key

  KeyInfo = mKeyInformation;

  for (Index = 0; Index < ARRAY_LENGTH (mKeyInformation); ++Index) {
    if (KeyInfo->AppleKey == 0) {
      if (KeyInfo != NULL) {
        KeyInfo->AppleKey      = Keys[NewKeyIndex];
        KeyInfo->CurrentStroke = TRUE;
        KeyInfo->NoStrokes     = 0;
      }

      break;
    }

    ++KeyInfo;
  }

NoNewKey:
  KeyInfo  = NULL;
  KeyInfo2 = mKeyInformation;

  for (Index = 0; Index < ARRAY_LENGTH (mKeyInformation); ++Index) {
    KeyInfo = KeyInfo2;

    if (KeyInfo2->CurrentStroke) {
      break;
    }

    ++KeyInfo2;
    KeyInfo = NULL;
  }

  Status = EFI_NOT_READY;

  if ((KeyInfo != NULL) || (mModifiers != Modifiers)) {
    mModifiers = Modifiers;

    // verify the timeframe the key has been pressed

    if (KeyInfo == NULL) {
      *NoKeys = 0;
    } else if (KeyInfo->NoStrokes < (KEY_STROKE_DELAY * 10)) {
      if (KeyInfo->NoStrokes > 0) {
        goto Return;
      }
    } else if ((KeyInfo->NoStrokes % KEY_STROKE_DELAY) > 0) {
      goto Return;
    }

    *NoKeys = 1;
    *Keys   = KeyInfo->AppleKey;
    Shifted = ((IS_APPLE_KEY_LETTER (KeyInfo->AppleKey) && CLockOn) != ((mModifiers & APPLE_MODIFIERS_SHIFT) != 0));

    InputKeyFromAppleKey (KeyInfo->AppleKey, Key, Shifted);

    Status = EFI_SUCCESS;
  }

Return:
  return Status;
}

// CreateAppleKeyDescriptorsFromKeyStrokes
/// 
///
/// @param 
///
/// @return 
/// @retval 
static
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
  UINTN                           NoKeys;
  EFI_CONSOLE_CONTROL_PROTOCOL    *Interface;
  EFI_CONSOLE_CONTROL_SCREEN_MODE Mode;
  UINTN                           Index;

  EfiZeroMem (&InputKey, sizeof (InputKey));

  Keys   = NULL;
  Status = EFI_UNSUPPORTED;

  if ((mAppleKeyMapAggregator != NULL) && (EventData != NULL) && (Modifiers != NULL)) {
    AppleModifiers = 0;
    NoKeys         = 0;

    GetAppleKeyStrokes (&AppleModifiers, &NoKeys, &Keys);

    Mode   = EfiConsoleControlScreenGraphics;
    Status = gBS->LocateProtocol (&gEfiConsoleControlProtocolGuid, NULL, (VOID *)&Interface);

    if (!EFI_ERROR (Status)) {
      Interface->GetMode (Interface, &Mode, NULL, NULL);
    }

    if (Mode == EfiConsoleControlScreenGraphics) {
      for (Index = 0; Index < (NoKeys + 1); ++Index) {
        Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &InputKey);

        if (EFI_ERROR (Status)) {
          break;
        }
      }
    }

    *Modifiers = AppleModifiers;
    Status     = GetCurrentKeyStroke (AppleModifiers, &NoKeys, Keys, &InputKey);

    if (!EFI_ERROR (Status) && (NoKeys > 0)) {
      AppleKeyEventDataFromInputKey (EventData, Keys, &InputKey);
    }
  }

  return Status;
}

// KeyStrokePollNotifyFunction
/// 
///
/// @param 
///
/// @return 
/// @retval 
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

  EventData.AppleKeyEventData = NULL;
  Modifiers                   = 0;
  Status                      = AppleEventDataFromCurrentKeyStroke (&EventData, &Modifiers);

  if (!EFI_ERROR (Status)) {
    if (EventData.AppleKeyEventData != NULL) {
      EventCreateEventQuery (EventData, APPLE_EVENT_TYPE_KEY_DOWN, Modifiers);
    }

    if (mModifiers != Modifiers) {
      PartialModifers = SELECT_BITS (DIFF_BITS (mModifiers, Modifiers), mModifiers);

      if (PartialModifers != 0) {
        EventData.AppleKeyEventData = NULL;

        EventCreateEventQuery (EventData, APPLE_EVENT_TYPE_MODIFIER_UP, PartialModifers);
      }

      PartialModifers = SELECT_BITS (Modifiers, DIFF_BITS (mModifiers, Modifiers));

      if (PartialModifers != 0) {
        EventData.AppleKeyEventData = NULL;

        EventCreateEventQuery (EventData, APPLE_EVENT_TYPE_MODIFIER_DOWN, PartialModifers);
      }

      mModifiers = Modifiers;
    }
  }
}

// Initialize
/// 
///
/// @param 
///
/// @return 
/// @retval 
static
VOID
Initialize (
  VOID
  ) // sub_143C
{
  if (!mInitialized) {
    mInitialized = TRUE;

    EfiCommonLibZeroMem ((VOID *)&mKeyInformation, sizeof (mKeyInformation));

    mModifiers             = 0;
    mCLockOn           = FALSE;
    mPreviouslyCLockOn = FALSE;
  }
}

// EventCreateKeyStrokePollEvent
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EventCreateKeyStrokePollEvent (
  VOID
  ) // sub_13AC
{
  EFI_STATUS Status;

  Status = gBS->LocateProtocol (&gAppleKeyMapAggregatorProtocolGuid, NULL, (VOID **)&mAppleKeyMapAggregator);

  if (!EFI_ERROR (Status)) {
    Initialize ();
    mKeyStrokePollEvent = CreateNotifyEvent (
                            KeyStrokePollNotifyFunction,
                            NULL, EFI_TIMER_PERIOD_MILLISECONDS (10),
                            TRUE
                            );
    Status              = ((mKeyStrokePollEvent == NULL) ? EFI_OUT_OF_RESOURCES : EFI_SUCCESS);
  }

  return Status;
}

// EventCancelKeyStrokePollEvent
/// 
///
/// @param 
///
/// @return 
/// @retval 
VOID
EventCancelKeyStrokePollEvent (
  VOID
  ) // sub_1417
{
  CancelEvent (mKeyStrokePollEvent);

  mKeyStrokePollEvent = NULL;
}
