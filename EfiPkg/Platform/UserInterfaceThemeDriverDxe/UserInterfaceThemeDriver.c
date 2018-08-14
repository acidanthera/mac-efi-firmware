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
#include <PiDxe.h>

#include <Guid/AppleVariable.h>

#include <Protocol/AppleSmcIo.h>
#include <Protocol/UserInterfaceTheme.h>

#include <Library/BaseMemoryLib.h>
#include <Library/HobLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

// DEFAULT_BACKGROUND_COLOR
#define DEFAULT_BACKGROUND_COLOR  0

// InternalGetBackgroundColor
STATIC
EFI_STATUS
EFIAPI
InternalGetBackgroundColor (
  IN OUT UINT32  *BackgroundColor
  );

// mUserInterfaceThemeProtocol
STATIC EFI_USER_INTERFACE_THEME_PROTOCOL mUserInterfaceThemeProtocol = {
  USER_THEME_INTERFACE_PROTOCOL_REVISION,
  InternalGetBackgroundColor
};

// mDefaultBackgroundColorPresent
STATIC BOOLEAN mDefaultBackgroundColorPresent = FALSE;

// mLowBatteryUiBackgroundColor
STATIC UINT32 mLowBatteryUiBackgroundColor = DEFAULT_BACKGROUND_COLOR;

// mCriticalBatteryUi
STATIC BOOLEAN mCriticalBatteryUi = FALSE;

// mBackgroundColor
STATIC UINT32 mBackgroundColor = DEFAULT_BACKGROUND_COLOR;

// IsCriticalBatteryUi
STATIC
BOOLEAN
IsCriticalBatteryUi (
  VOID
  )
{
  BOOLEAN               Result;

  EFI_STATUS            Status;
  APPLE_SMC_IO_PROTOCOL *SmcIo;
  SMC_DATA              Value;
  UINTN                 DataSize;
  UINT8                 CriticalBatteryUi;

  Result = FALSE;

  Status = gBS->LocateProtocol (
                  &gAppleSmcIoProtocolGuid,
                  NULL,
                  (VOID **)&SmcIo
                  );

  if (!EFI_ERROR (Status)) {
    Status = SmcIo->SmcReadValue (
                      SmcIo,
                      SMC_MAKE_IDENTIFIER ('B', 'B', 'I', 'F'),
                      sizeof (Value),
                      &Value
                      );

    Result = TRUE;

    if (EFI_ERROR (Status) || ((Value & BIT0) == 0)) {
      DataSize = 0;

      Result = FALSE;

      Status = gRT->GetVariable (
                      L"critical-battery-ui",
                      &gAppleVendorVariableGuid,
                      0,
                      &DataSize,
                      &CriticalBatteryUi
                      );

      if (!EFI_ERROR (Status)) {
        Result = ((CriticalBatteryUi & 0x0F) != 0);
      }
    }
  }

  return Result;
}

// InternalGetBackgroundColor
STATIC
EFI_STATUS
EFIAPI
InternalGetBackgroundColor (
  IN OUT UINT32  *BackgroundColor
  )
{
  EFI_STATUS Status;

  UINTN      DataSize;

  Status = EFI_INVALID_PARAMETER;

  if (BackgroundColor != NULL) {
    if (mCriticalBatteryUi && IsCriticalBatteryUi ()) {
      CopyMem (
        (VOID *)BackgroundColor,
        (VOID *)&mLowBatteryUiBackgroundColor,
        sizeof (*BackgroundColor)
        );

      Status = EFI_SUCCESS;
    } else {
      CopyMem (
        (VOID *)BackgroundColor,
        (VOID *)&mBackgroundColor,
        sizeof (*BackgroundColor)
        );

      if (!mDefaultBackgroundColorPresent) {
        DataSize = 0;
        Status   = gRT->GetVariable (
                          L"BootUIPrefix",
                          &gAppleVendorVariableGuid,
                          0,
                          &DataSize,
                          NULL
                          );

        if (Status == EFI_BUFFER_TOO_SMALL) {
          ZeroMem (BackgroundColor, sizeof (*BackgroundColor));

          Status = EFI_SUCCESS;
        }
      }
    }
  }

  return Status;
}

// UserInterfaceThemeDriverMain
EFI_STATUS
EFIAPI
UserInterfaceThemeDriverMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_BOOT_MODE BootMode;
  UINTN         DataSize;
  EFI_STATUS    Status;
  UINT32        BackgroundColor;
  EFI_HANDLE    Handle;

  mDefaultBackgroundColorPresent = FALSE;

  mLowBatteryUiBackgroundColor = DEFAULT_BACKGROUND_COLOR;
  mCriticalBatteryUi           = IsCriticalBatteryUi ();

  BootMode = GetBootModeHob ();

  if (BootMode == 0x81) {
    BackgroundColor = DEFAULT_BACKGROUND_COLOR;
  } else {
    DataSize = sizeof (BackgroundColor);
    Status   = gRT->GetVariable (
                      L"DefaultBackgroundColor",
                      &gAppleVendorVariableGuid,
                      0,
                      &DataSize,
                      &BackgroundColor
                      );

    if (!EFI_ERROR (Status)) {
      mDefaultBackgroundColorPresent = TRUE;
    } else {
      BackgroundColor = DEFAULT_BACKGROUND_COLOR;
    }
  }

  CopyMem (
    (VOID *)&mBackgroundColor,
    (VOID *)&BackgroundColor,
    sizeof (BackgroundColor)
    );

  Handle = NULL;
  
  return gBS->InstallProtocolInterface (
                &Handle,
                &gEfiUserInterfaceThemeProtocolGuid,
                EFI_NATIVE_INTERFACE,
                &mUserInterfaceThemeProtocol
                );
}
