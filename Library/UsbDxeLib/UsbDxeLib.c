/** @file
  Common Dxe Libarary for USB
  HID class request

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

#include <Library/AppleDriverLib.h>
#include <Library/UsbDxeLib.h>

// UsbGetDescriptor
/** Usb Get Descriptor

  @param[in]  UsbIo             EFI_USB_IO_PROTOCOL
  @param[in]  Value             Device Request Value
  @param[in]  Index             Device Request Index 
  @param[in]  DescriptorLength  Descriptor Length
  @param[out] Descriptor        Descriptor Buffer to contain result
  @param[out] Status            Transfer Status

  @retval EFI_INVALID_PARAMETER  Parameter is error
  @retval EFI_SUCCESS            Success
  @retval EFI_TIMEOUT            Device has no response
**/
EFI_STATUS
UsbGetDescriptor (
  IN  EFI_USB_IO_PROTOCOL  *UsbIo,
  IN  UINT16               Value,
  IN  UINT16               Index,
  IN  UINT16               DescriptorLength,
  OUT VOID                 *Descriptor,
  OUT UINT32               *Status
  )
{
  EFI_STATUS             EfiStatus;

  EFI_USB_DEVICE_REQUEST DeviceRequest;

  ASSERT (UsbIo != NULL);
  ASSERT (DescriptorLength > 0);
  ASSERT (Descriptor != NULL);

  EfiStatus = EFI_INVALID_PARAMETER;

  if (UsbIo != NULL) {
    EfiZeroMem (&DeviceRequest, sizeof (DeviceRequest));

    DeviceRequest.RequestType = USB_DEV_GET_DESCRIPTOR_REQ_TYPE;
    DeviceRequest.Request     = USB_REQ_GET_DESCRIPTOR;
    DeviceRequest.Value       = Value;
    DeviceRequest.Index       = Index;
    DeviceRequest.Length      = DescriptorLength;
    EfiStatus                 = UsbIo->UsbControlTransfer (
                                         UsbIo,
                                         &DeviceRequest,
                                         EfiUsbDataIn,
                                         TIMEOUT_VALUE,
                                         Descriptor,
                                         DescriptorLength,
                                         Status
                                         );
  }

  return EfiStatus;
}

// UsbSetDescriptor
/** Usb Set Descriptor

  @param[in]  UsbIo             EFI_USB_IO_PROTOCOL
  @param[in]  Value             Device Request Value
  @param[in]  Index             Device Request Index 
  @param[in]  DescriptorLength  Descriptor Length
  @param[in]  Descriptor        Descriptor Buffer to set
  @param[out] Status            Transfer Status

  @retval EFI_INVALID_PARAMETER  Parameter is error
  @retval EFI_SUCCESS            Success
  @retval EFI_TIMEOUT            Device has no response
**/
EFI_STATUS
UsbSetDescriptor (
  IN  EFI_USB_IO_PROTOCOL  *UsbIo,
  IN  UINT16               Value,
  IN  UINT16               Index,
  IN  UINT16               DescriptorLength,
  IN  VOID                 *Descriptor,
  OUT UINT32               *Status
  )
{
  EFI_STATUS             EfiStatus;

  EFI_USB_DEVICE_REQUEST DeviceRequest;

  ASSERT (UsbIo != NULL);
  ASSERT (DescriptorLength > 0);
  ASSERT (Descriptor != NULL);
  ASSERT (Status != NULL);

  EfiStatus = EFI_INVALID_PARAMETER;

  if (UsbIo != NULL) {
    EfiZeroMem (&DeviceRequest, sizeof (DeviceRequest));

    DeviceRequest.RequestType = USB_DEV_SET_DESCRIPTOR_REQ_TYPE;
    DeviceRequest.Request     = USB_REQ_SET_DESCRIPTOR;
    DeviceRequest.Value       = Value;
    DeviceRequest.Index       = Index;
    DeviceRequest.Length      = DescriptorLength;
    EfiStatus                 = UsbIo->UsbControlTransfer (
                                         UsbIo,
                                         &DeviceRequest,
                                         EfiUsbDataOut,
                                         TIMEOUT_VALUE,
                                         Descriptor,
                                         DescriptorLength,
                                         Status
                                         );
  }

  return EfiStatus;
}

// UsbGetInterface
/** Usb Get Device Interface

  @param[in]  UsbIo       EFI_USB_IO_PROTOCOL
  @param[in]  Index       Interface index value
  @param[out] AltSetting  Alternate setting
  @param[out] Status      Trasnsfer status

  @retval EFI_INVALID_PARAMETER  Parameter is error
  @retval EFI_SUCCESS            Success
  @retval EFI_TIMEOUT            Device has no response
**/
EFI_STATUS
UsbGetInterface (
  IN  EFI_USB_IO_PROTOCOL  *UsbIo,
  IN  UINT16               Index,
  OUT UINT8                *AltSetting,
  OUT UINT32               *Status
  )
{
  EFI_STATUS             EfiStatus;

  EFI_USB_DEVICE_REQUEST DeviceRequest;

  ASSERT (UsbIo != NULL);
  ASSERT (AltSetting != NULL);
  ASSERT (Status != NULL);

  EfiStatus = EFI_INVALID_PARAMETER;

  if (UsbIo != NULL) {
    EfiZeroMem (&DeviceRequest, sizeof (DeviceRequest));

    DeviceRequest.RequestType = USB_DEV_GET_INTERFACE_REQ_TYPE;
    DeviceRequest.Request     = USB_REQ_GET_INTERFACE;
    DeviceRequest.Index       = Index;
    DeviceRequest.Length      = sizeof (*AltSetting);
    EfiStatus                 = UsbIo->UsbControlTransfer (
                                         UsbIo,
                                         &DeviceRequest,
                                         EfiUsbDataIn,
                                         TIMEOUT_VALUE,
                                         AltSetting,
                                         sizeof (*AltSetting),
                                         Status
                                         );
  }

  return EfiStatus;
}

// UsbSetInterface
/** Usb Set Device Interface

  @param[in]  UsbIo           EFI_USB_IO_PROTOCOL
  @param[in]  InterfaceIndex  Interface Number
  @param[in]  AltSetting      Alternate setting
  @param[out] Status          Trasnsfer status

  @retval EFI_INVALID_PARAMETER  Parameter is error
  @retval EFI_SUCCESS            Success
  @retval EFI_TIMEOUT            Device has no response
**/
EFI_STATUS
UsbSetInterface (
  IN  EFI_USB_IO_PROTOCOL  *UsbIo,
  IN  UINT16               InterfaceIndex,
  IN  UINT16               AltSetting,
  OUT UINT32               *Status
  )
{
  EFI_STATUS             EfiStatus;

  EFI_USB_DEVICE_REQUEST DeviceRequest;

  ASSERT (UsbIo != NULL);
  ASSERT (Status != NULL);

  EfiStatus = EFI_INVALID_PARAMETER;

  if (UsbIo != NULL) {
    EfiZeroMem (&DeviceRequest, sizeof (DeviceRequest));

    DeviceRequest.RequestType = USB_DEV_SET_INTERFACE_REQ_TYPE;
    DeviceRequest.Request     = USB_REQ_SET_INTERFACE;
    DeviceRequest.Value       = AltSetting;
    DeviceRequest.Index       = InterfaceIndex;
    EfiStatus                 = UsbIo->UsbControlTransfer (
                                         UsbIo,
                                         &DeviceRequest,
                                         EfiUsbNoData,
                                         TIMEOUT_VALUE,
                                         NULL,
                                         0,
                                         Status
                                         );
  }

  return EfiStatus;
}

// UsbGetConfiguration
/** Usb Get Device Configuration

  @param[in]  UsbIo        EFI_USB_IO_PROTOCOL
  @param[out] ConfigValue  Config Value
  @param[out] Status       Transfer Status

  @retval EFI_INVALID_PARAMETER  Parameter is error
  @retval EFI_SUCCESS            Success
  @retval EFI_TIMEOUT            Device has no response
**/
EFI_STATUS
UsbGetConfiguration (
  IN  EFI_USB_IO_PROTOCOL  *UsbIo,
  OUT UINT8                *ConfigValue,
  OUT UINT32               *Status
  )
{
  EFI_STATUS             EfiStatus;

  EFI_USB_DEVICE_REQUEST DeviceRequest;

  ASSERT (UsbIo != NULL);
  ASSERT (ConfigValue != NULL);
  ASSERT (Status != NULL);

  EfiStatus = EFI_INVALID_PARAMETER;

  if (UsbIo != NULL) {
    EfiZeroMem (&DeviceRequest, sizeof (DeviceRequest));

    DeviceRequest.RequestType = USB_DEV_GET_CONFIGURATION_REQ_TYPE;
    DeviceRequest.Request     = USB_REQ_GET_CONFIG;
    DeviceRequest.Length      = sizeof (*ConfigValue);
    EfiStatus                 = UsbIo->UsbControlTransfer (
                                         UsbIo,
                                         &DeviceRequest,
                                         EfiUsbDataIn,
                                         TIMEOUT_VALUE,
                                         ConfigValue,
                                         sizeof (*ConfigValue),
                                         Status
                                         );
  }

  return EfiStatus;
}

// UsbSetConfiguration
/** Usb Set Device Configuration

  @param[in]  UsbIo    EFI_USB_IO_PROTOCOL
  @param[in]  Value    Configuration Value to set
  @param[out] Status   Transfer status

  @retval EFI_INVALID_PARAMETER  Parameter is error
  @retval EFI_SUCCESS            Success
  @retval EFI_TIMEOUT            Device has no response
**/
EFI_STATUS
UsbSetConfiguration (
  IN  EFI_USB_IO_PROTOCOL  *UsbIo,
  IN  UINT16               Value,
  OUT UINT32               *Status
  )
{
  EFI_STATUS             EfiStatus;

  EFI_USB_DEVICE_REQUEST DeviceRequest;

  ASSERT (UsbIo != NULL);
  ASSERT (Status != NULL);

  EfiStatus = EFI_INVALID_PARAMETER;

  if (UsbIo != NULL) {
    EfiZeroMem (&DeviceRequest, sizeof (DeviceRequest));

    DeviceRequest.RequestType = USB_DEV_SET_CONFIGURATION_REQ_TYPE;
    DeviceRequest.Request     = USB_REQ_SET_CONFIG;
    DeviceRequest.Value       = Value;
    EfiStatus                 = UsbIo->UsbControlTransfer (
                                         UsbIo,
                                         &DeviceRequest,
                                         EfiUsbNoData,
                                         TIMEOUT_VALUE,
                                         NULL,
                                         0,
                                         Status
                                         );

    ASSERT_EFI_ERROR (EfiStatus);
  }

  return EfiStatus;
}

// UsbSetFeature
/** Usb Set Device Feature

  @param[in]  UsbIo      EFI_USB_IO_PROTOCOL
  @param[in]  Recipient  Interface/Device/Endpoint
  @param[in]  Value      Request value
  @param[in]  Target     Request Index
  @param[out] Status     Transfer status
  
  @retval EFI_INVALID_PARAMETER  Parameter is error
  @retval EFI_SUCCESS            Success
  @retval EFI_TIMEOUT            Device has no response
**/
EFI_STATUS
UsbSetFeature (
  IN  EFI_USB_IO_PROTOCOL  *UsbIo,
  IN  UINTN                Recipient,
  IN  UINT16               Value,
  IN  UINT16               Target,
  OUT UINT32               *Status
  )
{
  EFI_STATUS             EfiStatus;

  EFI_USB_DEVICE_REQUEST DeviceRequest;

  ASSERT (UsbIo != NULL);
  ASSERT (Status != NULL);

  EfiStatus = EFI_INVALID_PARAMETER;

  if (UsbIo != NULL) {
    EfiZeroMem (&DeviceRequest, sizeof (DeviceRequest));

    switch (Recipient) {
    case USB_TARGET_DEVICE:
    {
      DeviceRequest.RequestType = USB_DEV_SET_FEATURE_REQ_TYPE_D;
      break;
    }

    case USB_TARGET_INTERFACE:
    {
      DeviceRequest.RequestType = USB_DEV_SET_FEATURE_REQ_TYPE_I;
      break;
    }

    case USB_TARGET_ENDPOINT:
    {
      DeviceRequest.RequestType = USB_DEV_SET_FEATURE_REQ_TYPE_E;
      break;
    }
    }

    // Fill device request, see USB1.1 spec

    DeviceRequest.Request = USB_REQ_SET_FEATURE;
    DeviceRequest.Value   = Value;
    DeviceRequest.Index   = Target;
    EfiStatus             = UsbIo->UsbControlTransfer (
                                     UsbIo,
                                     &DeviceRequest,
                                     EfiUsbNoData,
                                     TIMEOUT_VALUE,
                                     NULL,
                                     0,
                                     Status
                                     );
  }

  return EfiStatus;
}

// UsbClearFeature
/** Usb Clear Device Feature

  @param[in]  UsbIo      EFI_USB_IO_PROTOCOL
  @param[in]  Recipient  Interface/Device/Endpoint
  @param[in]  Value      Request value
  @param[in]  Target     Request Index
  @param[out] Status     Transfer status
  
  @retval EFI_INVALID_PARAMETER  Parameter is error
  @retval EFI_SUCCESS            Success
  @retval EFI_TIMEOUT            Device has no response
**/
EFI_STATUS
UsbClearFeature (
  IN  EFI_USB_IO_PROTOCOL  *UsbIo,
  IN  UINTN                Recipient,
  IN  UINT16               Value,
  IN  UINT16               Target,
  OUT UINT32               *Status
  )
{
  EFI_STATUS             EfiStatus;

  EFI_USB_DEVICE_REQUEST DeviceRequest;

  ASSERT (UsbIo != NULL);
  ASSERT (Status != NULL);

  EfiStatus = EFI_INVALID_PARAMETER;

  if (UsbIo != NULL) {
    EfiZeroMem (&DeviceRequest, sizeof (DeviceRequest));

    switch (Recipient) {
    case USB_TARGET_DEVICE:
    {
      DeviceRequest.RequestType = USB_DEV_CLEAR_FEATURE_REQ_TYPE_D;
      break;
    }

    case USB_TARGET_INTERFACE:
    {
      DeviceRequest.RequestType = USB_DEV_CLEAR_FEATURE_REQ_TYPE_I;
      break;
    }

    case USB_TARGET_ENDPOINT:
    {
      DeviceRequest.RequestType = USB_DEV_CLEAR_FEATURE_REQ_TYPE_E;
      break;
    }
    }

    // Fill device request, see USB1.1 spec

    DeviceRequest.Request = USB_REQ_CLEAR_FEATURE;
    DeviceRequest.Value   = Value;
    DeviceRequest.Index   = Target;
    EfiStatus             = UsbIo->UsbControlTransfer (
                                     UsbIo,
                                     &DeviceRequest,
                                     EfiUsbNoData,
                                     TIMEOUT_VALUE,
                                     NULL,
                                     0,
                                     Status
                                     );
  }

  return EfiStatus;
}

// UsbGetStatus
/** Usb Get Device Status

  @param[in]  UsbIo         EFI_USB_IO_PROTOCOL
  @param[in]  Recipient     Interface/Device/Endpoint
  @param[in]  Target        Request index
  @param[out] DeviceStatus  Device status
  @param[out] Status        Transfer status

  @retval EFI_INVALID_PARAMETER  Parameter is error
  @retval EFI_SUCCESS            Success
  @retval EFI_TIMEOUT            Device has no response
**/
EFI_STATUS
UsbGetStatus (
  IN  EFI_USB_IO_PROTOCOL  *UsbIo,
  IN  UINTN                Recipient,
  IN  UINT16               Target,
  OUT UINT16               *DeviceStatus,
  OUT UINT32               *Status
  )
{
  EFI_STATUS             EfiStatus;

  EFI_USB_DEVICE_REQUEST DeviceRequest;

  ASSERT (UsbIo != NULL);
  ASSERT (DeviceStatus != NULL);
  ASSERT (Status != NULL);

  EfiStatus = EFI_INVALID_PARAMETER;

  if (UsbIo != NULL) {
    EfiZeroMem (&DeviceRequest, sizeof (DeviceRequest));

    switch (Recipient) {
    case USB_TARGET_DEVICE:
    {
      DeviceRequest.RequestType = USB_DEV_GET_STATUS_REQ_TYPE_D;
      break;
    }

    case USB_TARGET_INTERFACE:
    {
      DeviceRequest.RequestType = USB_DEV_GET_STATUS_REQ_TYPE_I;
      break;
    }

    case USB_TARGET_ENDPOINT:
    {
      DeviceRequest.RequestType = USB_DEV_GET_STATUS_REQ_TYPE_E;
      break;
    }
    }

    // Fill device request, see USB1.1 spec

    DeviceRequest.Request = USB_REQ_GET_STATUS;
    DeviceRequest.Value   = 0;
    DeviceRequest.Index   = Target;
    DeviceRequest.Length  = sizeof (*DeviceStatus);
    EfiStatus             = UsbIo->UsbControlTransfer (
                                     UsbIo,
                                     &DeviceRequest,
                                     EfiUsbDataIn,
                                     TIMEOUT_VALUE,
                                     DeviceStatus,
                                     sizeof (*DeviceStatus),
                                     Status
                                     );
  }

  return EfiStatus;
}

// UsbClearEndpointHalt
/** Clear endpoint stall

  @param[in]  UsbIo       EFI_USB_IO_PROTOCOL
  @param[in]  EndpointNo  Endpoint Number
  @param[out] Status      Transfer Status

  @retval EFI_NOT_FOUND     Can't find the Endpoint
  @retval EFI_DEVICE_ERROR  Hardware error
  @retval EFI_SUCCESS       Success
**/
EFI_STATUS
UsbClearEndpointHalt (
  IN  EFI_USB_IO_PROTOCOL  *UsbIo,
  IN  UINT8                EndpointNo,
  OUT UINT32               *Status
  )
{
  EFI_STATUS                   EfiStatus;

  EFI_USB_ENDPOINT_DESCRIPTOR  EndpointDescriptor;
  EFI_USB_INTERFACE_DESCRIPTOR InterfaceDescriptor;
  UINT8                        Index;

  ASSERT (UsbIo != NULL);
  ASSERT (Status != NULL);

  EfiZeroMem (&EndpointDescriptor, sizeof (EndpointDescriptor));

  // First seach the endpoint descriptor for that endpoint addr
  EfiStatus = UsbIo->UsbGetInterfaceDescriptor (
                       UsbIo,
                       &InterfaceDescriptor
                       );

  if (!EFI_ERROR (EfiStatus)) {
    for (Index = 0; Index < InterfaceDescriptor.NumEndpoints; ++Index) {
      EfiStatus = UsbIo->UsbGetEndpointDescriptor (
                           UsbIo,
                           Index,
                           &EndpointDescriptor
                           );

      if (!EFI_ERROR (EfiStatus)
       && (EndpointDescriptor.EndpointAddress == EndpointNo)) {
        break;
      }
    }

    EfiStatus = EFI_NOT_FOUND;

    if (Index != InterfaceDescriptor.NumEndpoints) {
      EfiStatus = UsbClearFeature (
                    UsbIo,
                    USB_TARGET_ENDPOINT,
                    USB_FEATURE_ENDPOINT_HALT,
                    EndpointDescriptor.EndpointAddress,
                    Status
                    );
    }
  }

  return EfiStatus;
}

// UsbGetHidDescriptor
/** Get Hid Descriptor

  @param[in]  UsbIo          EFI_USB_IO_PROTOCOL
  @param[in]  InterfaceNum   Hid interface number
  @param[out] HidDescriptor  Caller allocated Buffer to store Usb hid
                             descriptor
                             if successfully returned.

  @retval EFI_SUCCESS       Success
  @retval EFI_DEVICE_ERROR  Hardware error
  @retval EFI_TIMEOUT       Device has no response
**/
EFI_STATUS
UsbGetHidDescriptor (
  IN  EFI_USB_IO_PROTOCOL     *UsbIo,
  IN  UINT8                   InterfaceNum,
  OUT EFI_USB_HID_DESCRIPTOR  *HidDescriptor UNALIGNED
  )
{
  EFI_STATUS             Status;

  UINT32                 UsbStatus;
  EFI_USB_DEVICE_REQUEST DeviceRequest;

  ASSERT (UsbIo != NULL);
  ASSERT (HidDescriptor != NULL);

  DeviceRequest.RequestType = 0x81;
  DeviceRequest.Request     = 0x06;
  DeviceRequest.Value       = (UINT16) (0x21 << 8);
  DeviceRequest.Index       = InterfaceNum;
  DeviceRequest.Length      = sizeof (*HidDescriptor);
  Status                    = UsbIo->UsbControlTransfer (
                                       UsbIo,
                                       &DeviceRequest,
                                       EfiUsbDataIn,
                                       TIMEOUT_VALUE,
                                       HidDescriptor,
                                       sizeof (*HidDescriptor),
                                       &UsbStatus
                                       );

  return Status;
}

// UsbGetReportDescriptor
/** get Report Class descriptor

  @param[in]  UsbIo             EFI_USB_IO_PROTOCOL.
  @param[in]  InterfaceNum      Report interface number.
  @param[in]  DescriptorSize    Length of DescriptorBuffer.
  @param[out] DescriptorBuffer  Caller allocated Buffer to store Usb report
                                descriptor if successfully returned.

  @retval EFI_SUCCESS       Success
  @retval EFI_DEVICE_ERROR  Hardware error
  @retval EFI_TIMEOUT       Device has no response
**/
EFI_STATUS
UsbGetReportDescriptor (
  IN  EFI_USB_IO_PROTOCOL  *UsbIo,
  IN  UINT8                InterfaceNum,
  IN  UINT16               DescriptorSize,
  OUT UINT8                *DescriptorBuffer
  )
{
  EFI_STATUS             Status;

  UINT32                 UsbStatus;
  EFI_USB_DEVICE_REQUEST DeviceRequest;

  ASSERT (UsbIo != NULL);
  ASSERT (DescriptorSize > 0);
  ASSERT (DescriptorBuffer != NULL);

  // Fill Device request packet

  DeviceRequest.RequestType = 0x81;
  DeviceRequest.Request     = 0x06;
  DeviceRequest.Value       = (UINT16)(0x22 << 8);
  DeviceRequest.Index       = InterfaceNum;
  DeviceRequest.Length      = DescriptorSize;
  Status                    = UsbIo->UsbControlTransfer (
                                       UsbIo,
                                       &DeviceRequest,
                                       EfiUsbDataIn,
                                       TIMEOUT_VALUE,
                                       DescriptorBuffer,
                                       DescriptorSize,
                                       &UsbStatus
                                       );

  return Status;
}

// UsbGetProtocolRequest
/** Get Hid Protocol Request

  @param[in] UsbIo      EFI_USB_IO_PROTOCOL
  @param[in] Interface  Which interface the caller wants to get protocol
  @param[in] Protocol   Protocol value returned.

  @retval EFI_SUCCESS       Success
  @retval EFI_DEVICE_ERROR  Hardware error
  @retval EFI_TIMEOUT       Device has no response
**/
EFI_STATUS
UsbGetProtocolRequest (
  IN EFI_USB_IO_PROTOCOL  *UsbIo,
  IN UINT8                Interface,
  IN UINT8                *Protocol
  )
{
  EFI_STATUS             Status;

  UINT32                 UsbStatus;
  EFI_USB_DEVICE_REQUEST DeviceRequest;

  ASSERT (UsbIo != NULL);
  ASSERT (Protocol != NULL);

  // Fill Device request packet

  DeviceRequest.RequestType = 0xA1;

  // 10100001b;

  DeviceRequest.Request = EFI_USB_GET_PROTOCOL_REQUEST;
  DeviceRequest.Value   = 0;
  DeviceRequest.Index   = Interface;
  DeviceRequest.Length  = sizeof (*Protocol);
  Status                = UsbIo->UsbControlTransfer (
                                   UsbIo,
                                   &DeviceRequest,
                                   EfiUsbDataIn,
                                   TIMEOUT_VALUE,
                                   Protocol,
                                   sizeof (*Protocol),
                                   &UsbStatus
                                   );

  return Status;
}

// UsbSetProtocolRequest
/** Set Hid Protocol Request

  @param[in] UsbIo      EFI_USB_IO_PROTOCOL
  @param[in] Interface  Which interface the caller wants to set protocol
  @param[in] Protocol   Protocol value the caller wants to set.

  @retval EFI_SUCCESS       Success
  @retval EFI_DEVICE_ERROR  Hardware error
  @retval EFI_TIMEOUT       Device has no response
**/
EFI_STATUS
UsbSetProtocolRequest (
  IN EFI_USB_IO_PROTOCOL  *UsbIo,
  IN UINT8                Interface,
  IN UINT8                Protocol
  )
{
  EFI_STATUS             Status;

  UINT32                 UsbStatus;
  EFI_USB_DEVICE_REQUEST DeviceRequest;

  ASSERT (UsbIo != NULL);


  // Fill Device request packet

  DeviceRequest.RequestType = 0x21;

  // 00100001b;

  DeviceRequest.Request = EFI_USB_SET_PROTOCOL_REQUEST;
  DeviceRequest.Value   = Protocol;
  DeviceRequest.Index   = Interface;
  DeviceRequest.Length  = 0;
  Status                = UsbIo->UsbControlTransfer (
                                   UsbIo,
                                   &DeviceRequest,
                                   EfiUsbNoData,
                                   TIMEOUT_VALUE,
                                   NULL,
                                   0,
                                   &UsbStatus
                                   );

  return Status;
}

// UsbSetIdleRequest
/** Set Idel request.

  @param[in] UsbIo      EFI_USB_IO_PROTOCOL
  @param[in] Interface  Which interface the caller wants to set.
  @param[in] ReportId   Which report the caller wants to set.
  @param[in] Duration   Idle rate the caller wants to set.

  @retval EFI_SUCCESS       Success
  @retval EFI_DEVICE_ERROR  Hardware error
  @retval EFI_TIMEOUT       Device has no response
**/
EFI_STATUS
UsbSetIdleRequest (
  IN EFI_USB_IO_PROTOCOL  *UsbIo,
  IN UINT8                Interface,
  IN UINT8                ReportId,
  IN UINT8                Duration
  )
{
  EFI_STATUS             Status;

  UINT32                 UsbStatus;
  EFI_USB_DEVICE_REQUEST DeviceRequest;

  ASSERT (UsbIo != NULL);
  ASSERT (Duration > 0);


  // Fill Device request packet

  DeviceRequest.RequestType = 0x21;

  // 00100001b;

  DeviceRequest.Request = EFI_USB_SET_IDLE_REQUEST;
  DeviceRequest.Value   = (UINT16)((Duration << 8) | ReportId);
  DeviceRequest.Index   = Interface;
  DeviceRequest.Length  = 0;
  Status                = UsbIo->UsbControlTransfer (
                                   UsbIo,
                                   &DeviceRequest,
                                   EfiUsbNoData,
                                   TIMEOUT_VALUE,
                                   NULL,
                                   0,
                                   &UsbStatus
                                   );

  return Status;
}

// UsbGetIdleRequest
/** Get Idel request.

  @param[in]  UsbIo      EFI_USB_IO_PROTOCOL
  @param[in]  Interface  Which interface the caller wants to get.
  @param[in]  ReportId   Which report the caller wants to get.
  @param[out] Duration   Idle rate the caller wants to get.

  @retval EFI_SUCCESS       Success
  @retval EFI_DEVICE_ERROR  Hardware error
  @retval EFI_TIMEOUT       Device has no response
**/
EFI_STATUS
UsbGetIdleRequest (
  IN  EFI_USB_IO_PROTOCOL  *UsbIo,
  IN  UINT8                Interface,
  IN  UINT8                ReportId,
  OUT UINT8                *Duration
  )
{
  EFI_STATUS             Status;

  UINT32                 UsbStatus;
  EFI_USB_DEVICE_REQUEST DeviceRequest;

  ASSERT (UsbIo != NULL);
  ASSERT (Duration != NULL);

  // Fill Device request packet

  DeviceRequest.RequestType = 0xA1;

  // 10100001b;

  DeviceRequest.Request = EFI_USB_GET_IDLE_REQUEST;
  DeviceRequest.Value   = ReportId;
  DeviceRequest.Index   = Interface;
  DeviceRequest.Length  = sizeof (*Duration);
  Status                = UsbIo->UsbControlTransfer (
                                   UsbIo,
                                   &DeviceRequest,
                                   EfiUsbDataIn,
                                   TIMEOUT_VALUE,
                                   Duration,
                                   sizeof (*Duration),
                                   &UsbStatus
                                   );

  return Status;
}

// UsbSetReportRequest
/** Hid Set Report request.

  @param[in] UsbIo       EFI_USB_IO_PROTOCOL
  @param[in] Interface   Which interface the caller wants to set.
  @param[in] ReportId    Which report the caller wants to set.
  @param[in] ReportType  Type of report.
  @param[in] ReportLen   Length of report descriptor.
  @param[in] Report      Report Descriptor Buffer.

  @retval EFI_SUCCESS       Success
  @retval EFI_DEVICE_ERROR  Hardware error
  @retval EFI_TIMEOUT       Device has no response
**/
EFI_STATUS
UsbSetReportRequest (
  IN EFI_USB_IO_PROTOCOL  *UsbIo,
  IN UINT8                Interface,
  IN UINT8                ReportId,
  IN UINT8                ReportType,
  IN UINT16               ReportLen,
  IN UINT8                *Report
  )
{
  EFI_STATUS             Status;

  UINT32                 UsbStatus;
  EFI_USB_DEVICE_REQUEST DeviceRequest;

  ASSERT (UsbIo != NULL);
  ASSERT (ReportLen > 0);
  ASSERT (Report != NULL);

  // Fill Device request packet

  DeviceRequest.RequestType = 0x21;

  // 00100001b;

  DeviceRequest.Request = EFI_USB_SET_REPORT_REQUEST;
  DeviceRequest.Value   = (UINT16)((ReportType << 8) | ReportId);
  DeviceRequest.Index   = Interface;
  DeviceRequest.Length  = ReportLen;
  Status                = UsbIo->UsbControlTransfer (
                                   UsbIo,
                                   &DeviceRequest,
                                   EfiUsbDataOut,
                                   TIMEOUT_VALUE,
                                   Report,
                                   ReportLen,
                                   &UsbStatus
                                   );

  return Status;
}

// UsbGetReportRequest
/** Hid Set Report request.

  @param[in] UsbIo       EFI_USB_IO_PROTOCOL
  @param[in] Interface   Which interface the caller wants to set.
  @param[in] ReportId    Which report the caller wants to set.
  @param[in] ReportType  Type of report.
  @param[in] ReportLen   Length of report descriptor.
  @param[in] Report      Caller allocated Buffer to store Report Descriptor.

  @retval EFI_SUCCESS       Success
  @retval EFI_DEVICE_ERROR  Hardware error
  @retval EFI_TIMEOUT       Device has no response
**/
EFI_STATUS
UsbGetReportRequest (
  IN EFI_USB_IO_PROTOCOL  *UsbIo,
  IN UINT8                Interface,
  IN UINT8                ReportId,
  IN UINT8                ReportType,
  IN UINT16               ReportLen,
  IN UINT8                *Report
  )
{
  EFI_STATUS             Status;

  UINT32                 UsbStatus;
  EFI_USB_DEVICE_REQUEST DeviceRequest;

  ASSERT (UsbIo != NULL);
  ASSERT (ReportLen > 0);
  ASSERT (Report != NULL);

  // Fill Device request packet

  DeviceRequest.RequestType = 0xA1;

  // 10100001b;

  DeviceRequest.Request = EFI_USB_GET_REPORT_REQUEST;
  DeviceRequest.Value   = (UINT16)((ReportType << 8) | ReportId);
  DeviceRequest.Index   = Interface;
  DeviceRequest.Length  = ReportLen;
  Status                = UsbIo->UsbControlTransfer (
                                   UsbIo,
                                   &DeviceRequest,
                                   EfiUsbDataIn,
                                   TIMEOUT_VALUE,
                                   Report,
                                   ReportLen,
                                   &UsbStatus
                                   );

  return Status;
}
