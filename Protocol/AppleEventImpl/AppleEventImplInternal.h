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

#ifndef APPLE_EVENT_IMPL_INTERNAL_H_
#define APPLE_EVENT_IMPL_INTERNAL_H_

#include APPLE_PROTOCOL_PRODUCER (AppleEventImpl)

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

// mAppleEventProtocol
extern APPLE_EVENT_PROTOCOL mAppleEventProtocol;

// mSimplePointerInstallNotifyEvent
extern EFI_EVENT mSimplePointerInstallNotifyEvent;

// mProtocolInstances
extern EFI_PROTOCOL_INSTANCE *mPointerProtocols;

// mEventHandleList
extern EFI_LIST mEventHandleList;

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

// EventUnregisterHandlers
VOID
EventUnregisterHandlers (
  VOID
  );

// EventSignalAndCloseQueryEvent
VOID
EventSignalAndCloseQueryEvent (
  VOID
  );

// EventCancelPollEvents
VOID
EventCancelPollEvents (
  VOID
  );

// EventCreateQueryEvent
VOID
EventCreateQueryEvent (
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

// EventInternalSetCursorPosition
EFI_STATUS
EventInternalSetCursorPosition (
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
