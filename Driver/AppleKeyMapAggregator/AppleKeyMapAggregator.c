///
/// @file      Driver/AppleKeyMapAggregator.c
///
///
///
/// @author    Download-Fritz
/// @date      15/03/2015: Initial version
/// @copyright The decompilation is of an educational purpose to better understand the behavior of the
///            Apple EFI implementation and making use of it. In no way is the content's usage licensed
///            or allowed. All rights remain at Apple Inc. To be used under the terms of 'Fair use'.
///

//
// CREDITS:
//   Reversed from AppleKeyMapAggregator.efi and AppleEvent.efi, which are Apple Inc. property
//   Decompiled by Download-Fritz
//

#include <AppleEfi.h>
#include <EfiDriverLib.h>
#include <LinkedList.h>

#include <IndustryStandard/AppleHid.h>

#include <Protocol/AppleKeyMapAggregator.h>
#include <Protocol/AppleKeyMapDatabase.h>
#include <Protocol/AppleKeyMapImpl.h>

#include <Library/AppleKeyMapLib.h>

#include <Driver/AppleKeyMapAggregator.h>

// AppleKeyMapAggregatorMain
///
/// @param[in] ImageHandle      The firmware allocated handle for the EFI image.  
/// @param[in] SystemTable      A pointer to the EFI System Table.
///
/// @retval EFI_SUCCESS         The entry point is executed successfully.
/// @retval EFI_ALREADY_STARTED The protocol has already been installed.
EFI_STATUS
EFIAPI
AppleKeyMapAggregatorMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  ) // start
{
  EFI_STATUS               Status;

  UINTN                    NumberHandles;
  EFI_HANDLE               *Buffer;
  APPLE_KEY_MAP_AGGREGATOR *Aggregator;
  EFI_HANDLE               Handle;

  EfiInitializeDriverLib (ImageHandle, SystemTable);

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gAppleKeyMapDatabaseProtocolGuid,
                  NULL,
                  &NumberHandles,
                  &Buffer
                  );

  if (!EFI_ERROR (Status)) {
    Status = EFI_DEVICE_ERROR;

    if (Buffer != NULL) {
      gBS->FreePool ((VOID *)Buffer);
    }
  } else {
    Aggregator                                          = (APPLE_KEY_MAP_AGGREGATOR *)EfiLibAllocateZeroPool (sizeof (*Aggregator));
    Aggregator->Signature                               = APPLE_KEY_MAP_AGGREGATOR_SIGNATURE;
    Aggregator->NextKeyStrokeIndex                      = 3000;
    Aggregator->DatabaseProtocol.Revision               = APPLE_KEY_MAP_DATABASE_PROTOCOL_REVISION;
    Aggregator->DatabaseProtocol.CreateKeyStrokesBuffer = AppleKeyMapCreateKeyStrokesBufferImpl;
    Aggregator->DatabaseProtocol.RemoveKeyStrokesBuffer = AppleKeyMapRemoveKeyStrokesBufferImpl;
    Aggregator->DatabaseProtocol.SetKeyStrokeBufferKeys = AppleKeyMapSetKeyStrokeBufferKeysImpl;
    Aggregator->AggregatorProtocol.Revision             = APPLE_KEY_MAP_AGGREGATOR_PROTOCOL_REVISION;
    Aggregator->AggregatorProtocol.GetKeyStrokes        = AppleKeyMapGetKeyStrokesImpl;
    Aggregator->AggregatorProtocol.ContainsKeyStrokes   = AppleKeyMapContainsKeyStrokesImpl;

    InitializeListHead (&Aggregator->KeyStrokesInfoList);

    Handle = NULL;
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &Handle,
                    &gAppleKeyMapDatabaseProtocolGuid,
                    (VOID *)&Aggregator->DatabaseProtocol,
                    &gAppleKeyMapAggregatorProtocolGuid,
                    (VOID *)&Aggregator->AggregatorProtocol,
                    NULL
                    );
  }

  return Status;
}
