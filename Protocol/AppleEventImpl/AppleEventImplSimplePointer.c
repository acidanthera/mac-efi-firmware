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
/// @file      Protocol/AppleEventImpl/AppleEventImplSimplePointer.c
///
///            
///
/// @author    Download-Fritz
/// @date      31/02/2015: Initial version
/// @copyright Copyright (C) 2005 - 2015 Apple Inc. All rights reserved.
///

#include <AppleEfi.h>

#include <EfiDriverLib.h>
#include <EfiCommonLib.h>

#include <IndustryStandard/AppleHid.h>

#include <Guid/AppleNvram.h>

#include EFI_PROTOCOL_CONSUMER (GraphicsOutput)
#include EFI_PROTOCOL_CONSUMER (SimplePointer)
#include <Protocol/AppleKeyMapAggregator.h>

#include <Library/EfiEventLib.h>
#include <Library/AppleKeyMapLib.h>
#include <Library/AppleKeyMapAggregatorLib.h>
#include <Library/AppleEventLib.h>
#ifdef CPU_IA32
#include <Library/AppleMathLib.h>
#endif // ifdef CPU_IA32

#include "AppleEventImplInternal.h"

// UI_SCALE_VARIABLE_NAME
#define UI_SCALE_VARIABLE_NAME  L"UIScale"

/// @{
#define MINIMAL_DOUBLE_CLICK_SPEED  374
#define MAXIMAL_CLICK_DURATION      74
#define MINIMAL_MOVEMENT            5
/// @}

// mSimplePointerInstallNotifyEvent
EFI_EVENT mSimplePointerInstallNotifyEvent = NULL;

// mSimplePointerInstallNotifyRegistration
static VOID *mSimplePointerInstallNotifyRegistration = NULL;

// mSimplePointerInstances
EFI_PROTOCOL_INSTANCE *mSimplePointerInstances = NULL;

// mNoSimplePointerInstances
static UINTN mNoSimplePointerInstances = 0;

// mSimplePointerPollEvent
static EFI_EVENT mSimplePointerPollEvent = NULL;

// mUiScale
static UINT8 mUiScale = 1;

// mLeftButtonInformation
static POINTER_BUTTON_INFORMATION mLeftButtonInformation = {
  APPLE_EVENT_TYPE_LEFT_BUTTON,
  0,
  0,
  0,
  FALSE,
  FALSE,
  { 0, 0 },
  { 0, 0 }
};

// mRightButtonInformation
static POINTER_BUTTON_INFORMATION mRightButtonInformation = {
  APPLE_EVENT_TYPE_RIGHT_BUTTON,
  0,
  0,
  0,
  FALSE,
  FALSE,
  { 0, 0 },
  { 0, 0 }
};

// mCursorPosition
static DIMENSION mCursorPosition;

// mMouseMoved
static BOOLEAN mMouseMoved;

// mScreenResolutionSet
static BOOLEAN mScreenResolutionSet;

// mScreenResolution
static DIMENSION mScreenResolution;

// AddProtocolInstance
/// 
///
/// @param 
///
/// @return 
/// @retval 
static
VOID
AddProtocolInstance (
  IN EFI_HANDLE Handle,
  IN VOID       *Interface
  )
{
  EFI_PROTOCOL_INSTANCE *Buffer;
  UINTN                 Index;

  Buffer = (EFI_PROTOCOL_INSTANCE *)EfiLibAllocateZeroPool (
                                      ((mNoSimplePointerInstances + 1) * sizeof (*mSimplePointerInstances))
                                      );

  if (Buffer != NULL) {
    EfiCommonLibCopyMem (
      (VOID *)Buffer,
      (VOID *)mSimplePointerInstances,
      (mNoSimplePointerInstances * sizeof (*mSimplePointerInstances))
      );

    Index                   = mNoSimplePointerInstances;
    ++mNoSimplePointerInstances;
    Buffer[Index].Handle    = Handle;
    Buffer[Index].Interface = Interface;
    Buffer[Index].Installed = TRUE;

    gBS->FreePool ((VOID *)mSimplePointerInstances);

    mSimplePointerInstances = Buffer;
  }
}

// RemoveUninstalledInstances
/// 
///
/// @param 
///
/// @return 
/// @retval 
static
VOID
RemoveUninstalledInstances (
  IN OUT EFI_PROTOCOL_INSTANCE  **Instances,
  IN     UINTN                  *NoInstances,
  IN     EFI_GUID               *Protocol
  ) // sub_BC6
{
  EFI_STATUS            Status;

  UINTN                 NumberHandles;
  EFI_HANDLE            *Buffer;
  EFI_PROTOCOL_INSTANCE *Instance;
  UINTN                 Index;
  UINTN                 NoMatches;
  UINTN                 Index2;
  EFI_PROTOCOL_INSTANCE *InstanceBuffer;

  Status = gBS->LocateHandleBuffer (ByProtocol, Protocol, NULL, &NumberHandles, &Buffer);

  if (!EFI_ERROR (Status)) {
    if (*NoInstances > 0) {
      Instance  = *Instances;
      Index     = 0;
      NoMatches = 0;

      do {
        if (Instance->Installed) {
          for (Index2 = 0; Index2 < NumberHandles; ++Index2) {
            if (Instance->Handle == Buffer[Index2]) {
              ++NoMatches;
              break;
            }
          }

          if (NumberHandles == Index2) {
            Instance->Installed = FALSE;
          }
        }

        ++Index;
        ++Instance;
      } while (Index < *NoInstances);

      if (NoMatches != *NoInstances) {
        InstanceBuffer = (EFI_PROTOCOL_INSTANCE *)EfiLibAllocateZeroPool (NoMatches * sizeof (*InstanceBuffer));

        if (InstanceBuffer != NULL) {
          if (*NoInstances > 0) {
            Instance = *Instances;
            Index2   = 0;
            Index    = 0;

            do {
              if (Instance->Installed) {
                EfiCommonLibCopyMem ((VOID *)&InstanceBuffer[Index2], (VOID *)Instance, sizeof (*Instance));

                ++Index2;
              }

              ++Index;
              ++Instance;
            } while (Index < *NoInstances);
          }

          gBS->FreePool ((VOID *)*Instances);

          *Instances   = InstanceBuffer;
          *NoInstances = NoMatches;
        }
      }

      gBS->FreePool ((VOID *)Buffer);
    }
  } else {
    gBS->FreePool ((VOID *)*Instances);

    *Instances   = NULL;
    *NoInstances = 0;
  }
}

// SimplePointerInstallNotifyFunction
/// 
///
/// @param 
///
/// @return 
/// @retval 
VOID
EFIAPI
SimplePointerInstallNotifyFunction (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  ) // sub_A76
{
  EFI_STATUS                  Status;
  UINTN                       NumberHandles;
  EFI_HANDLE                  *Buffer;
  UINTN                       Index;
  EFI_SIMPLE_POINTER_PROTOCOL *Interface;

  if (Event != NULL) {
    Status = gBS->LocateHandleBuffer (
                    ByRegisterNotify,
                    NULL,
                    mSimplePointerInstallNotifyRegistration,
                    &NumberHandles,
                    &Buffer
                    );
  } else {
    Status = gBS->LocateHandleBuffer (
                    ByProtocol,
                    &gEfiSimplePointerProtocolGuid,
                    NULL,
                    &NumberHandles,
                    &Buffer
                    );
  }

  if (!EFI_ERROR (Status)) {
    if (NumberHandles > 0) {
      Index = 0;

      do {
        Status = gBS->HandleProtocol (Buffer[Index], &gEfiSimplePointerProtocolGuid, (VOID **)&Interface);

        if (!EFI_ERROR (Status)) {
          AddProtocolInstance (Buffer[Index], (VOID *)Interface);
        }

        ++Index;
      } while (Index < NumberHandles);
    }

    gBS->FreePool ((VOID *)Buffer);
  }
}

// EventCreateSimplePointerInstallNotifyEvent
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EventCreateSimplePointerInstallNotifyEvent (
  VOID
  )
{
  EFI_STATUS Status;

  Status = gBS->CreateEvent (
                  EFI_EVENT_NOTIFY_SIGNAL,
                  EFI_TPL_NOTIFY,
                  SimplePointerInstallNotifyFunction,
                  NULL,
                  &mSimplePointerInstallNotifyEvent
                  );

  if (!EFI_ERROR (Status)) {
    Status = gBS->RegisterProtocolNotify (
                    &gEfiSimplePointerProtocolGuid,
                    mSimplePointerInstallNotifyEvent,
                    &mSimplePointerInstallNotifyRegistration
                    );

    if (EFI_ERROR (Status)) {
      gBS->CloseEvent (mSimplePointerInstallNotifyEvent);

      mSimplePointerInstallNotifyEvent = NULL;
    } else {
      SimplePointerInstallNotifyFunction (NULL, NULL);
    }
  }

  return Status;
}

// GetScreenResolution
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
GetScreenResolution (
  VOID
  )
{
  EFI_STATUS                           Status;

  EFI_GRAPHICS_OUTPUT_PROTOCOL         *Interface;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
  UINT32                               VerticalResolution;

  Status = gBS->HandleProtocol (gST->ConsoleOutHandle, &gEfiGraphicsOutputProtocolGuid, (VOID **)&Interface);

  if (!EFI_ERROR (Status)) {
    Info                         = Interface->Mode->Info;
    mScreenResolution.Horizontal = Info->HorizontalResolution;
    VerticalResolution           = Info->VerticalResolution;
    mScreenResolution.Vertical   = VerticalResolution;

    if (VerticalResolution == 0) {
      Status = EFI_NOT_READY;
    } else {
      mScreenResolutionSet = TRUE;
    }
  }

  return Status;
}

// GetUiScaleData
/// 
///
/// @param 
///
/// @return 
/// @retval 
INT64
GetUiScaleData (
  IN INTN  Movement
  ) // sub_1FC0
{
  INTN  AbsoluteValue;
  UINT8 Value;
  INTN  Factor;

  //
  // INTN  Mask;
  //
  // Mask          = (Movement >> ((sizeof (Movement) * 8) - 1));
  // AbsoluteValue = ((Movement + Mask) ^ Mask);
  //

  AbsoluteValue = ((Movement < 0) ? -Movement : Movement);
  Value         = Log2 (AbsoluteValue);
  Factor        = 5;

  if (Value <= 3) {
    Factor = (Log2 (AbsoluteValue) + 1);
  }

  return (Movement * Factor * mUiScale);
}

// CreatePointerEventQueryInformation
/// 
///
/// @param 
///
/// @return 
/// @retval 
APPLE_EVENT_QUERY_INFORMATION *
CreatePointerEventQueryInformation (
  IN APPLE_EVENT_TYPE    EventType,
  IN APPLE_MODIFIER_MAP  Modifiers
  ) // sub_1F96
{
  UINT32           FinalEventType;
  APPLE_EVENT_DATA EventData;

  FinalEventType = APPLE_EVENT_TYPE_MOUSE_MOVED;

  if (!BIT_SET ((UINT8)EventType, APPLE_EVENT_TYPE_MOUSE_MOVED)) {
    FinalEventType = APPLE_ALL_MOUSE_EVENTS;

    if (BIT_SET ((UINT8)EventType, APPLE_CLICK_MOUSE_EVENTS)) {
      FinalEventType = APPLE_CLICK_MOUSE_EVENTS;
    }
  }

  EventData.PointerEventType = EventType;

  return EventCreateAppleEventQueryInfo (EventData, FinalEventType, &mCursorPosition, Modifiers);
}

// HandleButtonInteraction
/// 
///
/// @param 
///
/// @return 
/// @retval 
VOID
HandleButtonInteraction (
  IN     EFI_STATUS                  PointerStatus,
  IN OUT POINTER_BUTTON_INFORMATION  *PointerInfo,
  IN     APPLE_MODIFIER_MAP          Modifiers
  ) // sub_1DE6
{
  APPLE_EVENT_QUERY_INFORMATION *Information;
  INT32                         ValueMovement;
  INT32                         HorizontalMovement;
  INT32                         VerticalMovement;
  APPLE_EVENT_TYPE              EventType;

  if (!EFI_ERROR (PointerStatus)) {
    if (!PointerInfo->PreviousButton) {
      if (PointerInfo->CurrentButton) {
        PointerInfo->NoButtonPressed  = 0;
        PointerInfo->PreviousPosition = mCursorPosition;
        Information                   = CreatePointerEventQueryInformation (
                                          ((UINT32)PointerInfo->Button | APPLE_EVENT_TYPE_MOUSE_DOWN),
                                          Modifiers
                                          );

        if (Information != NULL) {
          EventAddEventQuery (Information);
        }
      }
    } else if (!PointerInfo->CurrentButton) {
      Information = CreatePointerEventQueryInformation (
                      (APPLE_EVENT_TYPE)(PointerInfo->Button | APPLE_EVENT_TYPE_MOUSE_UP),
                      Modifiers
                      );

      if (Information != NULL) {
        EventAddEventQuery (Information);
      }

      if (PointerInfo->NoButtonPressed <= MAXIMAL_CLICK_DURATION) {
        ValueMovement      = (PointerInfo->PreviousPosition.Horizontal - mCursorPosition.Horizontal);
        HorizontalMovement = -ValueMovement;

        if ((PointerInfo->PreviousPosition.Horizontal - mCursorPosition.Horizontal) >= 0) {
          HorizontalMovement = ValueMovement;
        }

        ValueMovement    = (PointerInfo->PreviousPosition.Horizontal - mCursorPosition.Horizontal);
        VerticalMovement = -ValueMovement;

        if ((PointerInfo->PreviousPosition.Horizontal - mCursorPosition.Horizontal) >= 0) {
          VerticalMovement = ValueMovement;
        }

        if ((HorizontalMovement < MINIMAL_MOVEMENT) && (VerticalMovement < MINIMAL_MOVEMENT)) {
          EventType = APPLE_EVENT_TYPE_MOUSE_CLICK;

          if ((PointerInfo->PreviousEventType == APPLE_EVENT_TYPE_MOUSE_CLICK)
           && (PointerInfo->Polls <= MINIMAL_DOUBLE_CLICK_SPEED)) {
            ValueMovement      = (PointerInfo->CurrentPosition.Horizontal - mCursorPosition.Horizontal);
            HorizontalMovement = -ValueMovement;

            if ((PointerInfo->CurrentPosition.Horizontal - mCursorPosition.Horizontal) >= 0) {
              HorizontalMovement = ValueMovement;
            }

            ValueMovement    = (PointerInfo->CurrentPosition.Horizontal - mCursorPosition.Horizontal);
            VerticalMovement = -ValueMovement;

            if ((PointerInfo->CurrentPosition.Horizontal - mCursorPosition.Horizontal) >= 0) {
              VerticalMovement = ValueMovement;
            }

            if ((HorizontalMovement < MINIMAL_MOVEMENT) && (VerticalMovement < MINIMAL_MOVEMENT)) {
              EventType = APPLE_EVENT_TYPE_MOUSE_DOUBLE_CLICK;
            }
          }

          Information = CreatePointerEventQueryInformation (((UINT32)PointerInfo->Button | EventType), Modifiers);

          if (Information != NULL) {
            EventAddEventQuery (Information);
          }

          if (PointerInfo->PreviousEventType == APPLE_EVENT_TYPE_MOUSE_DOUBLE_CLICK) {
            EventType = (
              (PointerInfo->Polls <= MINIMAL_DOUBLE_CLICK_SPEED)
              ? APPLE_EVENT_TYPE_MOUSE_CLICK
              : APPLE_EVENT_TYPE_MOUSE_DOUBLE_CLICK
              );
          }

          PointerInfo->PreviousEventType = (UINTN)EventType;
          PointerInfo->CurrentPosition   = mCursorPosition;
          PointerInfo->Polls             = 0;
        }
      }
    }

    PointerInfo->PreviousButton = PointerInfo->CurrentButton;
  }

  if (PointerInfo->PreviousButton && PointerInfo->CurrentButton) {
    ++PointerInfo->NoButtonPressed;
  }

  ++PointerInfo->Polls;
}

// SimplePointerPollNotifyFunction
/// 
///
/// @param 
///
/// @return 
/// @retval 
VOID
EFIAPI
SimplePointerPollNotifyFunction (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  ) // sub_1A9B
{
  APPLE_MODIFIER_MAP            Modifiers;
  UINTN                         Index;
  EFI_PROTOCOL_INSTANCE         *Instance;
  EFI_SIMPLE_POINTER_PROTOCOL   *Interface;
  EFI_STATUS                    Status;
  EFI_SIMPLE_POINTER_STATE      State;
  INT64                         UiScaleX;
  INT64                         UiScaleY;
  INT64                         MovementY;
  INT64                         MovementX;
  DIMENSION                     Resolution;
  INTN                          Result;
  APPLE_EVENT_QUERY_INFORMATION *Information;
  APPLE_EVENT_DATA              EventData;

  Modifiers = GetModifierStrokes ();

  RemoveUninstalledInstances (&mSimplePointerInstances, &mNoSimplePointerInstances, &gEfiSimplePointerProtocolGuid);

  if (mNoSimplePointerInstances > 0) {
    Index    = 0;
    Instance = mSimplePointerInstances;

    do {
      Interface = (EFI_SIMPLE_POINTER_PROTOCOL *)Instance->Interface;
      Status    = Interface->GetState (Interface, &State);

      if (!EFI_ERROR (Status)) {
        UiScaleX  = GetUiScaleData ((INTN)State.RelativeMovementX);
        UiScaleY  = GetUiScaleData ((INTN)State.RelativeMovementY);
        MovementY = DIV_S64_X64 (UiScaleY, (INT64)Interface->Mode->ResolutionY);
        MovementX = DIV_S64_X64 (UiScaleX, (INT64)Interface->Mode->ResolutionX);

        if ((State.RelativeMovementX != 0) && (MovementX == 0)) {
          MovementX = -1;

          if (State.RelativeMovementX > 0) {
            MovementX = 1;
          }
        }

        if ((State.RelativeMovementY != 0) && (MovementY == 0)) {
          MovementY = -1;

          if (State.RelativeMovementY > 0) {
            MovementY = 1;
          }
        }

        Resolution.Horizontal = ((INT32)(mCursorPosition.Horizontal + MovementX));
        Resolution.Vertical   = ((INT32)(mCursorPosition.Vertical + MovementY));

        if (Resolution.Horizontal > mScreenResolution.Horizontal) {
          Resolution.Horizontal = mScreenResolution.Horizontal;
        } else if (Resolution.Horizontal < 0) {
          Resolution.Horizontal = 0;
        }

        if (Resolution.Vertical > mScreenResolution.Vertical) {
          Resolution.Vertical = mScreenResolution.Vertical;
        } else if (Resolution.Vertical < 0) {
          Resolution.Vertical = 0;
        }

        Result = EfiCompareMem ((VOID *)&mCursorPosition, (VOID *)&Resolution, sizeof (Resolution));

        if (Result != 0) {
          EfiCommonLibCopyMem ((VOID *)&Resolution, (VOID *)&mCursorPosition, sizeof (mCursorPosition));

          mMouseMoved = TRUE;
        }

        mLeftButtonInformation.PreviousButton  = mLeftButtonInformation.CurrentButton;
        mLeftButtonInformation.CurrentButton   = State.LeftButton;
        mRightButtonInformation.PreviousButton = mLeftButtonInformation.CurrentButton;
        mRightButtonInformation.CurrentButton  = State.RightButton;
      }

      ++Index;
      ++Instance;
    } while (Index < mNoSimplePointerInstances);

    HandleButtonInteraction (Status, &mLeftButtonInformation, Modifiers);
    HandleButtonInteraction (Status, &mRightButtonInformation, Modifiers);

    if (EFI_ERROR (Status)) {
      if (Status == EFI_UNSUPPORTED) {
        CancelEvent (mSimplePointerPollEvent);

        mSimplePointerPollEvent = NULL;
      }
    } else if (mMouseMoved == TRUE) {
      mMouseMoved                = FALSE;
      EventData.PointerEventType = APPLE_EVENT_TYPE_MOUSE_MOVED;
      Information                = EventCreateAppleEventQueryInfo (
                                     EventData,
                                     APPLE_EVENT_TYPE_MOUSE_MOVED,
                                     &mCursorPosition,
                                     Modifiers
                                     );

      if (Information != NULL) {
        EventAddEventQuery (Information);
      }
    }
  }
}

// EventCreateSimplePointerPollEvent
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EventCreateSimplePointerPollEvent (
  VOID
  ) // sub_19A2
{
  EFI_STATUS                  Status;

  UINTN                       DataSize;
  UINTN                       Index;
  EFI_PROTOCOL_INSTANCE       *Instance;
  EFI_SIMPLE_POINTER_PROTOCOL *Interface;

  DataSize = sizeof (mUiScale);

  gRT->GetVariable (UI_SCALE_VARIABLE_NAME, &gAppleVendorNvramGuid, NULL, &DataSize, (VOID *)&mUiScale);
  RemoveUninstalledInstances (&mSimplePointerInstances, &mNoSimplePointerInstances, &gEfiSimplePointerProtocolGuid);

  if (mNoSimplePointerInstances > 0) {
    Index    = 0;
    Instance = mSimplePointerInstances;

    do {
      Interface = (EFI_SIMPLE_POINTER_PROTOCOL *)Instance->Interface;

      Interface->Reset (Interface, FALSE);

      ++Interface;
      ++Index;
    } while (Index < mNoSimplePointerInstances);
  }

  GetScreenResolution ();
  EfiCommonLibZeroMem (&mCursorPosition, sizeof (mCursorPosition));

  mSimplePointerPollEvent = CreateNotifyEvent (
                              SimplePointerPollNotifyFunction,
                              NULL,
                              EFI_TIMER_PERIOD_MILLISECONDS (2),
                              TRUE
                              );
  Status                  = EFI_OUT_OF_RESOURCES;


  if (mSimplePointerPollEvent != NULL) {
    mRightButtonInformation.CurrentButton  = FALSE;
    mLeftButtonInformation.CurrentButton   = FALSE;
    mRightButtonInformation.PreviousButton = FALSE;
    mLeftButtonInformation.PreviousButton  = FALSE;
    Status                                 = EFI_SUCCESS;
  }

  return Status;
}

// EventCancelSimplePointerPollEvent
/// 
///
/// @param 
///
/// @return 
/// @retval 
VOID
EventCancelSimplePointerPollEvent (
  VOID
  ) // sub_1CE4
{
  CancelEvent (mSimplePointerPollEvent);
}

// EventInternalSetCursorPosition
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EventInternalSetCursorPosition (
  IN DIMENSION *Position
  ) // sub_1D09
{
  EFI_STATUS Status;

  if (!mScreenResolutionSet) {
    Status = GetScreenResolution ();

    if (EFI_ERROR (Status)) {
      Status = EFI_NOT_READY;

      goto Return;
    }
  }

  Status = EFI_INVALID_PARAMETER;

  if ((Position->Horizontal < mScreenResolution.Horizontal) && (Position->Vertical < mScreenResolution.Vertical)) {
    mCursorPosition.Horizontal = Position->Horizontal;
    mCursorPosition.Vertical   = Position->Vertical;
    Status                    = EFI_SUCCESS;
  }

Return:
  return Status;
}
