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
#include <EfiDebug.h>

#include <IndustryStandard/AppleHid.h>

#include EFI_PROTOCOL_CONSUMER (LoadedImage)

#include <Library/AppleDriverLib.h>

#include "AppleEventImplInternal.h"

// mLock
STATIC EFI_LOCK mLock;

// mQueryEvent
STATIC EFI_EVENT mQueryEvent = NULL;

// mQueryEventCreated
STATIC BOOLEAN mQueryEventCreated = FALSE;

// mEventQueryList
STATIC EFI_LIST mQueryList = INITIALIZE_LIST_HEAD_VARIABLE (mQueryList);

// EventImplUnload
EFI_STATUS
EFIAPI
EventImplUnload (
  IN EFI_HANDLE  ImageHandle
  ) // sub_945
{
  EFI_STATUS Status;

  if (mSimplePointerInstallNotifyEvent != NULL) {
    gBS->CloseEvent (mSimplePointerInstallNotifyEvent);
  }

  EventSignalAndCloseQueryEvent ();
  EventUnregisterHandlers ();
  EventCancelPollEvents ();

  Status = gBS->UninstallProtocolInterface (
                  ImageHandle,
                  &gAppleEventProtocolGuid,
                  (VOID *)&gAppleEventProtocol
                  );

  ASSERT_EFI_ERROR (Status);

  return Status;
}

// EventImplConstructor
EFI_STATUS
EFIAPI
EventImplConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                Status;

  EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;

  AppleInitializeDriverLib (ImageHandle, SystemTable);
  DxeInitializeDriverLib (ImageHandle, SystemTable);

  ASSERT_PROTOCOL_ALREADY_INSTALLED (NULL, &gAppleEventProtocolGuid);

  LoadedImage = NULL;
  Status      = gBS->InstallProtocolInterface (
                       &ImageHandle,
                       &gAppleEventProtocolGuid,
                       EFI_NATIVE_INTERFACE,
                       (VOID *)&gAppleEventProtocol
                       );

  if (!EFI_ERROR (Status)) {
    Status = gBS->HandleProtocol (
                    ImageHandle,
                    &gEfiLoadedImageProtocolGuid,
                    (VOID **)&LoadedImage
                    );

    if (!EFI_ERROR (Status)) {
      LoadedImage->Unload = EventImplUnload;

      EventCreateQueryEvent ();

      Status = EventCreateSimplePointerInstallNotifyEvent ();

      if (!EFI_ERROR (Status)) {
        goto Return;
      }
    }

    EventImplUnload (ImageHandle);
  }

Return:
  ASSERT_EFI_ERROR (Status);

  return Status;
}

// EventRemoveUnregisteredEvents
VOID
EventRemoveUnregisteredEvents (
  VOID
  ) // sub_B34
{
  EFI_LIST_ENTRY             *EventHandleEntry;
  EFI_LIST_ENTRY             *NextEventHandleEntry;
  APPLE_EVENT_HANDLE_PRIVATE *EventHandle;

  EventHandleEntry = GetFirstNode (&mEventHandleList);

  if (!IsListEmpty (&mEventHandleList)) {
    do {
      NextEventHandleEntry = GetNextNode (&mEventHandleList, EventHandleEntry);

      EventHandle = APPLE_EVENT_HANDLE_PRIVATE_FROM_LIST_ENTRY (
                      EventHandleEntry
                      );

      if (!EventHandle->Registered) {
        if (EventHandle->Name != NULL) {
          gBS->FreePool ((VOID *)EventHandle->Name);
        }

        RemoveEntryList (&EventHandle->This);
        gBS->FreePool ((VOID *)EventHandle);
      }

      EventHandleEntry = NextEventHandleEntry;
    } while (!IsNull (&mEventHandleList, NextEventHandleEntry));
  }
}

// EventUnregisterHandlers
VOID
EventUnregisterHandlers (
  VOID
  ) // sub_A31
{
  if (mPointerProtocols != NULL) {
    gBS->FreePool ((VOID *)mPointerProtocols);
  }

  // BUG: Memory leak, currently active events are not freed.

  EventRemoveUnregisteredEvents ();

  if (!IsListEmpty (&mEventHandleList)) {
    EventUnregisterHandler ((APPLE_EVENT_HANDLE_PRIVATE *)EFI_MAX_ADDRESS);
  }
}

// EventCreatePollEvents
EFI_STATUS
EventCreatePollEvents (
  VOID
  ) // sub_F1B
{
  EFI_STATUS Status;

  ASSERT (mNumberOfEventHandles == 0);

  Status = EventCreateSimplePointerPollEvent ();

  if (!EFI_ERROR (Status)) {
    Status = EventCreateKeyStrokePollEvent ();

    if (EFI_ERROR (Status)) {
      EventCancelSimplePointerPollEvent ();
    }
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}

// EventCancelPollEvents
VOID
EventCancelPollEvents (
  VOID
  ) // sub_F4D
{
  EventCancelSimplePointerPollEvent ();
  EventCancelKeyStrokePollEvent ();
}

// EventCreateAppleEventQueryInfo
APPLE_EVENT_QUERY_INFORMATION *
EventCreateAppleEventQueryInfo (
  IN APPLE_EVENT_DATA    EventData,
  IN APPLE_EVENT_TYPE    EventType,
  IN DIMENSION           *PointerPosition,
  IN APPLE_MODIFIER_MAP  Modifiers
  ) // sub_F64
{
  APPLE_EVENT_QUERY_INFORMATION *QueryInfo;

  EFI_TIME                      CreationTime;

  ASSERT (EventData.Raw != 0);
  ASSERT (EventType != APPLE_EVENT_TYPE_NONE);

  QueryInfo = EfiLibAllocateZeroPool (sizeof (*QueryInfo));

  if (QueryInfo != NULL) {
    gRT->GetTime (&CreationTime, NULL);

    QueryInfo->EventType           = EventType;
    QueryInfo->EventData           = EventData;
    QueryInfo->Modifiers           = Modifiers;
    QueryInfo->CreationTime.Year   = CreationTime.Year;
    QueryInfo->CreationTime.Month  = CreationTime.Month;
    QueryInfo->CreationTime.Day    = CreationTime.Day;
    QueryInfo->CreationTime.Hour   = CreationTime.Hour;
    QueryInfo->CreationTime.Minute = CreationTime.Minute;
    QueryInfo->CreationTime.Second = CreationTime.Second;
    QueryInfo->CreationTime.Pad1   = CreationTime.Pad1;

    if (PointerPosition != NULL) {
      EfiCopyMem (
        (VOID *)&QueryInfo->PointerPosition,
        (VOID *)PointerPosition,
        sizeof (*PointerPosition)
        );
    }
  }

  ASSERT (QueryInfo != NULL);

  return QueryInfo;
}

// FlagAllEventsReady
VOID
FlagAllEventsReady (
  VOID
  ) // sub_B96
{
  EFI_LIST_ENTRY             *EventHandleEntry;
  APPLE_EVENT_HANDLE_PRIVATE *EventHandle;

  EventHandleEntry = GetFirstNode (&mEventHandleList);

  if (!IsListEmpty (&mEventHandleList)) {
    do {
      EventHandle = APPLE_EVENT_HANDLE_PRIVATE_FROM_LIST_ENTRY (
                      EventHandleEntry
                      );

      EventHandle->Ready = TRUE;

      EventHandleEntry = GetNextNode (&mEventHandleList, EventHandleEntry);
    } while (!IsNull (&mEventHandleList, EventHandleEntry));
  }
}

// QueryEventNotifyFunction
VOID
EFIAPI
QueryEventNotifyFunction (
  IN EFI_EVENT  Event,
  IN VOID       *Context
) // sub_D56
{
  EFI_STATUS                 Status;

  EFI_LIST_ENTRY             *EventQueryEntry;
  APPLE_EVENT_QUERY          *EventQuery;
  EFI_LIST_ENTRY             *EventHandleEntry;
  APPLE_EVENT_HANDLE_PRIVATE *EventHandle;
  EFI_LIST_ENTRY             *NextEventQueryEntry;

  ASSERT (Event != NULL);

  if (mQueryEventCreated) {
    do {
      Status = EfiAcquireLockOrFail (&mLock);
    } while (Status != EFI_SUCCESS);

    FlagAllEventsReady ();

    EventQueryEntry = GetFirstNode (&mQueryList);

    while (!IsNull (&mQueryList, EventQueryEntry)) {
      EventQuery = APPLE_EVENT_QUERY_FROM_LIST_ENTRY (EventQueryEntry);

      EventHandleEntry = GetFirstNode (&mEventHandleList);

      while (!IsNull (&mEventHandleList, EventHandleEntry)) {
        EventHandle = APPLE_EVENT_HANDLE_PRIVATE_FROM_LIST_ENTRY (
                        EventHandleEntry
                        );

        if (EventHandle->Registered
         && EventHandle->Ready
         && ((EventQuery->Information->EventType & EventHandle->EventType) != 0)
         && (EventHandle->NotifyFunction != NULL)) {
          EventHandle->NotifyFunction (
                         EventQuery->Information,
                         EventHandle->NotifyContext
                         );
        }

        EventHandleEntry = GetNextNode (&mEventHandleList, EventHandleEntry);
      }

      if (((EventQuery->Information->EventType & APPLE_ALL_KEYBOARD_EVENTS) != 0)
        && (EventQuery->Information->EventData.KeyData != NULL)) {
        gBS->FreePool (
               (VOID *)EventQuery->Information->EventData.KeyData
               );
      }

      NextEventQueryEntry = GetNextNode (&mQueryList, &EventQuery->This);

      RemoveEntryList (EventQueryEntry);
      gBS->FreePool ((VOID *)EventQuery->Information);
      gBS->FreePool ((VOID *)EventQuery);

      EventQueryEntry = NextEventQueryEntry;
    }

    EventRemoveUnregisteredEvents ();
    EfiReleaseLock (&mLock);
  }
}

// EventCreateQueryEvent
VOID
EventCreateQueryEvent (
  VOID
  ) // sub_D01
{
  EFI_STATUS Status;

  EfiInitializeLock (&mLock, EFI_TPL_NOTIFY);

  Status = gBS->CreateEvent (
                  EFI_EVENT_NOTIFY_SIGNAL,
                  EFI_TPL_NOTIFY,
                  QueryEventNotifyFunction,
                  NULL,
                  &mQueryEvent
                  );

  ASSERT_EFI_ERROR (Status);

  if (!EFI_ERROR (Status)) {
    mQueryEventCreated = TRUE;
  }
}

// EventSignalAndCloseQueryEvent
VOID
EventSignalAndCloseQueryEvent (
  VOID
  ) // sub_E54
{
  ASSERT (mQueryEventCreated);
  ASSERT (mQueryEvent != NULL);

  gBS->SignalEvent (mQueryEvent);

  if (mQueryEventCreated && (mQueryEvent != NULL)) {
    gBS->CloseEvent (mQueryEvent);
  }
}

// EventAddEventQuery
VOID
EventAddEventQuery (
  IN APPLE_EVENT_QUERY_INFORMATION  *Information
  ) // sub_E98
{
  EFI_STATUS        Status;

  APPLE_EVENT_QUERY *EventQuery;

  ASSERT (mQueryEventCreated);
  ASSERT (mQueryEvent != NULL);

  if (mQueryEventCreated) {
    do {
      Status = EfiAcquireLockOrFail (&mLock);
    } while (Status != EFI_SUCCESS);

    EventQuery = EfiLibAllocatePool (sizeof (*EventQuery));

    if (EventQuery != NULL) {
      EventQuery->Signature   = APPLE_EVENT_QUERY_SIGNATURE;
      EventQuery->Information = Information;

      InsertTailList (&mQueryList, &EventQuery->This);
    }

    EfiReleaseLock (&mLock);
    gBS->SignalEvent (mQueryEvent);
  }
}

// EventCreateEventQuery
EFI_STATUS
EventCreateEventQuery (
  IN APPLE_EVENT_DATA    EventData,
  IN APPLE_EVENT_TYPE    EventType,
  IN APPLE_MODIFIER_MAP  Modifiers
  ) // sub_135B
{
  EFI_STATUS                    Status;

  APPLE_EVENT_QUERY_INFORMATION *Information;

  ASSERT (EventData.Raw != 0);
  ASSERT (EventType != APPLE_EVENT_TYPE_NONE);

  Status = EFI_INVALID_PARAMETER;

  if (EventData.Raw != 0) {
    Information = EventCreateAppleEventQueryInfo (
                    EventData,
                    EventType,
                    NULL,
                    Modifiers
                    );

    Status = EFI_OUT_OF_RESOURCES;

    if (Information != NULL) {
      EventAddEventQuery (Information);

      Status = EFI_SUCCESS;
    }
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}
