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
EFI_LIST_ENTRY mHandleList = INITIALIZE_LIST_HEAD_VARIABLE (mHandleList);

// mNoEventHandles
STATIC UINTN mNoEventHandles = 0;

// EventRegisterHandlerImpl
EFI_STATUS
EFIAPI
EventRegisterHandler (
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

      InsertTailList (&mHandleList, &Event->This);

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
EventUnregisterHandler (
  IN APPLE_EVENT_HANDLE  *EventHandle
  ) // sub_7DE
{
  EFI_STATUS         Status;

  APPLE_EVENT_HANDLE *Event;

  ASSERT (EventHandle != NULL);

  Status = EFI_INVALID_PARAMETER;
  Event  = APPLE_EVENT_HANDLE_FROM_LIST_ENTRY (&mHandleList);

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
              GetNextNode (&mHandleList, &Event->This)
              );
  } while (!IsNull (&mHandleList, &Event->This));

  if (mNoEventHandles == 0) {
    EventCancelPollEvents ();
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}

// EventSetCursorPositionImpl
/** This function is used to change the position of the cursor on the screen.

  @param[in] Position  The position where to position the cursor.
  
  @retval EFI_INVALID_PARAMETER  Position is invalid.
**/
EFI_STATUS
EFIAPI
EventSetCursorPosition (
  IN DIMENSION  *Position
  ) // sub_84D
{
  return EventInternalSetCursorPosition (Position);
}

// EventSetEventNameImpl
/** This function is used to assign a name to an event.

  @param[in, out] EventHandle  
  @param[in]      EventName 

  @retval EFI_SUCCESS            The event name was assigned successfully.
  @retval EFI_INVALID_PARAMETER  EventHandle or EventName is NULL.
  @retval EFI_OUT_OF_RESOURCES   There are not enough resources to allocate the
                                 event name.
**/
EFI_STATUS
EFIAPI
EventSetEventName (
  IN OUT APPLE_EVENT_HANDLE  *EventHandle,
  IN     CHAR8               *EventName
  ) // sub_1483
{
  EFI_STATUS Status;

  UINTN      AllocationSize;
  CHAR8      *Name;

  ASSERT (EventHandle != NULL);
  ASSERT (EventName != NULL);

  Status = EFI_INVALID_PARAMETER;

  if ((EventHandle != NULL) && (EventName != NULL)) {
    AllocationSize    = EfiAsciiStrSize (EventName);
    Name              = EfiLibAllocateZeroPool (AllocationSize);
    EventHandle->Name = Name;

    Status = EFI_OUT_OF_RESOURCES;

    if (EventHandle != NULL) {
      EfiAsciiStrCpy (Name, EventName);

      Status = EFI_SUCCESS;
    }
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}

// EventIsCapsLockOnImpl
/** Retrieves the state of the CapsLock key.

  @param[in, out] CLockOn  This parameter indicates the state of the CapsLock
                           key.

  @retval EFI_SUCCESS            The CapsLock state was successfully returned
                                 in CLockOn.
  @retval EFI_INVALID_PARAMETER  CLockOn is NULL.
**/
EFI_STATUS
EFIAPI
EventIsCapsLockOn (
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
