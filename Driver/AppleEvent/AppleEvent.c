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

#include APPLE_PROTOCOL_PRODUCER (AppleEventImpl)

#include <Driver/AppleEvent.h>

// AppleEventUnload
EFI_STATUS
EFIAPI
AppleEventUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  return EFI_SUCCESS;
}

EFI_DRIVER_ENTRY_POINT (AppleEventMain);

// AppleEventMain
/** 

  @param[in] ImageHandle  The firmware allocated handle for the EFI image.
  @param[in] SystemTable  A pointer to the EFI System Table.

  @retval EFI_SUCCESS          The entry point is executed successfully.
  @retval EFI_ALREADY_STARTED  The protocol has already been installed.
**/
EFI_STATUS
EFIAPI
AppleEventMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  ) // start
{
  return EventImplConstructor (ImageHandle, SystemTable);
}
