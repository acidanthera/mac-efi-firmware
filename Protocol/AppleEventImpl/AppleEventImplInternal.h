/** @file
  Copyright (C) 2005 - 2017, Apple Inc.  All rights reserved.<BR>

  This program and the accompanying materials have not been licensed.
  Neither is its usage, its redistribution, in source or binary form,
  licensed, nor implicitely or explicitely permitted, except when
  required by applicable law.

  Unless required by applicable law or agreed to in writing, software
  distributed is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
  OR CONDITIONS OF ANY KIND, either express or implied.
**/

#ifndef APPLE_EVENT_IMPL_INTERNAL_H_
#define APPLE_EVENT_IMPL_INTERNAL_H_

#include APPLE_PROTOCOL_PRODUCER (AppleEventImpl)

#define APPLE_EVENT_HANDLE_PRIVATE_SIGNATURE  \
  EFI_SIGNATURE_32 ('A', 'L', 's', 't')

#define APPLE_EVENT_HANDLE_PRIVATE_FROM_LIST_ENTRY(Handle)  \
  CR (                                                      \
    (Handle),                                               \
    APPLE_EVENT_HANDLE_PRIVATE,                             \
    This,                                                   \
    APPLE_EVENT_HANDLE_PRIVATE_SIGNATURE                    \
    )

#define ASSERT_APPLE_EVENT_HANDLE_SIGNATURE(Handle)      \
  ASSERT (                                               \
    ((APPLE_EVENT_HANDLE_PRIVATE *)(Handle))->Signature  \
      == APPLE_EVENT_HANDLE_PRIVATE_SIGNATURE            \
    )

// APPLE_EVENT_HANDLE_PRIVATE
typedef struct {
  UINT32                      Signature;       ///< 
  EFI_LIST_ENTRY              This;            ///< 
  BOOLEAN                     Ready;           ///< 
  BOOLEAN                     Registered;      ///< 
  APPLE_EVENT_TYPE            EventType;       ///< 
  APPLE_EVENT_NOTIFY_FUNCTION NotifyFunction;  ///< 
  VOID                        *NotifyContext;  ///< 
  CHAR8                       *Name;           ///< 
} APPLE_EVENT_HANDLE_PRIVATE;

// APPLE_EVENT_QUERY_SIGNATURE
#define APPLE_EVENT_QUERY_SIGNATURE  EFI_SIGNATURE_32 ('A', 'E', 'v', 'Q')

// APPLE_EVENT_QUERY_FROM_LIST_ENTRY
#define APPLE_EVENT_QUERY_FROM_LIST_ENTRY(ListEntry) \
  CR ((ListEntry), APPLE_EVENT_QUERY, This, APPLE_EVENT_QUERY_SIGNATURE)

// APPLE_EVENT_QUERY
typedef struct {
  UINT32                        Signature;     ///< 
  EFI_LIST_ENTRY                This;          ///< 
  APPLE_EVENT_QUERY_INFORMATION *Information;  ///< 
} APPLE_EVENT_QUERY;

// PROTOCOL_INSTANCE
typedef struct {
  EFI_HANDLE Handle;      ///<
  VOID       *Interface;  ///<
  BOOLEAN    Installed;   ///<
} EFI_PROTOCOL_INSTANCE;

// mSimplePointerInstallNotifyEvent
extern EFI_EVENT mSimplePointerInstallNotifyEvent;

// mProtocolInstances
extern EFI_PROTOCOL_INSTANCE *mPointerProtocols;

// mEventHandleList
extern EFI_LIST mEventHandleList;

// mNumberOfEventHandles
extern UINTN mNumberOfEventHandles;

// mCLockOn
extern BOOLEAN mCLockOn;

// KEY_STROKE_INFORMATION
typedef struct {
  APPLE_KEY AppleKey;         ///< 
  UINTN     NumberOfStrokes;  ///< 
  BOOLEAN   CurrentStroke;    ///< 
} KEY_STROKE_INFORMATION;

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

// EventRegisterHandler
EFI_STATUS
EFIAPI
EventRegisterHandler (
  IN  APPLE_EVENT_TYPE             Type,
  IN  APPLE_EVENT_NOTIFY_FUNCTION  NotifyFunction,
  OUT APPLE_EVENT_HANDLE           *Handle,
  IN  VOID                         *NotifyContext
  );

// EventUnregisterHandler
EFI_STATUS
EFIAPI
EventUnregisterHandler (
  IN APPLE_EVENT_HANDLE  EventHandle
  );

// EventSetCursorPosition
EFI_STATUS
EFIAPI
EventSetCursorPosition (
  IN DIMENSION  *Position
  );

// EventSetEventName
EFI_STATUS
EFIAPI
EventSetEventName (
  IN OUT APPLE_EVENT_HANDLE  Handle,
  IN     CHAR8               *Name
  );

// EventIsCapsLockOn
EFI_STATUS
EFIAPI
EventIsCapsLockOn (
  IN OUT BOOLEAN  *CapsLockOn
  );

// EventCancelPollEvents
VOID
EventCancelPollEvents (
  VOID
  );

// EventCreateSimplePointerInstallNotifyEvent
EFI_STATUS
EventCreateSimplePointerInstallNotifyEvent (
  VOID
  );

// EventCancelKeyStrokePollEvent
VOID
EventCancelKeyStrokePollEvent (
  VOID
  );

// EventCancelSimplePointerPollEvent
VOID
EventCancelSimplePointerPollEvent (
  VOID
  );

// EventCreateSimplePointerPollEvent
EFI_STATUS
EventCreateSimplePointerPollEvent (
  VOID
  );

// EventCreateKeyStrokePollEvent
EFI_STATUS
EventCreateKeyStrokePollEvent (
  VOID
  );

// EventCreateAppleEventQueryInfo
APPLE_EVENT_QUERY_INFORMATION *
EventCreateAppleEventQueryInfo (
  IN APPLE_EVENT_DATA    EventData,
  IN APPLE_EVENT_TYPE    EventType,
  IN DIMENSION           *PointerPosition,
  IN APPLE_MODIFIER_MAP  Modifiers
  );

// EventAddEventQuery
VOID
EventAddEventQuery (
  IN APPLE_EVENT_QUERY_INFORMATION  *Information
  );

// EventCreateEventQuery
EFI_STATUS
EventCreateEventQuery (
  IN APPLE_EVENT_DATA    EventData,
  IN APPLE_EVENT_TYPE    EventType,
  IN APPLE_MODIFIER_MAP  Modifiers
  );

// EventSetCursorPositionImpl
EFI_STATUS
EventSetCursorPositionImpl (
  IN DIMENSION  *Position
  );

// EventRemoveUnregisteredEvents
VOID
EventRemoveUnregisteredEvents (
  VOID
  );

// EventCreatePollEvents
EFI_STATUS
EventCreatePollEvents (
  VOID
  );

#endif // APPLE_EVENT_IMPL_INTERNAL_H_
