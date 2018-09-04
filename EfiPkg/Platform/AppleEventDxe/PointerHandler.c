#include <AppleMacEfi.h>

#include <Guid/AppleVariable.h>

#include <Protocol/GraphicsOutput.h>
#include <Protocol/UgaDraw.h>
#include <Protocol/SimplePointer.h>

#include <Library/AppleEventLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include "AppleEventInternal.h"

// UI_SCALE_VARIABLE_NAME
#define UI_SCALE_VARIABLE_NAME  L"UIScale"

#include <Library/BaseLib.h>

// POINTER_POLL_FREQUENCY
#define POINTER_POLL_FREQUENCY  20000

// MAXIMUM_DOUBLE_CLICK_SPEED
/// (EFI_TIMER_PERIOD_MILLISECONDS (748) / POINTER_POLL_FREQUENCY)
#define MAXIMUM_DOUBLE_CLICK_SPEED  374

// MAXIMAL_CLICK_DURATION
/// (EFI_TIMER_PERIOD_MILLISECONDS (148) / POINTER_POLL_FREQUENCY)
#define MAXIMAL_CLICK_DURATION  74

// MINIMAL_MOVEMENT
#define MINIMAL_MOVEMENT  5

// POINTER_BUTTON_INFORMATION
typedef struct {
  UINTN     Button;             ///< 
  UINTN     NumberOfStrokes;    ///< 
  UINTN     Polls;              ///< 
  UINTN     PreviousEventType;  ///< 
  BOOLEAN   PreviousButton;     ///< 
  BOOLEAN   CurrentButton;      ///< 
  DIMENSION PreviousPosition;   ///< 
  DIMENSION Position;           ///< 
} POINTER_BUTTON_INFORMATION;

// SIMPLE_POINTER_INSTANCE
typedef struct {
  EFI_HANDLE                  Handle;      ///<
  EFI_SIMPLE_POINTER_PROTOCOL *Interface;  ///<
  BOOLEAN                     Installed;   ///<
} SIMPLE_POINTER_INSTANCE;

// mSimplePointerInstallNotifyEvent
STATIC EFI_EVENT mSimplePointerInstallNotifyEvent = NULL;

// mSimplePointerInstallNotifyRegistration
STATIC VOID *mSimplePointerInstallNotifyRegistration = NULL;

// mSimplePointerInstances
STATIC SIMPLE_POINTER_INSTANCE *mPointerProtocols = NULL;

// mNoSimplePointerInstances
STATIC UINTN mNumberOfPointerProtocols = 0;

// mSimplePointerPollEvent
STATIC EFI_EVENT mSimplePointerPollEvent = NULL;

// mUiScale
STATIC UINT8 mUiScale = 1;

// mLeftButtonInfo
STATIC POINTER_BUTTON_INFORMATION mLeftButtonInfo = {
  APPLE_EVENT_TYPE_LEFT_BUTTON,
  0,
  0,
  0,
  FALSE,
  FALSE,
  { 0, 0 },
  { 0, 0 }
};

// mRightButtonInfo
STATIC POINTER_BUTTON_INFORMATION mRightButtonInfo = {
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
STATIC DIMENSION mCursorPosition;

// mMouseMoved
STATIC BOOLEAN mMouseMoved;

// mScreenResolutionSet
STATIC BOOLEAN mScreenResolutionSet;

// mScreenResolution
STATIC DIMENSION mResolution;

// InternalRegisterSimplePointerInterface
STATIC
VOID
InternalRegisterSimplePointerInterface (
  IN EFI_HANDLE                   Handle,
  IN EFI_SIMPLE_POINTER_PROTOCOL  *SimplePointer
  )
{
  SIMPLE_POINTER_INSTANCE *Instance;
  UINTN                   Index;

  Instance = AllocateZeroPool (
               (mNumberOfPointerProtocols + 1) * sizeof (*Instance)
               );

  if (Instance != NULL) {
    CopyMem (
      (VOID *)Instance,
      (VOID *)mPointerProtocols,
      (mNumberOfPointerProtocols * sizeof (*mPointerProtocols))
      );

    Index = mNumberOfPointerProtocols;

    ++mNumberOfPointerProtocols;

    Instance[Index].Handle    = Handle;
    Instance[Index].Interface = SimplePointer;
    Instance[Index].Installed = TRUE;

    if (mPointerProtocols != NULL) {
      FreePool ((VOID *)mPointerProtocols);
    }

    mPointerProtocols = Instance;
  }
}

// EventSimplePointerDesctructor
VOID
EventSimplePointerDesctructor (
  VOID
  )
{
  if (mPointerProtocols != NULL) {
    FreePool ((VOID *)mPointerProtocols);
  }
}

// InternalRemoveUninstalledInstances
STATIC
VOID
InternalRemoveUninstalledInstances (
  IN OUT SIMPLE_POINTER_INSTANCE  **Instances,
  IN     UINTN                    *NumberOfInstances,
  IN     EFI_GUID                 *Protocol
  )
{
  EFI_STATUS              Status;

  UINTN                   NumberHandles;
  EFI_HANDLE              *Buffer;
  SIMPLE_POINTER_INSTANCE *Instance;
  UINTN                   Index;
  UINTN                   NumberOfMatches;
  UINTN                   Index2;
  SIMPLE_POINTER_INSTANCE *InstanceBuffer;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  Protocol,
                  NULL,
                  &NumberHandles,
                  &Buffer
                  );

  if (!EFI_ERROR (Status)) {
    if (*NumberOfInstances > 0) {
      Instance        = *Instances;
      Index           = 0;
      NumberOfMatches = 0;

      do {
        if (Instance->Installed) {
          for (Index2 = 0; Index2 < NumberHandles; ++Index2) {
            if (Instance->Handle == Buffer[Index2]) {
              ++NumberOfMatches;
              break;
            }
          }

          if (NumberHandles == Index2) {
            Instance->Installed = FALSE;
          }
        }

        ++Index;
        ++Instance;
      } while (Index < *NumberOfInstances);

      if (NumberOfMatches != *NumberOfInstances) {
        InstanceBuffer = AllocateZeroPool (
                           NumberOfMatches * sizeof (*InstanceBuffer)
                           );

        if (InstanceBuffer != NULL) {
          if (*NumberOfInstances > 0) {
            Instance = *Instances;
            Index2   = 0;
            Index    = 0;

            do {
              if (Instance->Installed) {
                CopyMem (
                  (VOID *)&InstanceBuffer[Index2],
                  (VOID *)Instance, sizeof (*Instance)
                  );

                ++Index2;
              }

              ++Index;
              ++Instance;
            } while (Index < *NumberOfInstances);
          }

          FreePool ((VOID *)*Instances);

          *Instances         = InstanceBuffer;
          *NumberOfInstances = NumberOfMatches;
        }
      }
    }

    FreePool ((VOID *)Buffer);
  } else {
    FreePool ((VOID *)*Instances);

    *Instances         = NULL;
    *NumberOfInstances = 0;
  }
}

// InternalSimplePointerInstallNotifyFunction
STATIC
VOID
EFIAPI
InternalSimplePointerInstallNotifyFunction (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS                  Status;
  UINTN                       NumberHandles;
  EFI_HANDLE                  *Buffer;
  UINTN                       Index;
  EFI_SIMPLE_POINTER_PROTOCOL *SimplePointer;

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
        Status = gBS->HandleProtocol (
                        Buffer[Index],
                        &gEfiSimplePointerProtocolGuid,
                        (VOID **)&SimplePointer
                        );

        if (!EFI_ERROR (Status)) {
          InternalRegisterSimplePointerInterface (Buffer[Index], SimplePointer);
        }

        ++Index;
      } while (Index < NumberHandles);
    }

    FreePool ((VOID *)Buffer);
  }
}

// EventCreateSimplePointerInstallNotifyEvent
EFI_STATUS
EventCreateSimplePointerInstallNotifyEvent (
  VOID
  )
{
  EFI_STATUS Status;

  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  InternalSimplePointerInstallNotifyFunction,
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
      InternalSimplePointerInstallNotifyFunction (NULL, NULL);
    }
  }

  return Status;
}

// EventCreateSimplePointerInstallNotifyEvent
VOID
EventCloseSimplePointerInstallNotifyEvent (
  VOID
  )
{
  if (mSimplePointerInstallNotifyEvent != NULL) {
    gBS->CloseEvent (mSimplePointerInstallNotifyEvent);
  }
}

// InternalGetScreenResolution
STATIC
EFI_STATUS
InternalGetScreenResolution (
  VOID
  )
{
  EFI_STATUS                           Status;

  EFI_GRAPHICS_OUTPUT_PROTOCOL         *GraphicsOutput;
  EFI_UGA_DRAW_PROTOCOL                *UgaDraw;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
  UINT32                               HorizontalResolution;
  UINT32                               VerticalResolution;
  UINT32                               ColorDepth;
  UINT32                               RefreshRate;

  Status = gBS->HandleProtocol (
                  gST->ConsoleOutHandle,
                  &gEfiGraphicsOutputProtocolGuid,
                  (VOID **)&GraphicsOutput
                  );

  if (Status == EFI_UNSUPPORTED) {
    //
    // Fallback to default resolution.
    //
    mResolution.Horizontal = 800;
    mResolution.Vertical   = 600;

    Status = gBS->HandleProtocol (
                    gST->ConsoleOutHandle,
                    &gEfiUgaDrawProtocolGuid,
                    (VOID **)&UgaDraw
                    );

    if (!EFI_ERROR (Status)) {
      Status = UgaDraw->GetMode (
                    &HorizontalResolution,
                    &VerticalResolution,
                    &ColorDepth,
                    &RefreshRate);
    }

    if (!EFI_ERROR (Status)) {
      mResolution.Horizontal = HorizontalResolution;
      mResolution.Vertical   = VerticalResolution;
      // BUG: mScreenResolutionSet is not set to TRUE even when a legit resolution is read.
    }

    Status = EFI_SUCCESS;
  } else if (!EFI_ERROR (Status)) {
    Info                   = GraphicsOutput->Mode->Info;
    mResolution.Horizontal = Info->HorizontalResolution;
    mResolution.Vertical   = Info->VerticalResolution;

    if (Info->VerticalResolution > 0) {
      mScreenResolutionSet = TRUE;
    } else {
      Status = EFI_NOT_READY;
    }
  }

  return Status;
}

// InternalGetUiScaleData
STATIC
INT64
InternalGetUiScaleData (
  IN INT64  Movement
  )
{
  INT64 AbsoluteValue;
  INTN  Value;
  INT64 Factor;

  AbsoluteValue = ((Movement < 0) ? -Movement : Movement);
  Value         = HighBitSet64 (AbsoluteValue);
  Factor        = 5;

  if (Value <= 3) {
    Factor = (HighBitSet64 (AbsoluteValue) + 1);
  }

  return (INT64)(MultS64x64 (
                   (INT64)(INT32)mUiScale,
                   MultS64x64 (Movement, Factor)
                   ));
}

// InternalCreatePointerEventQueueInformation
STATIC
APPLE_EVENT_INFORMATION *
InternalCreatePointerEventQueueInformation (
  IN APPLE_EVENT_TYPE    EventType,
  IN APPLE_MODIFIER_MAP  Modifiers
  )
{
  UINT32           FinalEventType;
  APPLE_EVENT_DATA EventData;

  FinalEventType = APPLE_EVENT_TYPE_MOUSE_MOVED;

  if ((EventType & APPLE_EVENT_TYPE_MOUSE_MOVED) == 0) {
    FinalEventType = APPLE_ALL_MOUSE_EVENTS;

    if ((EventType & APPLE_CLICK_MOUSE_EVENTS) != 0) {
      FinalEventType = APPLE_CLICK_MOUSE_EVENTS;
    }
  }

  EventData.PointerEventType = EventType;

  return EventCreateAppleEventQueueInfo (
           EventData,
           FinalEventType,
           &mCursorPosition,
           Modifiers
           );
}

// InternalHandleButtonInteraction
STATIC
VOID
InternalHandleButtonInteraction (
  IN     EFI_STATUS                  PointerStatus,
  IN OUT POINTER_BUTTON_INFORMATION  *Pointer,
  IN     APPLE_MODIFIER_MAP          Modifiers
  )
{
  APPLE_EVENT_INFORMATION *Information;
  INT32                   ValueMovement;
  INT32                   HorizontalMovement;
  INT32                   VerticalMovement;
  APPLE_EVENT_TYPE        EventType;

  if (!EFI_ERROR (PointerStatus)) {
    if (!Pointer->PreviousButton) {
      if (Pointer->CurrentButton) {
        Pointer->NumberOfStrokes  = 0;
        Pointer->PreviousPosition = mCursorPosition;

        Information = InternalCreatePointerEventQueueInformation (
                        (APPLE_EVENT_TYPE)(
                          Pointer->Button | APPLE_EVENT_TYPE_MOUSE_DOWN
                          ),
                        Modifiers
                        );

        if (Information != NULL) {
          EventAddEventToQueue (Information);
        }
      }
    } else if (!Pointer->CurrentButton) {
      Information = InternalCreatePointerEventQueueInformation (
                      (APPLE_EVENT_TYPE)(
                        Pointer->Button | APPLE_EVENT_TYPE_MOUSE_UP
                        ),
                      Modifiers
                      );

      if (Information != NULL) {
        EventAddEventToQueue (Information);
      }

      if (Pointer->NumberOfStrokes <= MAXIMAL_CLICK_DURATION) {
        HorizontalMovement = ABS(Pointer->PreviousPosition.Horizontal - mCursorPosition.Horizontal);
        VerticalMovement   = ABS(Pointer->PreviousPosition.Vertical - mCursorPosition.Vertical);

        if ((HorizontalMovement <= MINIMAL_MOVEMENT)
         && (VerticalMovement <= MINIMAL_MOVEMENT)) {
          EventType = APPLE_EVENT_TYPE_MOUSE_CLICK;

          if ((Pointer->PreviousEventType == APPLE_EVENT_TYPE_MOUSE_CLICK)
           && (Pointer->Polls <= MAXIMUM_DOUBLE_CLICK_SPEED)) {
            HorizontalMovement = ABS(Pointer->Position.Horizontal - mCursorPosition.Horizontal);
            VerticalMovement   = ABS(Pointer->Position.Vertical - mCursorPosition.Vertical);

            if ((HorizontalMovement <= MINIMAL_MOVEMENT)
             && (VerticalMovement <= MINIMAL_MOVEMENT)) {
              EventType = APPLE_EVENT_TYPE_MOUSE_DOUBLE_CLICK;
            }
          }

          Information = InternalCreatePointerEventQueueInformation (
                          ((UINT32)Pointer->Button | EventType),
                          Modifiers
                          );

          if (Information != NULL) {
            EventAddEventToQueue (Information);
          }

          if (Pointer->PreviousEventType == APPLE_EVENT_TYPE_MOUSE_DOUBLE_CLICK) {
            EventType = ((Pointer->Polls <= MAXIMUM_DOUBLE_CLICK_SPEED)
                            ? APPLE_EVENT_TYPE_MOUSE_DOUBLE_CLICK
                            : APPLE_EVENT_TYPE_MOUSE_CLICK);
          }

          Pointer->PreviousEventType = (UINTN)EventType;
          Pointer->Position          = mCursorPosition;
          Pointer->Polls             = 0;
        }
      }
    }

    Pointer->PreviousButton = Pointer->CurrentButton;
  }

  if (Pointer->PreviousButton && Pointer->CurrentButton) {
    ++Pointer->NumberOfStrokes;
  }

  ++Pointer->Polls;
}

// InternalSimplePointerPollNotifyFunction
STATIC
VOID
EFIAPI
InternalSimplePointerPollNotifyFunction (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  APPLE_MODIFIER_MAP          Modifiers;
  UINTN                       Index;
  SIMPLE_POINTER_INSTANCE     *Instance;
  EFI_SIMPLE_POINTER_PROTOCOL *SimplePointer;
  EFI_STATUS                  Status;
  EFI_SIMPLE_POINTER_STATE    State;
  INT64                       UiScaleX;
  INT64                       UiScaleY;
  INT64                       MovementY;
  INT64                       MovementX;
  DIMENSION                   Resolution;
  INTN                        Result;
  APPLE_EVENT_INFORMATION     *Information;
  APPLE_EVENT_DATA            EventData;

  Modifiers = InternalGetModifierStrokes ();

  InternalRemoveUninstalledInstances (
    &mPointerProtocols,
    &mNumberOfPointerProtocols,
    &gEfiSimplePointerProtocolGuid
    );

  if (mNumberOfPointerProtocols > 0) {
    Index    = 0;
    Instance = mPointerProtocols;

    do {
      SimplePointer = (EFI_SIMPLE_POINTER_PROTOCOL *)Instance->Interface;
      Status        = SimplePointer->GetState (SimplePointer, &State);

      if (!EFI_ERROR (Status)) {
        UiScaleX = InternalGetUiScaleData ((INT64)State.RelativeMovementX);
        UiScaleY = InternalGetUiScaleData ((INT64)State.RelativeMovementY);

        MovementY = DivS64x64Remainder (
                      UiScaleY,
                      (INT64)SimplePointer->Mode->ResolutionY,
                      NULL
                      );

        MovementX = DivS64x64Remainder (
                      UiScaleX,
                      (INT64)SimplePointer->Mode->ResolutionX,
                      NULL
                      );

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

        Resolution.Horizontal = (INT32)(
                                  mCursorPosition.Horizontal + MovementX
                                  );

        Resolution.Vertical   = (INT32)(
                                  mCursorPosition.Vertical + MovementY
                                  );

        if (Resolution.Horizontal > mResolution.Horizontal) {
          Resolution.Horizontal = mResolution.Horizontal;
        } else if (Resolution.Horizontal < 0) {
          Resolution.Horizontal = 0;
        }

        if (Resolution.Vertical > mResolution.Vertical) {
          Resolution.Vertical = mResolution.Vertical;
        } else if (Resolution.Vertical < 0) {
          Resolution.Vertical = 0;
        }

        Result = CompareMem (
                   (VOID *)&mCursorPosition,
                   (VOID *)&Resolution,
                   sizeof (Resolution)
                   );

        if (Result != 0) {
          CopyMem (
            (VOID *)&Resolution,
            (VOID *)&mCursorPosition,
            sizeof (mCursorPosition)
            );

          mMouseMoved = TRUE;
        }

        mLeftButtonInfo.PreviousButton  = mLeftButtonInfo.CurrentButton;
        mLeftButtonInfo.CurrentButton   = State.LeftButton;
        mRightButtonInfo.PreviousButton = mLeftButtonInfo.CurrentButton;
        mRightButtonInfo.CurrentButton  = State.RightButton;
      }

      ++Index;
      ++Instance;
    } while (Index < mNumberOfPointerProtocols);

    InternalHandleButtonInteraction (Status, &mLeftButtonInfo, Modifiers);
    InternalHandleButtonInteraction (Status, &mRightButtonInfo, Modifiers);

    if (EFI_ERROR (Status)) {
      if (Status == EFI_UNSUPPORTED) {
        EventLibCancelEvent (mSimplePointerPollEvent);

        mSimplePointerPollEvent = NULL;
      }
    } else if (mMouseMoved == TRUE) {
      mMouseMoved = FALSE;

      EventData.PointerEventType = APPLE_EVENT_TYPE_MOUSE_MOVED;
      Information                = EventCreateAppleEventQueueInfo (
                                     EventData,
                                     APPLE_EVENT_TYPE_MOUSE_MOVED,
                                     &mCursorPosition,
                                     Modifiers
                                     );

      if (Information != NULL) {
        EventAddEventToQueue (Information);
      }
    }
  }
}

// EventCreateSimplePointerPollEvent
EFI_STATUS
EventCreateSimplePointerPollEvent (
  VOID
  )
{
  EFI_STATUS                  Status;

  UINTN                       DataSize;
  UINTN                       Index;
  SIMPLE_POINTER_INSTANCE       *Instance;
  EFI_SIMPLE_POINTER_PROTOCOL *SimplePointer;

  DataSize = sizeof (mUiScale);

  gRT->GetVariable (
         UI_SCALE_VARIABLE_NAME,
         &gAppleVendorVariableGuid,
         NULL,
         &DataSize,
         (VOID *)&mUiScale
         );

  InternalRemoveUninstalledInstances (
    &mPointerProtocols,
    &mNumberOfPointerProtocols,
    &gEfiSimplePointerProtocolGuid
    );

  if (mNumberOfPointerProtocols > 0) {
    Index    = 0;
    Instance = mPointerProtocols;

    do {
      SimplePointer = (EFI_SIMPLE_POINTER_PROTOCOL *)Instance->Interface;

      SimplePointer->Reset (SimplePointer, FALSE);

      ++Instance;
      ++Index;
    } while (Index < mNumberOfPointerProtocols);
  }

  InternalGetScreenResolution ();
  ZeroMem (&mCursorPosition, sizeof (mCursorPosition));

  mSimplePointerPollEvent = EventLibCreateNotifyTimerEvent (
                              InternalSimplePointerPollNotifyFunction,
                              NULL,
                              POINTER_POLL_FREQUENCY,
                              TRUE
                              );

  Status = EFI_OUT_OF_RESOURCES;


  if (mSimplePointerPollEvent != NULL) {
    mRightButtonInfo.CurrentButton  = FALSE;
    mLeftButtonInfo.CurrentButton   = FALSE;
    mRightButtonInfo.PreviousButton = FALSE;
    mLeftButtonInfo.PreviousButton  = FALSE;

    Status = EFI_SUCCESS;
  }

  return Status;
}

// EventCancelSimplePointerPollEvent
VOID
EventCancelSimplePointerPollEvent (
  VOID
  )
{
  EventLibCancelEvent (mSimplePointerPollEvent);

  mSimplePointerPollEvent = NULL;
}

// EventSetCursorPositionImpl 
EFI_STATUS
EventSetCursorPositionImpl (
  IN DIMENSION  *Position
  )
{
  EFI_STATUS Status;

  if (!mScreenResolutionSet) {
    Status = InternalGetScreenResolution ();

    if (EFI_ERROR (Status)) {
      Status = EFI_NOT_READY;
      goto Done;
    }
  }

  Status = EFI_INVALID_PARAMETER;

  // BUG: Does not check for negative position

  if ((Position->Horizontal < mResolution.Horizontal)
   && (Position->Vertical < mResolution.Vertical)) {
    mCursorPosition.Horizontal = Position->Horizontal;
    mCursorPosition.Vertical   = Position->Vertical;

    Status = EFI_SUCCESS;
  }

Done:
  return Status;
}
