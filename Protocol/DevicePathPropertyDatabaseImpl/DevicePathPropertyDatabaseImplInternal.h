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

#ifndef DEVICE_PATH_PROPERTY_DATABASE_IMPL_INTERNAL_H_
#define DEVICE_PATH_PROPERTY_DATABASE_IMPL_INTERNAL_H_

#include APPLE_PROTOCOL_PRODUCER (DevicePathPropertyDatabaseImpl)

/// @{
#define EFI_DEVICE_PATH_PROPERTY_NODE_SIGNATURE  \
  EFI_SIGNATURE_32 ('D', 'p', 'n', 0x00)

#define PROPERTY_NODE_FROM_LIST_ENTRY(Entry)   \
  ((EFI_DEVICE_PATH_PROPERTY_NODE *)(          \
    CR (                                       \
      Entry,                                   \
      EFI_DEVICE_PATH_PROPERTY_NODE_HDR,       \
      This,                                    \
      EFI_DEVICE_PATH_PROPERTY_NODE_SIGNATURE  \
      )                                        \
    ))

#define EFI_DEVICE_PATH_PROPERTY_NODE_SIZE(Node)                            \
          (sizeof ((Node)->Hdr) + EfiDevicePathSize (&(Node)->DevicePath))
/// @}

// EFI_DEVICE_PATH_PROPERTY_NODE_HDR
typedef struct {
  UINTN          Signature;           ///< 
  EFI_LIST_ENTRY This;                ///< 
  UINTN          NumberOfProperties;  ///< 
  EFI_LIST       Properties;          ///< 
} EFI_DEVICE_PATH_PROPERTY_NODE_HDR;

// DEVICE_PATH_PROPERTY_NODE
typedef struct {
  EFI_DEVICE_PATH_PROPERTY_NODE_HDR Hdr;         ///< 
  EFI_DEVICE_PATH_PROTOCOL          DevicePath;  ///< 
} EFI_DEVICE_PATH_PROPERTY_NODE;

/// @{
#define EFI_DEVICE_PATH_PROPERTY_SIGNATURE        \
          EFI_SIGNATURE_32 ('D', 'p', 'p', 0x00)

#define EFI_DEVICE_PATH_PROPERTY_FROM_LIST_ENTRY(Entry)  \
  CR (                                                   \
    (Entry),                                             \
    EFI_DEVICE_PATH_PROPERTY,                            \
    This,                                                \
    EFI_DEVICE_PATH_PROPERTY_SIGNATURE                   \
    )

#define EFI_DEVICE_PATH_PROPERTY_SIZE(Property)               \
  ((Property)->Name->Hdr.Size + (Property)->Value->Hdr.Size)

#define EFI_DEVICE_PATH_PROPERTY_VALUE_SIZE(Property)              \
  ((Property)->Value->Hdr.Size - sizeof ((Property)->Value->Hdr))

#define NEXT_EFI_DEVICE_PATH_PROPERTY(Property)                   \
  (EFI_DEVICE_PATH_PROPERTY *)(                                   \
    (UINTN)(Property) + EFI_DEVICE_PATH_PROPERTY_SIZE (Property)  \
    )
/// @}

// EFI_DEVICE_PATH_PROPERTY
typedef struct {
  UINTN                         Signature;  ///< 
  EFI_LIST_ENTRY                This;       ///< 
  EFI_DEVICE_PATH_PROPERTY_DATA *Name;      ///< 
  EFI_DEVICE_PATH_PROPERTY_DATA *Value;     ///< 
} EFI_DEVICE_PATH_PROPERTY;

// DppDbGetPropertyNode
EFI_DEVICE_PATH_PROPERTY_NODE *
DppDbGetPropertyNode (
  IN EFI_DEVICE_PATH_PROPERTY_DATABASE  *Database,
  IN EFI_DEVICE_PATH_PROTOCOL           *DevicePath
  );

// DppDbGetProperty
EFI_DEVICE_PATH_PROPERTY *
DppDbGetProperty (
  IN CHAR16                         *Name,
  IN EFI_DEVICE_PATH_PROPERTY_NODE  *Node
  );

// DppDbCallProtocol
VOID
DppDbCallProtocol (
  VOID
  );

#endif // DEVICE_PATH_PROPERTY_DATABASE_IMPL_INTERNAL_H_
