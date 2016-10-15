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

#ifndef APPLE_EVENT_IMPL_H_
#define APPLE_EVENT_IMPL_H_

#include <IndustryStandard/AppleHid.h>

#include APPLE_PROTOCOL_PRODUCER (AppleEvent)

#include <Library/AppleDriverLib.h>

// APPLE_EVENT_PROTOCOL_REVISION
#define APPLE_EVENT_PROTOCOL_REVISION  0x07

// EventImplInitialize
EFI_STATUS
EFIAPI
EventImplInitialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  );

// EventRegisterHandlerImpl
EFI_STATUS
EFIAPI
EventRegisterHandler (
  IN  UINT32                       Type,
  IN  APPLE_EVENT_NOTIFY_FUNCTION  NotifyFunction,
  OUT APPLE_EVENT_HANDLE           *Handle,
  IN  VOID                         *NotifyContext
  );

// EventUnregisterHandlerImpl
EFI_STATUS
EFIAPI
EventUnregisterHandler (
  IN APPLE_EVENT_HANDLE  EventHandle
  );

// EventSetCursorPositionImpl
EFI_STATUS
EFIAPI
EventSetCursorPosition (
  IN DIMENSION  *Position
  );

// EventSetEventNameImpl
EFI_STATUS
EFIAPI
EventSetEventName (
  IN OUT APPLE_EVENT_HANDLE  Handle,
  IN     CHAR8               *Name
  );

// EventIsCapsLockOnImpl
EFI_STATUS
EFIAPI
EventIsCapsLockOn (
  IN OUT BOOLEAN  *CapsLockOn
  );

#endif // APPLE_EVENT_IMPL_H_
