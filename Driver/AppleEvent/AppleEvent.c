#include <AppleEfi.h>
#include <EfiDriverLib.h>

#include <IndustryStandard/AppleHid.h>

#include <Library/AppleKeyMapLib.h>
#include <Library/AppleEventLib.h>

#include EFI_PROTOCOL_CONSUMER (LoadedImage)
#include <Protocol/AppleEvent.h>
#include <Protocol/AppleEventImpl.h>

#include <Driver/AppleEvent.h>

// mAppleEventProtocol
APPLE_EVENT_PROTOCOL mAppleEventProtocol = {
  APPLE_EVENT_PROTOCOL_REVISION,
  AppleEventRegisterHandlerImpl,
  AppleEventUnregisterHandlerImpl,
  AppleEventSetCursorPositionImpl,
  AppleEventSetEventNameImpl,
  AppleEventIsCapsLockActiveImpl
};

// UnloadAppleEventDummy
EFI_STATUS
EFIAPI
UnloadAppleEventDummy (
  IN EFI_HANDLE  ImageHandle
  )
{
  return EFI_SUCCESS;
}

// AppleEventMain
/// 
///
/// @param[in] ImageHandle The firmware allocated handle for the EFI image.  
/// @param[in] SystemTable A pointer to the EFI System Table.
///
/// @retval EFI_SUCCESS         The entry point is executed successfully.
/// @retval EFI_ALREADY_STARTED The protocol has already been installed.
EFI_STATUS
EFIAPI
AppleEventMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  ) // start
{
  EFI_STATUS                Status;

  EFI_LOADED_IMAGE_PROTOCOL *Interface;

  Status            = SystemTable->BootServices->HandleProtocol (ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **)&Interface);
  Interface->Unload = UnloadAppleEventDummy;

  return AppleEventInitialize (ImageHandle, SystemTable);
}
