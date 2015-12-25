/** @file
  Copyright (C) 2005 - 2015 Apple Inc.  All rights reserved.<BR>

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

#include <Library/AppleDriverLib.h>

#include <IndustryStandard/AppleHid.h>

#include EFI_PROTOCOL_CONSUMER (LoadedImage)

#include <Library/AppleEventLib.h>

#include "AppleEventImplInternal.h"

// mEfiLock
STATIC EFI_LOCK mEfiLock;

// mQueryEvent
STATIC EFI_EVENT mQueryEvent = NULL;

// mQueryEventCreated
STATIC BOOLEAN mQueryEventCreated = FALSE;

// mEventQueryList
STATIC EFI_LIST mEventQueryList = INITIALIZE_LIST_HEAD_VARIABLE (mEventQueryList);

// CreateTimerEvent
EFI_EVENT
CreateTimerEvent (
  IN EFI_EVENT_NOTIFY  NotifyFunction,
  IN VOID              *NotifyContext,
  IN UINT64            TriggerTime,
  IN BOOLEAN           SignalPeriodic,
  IN EFI_TPL           NotifyTpl
  )
{
  EFI_EVENT  Event;

  EFI_STATUS Status;

  ASSERT (NotifyTpl < EFI_TPL_CALLBACK);

  Event = NULL;

  if (NotifyTpl < EFI_TPL_CALLBACK) {
    Status = gBS->CreateEvent (
                    ((NotifyFunction != NULL) ? (EFI_EVENT_TIMER | EFI_EVENT_NOTIFY_SIGNAL) : EFI_EVENT_TIMER),
                    NotifyTpl,
                    NotifyFunction,
                    NotifyContext,
                    &Event
                    );

    ASSERT_EFI_ERROR (Status);

    if (!EFI_ERROR (Status)) {
      Status = gBS->SetTimer (Event, (SignalPeriodic ? TimerPeriodic : TimerRelative), TriggerTime);

      ASSERT_EFI_ERROR (Status);

      if (EFI_ERROR (Status)) {
        gBS->CloseEvent (Event);

        Event = NULL;
      }
    }
  }

  return Event;
}

// CreateNotifyEvent 
EFI_EVENT
CreateNotifyEvent (
  IN EFI_EVENT_NOTIFY  NotifyFunction,
  IN VOID              *NotifyContext,
  IN UINT64            TriggerTime,
  IN BOOLEAN           SignalPeriodic
  )
{
  return CreateTimerEvent (NotifyFunction, NotifyContext, TriggerTime, SignalPeriodic, EFI_TPL_NOTIFY);
}

// CancelEvent 
VOID
CancelEvent (
  IN EFI_EVENT  Event
  ) // sub_309
{
  EFI_STATUS Status;

  ASSERT (Event != NULL);

  Status = gBS->SetTimer (Event, TimerCancel, 0);

  ASSERT_EFI_ERROR (Status);

  if (!EFI_ERROR (Status)) {
    gBS->CloseEvent (Event);
  }
}

// UnloadAppleEvent
EFI_STATUS
EFIAPI
UnloadAppleEvent (
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

  Status = gBS->UninstallProtocolInterface (ImageHandle, &gAppleEventProtocolGuid, (VOID *)&mAppleEventProtocol);

  ASSERT_EFI_ERROR (Status);

  return Status;
}

// EventImplInitialize
EFI_STATUS
EFIAPI
EventImplInitialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                Status;

  EFI_LOADED_IMAGE_PROTOCOL *Interface;

  AppleInitializeDriverLib (ImageHandle, SystemTable);
  DxeInitializeDriverLib (ImageHandle, SystemTable);

  ASSERT_PROTOCOL_ALREADY_INSTALLED (NULL, &gAppleEventProtocolGuid);

  Interface = NULL;
  Status    = gBS->InstallProtocolInterface (
                     &ImageHandle,
                     &gAppleEventProtocolGuid,
                     EFI_NATIVE_INTERFACE,
                     (VOID *)&mAppleEventProtocol
                     );

  if (!EFI_ERROR (Status)) {
    Status = gBS->HandleProtocol (ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **)&Interface);

    if (!EFI_ERROR (Status)) {
      Interface->Unload = UnloadAppleEvent;

      EventCreateQueryEvent ();

      Status = EventCreateSimplePointerInstallNotifyEvent ();

      if (!EFI_ERROR (Status)) {
        goto Return;
      }
    }

    UnloadAppleEvent (ImageHandle);
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
  APPLE_EVENT_HANDLE *CurrentEvent;
  APPLE_EVENT_HANDLE *NextEvent;

  CurrentEvent = APPLE_EVENT_HANDLE_FROM_LIST_ENTRY (mEventHandleList.ForwardLink);

  if (!IsListEmpty (&mEventHandleList)) {
    do {
      NextEvent = APPLE_EVENT_HANDLE_FROM_LIST_ENTRY (GetNextNode (&mEventHandleList, &CurrentEvent->This));

      if (!CurrentEvent->Registered) {
        if (CurrentEvent->Name != NULL) {
          gBS->FreePool ((VOID *)CurrentEvent->Name);
        }

        RemoveEntryList (NextEvent->This.BackLink);
        gBS->FreePool ((VOID *)CurrentEvent);
      }

      CurrentEvent = NextEvent;
    } while (!IsNull (&mEventHandleList, &NextEvent->This));
  }
}

// EventUnregisterHandlers
VOID
EventUnregisterHandlers (
  VOID
  ) // sub_A31
{
  if (mSimplePointerInstances != NULL) {
    gBS->FreePool (mSimplePointerInstances);
  }

  EventRemoveUnregisteredEvents ();

  if (!IsListEmpty (&mEventHandleList)) {
    EventUnregisterHandlerImpl ((APPLE_EVENT_HANDLE *)EFI_MAX_ADDRESS);
  }
}

// EventCreatePollEvents
EFI_STATUS
EventCreatePollEvents (
  VOID
  ) // sub_F1B
{
  EFI_STATUS Status;

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

  QueryInfo = (APPLE_EVENT_QUERY_INFORMATION *)EfiLibAllocateZeroPool (sizeof (*QueryInfo));

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
      EfiCommonLibCopyMem ((VOID *)&QueryInfo->PointerPosition, (VOID *)PointerPosition, sizeof (*PointerPosition));
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
  APPLE_EVENT_HANDLE *EventHandle;

  if (!IsListEmpty (&mEventHandleList)) {
    for (
      EventHandle = APPLE_EVENT_HANDLE_FROM_LIST_ENTRY (mEventHandleList.ForwardLink);
      !IsNull (&mEventHandleList, &EventHandle->This);
      EventHandle = APPLE_EVENT_HANDLE_FROM_LIST_ENTRY (EventHandle->This.ForwardLink)
      ) {
      if (!EventHandle->Ready) {
        EventHandle->Ready = TRUE;
      }
    }
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
  EFI_STATUS         Status;

  APPLE_EVENT_QUERY  *EventQuery;
  APPLE_EVENT_HANDLE *EventHandle;

  ASSERT (Event != NULL);

  if (mQueryEventCreated) {
    do {
      Status = EfiAcquireLockOrFail (&mEfiLock);
    } while (Status != EFI_SUCCESS);

    FlagAllEventsReady ();

    if (!IsListEmpty (&mEventQueryList))  {
      EventQuery = APPLE_EVENT_QUERY_FROM_LIST_ENTRY (mEventQueryList.ForwardLink);

      do {
        for (
          EventHandle = APPLE_EVENT_HANDLE_FROM_LIST_ENTRY (mEventHandleList.ForwardLink);
          !IsNull (&mEventHandleList, &EventHandle->This);
          EventHandle = APPLE_EVENT_HANDLE_FROM_LIST_ENTRY (EventHandle->This.ForwardLink)
          ) {
          if ((EventHandle->Registered)
           && (EventHandle->Ready)
           && ((EventQuery->Information->EventType & EventHandle->EventType) != 0)
           && (EventHandle->NotifyFunction != NULL)) {
            EventHandle->NotifyFunction (EventQuery->Information, EventHandle->NotifyContext);
          }
        }

        if (((EventQuery->Information->EventType & APPLE_ALL_KEYBOARD_EVENTS) != 0)
         && (EventQuery->Information->EventData.AppleKeyEventData != NULL)) {
          gBS->FreePool ((VOID *)EventQuery->Information->EventData.AppleKeyEventData);
        }

        RemoveEntryList (EventQuery->This.ForwardLink->BackLink);
        gBS->FreePool ((VOID *)EventQuery->Information);
        gBS->FreePool ((VOID *)EventQuery);
      } while (!IsNull (&mEventQueryList, &EventQuery->This));
    }

    EventRemoveUnregisteredEvents ();
    EfiReleaseLock (&mEfiLock);
  }
}

// EventCreateQueryEvent
VOID
EventCreateQueryEvent (
  VOID
  ) // sub_D01
{
  EFI_STATUS Status;

  EfiInitializeLock (&mEfiLock, EFI_TPL_NOTIFY);

  Status = gBS->CreateEvent (EFI_EVENT_NOTIFY_SIGNAL, EFI_TPL_NOTIFY, QueryEventNotifyFunction, NULL, &mQueryEvent);

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
      Status = EfiAcquireLockOrFail (&mEfiLock);
    } while (Status != EFI_SUCCESS);

    EventQuery = (APPLE_EVENT_QUERY *)EfiLibAllocatePool (sizeof (*EventQuery));

    if (EventQuery != NULL) {
      EventQuery->Signature   = APPLE_EVENT_QUERY_SIGNATURE;
      EventQuery->Information = Information;

      InsertTailList (&mEventQueryList, &EventQuery->This);
    }

    EfiReleaseLock (&mEfiLock);
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
    Information = EventCreateAppleEventQueryInfo (EventData, EventType, NULL, Modifiers);
    Status      = EFI_OUT_OF_RESOURCES;

    if (Information != NULL) {
      EventAddEventQuery (Information);

      Status = EFI_SUCCESS;
    }
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}
