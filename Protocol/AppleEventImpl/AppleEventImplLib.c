#include <AppleEfi.h>
#include <EfiDriverLib.h>
#include <EfiDebug.h>

#include <Guid/AppleNvram.h>

#include <IndustryStandard/AppleHid.h>

#include EFI_PROTOCOL_CONSUMER (ConsoleControl)
#include EFI_PROTOCOL_CONSUMER (GraphicsOutput)
#include EFI_PROTOCOL_CONSUMER (LoadedImage)
#include EFI_PROTOCOL_CONSUMER (SimplePointer)
#include <Protocol/AppleKeyMapAggregator.h>
#include <Protocol/AppleEvent.h>

#include <Library/AppleKeyMapLib.h>
#include <Library/AppleEventLib.h>

#include <Protocol/AppleEventImpl.h>

// KEY_STROKE_DELAY
#define KEY_STROKE_DELAY  5

// mAppleEventHandleList
EFI_LIST mEventHandleList;

// mEfiLock
static EFI_LOCK mEfiLock;

// mQueryEvent
static EFI_EVENT mQueryEvent;

// mQueryEventCreated
static BOOLEAN mQueryEventCreated;

// mEventQueryList
static EFI_LIST mEventQueryList;

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

  Event = NULL;

  if (NotifyTpl < EFI_TPL_CALLBACK) {
    Status = gBS->CreateEvent (
      ((NotifyFunction != NULL) ? (EFI_EVENT_TIMER | EFI_EVENT_NOTIFY_SIGNAL) : EFI_EVENT_TIMER),
      NotifyTpl,
      NotifyFunction,
      NotifyContext,
      &Event
      );

    if (!EFI_ERROR (Status)) {
      Status = gBS->SetTimer (Event, (SignalPeriodic ? TimerPeriodic : TimerRelative), TriggerTime);

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
  if (mSimplePointerInstallNotifyEvent != NULL) {
    gBS->CloseEvent (mSimplePointerInstallNotifyEvent);
  }

  AppleEventSignalAndCloseQueryEvent ();
  AppleEventUnregisterHandlers ();
  AppleEventCancelPollEvents ();

  return gBS->UninstallProtocolInterface (ImageHandle, &gAppleEventProtocolGuid, (VOID *)&mAppleEventProtocol);
}

// AppleEventInitialize
EFI_STATUS
EFIAPI
AppleEventInitialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                Status;

  EFI_LOADED_IMAGE_PROTOCOL *Interface;

  EfiInitializeDriverLib (ImageHandle, SystemTable);
  DxeInitializeDriverLib (ImageHandle, SystemTable);

  Interface = NULL;
  Status    = gBS->InstallProtocolInterface (&ImageHandle, &gAppleEventProtocolGuid, EFI_NATIVE_INTERFACE, (VOID *)&mAppleEventProtocol);

  if (!EFI_ERROR (Status)) {
    Status = gBS->HandleProtocol (ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **)&Interface);

    if (!EFI_ERROR (Status)) {
      Interface->Unload = UnloadAppleEvent;

      AppleEventCreateQueryEvent ();

      Status = AppleEventCreateSimplePointerInstallNotifyEvent ();

      if (!EFI_ERROR (Status)) {
        goto returnStatus;
      }
    }

    UnloadAppleEvent (ImageHandle);
  }

returnStatus:
  return Status;
}

// AppleEventRemoveUnregisteredEvents
VOID
AppleEventRemoveUnregisteredEvents (
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
          gBS->FreePool (CurrentEvent->Name);
        }

        RemoveEntryList (NextEvent->This.BackLink);
        gBS->FreePool (CurrentEvent);
      }

      CurrentEvent = NextEvent;
    } while (!IsNull (&mEventHandleList, &NextEvent->This));
  }
}

// AppleEventUnregisterHandlers
VOID
AppleEventUnregisterHandlers (
  VOID
  ) // sub_A31
{
  if (mSimplePointerInstances != NULL) {
    gBS->FreePool (mSimplePointerInstances);
  }

  AppleEventRemoveUnregisteredEvents ();

  if (!IsListEmpty (&mEventHandleList)) {
    AppleEventUnregisterHandlerImpl ((APPLE_EVENT_HANDLE *)EFI_MAX_ADDRESS);
  }
}

// AppleEventCreatePollEvents
EFI_STATUS
AppleEventCreatePollEvents (
  VOID
  ) // sub_F1B
{
  EFI_STATUS Status;

  Status = AppleEventCreateSimplePointerPollEvent ();

  if (!EFI_ERROR (Status)) {
    Status = AppleEventCreateKeyStrokePollEvent ();

    if (EFI_ERROR (Status)) {
      AppleEventCancelSimplePointerPollEvent ();
    }
  }

  return Status;
}

// AppleEventCancelPollEvents
VOID
AppleEventCancelPollEvents (
  VOID
  ) // sub_F4D
{
  AppleEventCancelSimplePointerPollEvent ();
  AppleEventCancelKeyStrokePollEvent ();
}

// AppleEventCreateAppleEventQueryInformation
APPLE_EVENT_QUERY_INFORMATION *
AppleEventCreateAppleEventQueryInformation (
  IN APPLE_EVENT_DATA    EventData,
  IN APPLE_EVENT_TYPE    EventType,
  IN DIMENSION           *PointerPosition,
  IN APPLE_MODIFIER_MAP  Modifiers
  ) // sub_F64
{
  APPLE_EVENT_QUERY_INFORMATION *QueryInfo;

  EFI_TIME                      CreationTime;

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
          if ((EventHandle->Registered) && (EventHandle->Ready) && ((EventQuery->Information->EventType & EventHandle->EventType) != 0) && (EventHandle->NotifyFunction != NULL)) {
            EventHandle->NotifyFunction (EventQuery->Information, EventHandle->NotifyContext);
          }
        }

        if (((EventQuery->Information->EventType & APPLE_ALL_KEYBOARD_EVENTS) != 0)
         && (((VOID *)EventQuery->Information->EventData.AppleKeyEventData) != NULL)) {
          gBS->FreePool ((VOID *)EventQuery->Information->EventData.AppleKeyEventData);
        }

        RemoveEntryList (EventQuery->This.ForwardLink->BackLink);
        gBS->FreePool ((VOID *)EventQuery->Information);
        gBS->FreePool ((VOID *)EventQuery);
      } while (!IsNull (&mEventQueryList, &EventQuery->This));
    }

    AppleEventRemoveUnregisteredEvents ();
    EfiReleaseLock (&mEfiLock);
  }
}

// AppleEventCreateQueryEvent
VOID
AppleEventCreateQueryEvent (
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

// AppleEventSignalAndCloseQueryEvent
VOID
AppleEventSignalAndCloseQueryEvent (
  VOID
  ) // sub_E54
{
  gBS->SignalEvent (mQueryEvent);

  if (mQueryEventCreated && (mQueryEvent != NULL)) {
    gBS->CloseEvent (mQueryEvent);
  }
}

// AppleEventAddEventQuery
VOID
AppleEventAddEventQuery (
  IN APPLE_EVENT_QUERY_INFORMATION  *Information
  ) // sub_E98
{
  EFI_STATUS        Status;

  APPLE_EVENT_QUERY *EventQuery;

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

// AppleEventCreateEventQuery
EFI_STATUS
AppleEventCreateEventQuery (
  IN APPLE_EVENT_DATA    EventData,
  IN APPLE_EVENT_TYPE    EventType,
  IN APPLE_MODIFIER_MAP  Modifiers
  ) // sub_135B
{
  EFI_STATUS                    Status;

  APPLE_EVENT_QUERY_INFORMATION *Information;

  Status = EFI_INVALID_PARAMETER;

  if (EventData.Raw != 0) {
    Information = AppleEventCreateAppleEventQueryInformation (EventData, EventType, NULL, Modifiers);
    Status      = EFI_OUT_OF_RESOURCES;

    if (Information != NULL) {
      AppleEventAddEventQuery (Information);

      Status = EFI_SUCCESS;
    }
  }

  return Status;
}
