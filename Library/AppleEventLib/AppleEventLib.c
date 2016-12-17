/** @file
  Copyright (C) 2015, CupertinoNet.  All rights reserved.<BR>

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
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

// CreateNotifyEvent 
EFI_EVENT
CreateNotifyEvent (
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
