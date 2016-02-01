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

#include <Library/AppleDriverLib.h>

#include "AppleEventImplInternal.h"

// mEventHandleList
EFI_LIST_ENTRY mEventHandleList = INITIALIZE_LIST_HEAD_VARIABLE (mEventHandleList);

// mNoEventHandles
STATIC UINTN mNoEventHandles = 0;

// EventRegisterHandlerImpl
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

  ASSERT (EventHandle != NULL);
  ASSERT (NotifyFunction != NULL);
  ASSERT (EventType != APPLE_EVENT_TYPE_NONE);

  Status = EFI_INVALID_PARAMETER;

  if ((EventHandle != NULL)
   && (NotifyFunction != NULL)
   && (EventType != APPLE_EVENT_TYPE_NONE)) {
    *EventHandle = NULL;

    EventRemoveUnregisteredEvents ();

    Status = EFI_SUCCESS;

    if (mNoEventHandles == 0) {
      Status = EventCreatePollEvents ();
      
      if (EFI_ERROR (Status)) {
        goto Return;
      }
    }

    Event  = EfiLibAllocatePool (sizeof (*Event));
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
  ASSERT_EFI_ERROR (Status);

  return Status;
}

// EventUnregisterHandlerImpl
EFI_STATUS
EFIAPI
EventUnregisterHandlerImpl (
  IN APPLE_EVENT_HANDLE  *EventHandle
  ) // sub_7DE
{
  EFI_STATUS         Status;

  APPLE_EVENT_HANDLE *Event;

  ASSERT (EventHandle != NULL);

  Status = EFI_INVALID_PARAMETER;
  Event  = APPLE_EVENT_HANDLE_FROM_LIST_ENTRY (&mEventHandleList);

  do {
    if ((Event == EventHandle)
     || ((UINTN)EventHandle == EFI_MAX_ADDRESS)) {
      Event->Registered = FALSE;
      --mNoEventHandles;
      Status            = EFI_SUCCESS;

      if ((UINTN)EventHandle != EFI_MAX_ADDRESS) {
        break;
      }
    }

    Event = APPLE_EVENT_HANDLE_FROM_LIST_ENTRY (
              GetNextNode (&mEventHandleList, &Event->This)
              );
  } while (!IsNull (&mEventHandleList, &Event->This));

  if (mNoEventHandles == 0) {
    EventCancelPollEvents ();
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}

// EventSetCursorPositionImpl 
EFI_STATUS
EFIAPI
EventSetCursorPositionImpl (
  IN DIMENSION  *Position
  ) // sub_84D
{
  return EventInternalSetCursorPosition (Position);
}

// EventSetEventNameImpl
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

  ASSERT (EventHandle != NULL);
  ASSERT (EventName != NULL);

  Status = EFI_INVALID_PARAMETER;

  if ((EventHandle != NULL) && (EventName != NULL)) {
    AllocationSize    = EfiAsciiStrSize (EventName);
    Memory            = EfiLibAllocateZeroPool (AllocationSize);
    EventHandle->Name = Memory;

    Status = EFI_OUT_OF_RESOURCES;

    if (EventHandle != NULL) {
      EfiAsciiStrCpy (Memory, EventName);

      Status = EFI_SUCCESS;
    }
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}

// EventIsCapsLockOnImpl
EFI_STATUS
EFIAPI
EventIsCapsLockOnImpl (
  IN OUT BOOLEAN  *CLockOn
  ) // sub_3582
{
  EFI_STATUS Status;

  ASSERT (CLockOn != NULL);

  Status = EFI_INVALID_PARAMETER;

  if (CLockOn != NULL) {
    *CLockOn = mCLockOn;
    Status   = EFI_SUCCESS;
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}
