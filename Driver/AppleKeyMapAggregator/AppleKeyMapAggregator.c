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
#include <LinkedList.h>

#include <IndustryStandard/AppleHid.h>

#include APPLE_PROTOCOL_PRODUCER (AppleKeyMapImpl)

#include <Library/AppleDriverLib.h>

#include <Driver/AppleKeyMapAggregator.h>

EFI_DRIVER_ENTRY_POINT (AppleKeyMapAggregatorMain);

// AppleKeyMapAggregatorMain
/**

  @param[in] ImageHandle  The firmware allocated handle for the EFI image.
  @param[in] SystemTable  A pointer to the EFI System Table.

  @retval EFI_SUCCESS          The entry point is executed successfully.
  @retval EFI_ALREADY_STARTED  The protocol has already been installed.
**/
EFI_STATUS
EFIAPI
AppleKeyMapAggregatorMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  ) // start
{
  EFI_STATUS                       Status;

  UINTN                            NumberOfHandles;
  EFI_HANDLE                       *Buffer;
  APPLE_KEY_MAP_AGGREGATOR_PRIVATE *Private;
  EFI_HANDLE                       Handle;

  AppleInitializeDriverLib (ImageHandle, SystemTable);

  ASSERT_PROTOCOL_ALREADY_INSTALLED (NULL, &gAppleKeyMapDatabaseProtocolGuid);

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gAppleKeyMapDatabaseProtocolGuid,
                  NULL,
                  &NumberOfHandles,
                  &Buffer
                  );

  if (!EFI_ERROR (Status)) {
    Status = EFI_DEVICE_ERROR;

    if (Buffer != NULL) {
      gBS->FreePool ((VOID *)Buffer);
    }
  } else {
    Private                     = EfiLibAllocateZeroPool (sizeof (*Private));
    Private->Signature          = APPLE_KEY_MAP_AGGREGATOR_SIGNATURE;
    Private->NextKeyStrokeIndex = 3000;

    Private->Database.Revision               = APPLE_KEY_MAP_DATABASE_PROTOCOL_REVISION;
    Private->Database.CreateKeyStrokesBuffer = KeyMapCreateKeyStrokesBuffer;
    Private->Database.RemoveKeyStrokesBuffer = KeyMapRemoveKeyStrokesBuffer;
    Private->Database.SetKeyStrokeBufferKeys = KeyMapSetKeyStrokeBufferKeys;

    Private->Aggregator.Revision           = APPLE_KEY_MAP_AGGREGATOR_PROTOCOL_REVISION;
    Private->Aggregator.GetKeyStrokes      = KeyMapGetKeyStrokes;
    Private->Aggregator.ContainsKeyStrokes = KeyMapContainsKeyStrokes;

    InitializeListHead (&Private->KeyStrokesInfoList);

    Handle = NULL;
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &Handle,
                    &gAppleKeyMapDatabaseProtocolGuid,
                    (VOID *)&Private->Database,
                    &gAppleKeyMapAggregatorProtocolGuid,
                    (VOID *)&Private->Aggregator,
                    NULL
                    );

    ASSERT_EFI_ERROR (Status);
  }

  return Status;
}
