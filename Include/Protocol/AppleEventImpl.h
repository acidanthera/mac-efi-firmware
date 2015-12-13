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
/// @file      Include/Protocol/AppleEventImpl.h
///
///            
///
/// @author    Download-Fritz
/// @date      31/02/2015: Initial version
/// @copyright Copyright (C) 2005 - 2015 Apple Inc. All rights reserved.
///

#ifndef __APPLE_EVENT_IMPL_H__
#define __APPLE_EVENT_IMPL_H__

#include <IndustryStandard/AppleHid.h>

#include <Protocol/AppleEvent.h>

// APPLE_EVENT_PROTOCOL_REVISION
#define APPLE_EVENT_PROTOCOL_REVISION  0x07

// EventImplInitialize
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
EventImplInitialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  );

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
  IN  UINT32                       EventType,
  IN  APPLE_EVENT_NOTIFY_FUNCTION  NotifyFunction,
  OUT APPLE_EVENT_HANDLE           **EventHandle,
  IN  VOID                         *NotifyContext
  );

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
  );

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
  );

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
  );

// EventIsCapsLockOnImpl
/// 
///
/// @param 
///
/// @return 
/// @retval 
EFI_STATUS
EFIAPI
EventIsCapsLockOnImpl (
  IN OUT BOOLEAN  *CapsLockOn
  );

#endif // ifndef __APPLE_EVENT_IMPL_H__
