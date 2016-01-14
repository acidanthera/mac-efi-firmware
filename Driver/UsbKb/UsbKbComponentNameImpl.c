/** @file
  Copyright (C) 2004 - 2007, Intel Corporation<BR>
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php/

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include <AppleEfi.h>

#include "UsbKbLib.h"

#include "UsbKbComponentNameImpl.h"
#include "UsbKbDriverBindingImpl.h"

// mUsbKeyboardDriverNameTable
STATIC EFI_UNICODE_STRING_TABLE mUsbKeyboardDriverNameTable[] = {
  { LANGUAGE_CODE_ENGLISH, L"Usb Keyboard Driver" },
  { NULL,                  NULL                   }
};

// UsbKbComponentNameGetDriverName
/** Retrieves a Unicode string that is the user readable name of the EFI Driver.

  @param[in]  This        A pointer to the EFI_COMPONENT_NAME_PROTOCOL instance.
  @param[in]  Language    A pointer to a three character ISO 639-2 language identifier.
                          This is the language of the driver name that that the caller 
                          is requesting, and it must match one of the languages specified
                          in SupportedLanguages.  The number of languages supported by a 
                          driver is up to the driver writer.
  @param[out] DriverName  A pointer to the Unicode string to return.  This Unicode string
                          is the name of the driver specified by This in the language 
                          specified by Language.

  @retval EFI_SUCCESS            The Unicode string for the Driver specified by This
                                 and the language specified by Language was returned 
                                 in DriverName.
  @retval EFI_INVALID_PARAMETER  Language is NULL.
  @retval EFI_INVALID_PARAMETER  DriverName is NULL.
  @retval EFI_UNSUPPORTED        The driver specified by This does not support the
                                 language specified by Language.
**/
EFI_STATUS
EFIAPI
UsbKbComponentNameGetDriverName (
  IN  EFI_COMPONENT_NAME_PROTOCOL  *This,
  IN  CHAR8                        *Language,
  OUT CHAR16                       **DriverName
  )
{
  EFI_STATUS Status;

  ASSERT (This != NULL);
  ASSERT (Language != NULL);
  ASSERT (Language[0] != '\0');
  ASSERT (DriverName != NULL);
  ASSERT (Language[0] != L'\0');

  Status = EfiLibLookupUnicodeString (
             Language,
             gUsbKbComponentNameProtocol.SupportedLanguages,
             mUsbKeyboardDriverNameTable,
             DriverName
             );

  ASSERT (Status != EFI_INVALID_PARAMETER);

  return Status;
}

// UsbKbComponentNameGetControllerName
/** Retrieves a Unicode string that is the user readable name of the controller
    that is being managed by an EFI Driver.

  @param[in]  This              A pointer to the EFI_COMPONENT_NAME_PROTOCOL instance.
  @param[in]  ControllerHandle  The handle of a controller that the driver specified by 
                                This is managing.  This handle specifies the controller 
                                whose name is to be returned.
  @param[in]  ChildHandle       The handle of the child controller to retrieve the name 
                                of.  This is an optional parameter that may be NULL.  It 
                                will be NULL for device drivers.  It will also be NULL 
                                for a bus drivers that wish to retrieve the name of the 
                                bus controller.  It will not be NULL for a bus driver 
                                that wishes to retrieve the name of a child controller.
  @param[in]  Language          A pointer to a three character ISO 639-2 language 
                                identifier.  This is the language of the controller name 
                                that that the caller is requesting, and it must match one
                                of the languages specified in SupportedLanguages.  The 
                                number of languages supported by a driver is up to the 
                                driver writer.
  @param[out] ControllerName    A pointer to the Unicode string to return.  This Unicode
                                string is the name of the controller specified by 
                                ControllerHandle and ChildHandle in the language specified
                                by Language from the point of view of the driver specified
                                by This. 

  @retval EFI_SUCCESS            The Unicode string for the user readable name in the 
                                 language specified by Language for the driver 
                                 specified by This was returned in DriverName.
  @retval EFI_INVALID_PARAMETER  ControllerHandle is not a valid EFI_HANDLE.
  @retval EFI_INVALID_PARAMETER  ChildHandle is not NULL and it is not a valid EFI_HANDLE.
  @retval EFI_INVALID_PARAMETER  Language is NULL.
  @retval EFI_INVALID_PARAMETER  ControllerName is NULL.
  @retval EFI_UNSUPPORTED        The driver specified by This is not currently managing
                                 the controller specified by ControllerHandle and 
                                 ChildHandle.
  @retval EFI_UNSUPPORTED        The driver specified by This does not support the 
                                 language specified by Language.
**/
EFI_STATUS
EFIAPI
UsbKbComponentNameGetControllerName (
  IN  EFI_COMPONENT_NAME_PROTOCOL  *This,
  IN  EFI_HANDLE                   ControllerHandle,
  IN  EFI_HANDLE                   ChildHandle, OPTIONAL
  IN  CHAR8                        *Language,
  OUT CHAR16                       **ControllerName
  )
{
  EFI_STATUS                  Status;

  USB_KB_DEV                  *UsbKbDev;
  EFI_SIMPLE_TEXT_IN_PROTOCOL *SimpleTextIn;
  EFI_USB_IO_PROTOCOL         *UsbIoProtocol;

  ASSERT (This != NULL);
  ASSERT (ControllerHandle != NULL);
  ASSERT (ChildHandle == NULL);
  ASSERT (Language != NULL);
  ASSERT (ControllerName != NULL);

  Status = EFI_UNSUPPORTED;

  // This is a device driver, so ChildHandle must be NULL.
  if (ChildHandle == NULL) {
    // Check Controller's handle
    Status = gBS->OpenProtocol (
                    ControllerHandle,
                    &gEfiUsbIoProtocolGuid,
                    (VOID **)&UsbIoProtocol,
                    gUsbKbDriverBindingProtocol.DriverBindingHandle,
                    ControllerHandle,
                    EFI_OPEN_PROTOCOL_BY_DRIVER
                    );

    ASSERT_EFI_ERROR (Status);

    if (!EFI_ERROR (Status)) {
      Status = gBS->CloseProtocol (
                      ControllerHandle,
                      &gEfiUsbIoProtocolGuid,
                      gUsbKbDriverBindingProtocol.DriverBindingHandle,
                      ControllerHandle
                      );

      ASSERT_EFI_ERROR (Status);

      Status = EFI_UNSUPPORTED;
    } else if (Status == EFI_ALREADY_STARTED) {
      Status = EFI_UNSUPPORTED;
    } else {
      // Get the device context
      Status = gBS->OpenProtocol (
                      ControllerHandle,
                      &gEfiSimpleTextInProtocolGuid,
                      (VOID **)&SimpleTextIn,
                      gUsbKbDriverBindingProtocol.DriverBindingHandle,
                      ControllerHandle,
                      EFI_OPEN_PROTOCOL_GET_PROTOCOL
                      );

      ASSERT_EFI_ERROR (Status);

      if (!EFI_ERROR (Status)) {
        UsbKbDev = USB_KB_DEV_FROM_THIS (SimpleTextIn);
        Status   = EfiLibLookupUnicodeString (
                     Language,
                     gUsbKbComponentNameProtocol.SupportedLanguages,
                     UsbKbDev->ControllerNameTable,
                     ControllerName
                     );

        ASSERT (Status != EFI_INVALID_PARAMETER);
      }
    }
  }

  return Status;
}
