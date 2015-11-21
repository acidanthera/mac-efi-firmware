#ifndef __APPLE_EVENT_IMPL_H__
#define __APPLE_EVENT_IMPL_H__

// APPLE_EVENT_PROTOCOL_REVISION
#define APPLE_EVENT_PROTOCOL_REVISION  0x07

// KEY_STROKE_DELAY
#define KEY_STROKE_DELAY  5

// APPLE_EVENT_HANDLE_SIGNATURE
#define APPLE_EVENT_HANDLE_SIGNATURE  EFI_SIGNATURE_32 ('A', 'L', 's', 't')

// APPLE_EVENT_HANDLE_FROM_LIST_ENTRY
#define APPLE_EVENT_HANDLE_FROM_LIST_ENTRY(Event) CR (Event, APPLE_EVENT_HANDLE, This, APPLE_EVENT_HANDLE_SIGNATURE)

// APPLE_EVENT_QUERY_SIGNATURE
#define APPLE_EVENT_QUERY_SIGNATURE  EFI_SIGNATURE_32 ('A', 'E', 'v', 'Q')

// APPLE_EVENT_QUERY_FROM_LIST_ENTRY
#define APPLE_EVENT_QUERY_FROM_LIST_ENTRY(ListEntry) CR (ListEntry, APPLE_EVENT_QUERY, This, APPLE_EVENT_QUERY_SIGNATURE)

// _APPLE_EVENT_QUERY
typedef struct _APPLE_EVENT_QUERY {
  UINT32                        Signature;
  EFI_LIST_ENTRY                This;
  APPLE_EVENT_QUERY_INFORMATION *Information;
} APPLE_EVENT_QUERY;

// _PROTOCOL_INSTANCE
typedef struct _EFI_PROTOCOL_INSTANCE {
  EFI_HANDLE Handle;
  VOID       *Interface;
  BOOLEAN    Installed;
} EFI_PROTOCOL_INSTANCE;

// mAppleEventProtocol
extern APPLE_EVENT_PROTOCOL mAppleEventProtocol;

// mSimplePointerInstallNotifyEvent
extern EFI_EVENT mSimplePointerInstallNotifyEvent;

// mProtocolInstances
extern EFI_PROTOCOL_INSTANCE *mSimplePointerInstances;

// mEventHandleList
extern EFI_LIST mEventHandleList;

// mCapsLockActive
extern BOOLEAN mCLockActive;

// _KEY_STROKE_INFORMATION
typedef struct _KEY_STROKE_INFORMATION {
  APPLE_KEY AppleKey;
  UINTN     NoStrokes;
  BOOLEAN   CurrentStroke;
} KEY_STROKE_INFORMATION;

// _POINTER_BUTTON_INFORMATION
typedef struct _POINTER_BUTTON_INFORMATION {
  UINTN     Button;
  UINTN     NoButtonPressed;
  UINTN     Polls;
  UINTN     PreviousEventType;
  BOOLEAN   PreviousButton;
  BOOLEAN   CurrentButton;
  DIMENSION PreviousPosition;
  DIMENSION CurrentPosition;
} POINTER_BUTTON_INFORMATION;

// AppleEventInitialize
EFI_STATUS
EFIAPI
AppleEventInitialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  );

// AppleEventRegisterHandlerImpl
EFI_STATUS
EFIAPI
AppleEventRegisterHandlerImpl (
  IN     UINT32                       EventType,
  IN     APPLE_EVENT_NOTIFY_FUNCTION  NotifyFunction,
     OUT APPLE_EVENT_HANDLE           **EventHandle,
  IN     VOID                         *NotifyContext
  );

// AppleEventUnregisterHandlerImpl
EFI_STATUS
EFIAPI
AppleEventUnregisterHandlerImpl (
  IN APPLE_EVENT_HANDLE  *EventHandle
  );

// AppleEventSetCursorPositionImpl
EFI_STATUS
EFIAPI
AppleEventSetCursorPositionImpl (
  IN DIMENSION  *Position
  );

// AppleEventSetEventNameImpl
EFI_STATUS
EFIAPI
AppleEventSetEventNameImpl (
  IN OUT APPLE_EVENT_HANDLE  *EventHandle,
  IN     CHAR8               *EventName
  );

// AppleEventIsCapsLockActiveImpl
EFI_STATUS
EFIAPI
AppleEventIsCapsLockActiveImpl (
  IN OUT BOOLEAN  *CapsLockAcvtive
  );

// AppleEventUnregisterHandlers
VOID
AppleEventUnregisterHandlers (
  VOID
  );

// AppleEventSignalAndCloseQueryEvent
VOID
AppleEventSignalAndCloseQueryEvent (
  VOID
  );

// AppleEventCancelPollEvents
VOID
AppleEventCancelPollEvents (
  VOID
  );

// AppleEventCreateQueryEvent
VOID
AppleEventCreateQueryEvent (
  VOID
  );

// AppleEventCreateSimplePointerInstallNotifyEvent
EFI_STATUS
AppleEventCreateSimplePointerInstallNotifyEvent (
  VOID
  );

// AppleEventCancelKeyStrokePollEvent
VOID
AppleEventCancelKeyStrokePollEvent (
  VOID
  );

// AppleEventCancelSimplePointerPollEvent
VOID
AppleEventCancelSimplePointerPollEvent (
  VOID
  );

// AppleEventCreateSimplePointerPollEvent
EFI_STATUS
AppleEventCreateSimplePointerPollEvent (
  VOID
  );

// AppleEventCreateKeyStrokePollEvent
EFI_STATUS
AppleEventCreateKeyStrokePollEvent (
  VOID
  );

// AppleEventCreateAppleEventQueryInformation
APPLE_EVENT_QUERY_INFORMATION *
AppleEventCreateAppleEventQueryInformation (
  IN APPLE_EVENT_DATA    EventData,
  IN APPLE_EVENT_TYPE    EventType,
  IN DIMENSION           *PointerPosition,
  IN APPLE_MODIFIER_MAP  Modifiers
  );

// AppleEventAddEventQuery
VOID
AppleEventAddEventQuery (
  IN APPLE_EVENT_QUERY_INFORMATION  *Information
  );

// AppleEventCreateEventQuery
EFI_STATUS
AppleEventCreateEventQuery (
  IN APPLE_EVENT_DATA    EventData,
  IN APPLE_EVENT_TYPE    EventType,
  IN APPLE_MODIFIER_MAP  Modifiers
  );

// InternalSetCursorPosition
EFI_STATUS
InternalSetCursorPosition (
  IN DIMENSION  *Position
  );

// AppleEventRemoveUnregisteredEvents
VOID
AppleEventRemoveUnregisteredEvents (
  VOID
  );

// AppleEventCreatePollEvents
EFI_STATUS
AppleEventCreatePollEvents (
  VOID
  );

#endif // ifndef __APPLE_EVENT_IMPL_H__
