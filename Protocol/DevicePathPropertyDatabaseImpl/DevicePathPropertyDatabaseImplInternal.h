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
/// @file      Protocol/DevicePathPropertyDatabaseImpl/DevicePathPropertyDatabaseImplInternal.h
///
///            
///
/// @author    Download-Fritz
/// @date      12/12/2015: Initial version
/// @copyright Copyright (C) 2005 - 2015 Apple Inc. All rights reserved.
///

#ifndef __DEVICE_PATH_PROPERTY_DATABASE_IMPL_INTERNAL_H__
#define __DEVICE_PATH_PROPERTY_DATABASE_IMPL_INTERNAL_H__

#include <Protocol/DevicePathPropertyDatabaseImpl.h>

// EFI_DEVICE_PATH_PROPERTY_NODE_SIGNATURE
#define EFI_DEVICE_PATH_PROPERTY_NODE_SIGNATURE  EFI_SIGNATURE_32 ('D', 'p', 'n', 0x00)
#define PROPERTY_NODE_FROM_LIST_ENTRY(Entry)     _CR (Entry, EFI_DEVICE_PATH_PROPERTY_NODE, Hdr.This);
#define EFI_DEVICE_PATH_PROPERTY_NODE_SIZE(Node) (sizeof ((Node)->Hdr) + EfiDevicePathSize (&(Node)->DevicePath))

// _DEVICE_PATH_PROPERTY_NODE
typedef struct _DEVICE_PATH_PROPERTY_NODE {
  struct {
    UINTN          Signature;     ///< 
    EFI_LIST_ENTRY This;          ///< 
    UINTN          NoProperties;  ///< 
    EFI_LIST       Properties;    ///< 
  }                        Hdr;         ///< 
  EFI_DEVICE_PATH_PROTOCOL DevicePath;  ///< 
} EFI_DEVICE_PATH_PROPERTY_NODE;

/// @{
#define EFI_DEVICE_PATH_PROPERTY_SIGNATURE  EFI_SIGNATURE_32 ('D', 'p', 'p', 0x00)

#define EFI_DEVICE_PATH_PROPERTY_FROM_LIST_ENTRY(Entry) \
  CR (Entry, EFI_DEVICE_PATH_PROPERTY, This, EFI_DEVICE_PATH_PROPERTY_SIGNATURE)

#define NEXT_EFI_DEVICE_PATH_PROPERTY(Property) \
  (EFI_DEVICE_PATH_PROPERTY *)((UINTN)(Property) + EFI_DEVICE_PATH_PROPERTY_SIZE (Property))

#define EFI_DEVICE_PATH_PROPERTY_SIZE(Property)       ((Property)->Name->Hdr.Size + (Property)->Value->Hdr.Size)
#define EFI_DEVICE_PATH_PROPERTY_VALUE_SIZE(Property) ((Property)->Value->Hdr.Size - sizeof ((Property)->Value->Hdr))
/// @}

// _EFI_DEVICE_PATH_PROPERTY
typedef struct _EFI_DEVICE_PATH_PROPERTY {
  UINTN                         Signature;  ///< 
  EFI_LIST_ENTRY                This;       ///< 
  EFI_DEVICE_PATH_PROPERTY_DATA *Name;      ///< 
  EFI_DEVICE_PATH_PROPERTY_DATA *Value;     ///< 
} EFI_DEVICE_PATH_PROPERTY;

// DevicePathPropertyDbGetPropertyNode
/// 
/// @param
///
/// @return
/// @retval
EFI_DEVICE_PATH_PROPERTY_NODE *
DevicePathPropertyDbGetPropertyNode (
  IN EFI_DEVICE_PATH_PROPERTY_DATABASE  *Database,
  IN EFI_DEVICE_PATH_PROTOCOL           *DevicePath
  );

// DevicePathPropertyDbGetProperty
/// 
/// @param
///
/// @return
/// @retval
EFI_DEVICE_PATH_PROPERTY *
DevicePathPropertyDbGetProperty (
  IN CHAR16                         *Name,
  IN EFI_DEVICE_PATH_PROPERTY_NODE  *Node
  );

// DevicePathPropertyDbCallProtocol
/// 
/// @param
///
/// @return
/// @retval
VOID
DevicePathPropertyDbCallProtocol (
  VOID
  );

#endif // ifndef __DEVICE_PATH_PROPERTY_DATABASE_IMPL_INTERNAL_H__
