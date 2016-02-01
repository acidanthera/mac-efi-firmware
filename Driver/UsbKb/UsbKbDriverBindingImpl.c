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

#include EFI_GUID_DEFINITION (HotPlugDevice)

#include EFI_PROTOCOL_CONSUMER (UsbIo)
#include APPLE_PROTOCOL_PRODUCER (ApplePlatformInfoDatabase)
#include "UsbKbComponentNameImpl.h"
#include "UsbKbDriverBindingImpl.h"
#include "UsbKbSimpleTextInputImpl.h"

#include <Library/UsbDxeLib.h>
#include "UsbKbLib.h"

// APPLE_PLATFORM_INFO_KEYBOARD_GUID
#define APPLE_PLATFORM_INFO_KEYBOARD_GUID  \
  { 0x51871CB9, 0xE25D, 0x44B4, { 0x96, 0x99, 0x0E, 0xE8, 0x64, 0x4C, 0xED, 0x69 } }

// gApplePlatformInfoKeyboardGuid
STATIC EFI_GUID gApplePlatformInfoKeyboardGuid = APPLE_PLATFORM_INFO_KEYBOARD_GUID;

// mPlatformInfo
STATIC APPLE_PLATFORM_INFO_DATABASE_PROTOCOL *mPlatformInfo = NULL;

// mIdsInitialized
STATIC BOOLEAN mIdsInitialized = FALSE;

// UsbKbBindingSupported
/** Supported.

  @param[in] This                 EFI_DRIVER_BINDING_PROTOCOL
  @param[in] Controller           Controller handle
  @param[in] RemainingDevicePath  EFI_DEVICE_PATH_PROTOCOL

  @retval EFI_STATUS  Success.
**/
EFI_STATUS
EFIAPI
UsbKbBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
{
  EFI_STATUS          Status;

  EFI_STATUS          CloseStatus;
  EFI_USB_IO_PROTOCOL *UsbIo;

  ASSERT (This != NULL);
  ASSERT (Controller != NULL);
  ASSERT (RemainingDevicePath != NULL);

  // Check if USB_IO protocol is attached on the controller handle.
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiUsbIoProtocolGuid,
                  (VOID **)&UsbIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );

  if (!EFI_ERROR (Status)) {
    // Use the USB I/O protocol interface to check whether the Controller is
    // the Keyboard controller that can be managed by this driver.
    Status = EFI_SUCCESS;

    ASSERT (IsUsbKeyboard (UsbIo));

    if (!IsUsbKeyboard (UsbIo)) {
      Status = EFI_UNSUPPORTED;
    }

    CloseStatus = gBS->CloseProtocol (
                         Controller,
                         &gEfiUsbIoProtocolGuid,
                         This->DriverBindingHandle,
                         Controller
                         );

    ASSERT_EFI_ERROR (CloseStatus);
  }

  return Status;
}

// UsbKbBindingStart
/** Start.

  @param[in] This                 EFI_DRIVER_BINDING_PROTOCOL
  @param[in] Controller           Controller handle
  @param[in] RemainingDevicePath  EFI_DEVICE_PATH_PROTOCOL

  @retval EFI_SUCCESS           Success
  @retval EFI_OUT_OF_RESOURCES  Can't allocate memory
  @retval EFI_UNSUPPORTED       The Start routine fail
**/
EFI_STATUS
EFIAPI
UsbKbBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
{
  EFI_STATUS                      Status;

  EFI_USB_IO_PROTOCOL             *UsbIo;
  USB_KB_DEV                      *UsbKbDev;
  UINT8                           EndpointNumber;
  EFI_USB_ENDPOINT_DESCRIPTOR     EndpointDescriptor;
  UINT8                           Index;
  UINT8                           EndpointAddress;
  UINT8                           PollingInterval;
  UINT8                           PacketSize;
  BOOLEAN                         Found;
  APPLE_KEY_MAP_DATABASE_PROTOCOL *AppleKeyMapDb;
  EFI_DEV_PATH_PTR                DevicePath;
  UINTN                           Length;
  EFI_GUID                        NameGuid;
  UINT32                          Data;
  UINT8                           InterfaceNum;
  EFI_USB_HID_DESCRIPTOR          HidDescriptor;
  UINT32                          Value;
  UINTN                           Shift;

  ASSERT (This != NULL);
  ASSERT (Controller != NULL);
  ASSERT (RemainingDevicePath != NULL);

  UsbKbDev = NULL;
  Found    = FALSE;

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiUsbIoProtocolGuid,
                  (VOID **)&UsbIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );

  if (!EFI_ERROR (Status)) {
    Status = gBS->LocateProtocol (
      &gAppleKeyMapDatabaseProtocolGuid,
      NULL,
      (VOID **)&AppleKeyMapDb
      );

    if (EFI_ERROR (Status)) {
      gBS->CloseProtocol (
             Controller,
             &gEfiUsbIoProtocolGuid,
             This->DriverBindingHandle,
             Controller
             );
    } else {
      UsbKbDev = EfiLibAllocateZeroPool (sizeof (*UsbKbDev));
      
      if (UsbKbDev == NULL) {
        gBS->CloseProtocol (
               Controller,
               &gEfiUsbIoProtocolGuid,
               This->DriverBindingHandle,
               Controller
               );

        Status = EFI_OUT_OF_RESOURCES;
      } else {
        // Get the Device Path Protocol on Controller's handle
        Status = gBS->OpenProtocol (
                        Controller,
                        &gEfiDevicePathProtocolGuid,
                        (VOID **)&UsbKbDev->DevicePath,
                        This->DriverBindingHandle,
                        Controller,
                        EFI_OPEN_PROTOCOL_GET_PROTOCOL
                        );

        if (EFI_ERROR (Status)) {
          gBS->FreePool ((VOID *)UsbKbDev);
          gBS->CloseProtocol (
                 Controller,
                 &gEfiUsbIoProtocolGuid,
                 This->DriverBindingHandle,
                 Controller
                 );
        } else {
          // Report that the usb keyboard is being enabled
          KbdReportStatusCode (
            UsbKbDev->DevicePath,
            EFI_PROGRESS_CODE,
            (EFI_PERIPHERAL_KEYBOARD | EFI_P_PC_ENABLE)
            );

          // This is pretty close to keyboard detection, so log progress
          KbdReportStatusCode (
            UsbKbDev->DevicePath,
            EFI_PROGRESS_CODE,
            (EFI_PERIPHERAL_KEYBOARD | EFI_P_PC_PRESENCE_DETECT)
            );


          // Initialize UsbKbDev

          UsbKbDev->UsbIo    = UsbIo;
          UsbKbDev->KeyMapDb = AppleKeyMapDb;
          Status             = AppleKeyMapDb->CreateKeyStrokesBuffer (
                                                AppleKeyMapDb,
                                                6,
                                                &UsbKbDev->KeyMapDbIndex
                                                );

          if (EFI_ERROR (Status)) {
            gBS->FreePool ((VOID *)UsbKbDev);
            gBS->CloseProtocol (
                   Controller,
                   &gEfiUsbIoProtocolGuid,
                   This->DriverBindingHandle,
                   Controller
                   );
          } else {
            Status = UsbIo->UsbGetDeviceDescriptor (
                              UsbIo,
                              &UsbKbDev->DeviceDescriptor
                              );

            if (EFI_ERROR (Status)) {
              Status = EFI_UNSUPPORTED;
            } else {
              // Get interface & endpoint descriptor
              Status = UsbIo->UsbGetInterfaceDescriptor (
                                UsbIo,
                                &UsbKbDev->InterfaceDescriptor
                                );

              if (EFI_ERROR (Status)) {
                Status = EFI_UNSUPPORTED;
              } else {
                EndpointNumber = UsbKbDev->InterfaceDescriptor.NumEndpoints;

                if (EndpointNumber == 0) {
                  gBS->FreePool ((VOID *)UsbKbDev);
                  gBS->CloseProtocol (
                         Controller,
                         &gEfiUsbIoProtocolGuid,
                         This->DriverBindingHandle,
                         Controller
                         );

                  Status = EFI_UNSUPPORTED;
                } else {
                  InterfaceNum = UsbKbDev->InterfaceDescriptor.InterfaceNumber;

                  for (Index = 0; Index < EndpointNumber; ++Index) {
                    UsbIo->UsbGetEndpointDescriptor (
                             UsbIo,
                             Index,
                             &EndpointDescriptor
                             );

                    if ((EndpointDescriptor.Attributes & 0x03) == 0x03) {
                      // We only care interrupt endpoint here
                      UsbKbDev->EndpointDescriptor = EndpointDescriptor;
                      Found                           = TRUE;
                    }
                  }

                  if (!Found) {
                    // No interrupt endpoint found, then return unsupported.

                    gBS->FreePool ((VOID *)UsbKbDev);
                    gBS->CloseProtocol (
                      Controller,
                      &gEfiUsbIoProtocolGuid,
                      This->DriverBindingHandle,
                      Controller
                      );

                    Status = EFI_UNSUPPORTED;
                  } else {
                    UsbKbDev->Signature                 = USB_KB_DEV_SIGNATURE;
                    UsbKbDev->SimpleInput.Reset         = UsbKbReset;
                    UsbKbDev->SimpleInput.ReadKeyStroke = UsbKbReadKeyStroke;

                    Status = gBS->CreateEvent (
                                    EFI_EVENT_NOTIFY_WAIT,
                                    EFI_TPL_NOTIFY,
                                    UsbKbWaitForKey,
                                    UsbKbDev,
                                    &(UsbKbDev->SimpleInput.WaitForKey)
                                    );

                    if (EFI_ERROR (Status)) {
                      gBS->FreePool ((VOID *)UsbKbDev);
                      gBS->CloseProtocol (
                             Controller,
                             &gEfiUsbIoProtocolGuid,
                             This->DriverBindingHandle,
                             Controller
                             );
                    } else {
                      DevicePath.DevPath = UsbKbDev->DevicePath;
                      NameGuid           = gApplePlatformInfoKeyboardGuid;
                      Shift              = 20;
                      Value              = 0;
                      Index              = 1;

                      do {
                        if ((DevicePathType (DevicePath.DevPath) == HARDWARE_DEVICE_PATH)
                         && (DevicePathSubType (DevicePath.DevPath) == HW_PCI_DP)) {
                          Value = ((
                                    (UINT32)(DevicePath.Pci->Device & 0x1F)
                                      | (DevicePath.Pci->Function << 5)
                                    ) << 24);
                        }

                        if ((DevicePathType (DevicePath.DevPath) == MESSAGING_DEVICE_PATH)
                         && (DevicePathSubType (DevicePath.DevPath) == MSG_USB_DP)) {
                          Value |= ((
                                     (UINT32)DevicePath.Usb->ParentPortNumber + 1
                                     ) << Shift);
                          Shift -= 4;
                        }
                      } while (!IsDevicePathEnd (DevicePath.DevPath)
                            && ((Index++) < 20));

                      Length = 4;

                      if (mPlatformInfo == NULL) {
                        gBS->LocateProtocol (
                               &gApplePlatformInfoDatabaseProtocolGuid,
                               NULL,
                               (VOID **)&mPlatformInfo
                               );

                        if (mPlatformInfo == NULL) {
                          goto Skip;
                        }
                      }

                      Status = mPlatformInfo->GetFirstDataSize (
                                                mPlatformInfo,
                                                &NameGuid,
                                                &Length
                                                );

                      if (!EFI_ERROR (Status) && (Length == sizeof (Data))) {
                        Status = mPlatformInfo->GetFirstData (
                                                  mPlatformInfo,
                                                  &NameGuid,
                                                  &Data,
                                                  &Length
                                                  );

                        if ((Status == EFI_SUCCESS)
                         && (Value == Data)
                         && !mIdsInitialized) {
                          Status = UsbGetHidDescriptor (
                                     UsbIo,
                                     InterfaceNum,
                                     &HidDescriptor
                                     );

                          if (Status == EFI_SUCCESS) {
                            gKeyboardInfoCountryCode = HidDescriptor.CountryCode;
                          }

                          gKeyboardInfoIdVendor  = UsbKbDev->DeviceDescriptor.IdVendor;
                          gKeyboardInfoIdProduct = UsbKbDev->DeviceDescriptor.IdProduct;
                          mIdsInitialized        = TRUE;

                          gBS->InstallProtocolInterface (
                            NULL,
                            &gEfiKeyboardInfoProtocolGuid,
                            EFI_NATIVE_INTERFACE,
                            (VOID *)&gUsbKbKeyboardInformationProtocol
                            );
                        }
                      }

                    Skip:
                      // Install simple txt in protocol interface
                      // for the usb keyboard device.
                      // Usb keyboard is a hot plug device, and expected to
                      // work immediately when plugging into system, so a
                      // HotPlugDeviceGuid is installed onto the usb keyboard
                      // device handle, to distinguish it from other
                      // conventional console devices.

                      Status = gBS->InstallMultipleProtocolInterfaces (
                                      &Controller,
                                      &gEfiSimpleTextInProtocolGuid,
                                      &UsbKbDev->SimpleInput,
                                      &gEfiHotPlugDeviceGuid,
                                      NULL,
                                      NULL
                                      );

                      if (EFI_ERROR (Status)) {
                        gBS->CloseEvent (UsbKbDev->SimpleInput.WaitForKey);
                        gBS->FreePool ((VOID *)UsbKbDev);
                        gBS->CloseProtocol (
                               Controller,
                               &gEfiUsbIoProtocolGuid,
                               This->DriverBindingHandle,
                               Controller
                               );
                      } else {
                        Status = UsbKbDev->SimpleInput.Reset (
                                                         &UsbKbDev->SimpleInput,
                                                         TRUE
                                                         );

                        if (EFI_ERROR (Status)) {
                          gBS->UninstallMultipleProtocolInterfaces (
                                 Controller,
                                 &gEfiSimpleTextInProtocolGuid,
                                 &UsbKbDev->SimpleInput,
                                 &gEfiHotPlugDeviceGuid,
                                 NULL,
                                 NULL
                                 );

                          gBS->CloseEvent (UsbKbDev->SimpleInput.WaitForKey);
                          gBS->FreePool ((VOID *)UsbKbDev);
                          gBS->CloseProtocol (
                                 Controller,
                                 &gEfiUsbIoProtocolGuid,
                                 This->DriverBindingHandle,
                                 Controller
                                 );
                        } else {
                          // submit async interrupt transfer

                          EndpointAddress = UsbKbDev->EndpointDescriptor.EndpointAddress;
                          PollingInterval = UsbKbDev->EndpointDescriptor.Interval;
                          PacketSize      = (UINT8)UsbKbDev->EndpointDescriptor.MaxPacketSize;
                          Status          = UsbIo->UsbAsyncInterruptTransfer (
                                                     UsbIo,
                                                     EndpointAddress,
                                                     TRUE,
                                                     PollingInterval,
                                                     PacketSize,
                                                     KeyboardHandler,
                                                     UsbKbDev
                                                     );

                          if (EFI_ERROR (Status)) {
                            gBS->UninstallMultipleProtocolInterfaces (
                                   Controller,
                                   &gEfiSimpleTextInProtocolGuid,
                                   &UsbKbDev->SimpleInput,
                                   &gEfiHotPlugDeviceGuid,
                                   NULL,
                                   NULL
                                   );

                            gBS->CloseEvent (UsbKbDev->SimpleInput.WaitForKey);
                            gBS->FreePool ((VOID *)UsbKbDev);
                            gBS->CloseProtocol (
                                   Controller,
                                   &gEfiUsbIoProtocolGuid,
                                   This->DriverBindingHandle,
                                   Controller
                                   );
                          } else {
                            UsbKbDev->ControllerNameTable = NULL;

                            EfiLibAddUnicodeString (
                              LANGUAGE_CODE_ENGLISH,
                              gUsbKbComponentNameProtocol.SupportedLanguages,
                              &UsbKbDev->ControllerNameTable,
                              L"Generic Usb Keyboard"
                              );

                            Status = EFI_SUCCESS;
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  return Status;
}
// UsbKbBindingStop
/** Stop.

  @param[in] This               EFI_DRIVER_BINDING_PROTOCOL
  @param[in] Controller         Controller handle
  @param[in] NumberOfChildren   Child handle number
  @param[in] ChildHandleBuffer  Child handle Buffer

  @retval EFI_SUCCESS      Success
  @retval EFI_UNSUPPORTED  Can't support
**/
EFI_STATUS
EFIAPI
UsbKbBindingStop (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN UINTN                        NumberOfChildren,
  IN EFI_HANDLE                   *ChildHandleBuffer
  )
{
  EFI_STATUS                  Status;

  EFI_SIMPLE_TEXT_IN_PROTOCOL *SimpleInput;
  USB_KB_DEV                  *UsbKbDev;

  ASSERT (This != NULL);
  ASSERT (Controller != NULL);
  ASSERT (NumberOfChildren > 0);
  ASSERT (ChildHandleBuffer != NULL);

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiSimpleTextInProtocolGuid,
                  (VOID **)&SimpleInput,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  ASSERT_EFI_ERROR (Status);

  Status = EFI_UNSUPPORTED;

  if (!EFI_ERROR (Status)) {
    // Get USB_KB_DEV instance.
    UsbKbDev = USB_KB_DEV_FROM_THIS (SimpleInput);

    gBS->CloseProtocol (
           Controller,
           &gEfiSimpleTextInProtocolGuid,
           This->DriverBindingHandle,
           Controller
           );

    // Uninstall the Asyn Interrupt Transfer from this device
    // will disable the key data input from this device
    KbdReportStatusCode (
      UsbKbDev->DevicePath,
      EFI_PROGRESS_CODE,
      (EFI_PERIPHERAL_KEYBOARD | EFI_P_PC_DISABLE)
      );

    // Destroy asynchronous interrupt transfer
    UsbKbDev->UsbIo->UsbAsyncInterruptTransfer (
                                UsbKbDev->UsbIo,
                                UsbKbDev->EndpointDescriptor.EndpointAddress,
                                FALSE,
                                UsbKbDev->EndpointDescriptor.Interval,
                                0,
                                NULL,
                                NULL
                                );

    gBS->CloseProtocol (
           Controller,
           &gEfiUsbIoProtocolGuid,
           This->DriverBindingHandle,
           Controller
           );

    UsbKbDev->KeyMapDb->RemoveKeyStrokesBuffer (
                          UsbKbDev->KeyMapDb,
                          UsbKbDev->KeyMapDbIndex
                          );

    Status = gBS->UninstallMultipleProtocolInterfaces (
                    Controller,
                    &gEfiSimpleTextInProtocolGuid,
                    &UsbKbDev->SimpleInput,
                    &gEfiHotPlugDeviceGuid,
                    NULL,
                    NULL
                    );

    // free all the resources.

    gBS->CloseEvent (UsbKbDev->RepeatTimer);
    gBS->CloseEvent (UsbKbDev->DelayedRecoveryEvent);
    gBS->CloseEvent ((UsbKbDev->SimpleInput).WaitForKey);

    if (UsbKbDev->ControllerNameTable != NULL) {
      EfiLibFreeUnicodeStringTable (UsbKbDev->ControllerNameTable);
    }

    gBS->FreePool ((VOID *)UsbKbDev);
  }

  return Status;
}
