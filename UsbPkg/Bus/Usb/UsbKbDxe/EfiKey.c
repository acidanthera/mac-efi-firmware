/*++

Copyright (c) 2004 - 2009, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  EfiKey.c
    
Abstract:

  USB Keyboard Driver

Revision History

--*/

#include "EfiKey.h"
#include "Keyboard.h"

//
// Prototypes
// Driver model protocol interface
//
EFI_STATUS
EFIAPI
USBKeyboardDriverBindingEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EFIAPI
USBKeyboardDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
USBKeyboardDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
USBKeyboardDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  );

//
// Simple Text In Protocol Interface
//
STATIC
EFI_STATUS
EFIAPI
USBKeyboardReset (
  IN  EFI_SIMPLE_TEXT_INPUT_PROTOCOL  *This,
  IN  BOOLEAN                      ExtendedVerification
  );

STATIC
EFI_STATUS
EFIAPI
USBKeyboardReadKeyStroke (
  IN  EFI_SIMPLE_TEXT_INPUT_PROTOCOL   *This,
  OUT EFI_INPUT_KEY                 *Key
  );

STATIC
VOID
EFIAPI
USBKeyboardWaitForKey (
  IN  EFI_EVENT               Event,
  IN  VOID                    *Context
  );

//
// Keyboard Info Protocol Interface
//
STATIC
EFI_STATUS
EFIAPI
USBKeyboardInfoGetInfo (
  OUT UINT16  *IdVendor,
  OUT UINT16  *IdProduct,
  OUT UINT8   *CountryCode
  );

//
//  Helper functions
//
STATIC
EFI_STATUS
USBKeyboardCheckForKey (
  IN  USB_KB_DEV      *UsbKeyboardDevice
  );

//
// USB Keyboard Driver Global Variables
//
EFI_DRIVER_BINDING_PROTOCOL gUsbKeyboardDriverBinding = {
  USBKeyboardDriverBindingSupported,
  USBKeyboardDriverBindingStart,
  USBKeyboardDriverBindingStop,
  0x10,
  NULL,
  NULL
};

STATIC EFI_KEYBOARD_INFO_PROTOCOL gUsbKeyboardInfo = {
  USBKeyboardInfoGetInfo
};

STATIC UINT16 gUsbKeyboardInfoIdVendor = 0;

STATIC UINT8 gUsbKeyboardInfoCountryCode = 0;

STATIC UINT16 gUsbKeyboardInfoIdProduct = 0;

STATIC BOOLEAN gUsbKeyboardInfoInitialized = FALSE;

STATIC APPLE_PLATFORM_INFO_DATABASE_PROTOCOL *gPlatformInfo = NULL;

EFI_STATUS
EFIAPI
USBKeyboardDriverBindingEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++
  
  Routine Description:
    Driver Entry Point.
        
  Arguments:
    ImageHandle - EFI_HANDLE
    SystemTable - EFI_SYSTEM_TABLE
  Returns:
    EFI_STATUS
  
--*/       
{
  return EfiLibInstallAllDriverProtocols (
          ImageHandle,
          SystemTable,
          &gUsbKeyboardDriverBinding,
          ImageHandle,
          &gUsbKeyboardComponentName,
          NULL,
          NULL
          );
}


EFI_STATUS
EFIAPI
USBKeyboardDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
/*++
  
  Routine Description:
    Supported.
    
  Arguments:
    This          - EFI_DRIVER_BINDING_PROTOCOL
    Controller    - Controller handle
    RemainingDevicePath - EFI_DEVICE_PATH_PROTOCOL 
  Returns:
    EFI_STATUS
  
--*/ 
{
  EFI_STATUS          OpenStatus;
  EFI_USB_IO_PROTOCOL *UsbIo;
  EFI_STATUS          Status;

  //
  // Check if USB_IO protocol is attached on the controller handle.
  //
  OpenStatus = gBS->OpenProtocol (
                      Controller,
                      &gEfiUsbIoProtocolGuid,
                      (VOID **)&UsbIo,
                      This->DriverBindingHandle,
                      Controller,
                      EFI_OPEN_PROTOCOL_BY_DRIVER
                      );
  if (EFI_ERROR (OpenStatus)) {
    return OpenStatus;
  }
   
  //
  // Use the USB I/O protocol interface to check whether the Controller is
  // the Keyboard controller that can be managed by this driver.
  //
  Status = EFI_SUCCESS;

  if (!IsUSBKeyboard (UsbIo)) {
    Status = EFI_UNSUPPORTED;
  }

  gBS->CloseProtocol (
        Controller,
        &gEfiUsbIoProtocolGuid,
        This->DriverBindingHandle,
        Controller
        );

  return Status;
}

EFI_STATUS
EFIAPI
USBKeyboardDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
/*++
  
  Routine Description:
    Start.
  
  Arguments:
    This       - EFI_DRIVER_BINDING_PROTOCOL
    Controller - Controller handle
    RemainingDevicePath - EFI_DEVICE_PATH_PROTOCOL
  Returns:
    EFI_SUCCESS          - Success
    EFI_OUT_OF_RESOURCES - Can't allocate memory
    EFI_UNSUPPORTED      - The Start routine fail
--*/       
{ 
  EFI_STATUS                      Status;
  EFI_USB_IO_PROTOCOL             *UsbIo;
  USB_KB_DEV                      *UsbKeyboardDevice;
  UINT8                           EndpointNumber;
  EFI_USB_ENDPOINT_DESCRIPTOR     EndpointDescriptor;
  UINT8                           Index;
  UINT8                           EndpointAddr;
  UINT8                           PollingInterval;
  UINT8                           PacketSize;
  BOOLEAN                         Found;
  APPLE_KEY_MAP_DATABASE_PROTOCOL *KeyMapDb;
  EFI_DEVICE_PATH_PROTOCOL        **DevicePath;
  EFI_DEV_PATH_PTR                DevPath;
  UINT32                          Length;
  EFI_GUID                        NameGuid;
  APPLE_KEYBOARD_INFO             Data;
  UINT8                           InterfaceNum;
  EFI_USB_HID_DESCRIPTOR          HidDescriptor;
  UINT32                          Value;
  UINTN                           Shift;
  
  UsbKeyboardDevice = NULL;
  Found             = FALSE;

  // TODO: Some kind of stack protector code?

  //
  // Open USB_IO Protocol
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiUsbIoProtocolGuid,
                  (VOID **)&UsbIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Open KEY_MAP_DATABASE Protocol
  //
  Status = gBS->LocateProtocol (
                  &gAppleKeyMapDatabaseProtocolGuid,
                  NULL,
                  (VOID **)&KeyMapDb
                  );

  if (EFI_ERROR (Status)) {
    gBS->CloseProtocol (
           Controller,
           &gEfiUsbIoProtocolGuid,
           This->DriverBindingHandle,
           Controller
           );
    return Status;
  }

  UsbKeyboardDevice = AllocateZeroPool (sizeof (USB_KB_DEV));
  if (UsbKeyboardDevice == NULL) {
    gBS->CloseProtocol (
          Controller,
          &gEfiUsbIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );
    return EFI_OUT_OF_RESOURCES;
  }
  //
  // Get the Device Path Protocol on Controller's handle
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &UsbKeyboardDevice->DevicePath,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (EFI_ERROR (Status)) {
    gBS->FreePool (UsbKeyboardDevice);
    gBS->CloseProtocol (
          Controller,
          &gEfiUsbIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );
    return Status;
  }
  //
  // Report that the usb keyboard is being enabled
  //
  REPORT_STATUS_CODE_WITH_DEVICE_PATH (
    EFI_PROGRESS_CODE,
    (EFI_PERIPHERAL_KEYBOARD | EFI_P_PC_ENABLE),
    UsbKeyboardDevice->DevicePath
    );

  //
  // This is pretty close to keyboard detection, so log progress
  //
  REPORT_STATUS_CODE_WITH_DEVICE_PATH (
    EFI_PROGRESS_CODE,
    (EFI_PERIPHERAL_KEYBOARD | EFI_P_PC_PRESENCE_DETECT),
    UsbKeyboardDevice->DevicePath
    );

  //
  // Initialize UsbKeyboardDevice
  //
  UsbKeyboardDevice->UsbIo = UsbIo;

  UsbKeyboardDevice->KeyMapDb = KeyMapDb;

  Status = KeyMapDb->CreateKeyStrokesBuffer (
                       KeyMapDb,
                       6,
                       &UsbKeyboardDevice->KeyMapDbIndex
                       );

  if (EFI_ERROR (Status)) {
    gBS->FreePool (UsbKeyboardDevice);
    gBS->CloseProtocol (
          Controller,
          &gEfiUsbIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );
    return EFI_UNSUPPORTED;
  }

  //
  // Get device descriptor
  //
  Status = UsbIo->UsbGetDeviceDescriptor (
                   UsbIo,
                   &UsbKeyboardDevice->DeviceDescriptor
                   );

  if (EFI_ERROR (Status)) {
    //
    // Could not get device descriptor, then return unsupported.
    //
    gBS->FreePool (UsbKeyboardDevice);
    gBS->CloseProtocol (
          Controller,
          &gEfiUsbIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );
    return EFI_UNSUPPORTED;
  }

  //
  // Get interface & endpoint descriptor
  //
  Status = UsbIo->UsbGetInterfaceDescriptor (
                   UsbIo,
                   &UsbKeyboardDevice->InterfaceDescriptor
                   );

  if (EFI_ERROR (Status)) {
    //
    // Could not get interface descriptor, then return unsupported.
    //
    gBS->FreePool (UsbKeyboardDevice);
    gBS->CloseProtocol (
          Controller,
          &gEfiUsbIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );
    return EFI_UNSUPPORTED;
  }

  DevicePath = &UsbKeyboardDevice->DevicePath;

  EndpointNumber = UsbKeyboardDevice->InterfaceDescriptor.NumEndpoints;

  if (EndpointNumber != 0) {
    gBS->FreePool (UsbKeyboardDevice);
    gBS->CloseProtocol (
          Controller,
          &gEfiUsbIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );
    return EFI_UNSUPPORTED;
  }

  InterfaceNum = UsbKeyboardDevice->InterfaceDescriptor.InterfaceNumber;

  for (Index = 0; Index < EndpointNumber; Index++) {

    UsbIo->UsbGetEndpointDescriptor (
            UsbIo,
            Index,
            &EndpointDescriptor
            );

    // BUG: Status is not checked.

    if ((EndpointDescriptor.Attributes & 0x03) == 0x03) {
      //
      // We only care interrupt endpoint here
      //
      UsbKeyboardDevice->IntEndpointDescriptor  = EndpointDescriptor;
      Found = TRUE;
    }
  }

  if (!Found) {
    //
    // No interrupt endpoint found, then return unsupported.
    //
    gBS->FreePool (UsbKeyboardDevice);
    gBS->CloseProtocol (
          Controller,
          &gEfiUsbIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );
    return EFI_UNSUPPORTED;
  }

  UsbKeyboardDevice->Signature                  = USB_KB_DEV_SIGNATURE;
  UsbKeyboardDevice->SimpleInput.Reset          = USBKeyboardReset;
  UsbKeyboardDevice->SimpleInput.ReadKeyStroke  = USBKeyboardReadKeyStroke;

  Status = gBS->CreateEvent (
                  EVT_NOTIFY_WAIT,
                  TPL_NOTIFY,
                  USBKeyboardWaitForKey,
                  UsbKeyboardDevice,
                  &(UsbKeyboardDevice->SimpleInput.WaitForKey)
                  );

  if (EFI_ERROR (Status)) {
    gBS->FreePool (UsbKeyboardDevice);
    gBS->CloseProtocol (
          Controller,
          &gEfiUsbIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );
    return Status;
  }

  //
  // Prepare Keyboard Info Protocol installation
  //
  DevPath.DevPath = *DevicePath;
  NameGuid        = gAppleKeyboardPlatformInfoGuid;
  Shift           = 20;
  Value           = 0;

  for (Index = 0; Index < 20; Index++) {
    if ((DevicePathType (DevPath.DevPath) == HARDWARE_DEVICE_PATH)
     && (DevicePathSubType (DevPath.DevPath) == HW_PCI_DP)) {
      Value   = ((UINT32)DevPath.Pci->Function << 5);
      Value  |= (DevPath.Pci->Device & 0x1F);
      Value <<= 24;
    }

    if ((DevicePathType (DevPath.DevPath) == MESSAGING_DEVICE_PATH)
     && (DevicePathSubType (DevPath.DevPath) == MSG_USB_DP)) {
      Value |= (((UINT32)DevPath.Usb->ParentPortNumber + 1) << Shift);

      Shift -= 4;
    }

    if (IsDevicePathEnd (DevPath.DevPath)) {
      break;
    }
  }

  Length = sizeof (Data);

  if (gPlatformInfo == NULL) {
    // BUG: Status is not checked.

    gBS->LocateProtocol (
           &gApplePlatformInfoDatabaseProtocolGuid,
           NULL,
           (VOID **)&gPlatformInfo
           );
  }

  if (gPlatformInfo != NULL) {
    Status = gPlatformInfo->GetFirstDataSize (
                              gPlatformInfo,
                              &NameGuid,
                              &Length
                              );

    if (!EFI_ERROR (Status) && (Length == sizeof (Data))) {
      Status = gPlatformInfo->GetFirstData (
                                gPlatformInfo,
                                &NameGuid,
                                &Data,
                                &Length
                                );

      // BUG: Check with EFI_ERROR().

      if ((Status == EFI_SUCCESS)
       && (Data == Value)
       && !gUsbKeyboardInfoInitialized) {
        Status = UsbGetHidDescriptor (
                    UsbIo,
                    InterfaceNum,
                    &HidDescriptor
                    );

        if (Status == EFI_SUCCESS) {
          gUsbKeyboardInfoCountryCode = HidDescriptor.CountryCode;
        }

        gUsbKeyboardInfoIdVendor  = UsbKeyboardDevice->DeviceDescriptor.IdVendor;
        gUsbKeyboardInfoIdProduct = UsbKeyboardDevice->DeviceDescriptor.IdProduct;

        gUsbKeyboardInfoInitialized = TRUE;

        gBS->InstallProtocolInterface (
               NULL,
               &gEfiKeyboardInfoProtocolGuid,
               EFI_NATIVE_INTERFACE,
               (VOID *)&gUsbKeyboardInfo
               );
      }
    }
  }

  //
  // Install simple txt in protocol interface
  // for the usb keyboard device.
  // Usb keyboard is a hot plug device, and expected to work immediately
  // when plugging into system, so a HotPlugDeviceGuid is installed onto
  // the usb keyboard device handle, to distinguish it from other conventional
  // console devices.
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Controller,
                  &gEfiSimpleTextInProtocolGuid,
                  &UsbKeyboardDevice->SimpleInput,                 
                  &gEfiHotPlugDeviceGuid,
                  NULL,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    gBS->CloseEvent (UsbKeyboardDevice->SimpleInput.WaitForKey);
    gBS->FreePool (UsbKeyboardDevice);
    gBS->CloseProtocol (
          Controller,
          &gEfiUsbIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );
    return Status;
  }

  //
  // Reset USB Keyboard Device
  //
  Status = UsbKeyboardDevice->SimpleInput.Reset (
                                            &UsbKeyboardDevice->SimpleInput,
                                            TRUE
                                            );
  if (EFI_ERROR (Status)) {
    gBS->UninstallMultipleProtocolInterfaces (
           Controller,
           &gEfiSimpleTextInProtocolGuid,
           &UsbKeyboardDevice->SimpleInput,                          
           &gEfiHotPlugDeviceGuid,
           NULL,
           NULL
           );
    gBS->CloseEvent (UsbKeyboardDevice->SimpleInput.WaitForKey);
    gBS->FreePool (UsbKeyboardDevice);
    gBS->CloseProtocol (
          Controller,
          &gEfiUsbIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );
    return Status;
  }
  //
  // submit async interrupt transfer
  //
  EndpointAddr    = UsbKeyboardDevice->IntEndpointDescriptor.EndpointAddress;
  PollingInterval = UsbKeyboardDevice->IntEndpointDescriptor.Interval;
  PacketSize      = (UINT8) (UsbKeyboardDevice->IntEndpointDescriptor.MaxPacketSize);

  Status = UsbIo->UsbAsyncInterruptTransfer (
                    UsbIo,
                    EndpointAddr,
                    TRUE,
                    PollingInterval,
                    PacketSize,
                    KeyboardHandler,
                    UsbKeyboardDevice
                    );

  if (EFI_ERROR (Status)) {

    gBS->UninstallMultipleProtocolInterfaces (
           Controller,
           &gEfiSimpleTextInProtocolGuid,
           &UsbKeyboardDevice->SimpleInput,
           &gEfiHotPlugDeviceGuid,
           NULL,
           NULL
           );
    gBS->CloseEvent (UsbKeyboardDevice->SimpleInput.WaitForKey);
    gBS->FreePool (UsbKeyboardDevice);
    gBS->CloseProtocol (
          Controller,
          &gEfiUsbIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );
    return Status;
  }

  UsbKeyboardDevice->ControllerNameTable = NULL;
  AddUnicodeString (
    "eng",
    gUsbKeyboardComponentName.SupportedLanguages,
    &UsbKeyboardDevice->ControllerNameTable,
    L"Generic Usb Keyboard"
    );

  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
USBKeyboardDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  )
/*++
  
  Routine Description:
    Stop.
  
  Arguments:
    This              - EFI_DRIVER_BINDING_PROTOCOL
    Controller        - Controller handle
    NumberOfChildren  - Child handle number
    ChildHandleBuffer - Child handle buffer 
  Returns:
    EFI_SUCCESS       - Success
    EFI_UNSUPPORTED   - Can't support 
--*/       
{
  EFI_STATUS                  Status;
  EFI_SIMPLE_TEXT_INPUT_PROTOCOL *SimpleInput;
  USB_KB_DEV                  *UsbKeyboardDevice;

  // TODO: Some kind of stack protector code?

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiSimpleTextInProtocolGuid,
                  (VOID **)&SimpleInput,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }
  //
  // Get USB_KB_DEV instance.
  //
  UsbKeyboardDevice = USB_KB_DEV_FROM_THIS (SimpleInput);

  gBS->CloseProtocol (
        Controller,
        &gEfiSimpleTextInProtocolGuid,
        This->DriverBindingHandle,
        Controller
        );

  //
  // Uninstall the Asyn Interrupt Transfer from this device
  // will disable the key data input from this device
  //
  REPORT_STATUS_CODE_WITH_DEVICE_PATH (
    EFI_PROGRESS_CODE,
    (EFI_PERIPHERAL_KEYBOARD | EFI_P_PC_DISABLE),
    UsbKeyboardDevice->DevicePath
    );

  //
  // Destroy asynchronous interrupt transfer
  //
  UsbKeyboardDevice->UsbIo->UsbAsyncInterruptTransfer (
                              UsbKeyboardDevice->UsbIo,
                              UsbKeyboardDevice->IntEndpointDescriptor.EndpointAddress,
                              FALSE,
                              UsbKeyboardDevice->IntEndpointDescriptor.Interval,
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

  UsbKeyboardDevice->KeyMapDb->RemoveKeyStrokesBuffer (
                                 UsbKeyboardDevice->KeyMapDb,
                                 UsbKeyboardDevice->KeyMapDbIndex
                                 );

  Status = gBS->UninstallMultipleProtocolInterfaces (
                  Controller,
                  &gEfiSimpleTextInProtocolGuid,
                  &UsbKeyboardDevice->SimpleInput,
                  &gEfiHotPlugDeviceGuid,
                  NULL,
                  NULL
                  );
  //
  // free all the resources.
  //
  gBS->CloseEvent (UsbKeyboardDevice->RepeatTimer);
  gBS->CloseEvent (UsbKeyboardDevice->DelayedRecoveryEvent);
  gBS->CloseEvent ((UsbKeyboardDevice->SimpleInput).WaitForKey);    
  
  if (UsbKeyboardDevice->ControllerNameTable != NULL) {
    FreeUnicodeStringTable (UsbKeyboardDevice->ControllerNameTable);
  }

  gBS->FreePool (UsbKeyboardDevice);

  return Status;

}

STATIC
EFI_STATUS
USBKeyboardReadKeyStrokeWorker (
  IN  USB_KB_DEV                        *UsbKeyboardDevice,
  OUT EFI_KEY_DATA                      *KeyData
  )
/*++

  Routine Description:
    Reads the next keystroke from the input device. The WaitForKey Event can 
    be used to test for existance of a keystroke via WaitForEvent () call.

  Arguments:
    UsbKeyboardDevice     - Usb keyboard private structure.
    KeyData               - A pointer to a buffer that is filled in with the keystroke 
                            state data for the key that was pressed.

  Returns:
    EFI_SUCCESS           - The keystroke information was returned.
    EFI_NOT_READY         - There was no keystroke data availiable.
    EFI_DEVICE_ERROR      - The keystroke information was not returned due to 
                            hardware errors.
    EFI_INVALID_PARAMETER - KeyData is NULL.                        

--*/
{

  EFI_STATUS                        Status;
  UINT8                             KeyChar;

  if (KeyData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // if there is no saved ASCII byte, fetch it
  // by calling USBKeyboardCheckForKey().
  //
  if (UsbKeyboardDevice->CurKeyChar == 0) {
    Status = USBKeyboardCheckForKey (UsbKeyboardDevice);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  KeyData->Key.UnicodeChar = 0;
  KeyData->Key.ScanCode    = SCAN_NULL;

  KeyChar = UsbKeyboardDevice->CurKeyChar;

  UsbKeyboardDevice->CurKeyChar = 0;

  //
  // Translate saved ASCII byte into EFI_INPUT_KEY
  //
  Status = USBKeyCodeToEFIScanCode (UsbKeyboardDevice, KeyChar, &KeyData->Key);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
  
}
EFI_STATUS
EFIAPI
USBKeyboardReset (
  IN  EFI_SIMPLE_TEXT_INPUT_PROTOCOL   *This,
  IN  BOOLEAN                       ExtendedVerification
  )
/*++

  Routine Description:
    Implements EFI_SIMPLE_TEXT_INPUT_PROTOCOL.Reset() function.
  
  Arguments:
    This      The EFI_SIMPLE_TEXT_INPUT_PROTOCOL instance.
    ExtendedVerification
              Indicates that the driver may perform a more exhaustive
              verification operation of the device during reset.              
    
  Returns:  
    EFI_SUCCESS      - Success
    EFI_DEVICE_ERROR - Hardware Error
--*/      
{
  EFI_STATUS          Status;
  USB_KB_DEV          *UsbKeyboardDevice;

  UsbKeyboardDevice = USB_KB_DEV_FROM_THIS (This);

  REPORT_STATUS_CODE_WITH_DEVICE_PATH (
    EFI_PROGRESS_CODE,
    (EFI_PERIPHERAL_KEYBOARD | EFI_P_PC_RESET),
    UsbKeyboardDevice->DevicePath
    );

  //
  // Non Exhaustive reset:
  // only reset private data structures.
  //
  if (!ExtendedVerification) {
    //
    // Clear the key buffer of this Usb keyboard
    //
    REPORT_STATUS_CODE_WITH_DEVICE_PATH (
      EFI_PROGRESS_CODE,
      (EFI_PERIPHERAL_KEYBOARD | EFI_P_KEYBOARD_PC_CLEAR_BUFFER),
      UsbKeyboardDevice->DevicePath
      );

    InitUSBKeyBuffer (&(UsbKeyboardDevice->KeyboardBuffer));
    UsbKeyboardDevice->CurKeyChar = 0;
    return EFI_SUCCESS;
  }
  
  //
  // Exhaustive reset
  //
  Status                        = InitUSBKeyboard (UsbKeyboardDevice);
  UsbKeyboardDevice->CurKeyChar = 0;
  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
USBKeyboardReadKeyStroke (
  IN  EFI_SIMPLE_TEXT_INPUT_PROTOCOL   *This,
  OUT EFI_INPUT_KEY                 *Key
  )
/*++

  Routine Description:
    Implements EFI_SIMPLE_TEXT_INPUT_PROTOCOL.ReadKeyStroke() function.
  
  Arguments:
    This     The EFI_SIMPLE_TEXT_INPUT_PROTOCOL instance.
    Key      A pointer to a buffer that is filled in with the keystroke
             information for the key that was pressed.
    
  Returns:  
    EFI_SUCCESS - Success
--*/       
{
  USB_KB_DEV   *UsbKeyboardDevice;
  EFI_STATUS   Status;
  EFI_KEY_DATA KeyData;

  UsbKeyboardDevice = USB_KB_DEV_FROM_THIS (This);

  Status = USBKeyboardReadKeyStrokeWorker (UsbKeyboardDevice, &KeyData);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  CopyMem (Key, &KeyData.Key, sizeof (EFI_INPUT_KEY));

  return EFI_SUCCESS;

}

STATIC
VOID
EFIAPI
USBKeyboardWaitForKey (
  IN  EFI_EVENT               Event,
  IN  VOID                    *Context
  )
/*++

  Routine Description:
    Handler function for WaitForKey event.    
  
  Arguments:
    Event        Event to be signaled when a key is pressed.
    Context      Points to USB_KB_DEV instance.
    
  Returns:  
    VOID
--*/       
{
  USB_KB_DEV  *UsbKeyboardDevice;

  UsbKeyboardDevice = (USB_KB_DEV *) Context;

  if (UsbKeyboardDevice->CurKeyChar == 0) {

    if (EFI_ERROR (USBKeyboardCheckForKey (UsbKeyboardDevice))) {
      return ;
    }
  }
  //
  // If has key pending, signal the event.
  //
  gBS->SignalEvent (Event);
}


STATIC
EFI_STATUS
USBKeyboardCheckForKey (
  IN  USB_KB_DEV    *UsbKeyboardDevice
  )
/*++

  Routine Description:
    Check whether there is key pending.
  
  Arguments:
    UsbKeyboardDevice    The USB_KB_DEV instance.
    
  Returns:  
    EFI_SUCCESS  - Success
--*/       
{
  EFI_STATUS  Status;
  UINT8       KeyChar;

  //
  // Fetch raw data from the USB keyboard input,
  // and translate it into ASCII data.
  //
  Status = USBParseKey (UsbKeyboardDevice, &KeyChar);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  UsbKeyboardDevice->CurKeyChar = KeyChar;
  return EFI_SUCCESS;
}


STATIC
EFI_STATUS
EFIAPI
USBKeyboardInfoGetInfo (
  OUT UINT16  *IdVendor,
  OUT UINT16  *IdProduct,
  OUT UINT8   *CountryCode
  )
{
  // BUG: Arguments are not checked.

  *IdVendor    = gUsbKeyboardInfoIdVendor;
  *IdProduct   = gUsbKeyboardInfoIdProduct;
  *CountryCode = gUsbKeyboardInfoCountryCode;

  return EFI_SUCCESS;
}


