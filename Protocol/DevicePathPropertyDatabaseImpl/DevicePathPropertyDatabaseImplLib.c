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

#include <Library/AppleDriverLib.h>

#include "DevicePathPropertyDatabaseImplInternal.h"

// TODO: Move to own header
//
#define UNKNOWN_PROTOCOL_GUID \
  { 0xC649D4F3, 0xD502, 0x4DAA, { 0xA1, 0x39, 0x39, 0x4A, 0xCC, 0xF2, 0xA6, 0x3B } }

EFI_GUID mUnknownProtocolGuid = UNKNOWN_PROTOCOL_GUID;
//

// DevicePathPropertyDbGetPropertyNode
EFI_DEVICE_PATH_PROPERTY_NODE *
DevicePathPropertyDbGetPropertyNode (
  IN EFI_DEVICE_PATH_PROPERTY_DATABASE  *Database,
  IN EFI_DEVICE_PATH_PROTOCOL           *DevicePath
  ) // sub_AC5
{
  EFI_DEVICE_PATH_PROPERTY_NODE *Node;
  UINTN                         DevicePathSize;
  BOOLEAN                       IsNodeNull;
  UINTN                         DevicePathSize2;
  INTN                          Result;

  Node           = PROPERTY_NODE_FROM_LIST_ENTRY (GetFirstNode (&Database->Nodes));
  DevicePathSize = EfiDevicePathSize (DevicePath);

  do {
    IsNodeNull = IsNull (&Database->Nodes, &Node->Hdr.This);

    if (IsNodeNull) {
      Node = NULL;

      break;
    }

    DevicePathSize2 = EfiDevicePathSize (&Node->DevicePath);

    if (DevicePathSize == DevicePathSize2) {
      Result = EfiCompareMem (DevicePath, &Node->DevicePath, DevicePathSize);

      if (Result == 0) {
        break;
      }
    }

    Node = PROPERTY_NODE_FROM_LIST_ENTRY (GetNextNode (&Database->Nodes, &Node->Hdr.This));
  } while (TRUE);

  return Node;
}

// DevicePathPropertyDbGetProperty
EFI_DEVICE_PATH_PROPERTY *
DevicePathPropertyDbGetProperty (
  IN CHAR16                         *Name,
  IN EFI_DEVICE_PATH_PROPERTY_NODE  *Node
  )
{
  EFI_DEVICE_PATH_PROPERTY *Property;

  BOOLEAN                  IsPropertyNull;
  INTN                     Result;

  Property = EFI_DEVICE_PATH_PROPERTY_FROM_LIST_ENTRY (GetFirstNode (&Node->Hdr.Properties));

  do {
    IsPropertyNull = IsNull (&Node->Hdr.Properties, &Property->This);

    if (IsPropertyNull) {
      Property = NULL;

      break;
    }

    Result = EfiStrCmp (Name, (CHAR16 *)&Property->Value->Data);

    if (Result == 0) {
      break;
    }

    Property = EFI_DEVICE_PATH_PROPERTY_FROM_LIST_ENTRY (GetNextNode (&Node->Hdr.Properties, &Property->This));
  } while (TRUE);

  return Property;
}

// DevicePathPropertyDbCallProtocol
VOID
DevicePathPropertyDbCallProtocol (
  VOID
  ) // sub_BB0
{
  EFI_STATUS Status;
  UINTN      NoHandles;
  EFI_HANDLE *Buffer;
  UINTN      Index;
  VOID       *Interface;

  Buffer = NULL;
  Status = gBS->LocateHandleBuffer (ByProtocol, &mUnknownProtocolGuid, NULL, &NoHandles, &Buffer);

  if (Status == EFI_SUCCESS) {
    for (Index = 0; Index < NoHandles; ++Index) {
      Status = gBS->HandleProtocol (Buffer[Index], &mUnknownProtocolGuid, &Interface);

      if (Status == EFI_SUCCESS) {
        if (*(UINT32 *)((UINTN)Interface + sizeof (UINT32)) == 0) {
          (*(VOID (EFIAPI **)(VOID *))((UINTN)Interface + 232)) (Interface);
        }
      }
    }
  }

  if (Buffer != NULL) {
    gBS->FreePool ((VOID *)Buffer);
  }
}
