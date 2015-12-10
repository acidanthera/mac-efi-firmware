//
// Copyright (C) 2005 - 2015 Apple Inc. All rights reserved.
//
// This program and the accompanying materials have not been licensed.
// Neither is its usage, its redistribution, in source or binary form,
// licensed, nor implicitely or explicitely permitted, except when
// required by applicable law.
//
// Unless required by applicable law or agreed to in writing, software
// distributed is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
// OR CONDITIONS OF ANY KIND, either express or implied.
//

///
/// @file      Include/Protocol/DevicePathPropertyDatabaseImpl.h
///
///            Private data of the Apple protocol to manage Device Properties from firmware.
///
/// @author    Download-Fritz
/// @date      23/02/2015: Initial version
/// @date      15/03/2015: Updated documentation
/// @copyright Copyright (C) 2005 - 2015 Apple Inc. All rights reserved.
///

#ifndef __DEVICE_PATH_PROPERTY_DATABASE_IMPL_H__
#define __DEVICE_PATH_PROPERTY_DATABASE_IMPL_H__

// EFI_DEVICE_PATH_PROPERTY_DATABASE_PROTOCOL_REVISION
#define EFI_DEVICE_PATH_PROPERTY_DATABASE_PROTOCOL_REVISION  0x010000

#define APPLE_PATH_PROPERTIES_VARIABLE_NAME    L"AAPL,PathProperties"
#define APPLE_PATH_PROPERTY_VARIABLE_MAX_SIZE  768

#define EFI_DEVICE_PATH_PROPERTY_DATABASE_SIGNATURE  EFI_SIGNATURE_32 ('D', 'p', 'p', 'P')

#define PROPERTY_DATABASE_FROM_PROTOCOL(This) \
  CR (This, EFI_DEVICE_PATH_PROPERTY_DATABASE, Protocol, EFI_DEVICE_PATH_PROPERTY_DATABASE_SIGNATURE)

// _DEVICE_PATH_PROPERTY_DATABASE
typedef struct _DEVICE_PATH_PROPERTY_DATABASE {
  UINTN                                      Signature;  ///< 
  EFI_LIST                                   Nodes;      ///< 
  EFI_DEVICE_PATH_PROPERTY_DATABASE_PROTOCOL Protocol;   ///< 
  BOOLEAN                                    Modified;   ///< 
} EFI_DEVICE_PATH_PROPERTY_DATABASE;

// EFI_DEVICE_PATH_PROPERTY_NODE_SIGNATURE
#define EFI_DEVICE_PATH_PROPERTY_NODE_SIGNATURE  EFI_SIGNATURE_32 ('D', 'p', 'n', 0x00)
#define PROPERTY_NODE_FROM_LIST_ENTRY(Entry)     _CR (Entry, EFI_DEVICE_PATH_PROPERTY_NODE, Hdr.This);
#define EFI_DEVICE_PATH_PROPERTY_NODE_SIZE(Node) (sizeof ((Node)->Hdr) + EfiDevicePathSize (&(Node)->DevicePath))

// _DEVICE_PATH_PROPERTY_NODE
typedef struct _DEVICE_PATH_PROPERTY_NODE {
  struct {
    UINTN          Signature;           ///< 
    EFI_LIST_ENTRY This;                ///< 
    UINTN          NoProperties;        ///< 
    EFI_LIST       Properties;          ///< 
  }                        Hdr;         ///< 
  EFI_DEVICE_PATH_PROTOCOL DevicePath;  ///< 
} EFI_DEVICE_PATH_PROPERTY_NODE;

#define EFI_DEVICE_PATH_PROPERTY_SIGNATURE            EFI_SIGNATURE_32 ('D', 'p', 'p', 0x00)

#define PROPERTY_FROM_LIST_ENTRY(Entry) \
  CR (Entry, EFI_DEVICE_PATH_PROPERTY, This, EFI_DEVICE_PATH_PROPERTY_SIGNATURE)

#define NEXT_EFI_DEVICE_PATH_PROPERTY(Property) \
  (EFI_DEVICE_PATH_PROPERTY *)((UINTN)(Property) + EFI_DEVICE_PATH_PROPERTY_SIZE (Property))

#define EFI_DEVICE_PATH_PROPERTY_SIZE(Property)       ((Property)->Name->Hdr.Size + (Property)->Value->Hdr.Size)
#define EFI_DEVICE_PATH_PROPERTY_VALUE_SIZE(Property) ((Property)->Value->Hdr.Size - sizeof ((Property)->Value->Hdr))

// _DEVICE_PATH_PROPERTY
typedef struct _DEVICE_PATH_PROPERTY {
  UINTN                         Signature;  ///< 
  EFI_LIST_ENTRY                This;       ///< 
  EFI_DEVICE_PATH_PROPERTY_DATA *Name;      ///< 
  EFI_DEVICE_PATH_PROPERTY_DATA *Value;     ///< 
} EFI_DEVICE_PATH_PROPERTY;

// DevicePathPropertyDbGetPropertyValue
/// Locates a device property in the database and returns its value into Value.
///
/// @param[in]      This       A pointer to the protocol instance.
/// @param[in]      DevicePath The device path of the device to get the property of.
/// @param[in]      Name       The Name of the requested property.
/// @param[out]     Value      The buffer allocated by the caller to return the value of the property into.
/// @param[in, out] Size       On input the size of the allocated Value buffer.
///                            On output the size required to fill the buffer.
///
/// @return                      The status of the operation is returned.
/// @retval EFI_BUFFER_TOO_SMALL The memory required to return the value exceeds the size of the allocated buffer.
///                              The required size to complete the operation has been returned into Size.
/// @retval EFI_NOT_FOUND        The given device path does not have a property with the specified Name.
/// @retval EFI_SUCCESS          The operation completed successfully and the Value buffer has been filled.
EFI_STATUS
EFIAPI
DevicePathPropertyDbGetPropertyValue (
  IN     EFI_DEVICE_PATH_PROPERTY_DATABASE_PROTOCOL  *This,
  IN     EFI_DEVICE_PATH_PROTOCOL                    *DevicePath,
  IN     CHAR16                                      *Name,
     OUT VOID                                        *Value,
  IN OUT UINTN                                       *Size
  );

// DevicePathPropertyDbSetProperty
/// Sets the sepcified property of the given device path to the provided Value.
///
/// @param[in] This       A pointer to the protocol instance.
/// @param[in] DevicePath The device path of the device to set the property of.
/// @param[in] Name       The Name of the desired property.
/// @param[in] Value      The buffer holding the value to set the property to.
/// @param[in] Size       The size of the Value buffer.
///
/// @return                      The status of the operation is returned.
/// @retval EFI_OUT_OF_RESOURCES The memory necessary to complete the operation could not be allocated.
/// @retval EFI_SUCCESS          The operation completed successfully and the Value buffer has been filled.
EFI_STATUS
EFIAPI
DevicePathPropertyDbSetProperty (
  IN EFI_DEVICE_PATH_PROPERTY_DATABASE_PROTOCOL  *This,
  IN EFI_DEVICE_PATH_PROTOCOL                    *DevicePath,
  IN CHAR16                                      *Name,
  IN VOID                                        *Value,
  IN UINTN                                       Size
  );

// DevicePathPropertyDbRemoveProperty
/// Removes the sepcified property from the given device path.
///
/// @param[in] This       A pointer to the protocol instance.
/// @param[in] DevicePath The device path of the device to set the property of.
/// @param[in] Name       The Name of the desired property.
///
/// @return               The status of the operation is returned.
/// @retval EFI_NOT_FOUND The given device path does not have a property with the specified Name.
/// @retval EFI_SUCCESS   The operation completed successfully.
EFI_STATUS
EFIAPI
DevicePathPropertyDbRemoveProperty (
  IN EFI_DEVICE_PATH_PROPERTY_DATABASE_PROTOCOL  *This,
  IN EFI_DEVICE_PATH_PROTOCOL                    *DevicePath,
  IN CHAR16                                      *Name
  );

// DevicePathPropertyDbGetPropertyBuffer
/// Returns a buffer of all device properties into Buffer.
///
/// @param[in]      This   A pointer to the protocol instance.
/// @param[out]     Buffer The buffer allocated by the caller to return the property buffer into.
/// @param[in, out] Size   On input the size of the allocated Buffer.
///                        On output the size required to fill the buffer.
///
/// @return                      The status of the operation is returned.
/// @retval EFI_BUFFER_TOO_SMALL The memory required to return the value exceeds the size of the allocated buffer.
///                              The required size to complete the operation has been returned into Size.
/// @retval EFI_SUCCESS          The operation completed successfully.
EFI_STATUS
EFIAPI
DevicePathPropertyDbGetPropertyBuffer (
  IN     EFI_DEVICE_PATH_PROPERTY_DATABASE_PROTOCOL  *This,
  OUT    EFI_DEVICE_PATH_PROPERTY_BUFFER             *Buffer,
  IN OUT UINTN                                       *Size
  );

#endif // ifndef __DEVICE_PATH_PROPERTY_DATABASE_IMPL_H__
