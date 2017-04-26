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
#include <EfiDebug.h>

#include <Library/AppleDriverLib.h>

// CreateTimerEvent
EFI_EVENT
CreateTimerEvent (
  IN EFI_EVENT_NOTIFY  NotifyFunction,
  IN VOID              *NotifyContext,
  IN UINT64            TriggerTime,
  IN BOOLEAN           SignalPeriodic,
  IN EFI_TPL           NotifyTpl
  )
{
  EFI_EVENT  Event;

  EFI_STATUS Status;

  ASSERT (NotifyTpl > EFI_TPL_CALLBACK);

  Event = NULL;

  if (NotifyTpl > EFI_TPL_CALLBACK) {
    Status = gBS->CreateEvent (
                    ((NotifyFunction != NULL)
                      ? (EFI_EVENT_TIMER | EFI_EVENT_NOTIFY_SIGNAL)
                      : EFI_EVENT_TIMER),
                    NotifyTpl,
                    NotifyFunction,
                    NotifyContext,
                    &Event
                    );

    ASSERT_EFI_ERROR (Status);

    if (!EFI_ERROR (Status)) {
      Status = gBS->SetTimer (
                      Event,
                      (SignalPeriodic ? TimerPeriodic : TimerRelative),
                      TriggerTime
                      );

      ASSERT_EFI_ERROR (Status);

      if (EFI_ERROR (Status)) {
        gBS->CloseEvent (Event);

        Event = NULL;
      }
    }
  }

  return Event;
}

// CreateNotifyTimerEvent 
EFI_EVENT
CreateNotifyTimerEvent (
  IN EFI_EVENT_NOTIFY  NotifyFunction,
  IN VOID              *NotifyContext,
  IN UINT64            TriggerTime,
  IN BOOLEAN           SignalPeriodic
  )
{
  return CreateTimerEvent (
           NotifyFunction,
           NotifyContext,
           TriggerTime,
           SignalPeriodic,
           EFI_TPL_NOTIFY
           );
}

// CancelEvent 
VOID
CancelEvent (
  IN EFI_EVENT  Event
  ) // sub_309
{
  EFI_STATUS Status;

  ASSERT (Event != NULL);

  Status = gBS->SetTimer (Event, TimerCancel, 0);

  ASSERT_EFI_ERROR (Status);

  if (!EFI_ERROR (Status)) {
    Status = gBS->CloseEvent (Event);

    ASSERT_EFI_ERROR (Status);
  }
}
