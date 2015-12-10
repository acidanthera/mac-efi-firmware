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
/// @file      Protocol/AppleEventImpl/AppleEventImpl.c
///
///            
///
/// @author    Download-Fritz
/// @date      31/02/2015: Initial version
/// @copyright Copyright (C) 2005 - 2015 Apple Inc. All rights reserved.
///

#include <AppleEfi.h>

#include <EfiDriverLib.h>

#include <Guid/AppleNvram.h>

#include <IndustryStandard/AppleHid.h>

#include EFI_PROTOCOL_CONSUMER (ConsoleControl)
#include EFI_PROTOCOL_CONSUMER (GraphicsOutput)
#include EFI_PROTOCOL_CONSUMER (SimplePointer)
#include <Protocol/AppleKeyMapAggregator.h>
#include <Protocol/AppleEventImpl.h>

#include <Library/AppleEventLib.h>
#include <Library/AppleKeyMapLib.h>

// mEventHandleList
EFI_LIST_ENTRY mEventHandleList = INITIALIZE_LIST_HEAD_VARIABLE (mEventHandleList);

// mNoEventHandles
static UINTN mNoEventHandles = 0;

// EventRegisterHandlerImpl
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
EventRegisterHandlerImpl (
  IN  APPLE_EVENT_TYPE             EventType,
  IN  APPLE_EVENT_NOTIFY_FUNCTION  NotifyFunction,
  OUT APPLE_EVENT_HANDLE           **EventHandle,
  IN  VOID                         *NotifyContext
  ) // sub_70E
{
  EFI_STATUS         Status;

  APPLE_EVENT_HANDLE *Event;

  Status = EFI_INVALID_PARAMETER;

  if ((EventHandle != NULL) && (NotifyFunction != NULL) && (EventType != APPLE_EVENT_TYPE_NONE)) {
    *EventHandle = NULL;

    EventRemoveUnregisteredEvents ();

    Status = EFI_SUCCESS;

    if (mNoEventHandles == 0) {
      Status = EventCreatePollEvents ();
      
      if (EFI_ERROR (Status)) {
        goto Return;
      }
    }

    Event  = (APPLE_EVENT_HANDLE *)EfiLibAllocatePool (sizeof (*Event));
    Status = EFI_OUT_OF_RESOURCES;

    if (Event != NULL) {
      Event->Signature      = APPLE_EVENT_HANDLE_SIGNATURE;
      Event->Ready          = FALSE;
      Event->Registered     = TRUE;
      Event->EventType      = EventType;
      Event->NotifyFunction = NotifyFunction;
      Event->NotifyContext  = NotifyContext;
      Event->Name           = NULL;
      ++mNoEventHandles;

      InsertTailList (&mEventHandleList, &Event->This);

      *EventHandle = Event;
      Status       = EFI_SUCCESS;
    }
  }

Return:
  return Status;
}

// EventUnregisterHandlerImpl
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
EventUnregisterHandlerImpl (
  IN APPLE_EVENT_HANDLE  *EventHandle
  ) // sub_7DE
{
  EFI_STATUS         Status;

  APPLE_EVENT_HANDLE *Event;

  Status = EFI_INVALID_PARAMETER;
  Event  = APPLE_EVENT_HANDLE_FROM_LIST_ENTRY (&mEventHandleList);

  do {
    if ((Event == EventHandle) || ((UINTN)EventHandle == EFI_MAX_ADDRESS)) {
      Event->Registered = FALSE;
      --mNoEventHandles;
      Status            = EFI_SUCCESS;

      if ((UINTN)EventHandle != EFI_MAX_ADDRESS) {
        break;
      }
    }

    Event = APPLE_EVENT_HANDLE_FROM_LIST_ENTRY (GetNextNode (&mEventHandleList, &Event->This));
  } while (!IsNull (&mEventHandleList, &Event->This));

  if (mNoEventHandles == 0) {
    EventCancelPollEvents ();
  }

  return Status;
}

// EventSetCursorPositionImpl
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
EventSetCursorPositionImpl (
  IN DIMENSION  *Position
  ) // sub_84D
{
  return EventInternalSetCursorPosition (Position);
}

// EventSetEventNameImpl
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
EventSetEventNameImpl (
  IN OUT APPLE_EVENT_HANDLE  *EventHandle,
  IN     CHAR8               *EventName
  ) // sub_1483
{
  EFI_STATUS Status;

  UINTN      AllocationSize;
  CHAR8      *Memory;

  Status = EFI_INVALID_PARAMETER;

  if ((EventHandle != NULL) && (EventName != NULL)) {
    AllocationSize    = EfiAsciiStrSize (EventName);
    Memory            = (CHAR8 *)EfiLibAllocateZeroPool (AllocationSize);
    EventHandle->Name = Memory;

    Status = EFI_OUT_OF_RESOURCES;

    if (EventHandle != NULL) {
      EfiAsciiStrCpy (Memory, EventName);

      Status = EFI_SUCCESS;
    }
  }

  return Status;
}

// EventIsCapsLockActiveImpl
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
EventIsCapsLockActiveImpl (
  IN OUT BOOLEAN  *CapsLockActive
  ) // sub_3582
{
  EFI_STATUS Status;

  Status = EFI_INVALID_PARAMETER;

  if (CapsLockActive != NULL) {
    *CapsLockActive = mCLockOn;
    Status          = EFI_SUCCESS;
  }

  return Status;
}
