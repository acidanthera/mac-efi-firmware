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
/// @file      Protocol/AppleEventImpl/AppleEventImplInternal.h
///
///            
///
/// @author    Download-Fritz
/// @date      11/12/2015: Initial version
/// @copyright Copyright (C) 2005 - 2015 Apple Inc. All rights reserved.
///

#ifndef __APPLE_EVENT_IMPL_INTERNAL_H__
#define __APPLE_EVENT_IMPL_INTERNAL_H__

#include <Protocol/AppleEventImpl.h>

// APPLE_EVENT_QUERY_SIGNATURE
#define APPLE_EVENT_QUERY_SIGNATURE  EFI_SIGNATURE_32 ('A', 'E', 'v', 'Q')

// APPLE_EVENT_QUERY_FROM_LIST_ENTRY
#define APPLE_EVENT_QUERY_FROM_LIST_ENTRY(ListEntry) \
  CR (ListEntry, APPLE_EVENT_QUERY, This, APPLE_EVENT_QUERY_SIGNATURE)

// _APPLE_EVENT_QUERY
typedef struct _APPLE_EVENT_QUERY {
  UINT32                        Signature;     ///< 
  EFI_LIST_ENTRY                This;          ///< 
  APPLE_EVENT_QUERY_INFORMATION *Information;  ///< 
} APPLE_EVENT_QUERY;

// _PROTOCOL_INSTANCE
typedef struct _EFI_PROTOCOL_INSTANCE {
  EFI_HANDLE Handle;
  VOID       *Interface;
  BOOLEAN    Installed;
} EFI_PROTOCOL_INSTANCE;

// mAppleEventHandleList
extern EFI_LIST mEventHandleList;

// mAppleEventProtocol
extern APPLE_EVENT_PROTOCOL mAppleEventProtocol;

// mSimplePointerInstallNotifyEvent
extern EFI_EVENT mSimplePointerInstallNotifyEvent;

// mProtocolInstances
extern EFI_PROTOCOL_INSTANCE *mSimplePointerInstances;

// mEventHandleList
extern EFI_LIST mEventHandleList;

// mCLockOn
extern BOOLEAN mCLockOn;

// _KEY_STROKE_INFORMATION
typedef struct _KEY_STROKE_INFORMATION {
  APPLE_KEY AppleKey;       ///< 
  UINTN     NoStrokes;      ///< 
  BOOLEAN   CurrentStroke;  ///< 
} KEY_STROKE_INFORMATION;

// _POINTER_BUTTON_INFORMATION
typedef struct _POINTER_BUTTON_INFORMATION {
  UINTN     Button;             ///< 
  UINTN     NoButtonPressed;    ///< 
  UINTN     Polls;              ///< 
  UINTN     PreviousEventType;  ///< 
  BOOLEAN   PreviousButton;     ///< 
  BOOLEAN   CurrentButton;      ///< 
  DIMENSION PreviousPosition;   ///< 
  DIMENSION CurrentPosition;    ///< 
} POINTER_BUTTON_INFORMATION;

// EventUnregisterHandlers
/// 
///
/// @param 
///
/// @return 
/// @retval 
VOID
EventUnregisterHandlers (
  VOID
  );

// EventSignalAndCloseQueryEvent
/// 
///
/// @param 
///
/// @return 
/// @retval 
VOID
EventSignalAndCloseQueryEvent (
  VOID
  );

// EventCancelPollEvents
/// 
///
/// @param 
///
/// @return 
/// @retval 
VOID
EventCancelPollEvents (
  VOID
  );

// EventCreateQueryEvent
/// 
///
/// @param 
///
/// @return 
/// @retval 
VOID
EventCreateQueryEvent (
  VOID
  );

// EventCreateSimplePointerInstallNotifyEvent
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EventCreateSimplePointerInstallNotifyEvent (
  VOID
  );

// EventCancelKeyStrokePollEvent
/// 
///
/// @param 
///
/// @return 
/// @retval 
VOID
EventCancelKeyStrokePollEvent (
  VOID
  );

// EventCancelSimplePointerPollEvent
/// 
///
/// @param 
///
/// @return 
/// @retval 
VOID
EventCancelSimplePointerPollEvent (
  VOID
  );

// EventCreateSimplePointerPollEvent
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EventCreateSimplePointerPollEvent (
  VOID
  );

// EventCreateKeyStrokePollEvent
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EventCreateKeyStrokePollEvent (
  VOID
  );

// EventCreateAppleEventQueryInfo
/// 
///
/// @param 
///
/// @return 
/// @retval 
APPLE_EVENT_QUERY_INFORMATION *
EventCreateAppleEventQueryInfo (
  IN APPLE_EVENT_DATA    EventData,
  IN APPLE_EVENT_TYPE    EventType,
  IN DIMENSION           *PointerPosition,
  IN APPLE_MODIFIER_MAP  Modifiers
  );

// EventAddEventQuery
/// 
///
/// @param 
///
/// @return 
/// @retval 
VOID
EventAddEventQuery (
  IN APPLE_EVENT_QUERY_INFORMATION  *Information
  );

// EventCreateEventQuery
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EventCreateEventQuery (
  IN APPLE_EVENT_DATA    EventData,
  IN APPLE_EVENT_TYPE    EventType,
  IN APPLE_MODIFIER_MAP  Modifiers
  );

// EventInternalSetCursorPosition
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EventInternalSetCursorPosition (
  IN DIMENSION  *Position
  );

// EventRemoveUnregisteredEvents
/// 
///
/// @param 
///
/// @return 
/// @retval 
VOID
EventRemoveUnregisteredEvents (
  VOID
  );

// EventCreatePollEvents
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EventCreatePollEvents (
  VOID
  );

#endif // ifndef __APPLE_EVENT_IMPL_INTERNAL_H__
