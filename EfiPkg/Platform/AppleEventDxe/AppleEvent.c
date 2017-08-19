/** @file
  Copyright (c) 2005 - 2017, Apple Inc.  All rights reserved.<BR>

  This program and the accompanying materials have not been licensed.
  Neither is its usage, its redistribution, in source or binary form,
  licensed, nor implicitely or explicitely permitted, except when
  required by applicable law.

  Unless required by applicable law or agreed to in writing, software
  distributed is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
  OR CONDITIONS OF ANY KIND, either express or implied.
**/

#include <AppleMacEfi.h>

#include <Protocol/LoadedImage.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "AppleEventInternal.h"

// APPLE_EVENT_HANDLE_PRIVATE_SIGNATURE
#define APPLE_EVENT_HANDLE_PRIVATE_SIGNATURE  \
  SIGNATURE_32 ('A', 'L', 's', 't')

// APPLE_EVENT_HANDLE_PRIVATE_FROM_LIST_ENTRY
#define APPLE_EVENT_HANDLE_PRIVATE_FROM_LIST_ENTRY(Handle)  \
  CR (                                                      \
    (Handle),                                               \
    APPLE_EVENT_HANDLE_PRIVATE,                             \
    Link,                                                   \
    APPLE_EVENT_HANDLE_PRIVATE_SIGNATURE                    \
    )

// APPLE_EVENT_HANDLE_PRIVATE
typedef struct {
  UINT32                      Signature;       ///< 
  LIST_ENTRY                  Link;            ///< 
  BOOLEAN                     Ready;           ///< 
  BOOLEAN                     Registered;      ///< 
  APPLE_EVENT_TYPE            EventType;       ///< 
  APPLE_EVENT_NOTIFY_FUNCTION NotifyFunction;  ///< 
  VOID                        *NotifyContext;  ///< 
  CHAR8                       *Name;           ///< 
} APPLE_EVENT_HANDLE_PRIVATE;

// mEventHandles
STATIC
LIST_ENTRY mEventHandles = INITIALIZE_LIST_HEAD_VARIABLE (mEventHandles);

// mNumberOfEventHandles
STATIC UINTN mNumberOfEventHandles = 0;

// EventSignalEvents
VOID
EventSignalEvents (
  IN APPLE_EVENT_INFORMATION  *EventInformation
  )
{
  LIST_ENTRY                 *EventHandleEntry;
  APPLE_EVENT_HANDLE_PRIVATE *EventHandle;

  EventHandleEntry = GetFirstNode (&mEventHandles);

  while (!IsNull (&mEventHandles, EventHandleEntry)) {
    EventHandle = APPLE_EVENT_HANDLE_PRIVATE_FROM_LIST_ENTRY (
                    EventHandleEntry
                    );

    if (EventHandle->Registered
      && EventHandle->Ready
      && ((EventInformation->EventType & EventHandle->EventType) != 0)
      && (EventHandle->NotifyFunction != NULL)) {
      EventHandle->NotifyFunction (
                      EventInformation,
                      EventHandle->NotifyContext
                      );
    }

    EventHandleEntry = GetNextNode (&mEventHandles, EventHandleEntry);
  }
}

// InternalFlagAllEventsReady
VOID
InternalFlagAllEventsReady (
  VOID
  )
{
  LIST_ENTRY                 *EventHandleEntry;
  APPLE_EVENT_HANDLE_PRIVATE *EventHandle;

  EventHandleEntry = GetFirstNode (&mEventHandles);

  if (!IsListEmpty (&mEventHandles)) {
    do {
      EventHandle = APPLE_EVENT_HANDLE_PRIVATE_FROM_LIST_ENTRY (
                      EventHandleEntry
                      );

      EventHandle->Ready = TRUE;

      EventHandleEntry = GetNextNode (&mEventHandles, EventHandleEntry);
    } while (!IsNull (&mEventHandles, EventHandleEntry));
  }
}

// InternalRemoveUnregisteredEvents
VOID
InternalRemoveUnregisteredEvents (
  VOID
  ) 
{
  LIST_ENTRY                 *EventHandleEntry;
  LIST_ENTRY                 *NextEventHandleEntry;
  APPLE_EVENT_HANDLE_PRIVATE *EventHandle;

  EventHandleEntry = GetFirstNode (&mEventHandles);

  if (!IsListEmpty (&mEventHandles)) {
    do {
      NextEventHandleEntry = GetNextNode (&mEventHandles, EventHandleEntry);

      EventHandle = APPLE_EVENT_HANDLE_PRIVATE_FROM_LIST_ENTRY (
                      EventHandleEntry
                      );

      if (!EventHandle->Registered) {
        if (EventHandle->Name != NULL) {
          FreePool ((VOID *)EventHandle->Name);
        }

        RemoveEntryList (&EventHandle->Link);
        FreePool ((VOID *)EventHandle);
      }

      EventHandleEntry = NextEventHandleEntry;
    } while (!IsNull (&mEventHandles, NextEventHandleEntry));
  }
}

// InternalCreatePollEvents
STATIC
EFI_STATUS
InternalCreatePollEvents (
  VOID
  )
{
  EFI_STATUS Status;

  Status = EventCreateSimplePointerPollEvent ();

  if (!EFI_ERROR (Status)) {
    Status = EventCreateKeyStrokePollEvent ();

    if (EFI_ERROR (Status)) {
      EventCancelSimplePointerPollEvent ();
    }
  }

  return Status;
}

// InternalCancelPollEvents
VOID
InternalCancelPollEvents (
  VOID
  )
{
  EventCancelSimplePointerPollEvent ();
  EventCancelKeyStrokePollEvent ();
}

// EventRegisterHandler
EFI_STATUS
EFIAPI
EventRegisterHandler (
  IN  APPLE_EVENT_TYPE             Type,
  IN  APPLE_EVENT_NOTIFY_FUNCTION  NotifyFunction,
  OUT APPLE_EVENT_HANDLE           *Handle,
  IN  VOID                         *NotifyContext
  )
{
  EFI_STATUS                 Status;

  APPLE_EVENT_HANDLE_PRIVATE *EventHandle;

  Status = EFI_INVALID_PARAMETER;

  if ((Handle != NULL)
   && (NotifyFunction != NULL)
   && (Type != APPLE_EVENT_TYPE_NONE)) {
    *Handle = NULL;

    InternalRemoveUnregisteredEvents ();

    Status = EFI_SUCCESS;

    if (mNumberOfEventHandles == 0) {
      Status = InternalCreatePollEvents ();
      
      if (EFI_ERROR (Status)) {
        goto Done;
      }
    }

    EventHandle = AllocatePool (sizeof (*EventHandle));

    Status = EFI_OUT_OF_RESOURCES;

    if (EventHandle != NULL) {
      EventHandle->Signature      = APPLE_EVENT_HANDLE_PRIVATE_SIGNATURE;
      EventHandle->Ready          = FALSE;
      EventHandle->Registered     = TRUE;
      EventHandle->EventType      = Type;
      EventHandle->NotifyFunction = NotifyFunction;
      EventHandle->NotifyContext  = NotifyContext;
      EventHandle->Name           = NULL;

      ++mNumberOfEventHandles;

      InsertTailList (&mEventHandles, &EventHandle->Link);

      *Handle = EventHandle;

      Status = EFI_SUCCESS;
    }
  }

Done:
  return Status;
}

// EventUnregisterHandler
EFI_STATUS
EFIAPI
EventUnregisterHandler (
  IN APPLE_EVENT_HANDLE  Handle
  )
{
  EFI_STATUS                 Status;

  LIST_ENTRY                 *EventHandleEntry;
  APPLE_EVENT_HANDLE_PRIVATE *EventHandle;

  Status = EFI_INVALID_PARAMETER;

  EventHandleEntry = GetFirstNode (&mEventHandles);

  while (!IsNull (&mEventHandles, EventHandleEntry)) {
    EventHandle = APPLE_EVENT_HANDLE_PRIVATE_FROM_LIST_ENTRY (
                    EventHandleEntry
                    );

    if (((UINTN)EventHandle == (UINTN)Handle)
      || ((UINTN)Handle == (UINTN)-1)) {
      EventHandle->Registered = FALSE;
      --mNumberOfEventHandles;

      Status = EFI_SUCCESS;

      if ((UINTN)Handle != (UINTN)-1) {
        break;
      }
    }

    EventHandleEntry = GetNextNode (&mEventHandles, EventHandleEntry);
  }

  if (mNumberOfEventHandles == 0) {
    InternalCancelPollEvents ();
  }

  return Status;
}

// EventSetCursorPosition
/** This function is used to change the position of the cursor on the screen.

  @param[in] Position  The position where to position the cursor.
  
  @retval EFI_INVALID_PARAMETER  Position is invalid.
**/
EFI_STATUS
EFIAPI
EventSetCursorPosition (
  IN DIMENSION  *Position
  )
{
  return EventSetCursorPositionImpl (Position);
}

// EventSetEventName
/** This function is used to assign a name to an event.

  @param[in, out] Handle  
  @param[in]      Name 

  @retval EFI_SUCCESS            The event name was assigned successfully.
  @retval EFI_INVALID_PARAMETER  EventHandle or EventName is NULL.
  @retval EFI_OUT_OF_RESOURCES   There are not enough resources to allocate the
                                 event name.
**/
EFI_STATUS
EFIAPI
EventSetEventName (
  IN OUT APPLE_EVENT_HANDLE  Handle,
  IN     CHAR8               *Name
  )
{
  EFI_STATUS Status;

  UINTN      AllocationSize;
  CHAR8      *EventName;

  Status = EFI_INVALID_PARAMETER;

  if ((Handle != NULL) && (Name != NULL)) {
    AllocationSize = AsciiStrSize (Name);
    EventName      = AllocateZeroPool (AllocationSize);

    ((APPLE_EVENT_HANDLE_PRIVATE *)Handle)->Name = EventName;

    Status = EFI_OUT_OF_RESOURCES;

    if (EventName != NULL) {
      AsciiStrCpy (EventName, Name);

      Status = EFI_SUCCESS;
    }
  }

  return Status;
}

// EventIsCapsLockOnImpl
/** Retrieves the state of the CapsLock key.

  @param[in, out] CLockOn  This parameter indicates the state of the CapsLock
                           key.

  @retval EFI_SUCCESS            The CapsLock state was successfully returned
                                 in CLockOn.
  @retval EFI_INVALID_PARAMETER  CLockOn is NULL.
**/
EFI_STATUS
EFIAPI
EventIsCapsLockOn (
  IN OUT BOOLEAN  *CLockOn
  )
{
  return EventIsCapsLockOnImpl (CLockOn);
}

// InternalUnregisterHandlers
STATIC
VOID
InternalUnregisterHandlers (
  VOID
  )
{
  EventSimplePointerDesctructor ();

  InternalRemoveUnregisteredEvents ();

  if (!IsListEmpty (&mEventHandles)) {
    EventUnregisterHandler ((APPLE_EVENT_HANDLE_PRIVATE *)((UINTN)-1));
  }
}

// mAppleEventProtocol
STATIC APPLE_EVENT_PROTOCOL mAppleEventProtocol = {
  APPLE_EVENT_PROTOCOL_REVISION,
  EventRegisterHandler,
  EventUnregisterHandler,
  EventSetCursorPosition,
  EventSetEventName,
  EventIsCapsLockOn
};

// AppleEventUnload
EFI_STATUS
EFIAPI
AppleEventUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  EFI_STATUS Status;

  EventCloseSimplePointerInstallNotifyEvent ();

  InternalSignalAndCloseQueueEvent ();
  InternalUnregisterHandlers ();
  InternalCancelPollEvents ();

  Status = gBS->UninstallProtocolInterface (
                  ImageHandle,
                  &gAppleEventProtocolGuid,
                  (VOID *)&mAppleEventProtocol
                  );

  return Status;
}

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
  )
{
  EFI_STATUS                Status;

  EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;

  LoadedImage = NULL;
  Status      = gBS->InstallProtocolInterface (
                       &ImageHandle,
                       &gAppleEventProtocolGuid,
                       EFI_NATIVE_INTERFACE,
                       (VOID *)&mAppleEventProtocol
                       );

  // BUG: Use the EDK2 inf to handle unload.

  if (!EFI_ERROR (Status)) {
    Status = gBS->HandleProtocol (
                    ImageHandle,
                    &gEfiLoadedImageProtocolGuid,
                    (VOID **)&LoadedImage
                    );

    if (!EFI_ERROR (Status)) {
      LoadedImage->Unload = AppleEventUnload;

      InternalCreateQueueEvent ();

      Status = EventCreateSimplePointerInstallNotifyEvent ();

      if (!EFI_ERROR (Status)) {
        goto Done;
      }
    }

    AppleEventUnload (ImageHandle);
  }

Done:
  return Status;
}
