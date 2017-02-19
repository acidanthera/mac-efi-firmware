/** @file
  Apple protocol to manage Device Properties from firmware.

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

// DppDbGetPropertyValueImpl
/** Locates a device property in the database and returns its value into Value.

  @param[in]      This        A pointer to the protocol instance.
  @param[in]      DevicePath  The device path of the device to get the property
                              of.
  @param[in]      Name        The Name of the requested property.
  @param[out]     Value       The Buffer allocated by the caller to return the
                              value of the property into.
  @param[in, out] Size        On input the size of the allocated Value Buffer.
                              On output the size required to fill the Buffer.

  @return                       The status of the operation is returned.
  @retval EFI_BUFFER_TOO_SMALL  The memory required to return the value exceeds
                                the size of the allocated Buffer.
                                The required size to complete the operation has
                                been returned into Size.
  @retval EFI_NOT_FOUND         The given device path does not have a property
                                with the specified Name.
  @retval EFI_SUCCESS           The operation completed successfully and the
                                Value Buffer has been filled.
**/
EFI_STATUS
EFIAPI
DppDbGetPropertyValue (
  IN     EFI_DEVICE_PATH_PROPERTY_DATABASE_PROTOCOL  *This,
  IN     EFI_DEVICE_PATH_PROTOCOL                    *DevicePath,
  IN     CHAR16                                      *Name,
  OUT    VOID                                        *Value, OPTIONAL
  IN OUT UINTN                                       *Size
  ) // sub_593
{
  EFI_STATUS                        Status;

  EFI_DEVICE_PATH_PROPERTY_DATABASE *Database;
  EFI_DEVICE_PATH_PROPERTY_NODE     *Node;
  EFI_DEVICE_PATH_PROPERTY          *Property;
  UINTN                             PropertySize;
  BOOLEAN                           BufferTooSmall;

  ASSERT (This != NULL);
  ASSERT (DevicePath != NULL);
  ASSERT (Name != NULL);
  ASSERT (Size != NULL);
  ASSERT ((((*Size > 0) ? 1 : 0) ^ ((Value == NULL) ? 1 : 0)) != 0);

  Database = PROPERTY_DATABASE_FROM_PROTOCOL (This);
  Node     = DppDbGetPropertyNode (Database, DevicePath);
  Status   = EFI_NOT_FOUND;

  if (Node != NULL) {
    Property = DppDbGetProperty (Name, Node);
    Status   = EFI_NOT_FOUND;

    if (Property != NULL) {
      PropertySize   = EFI_DEVICE_PATH_PROPERTY_VALUE_SIZE (Property);
      BufferTooSmall = (BOOLEAN)(PropertySize > *Size);
      *Size          = PropertySize;
      Status         = EFI_BUFFER_TOO_SMALL;

      if (!BufferTooSmall) {
        EfiCopyMem (Value, (VOID *)&Property->Value->Data, PropertySize);

        Status = EFI_SUCCESS;
      }
    }
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}

// DppDbSetPropertyImpl
/** Sets the sepcified property of the given device path to the provided Value.

  @param[in] This        A pointer to the protocol instance.
  @param[in] DevicePath  The device path of the device to set the property of.
  @param[in] Name        The Name of the desired property.
  @param[in] Value       The Buffer holding the value to set the property to.
  @param[in] Size        The size of the Value Buffer.

  @return                       The status of the operation is returned.
  @retval EFI_OUT_OF_RESOURCES  The memory necessary to complete the operation
                                could not be allocated.
  @retval EFI_SUCCESS           The operation completed successfully and the
                                Value Buffer has been filled.
**/
EFI_STATUS
EFIAPI
DppDbSetProperty (
  IN EFI_DEVICE_PATH_PROPERTY_DATABASE_PROTOCOL  *This,
  IN EFI_DEVICE_PATH_PROTOCOL                    *DevicePath,
  IN CHAR16                                      *Name,
  IN VOID                                        *Value,
  IN UINTN                                       Size
  ) // sub_617
{
  EFI_STATUS                        Status;

  EFI_DEVICE_PATH_PROPERTY_DATABASE *Database;
  EFI_DEVICE_PATH_PROPERTY_NODE     *Node;
  UINTN                             DevicePathSize;
  EFI_DEVICE_PATH_PROPERTY          *Property;
  INTN                              Result;
  UINTN                             PropertyNameSize;
  UINTN                             PropertyDataSize;
  EFI_DEVICE_PATH_PROPERTY_DATA     *PropertyData;

  ASSERT (This != NULL);
  ASSERT (DevicePath != NULL);
  ASSERT (Name != NULL);
  ASSERT (Value != NULL);
  ASSERT (Size > 0);

  Database = PROPERTY_DATABASE_FROM_PROTOCOL (This);
  Node     = DppDbGetPropertyNode (Database, DevicePath);

  if (Node == NULL) {
    DevicePathSize = EfiDevicePathSize (DevicePath);
    Node           = EfiLibAllocateZeroPool (sizeof (Node) + DevicePathSize);
    Status         = EFI_OUT_OF_RESOURCES;

    if (Node == NULL) {
      goto Done;
    } else {
      Node->Hdr.Signature = EFI_DEVICE_PATH_PROPERTY_NODE_SIGNATURE;

      InitializeListHead (&Node->Hdr.Properties);
      EfiCopyMem (
        (VOID *)&Node->DevicePath,
        (VOID *)DevicePath,
        DevicePathSize
        );

      InsertTailList (&Database->Nodes, &Node->Hdr.This);

      Database->Modified = TRUE;
    }
  }

  Property = DppDbGetProperty (Name, Node);

  if (Property != NULL) {
    if (Property->Value->Hdr.Size == Size) {
      Result = EfiCompareMem ((VOID *)&Property->Value->Data, Value, Size);

      if (Result == 0) {
        Status = EFI_SUCCESS;
        goto Done;
      }
    }

    RemoveEntryList (&Property->This);

    --Node->Hdr.NumberOfProperties;

    gBS->FreePool ((VOID *)Property->Name);
    gBS->FreePool ((VOID *)Property->Value);
    gBS->FreePool ((VOID *)Property);
  }

  Database->Modified = TRUE;
  Property           = EfiLibAllocateZeroPool (sizeof (Property));
  Status             = EFI_OUT_OF_RESOURCES;

  if (Property != NULL) {
    PropertyNameSize   = (EfiStrSize (Name) + sizeof (PropertyData->Hdr));
    PropertyData       = EfiLibAllocateZeroPool (PropertyNameSize);
    Property->Name     = PropertyData;

    if (PropertyData != NULL) {
      PropertyDataSize = (Size + sizeof (PropertyData->Hdr));
      PropertyData     = EfiLibAllocateZeroPool (PropertyDataSize);
      Property->Value  = PropertyData;

      if (PropertyData != NULL) {
        Property->Signature = EFI_DEVICE_PATH_PROPERTY_SIGNATURE;

        EfiStrCpy ((CHAR16 *)&Property->Name->Data, Name);

        Property->Name->Hdr.Size = (UINT32)PropertyNameSize;

        EfiCopyMem ((VOID *)&Property->Value->Data, (VOID *)Value, Size);

        Property->Value->Hdr.Size = (UINT32)PropertyDataSize;

        InsertTailList (&Node->Hdr.Properties, &Property->This);

        Status = EFI_SUCCESS;
        ++Node->Hdr.NumberOfProperties;
      }
    }
  }

Done:
  ASSERT_EFI_ERROR (Status);

  return Status;
}

// DppDbRemovePropertyImpl
/** Removes the sepcified property from the given device path.

  @param[in] This        A pointer to the protocol instance.
  @param[in] DevicePath  The device path of the device to set the property of.
  @param[in] Name        The Name of the desired property.

  @return                The status of the operation is returned.
  @retval EFI_NOT_FOUND  The given device path does not have a property with
                         the specified Name.
  @retval EFI_SUCCESS    The operation completed successfully.
**/
EFI_STATUS
EFIAPI
DppDbRemoveProperty (
  IN EFI_DEVICE_PATH_PROPERTY_DATABASE_PROTOCOL  *This,
  IN EFI_DEVICE_PATH_PROTOCOL                    *DevicePath,
  IN CHAR16                                      *Name
  ) // sub_7FF
{
  EFI_STATUS                        Status;

  EFI_DEVICE_PATH_PROPERTY_DATABASE *Database;
  EFI_DEVICE_PATH_PROPERTY_NODE     *Node;
  EFI_DEVICE_PATH_PROPERTY          *Property;

  ASSERT (This != NULL);
  ASSERT (DevicePath != NULL);
  ASSERT (Name != NULL);

  Database = PROPERTY_DATABASE_FROM_PROTOCOL (This);
  Node     = DppDbGetPropertyNode (Database, DevicePath);

  if (Node == NULL) {
    Status = EFI_NOT_FOUND;
  } else {
    Property = DppDbGetProperty (Name, Node);
    Status   = EFI_NOT_FOUND;

    if (Property != NULL) {
      Database->Modified = TRUE;

      RemoveEntryList (&Property->This);

      --Node->Hdr.NumberOfProperties;

      gBS->FreePool ((VOID *)Property);

      Status = EFI_SUCCESS;

      if (Node->Hdr.NumberOfProperties == 0) {
        RemoveEntryList (&Node->Hdr.This);

        gBS->FreePool ((VOID *)Node);
      }
    }
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}

// DppDbGetPropertyBufferImpl
/** Returns a Buffer of all device properties into Buffer.

  @param[in]      This    A pointer to the protocol instance.
  @param[out]     Buffer  The Buffer allocated by the caller to return the
                          property Buffer into.
  @param[in, out] Size    On input the size of the allocated Buffer.
                          On output the size required to fill the Buffer.

  @return                       The status of the operation is returned.
  @retval EFI_BUFFER_TOO_SMALL  The memory required to return the value exceeds
                                the size of the allocated Buffer.
                                The required size to complete the operation has
                                been returned into Size.
  @retval EFI_SUCCESS           The operation completed successfully.
**/
EFI_STATUS
EFIAPI
DppDbGetPropertyBuffer (
  IN     EFI_DEVICE_PATH_PROPERTY_DATABASE_PROTOCOL  *This,
  OUT    EFI_DEVICE_PATH_PROPERTY_BUFFER             *Buffer, OPTIONAL
  IN OUT UINTN                                       *Size
  ) // sub_893
{
  EFI_STATUS                           Status;

  EFI_LIST                             *Nodes;
  BOOLEAN                              Result;
  EFI_DEVICE_PATH_PROPERTY_NODE        *NodeIterator;
  UINTN                                BufferSize;
  EFI_DEVICE_PATH_PROPERTY             *Property;
  UINT32                               NumberOfNodes;
  EFI_DEVICE_PATH_PROPERTY_BUFFER_NODE *BufferNode;
  VOID                                 *BufferPtr;

  ASSERT (This != NULL);
  ASSERT (Buffer != NULL);
  ASSERT (Size != NULL);

  Nodes  = &(PROPERTY_DATABASE_FROM_PROTOCOL (This))->Nodes;
  Result = IsListEmpty (Nodes);

  if (Result) {
    *Size  = 0;
    Status = EFI_SUCCESS;
  } else {
    DppDbCallProtocol ();

    NodeIterator  = PROPERTY_NODE_FROM_LIST_ENTRY (GetFirstNode (Nodes));
    Result        = IsNull (Nodes, &NodeIterator->Hdr.This);
    BufferSize    = sizeof (Buffer->Hdr);
    NumberOfNodes = 0;

    while (!Result) {
      Property = EFI_DEVICE_PATH_PROPERTY_FROM_LIST_ENTRY (
                   GetFirstNode (&NodeIterator->Hdr.Properties)
                   );

      Result = IsNull (&NodeIterator->Hdr.Properties, &Property->This);

      while (!Result) {
        BufferSize += EFI_DEVICE_PATH_PROPERTY_SIZE (Property);
        Property    = EFI_DEVICE_PATH_PROPERTY_FROM_LIST_ENTRY (
                        GetNextNode (&NodeIterator->Hdr.Properties, &Property->This)
                        );

        Result = IsNull (&NodeIterator->Hdr.Properties, &Property->This);
      }

      NodeIterator = PROPERTY_NODE_FROM_LIST_ENTRY (
               GetNextNode (Nodes, &NodeIterator->Hdr.This)
               );

      Result      = IsNull (Nodes, &NodeIterator->Hdr.This);
      BufferSize += EFI_DEVICE_PATH_PROPERTY_NODE_SIZE (NodeIterator);
      ++NumberOfNodes;
    }

    Result = (BOOLEAN)(*Size < BufferSize);
    *Size  = BufferSize;
    Status = EFI_BUFFER_TOO_SMALL;

    if (!Result) {
      Buffer->Hdr.Size          = (UINT32)BufferSize;
      Buffer->Hdr.MustBe1       = 1;
      Buffer->Hdr.NumberOfNodes = NumberOfNodes;
      NodeIterator              = PROPERTY_NODE_FROM_LIST_ENTRY (
                                    GetFirstNode (Nodes)
                                    );

      Result = IsNull (&NodeIterator->Hdr.This, &NodeIterator->Hdr.This);
      Status = EFI_SUCCESS;

      if (!Result) {
        BufferNode = &Buffer->Node;

        do {
          BufferSize = EfiDevicePathSize (&NodeIterator->DevicePath);

          EfiCopyMem (
            (VOID *)&BufferNode->DevicePath,
            (VOID *)&NodeIterator->DevicePath,
            BufferSize
            );

          BufferNode->Hdr.NumberOfProperties = (UINT32)NodeIterator->Hdr.NumberOfProperties;

          Property = EFI_DEVICE_PATH_PROPERTY_FROM_LIST_ENTRY (
                       GetFirstNode (&NodeIterator->Hdr.Properties)
                       );

          Result      = IsNull (&NodeIterator->Hdr.Properties, &Property->This);
          BufferSize += sizeof (BufferNode->Hdr);
          BufferPtr   = (VOID *)((UINTN)Buffer + BufferSize);

          while (!Result) {
            EfiCopyMem (
              BufferPtr,
              (VOID *)Property->Name,
              (UINTN)Property->Name->Hdr.Size
              );

            EfiCopyMem (
              (VOID *)((UINTN)BufferPtr + (UINTN)Property->Name->Hdr.Size),
              Property->Value,
              (UINTN)Property->Value->Hdr.Size
              );

            BufferPtr = (VOID *)(
                          (UINTN)BufferPtr
                            + Property->Name->Hdr.Size
                              + Property->Value->Hdr.Size
                          );

            BufferSize += EFI_DEVICE_PATH_PROPERTY_SIZE (Property);
            Property    = EFI_DEVICE_PATH_PROPERTY_FROM_LIST_ENTRY (
                            GetNextNode (
                              &NodeIterator->Hdr.Properties,
                              &Property->This
                            )
                            );

            Result = IsNull (&NodeIterator->Hdr.Properties, &Property->This);
          }

          BufferNode->Hdr.Size = (UINT32)BufferSize;
          BufferNode           = (EFI_DEVICE_PATH_PROPERTY_BUFFER_NODE *)(
                                    (UINTN)BufferNode + BufferSize
                                    );

          NodeIterator = PROPERTY_NODE_FROM_LIST_ENTRY (
                   GetNextNode (Nodes, &NodeIterator->Hdr.This)
                   );
        } while (!IsNull (&NodeIterator->Hdr.This, &NodeIterator->Hdr.This));
      }
    }
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}
