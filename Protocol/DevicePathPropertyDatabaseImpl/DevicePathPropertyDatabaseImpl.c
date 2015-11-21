///
/// @file      ProtocolImpl/DevicePathPropertyDatabase/DevicePathPropertyDatabase.c
///
///            Apple protocol to manage Device Properties from firmware.
///
/// @author    Download-Fritz
/// @date      23/02/2015: Initial version
/// @date      15/03/2015: Updated documentation
/// @copyright The decompilation is of an educational purpose to better understand the behavior of the
///            Apple EFI implementation and making use of it. In no way is the content's usage licensed
///            or allowed. All rights remain at Apple Inc. To be used under the terms of 'Fair use'.
///

//
// CREDITS:
//   Reversed from EfiDevicePathPropertyDatabase.efi, which is Apple Inc. property
//   Decompiled by Download-Fritz
//

#include <AppleEfi.h>
#include <EfiDriverLib.h>
#include <LinkedList.h>
#include <EfiDebug.h>

#include <Guid/AppleNvram.h>

#include <Protocol/DevicePathPropertyDatabase.h>
#include <Protocol/DevicePathPropertyDatabaseImpl.h>

//
static EFI_GUID mUnknownProtocolGuid = { 0xC649D4F3, 0xD502, 0x4DAA, { 0xA1, 0x39, 0x39, 0x4A, 0xCC, 0xF2, 0xA6, 0x3B } };
//

// GetPropertyNode
/// 
/// @param
///
/// @return
/// @retval
EFI_DEVICE_PATH_PROPERTY_NODE *
GetPropertyNode (
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

// GetProperty
/// 
/// @param
///
/// @return
/// @retval
EFI_DEVICE_PATH_PROPERTY *
GetProperty (
  IN CHAR16                         *Name,
  IN EFI_DEVICE_PATH_PROPERTY_NODE  *Node
  )
{
  EFI_DEVICE_PATH_PROPERTY *Property;

  BOOLEAN                  IsPropertyNull;
  INTN                     Result;

  Property = PROPERTY_FROM_LIST_ENTRY (GetFirstNode (&Node->Hdr.Properties));

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

    Property = PROPERTY_FROM_LIST_ENTRY (GetNextNode (&Node->Hdr.Properties, &Property->This));
  } while (TRUE);

  return Property;
}

// CallProtocol
/// 
/// @param
///
/// @return
/// @retval
VOID
CallProtocol (
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
        if (*((UINT32 *)((UINTN)Interface + sizeof (UINT32))) == 0) {
          (*(VOID (EFIAPI **)(VOID *))((UINTN)Interface + 232)) (Interface);
        }
      }
    }
  }

  if (Buffer != NULL) {
    gBS->FreePool (Buffer);
  }
}

// DevicePathPropertyDbGetPropertyValue
/// Locates a device property in the database and returns its value into Value.
///
/// @param[in]      This       A pointer to the protocol instance.
/// @param[in]      DevicePath The device path of the device to get the property of.
/// @param[in]      Name       The Name of the requested property.
/// @param[out]     Value      The buffer allocated by the caller to return the value of the property into.
/// @param[in, out] Size     On input the size of the allocated Value buffer.
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
  OUT    VOID                                        *Value,
  IN OUT UINTN                                       *Size
  ) // sub_593
{
  EFI_STATUS                        Status;

  EFI_DEVICE_PATH_PROPERTY_DATABASE *Database;
  EFI_DEVICE_PATH_PROPERTY_NODE     *Node;
  EFI_DEVICE_PATH_PROPERTY          *Property;
  UINTN                             PropertySize;
  BOOLEAN                           BufferTooSmall;

  Database = PROPERTY_DATABASE_FROM_PROTOCOL (This);
  Node     = GetPropertyNode (Database, DevicePath);
  Status   = EFI_NOT_FOUND;

  if (Node != NULL) {
    Property = GetProperty (Name, Node);
    Status   = EFI_NOT_FOUND;

    if (Property != NULL) {
      PropertySize   = EFI_DEVICE_PATH_PROPERTY_VALUE_SIZE (Property);
      BufferTooSmall = (PropertySize > *Size);
      *Size          = PropertySize;
      Status         = EFI_BUFFER_TOO_SMALL;

      if (!BufferTooSmall) {
        gBS->CopyMem (Value, (VOID *)&Property->Value->Data, PropertySize);

        Status = EFI_SUCCESS;
      }
    }
  }

  return Status;
}

// DevicePathPropertyDbSetProperty
/// Sets the sepcified property of the given device path to the provided Value.
///
/// @param[in]  This       A pointer to the protocol instance.
/// @param[in]  DevicePath The device path of the device to set the property of.
/// @param[in]  Name       The Name of the desired property.
/// @param[in]  Value      The buffer holding the value to set the property to.
/// @param[out] Size       The size of the Value buffer.
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

  Database = PROPERTY_DATABASE_FROM_PROTOCOL (This);
  Node     = GetPropertyNode (Database, DevicePath);

  if (Node == NULL) {
    DevicePathSize = EfiDevicePathSize (DevicePath);
    Node           = (EFI_DEVICE_PATH_PROPERTY_NODE *)EfiLibAllocateZeroPool (sizeof (Node) + DevicePathSize);
    Status         = EFI_OUT_OF_RESOURCES;

    if (Node == NULL) {
      goto Return;
    } else {
      Node->Hdr.Signature = EFI_DEVICE_PATH_PROPERTY_NODE_SIGNATURE;

      InitializeListHead (&Node->Hdr.Properties);
      gBS->CopyMem ((VOID *)&Node->DevicePath, (VOID *)DevicePath, DevicePathSize);
      InsertTailList (&Database->Nodes, &Node->Hdr.This);

      Database->Modified = TRUE;
    }
  }

  Property = GetProperty (Name, Node);

  if (Property != NULL) {
    if (Property->Value->Hdr.Size == Size) {
      Result = EfiCompareMem ((VOID *)&Property->Value->Data, Value, Size);

      if (Result == 0) {
        Status = EFI_SUCCESS;

        goto Return;
      }
    }

    RemoveEntryList (&Property->This);

    --Node->Hdr.NoProperties;

    gBS->FreePool ((VOID *)Property->Name);
    gBS->FreePool ((VOID *)Property->Value);
    gBS->FreePool ((VOID *)Property);
  }

  Database->Modified = TRUE;
  Property           = (EFI_DEVICE_PATH_PROPERTY *)EfiLibAllocateZeroPool (sizeof (Property));
  Status             = EFI_OUT_OF_RESOURCES;

  if (Property != NULL) {
    PropertyNameSize   = (EfiStrSize (Name) + sizeof (PropertyData->Hdr));
    PropertyData       = (EFI_DEVICE_PATH_PROPERTY_DATA *)EfiLibAllocateZeroPool (PropertyNameSize);
    Property->Name     = PropertyData;

    if (PropertyData != NULL) {
      PropertyDataSize = (Size + sizeof (PropertyData->Hdr));
      PropertyData     = (EFI_DEVICE_PATH_PROPERTY_DATA *)EfiLibAllocateZeroPool (PropertyDataSize);
      Property->Value  = PropertyData;

      if (PropertyData != NULL) {
        Property->Signature = EFI_DEVICE_PATH_PROPERTY_SIGNATURE;

        EfiStrCpy ((CHAR16 *)&Property->Name->Data, Name);

        Property->Name->Hdr.Size = (UINT32)PropertyNameSize;

        gBS->CopyMem ((VOID *)&Property->Value->Data, (VOID *)Value, Size);

        Property->Value->Hdr.Size = (UINT32)PropertyDataSize;

        InsertTailList (&Node->Hdr.Properties, &Property->This);

        Status = EFI_SUCCESS;
        ++Node->Hdr.NoProperties;
      }
    }
  }

Return:
  return Status;
}

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
  ) // sub_7FF
{
  EFI_STATUS                        Status;

  EFI_DEVICE_PATH_PROPERTY_DATABASE *Database;
  EFI_DEVICE_PATH_PROPERTY_NODE     *Node;
  EFI_DEVICE_PATH_PROPERTY          *Property;

  Database = PROPERTY_DATABASE_FROM_PROTOCOL (This);
  Node     = GetPropertyNode (Database, DevicePath);

  if (Node == NULL) {
    Status = EFI_NOT_FOUND;
  } else {
    Property = GetProperty (Name, Node);
    Status   = EFI_NOT_FOUND;
    if (Property != NULL) {
      Database->Modified = TRUE;

      RemoveEntryList (&Property->This);

      --Node->Hdr.NoProperties;

      gBS->FreePool ((VOID *)Property);

      Status = EFI_SUCCESS;

      if (Node->Hdr.NoProperties == 0) {
        RemoveEntryList (&Node->Hdr.This);

        gBS->FreePool ((VOID *)Node);
      }
    }
  }

  return Status;
}

// DevicePathPropertyDbGetPropertyBuffer
/// Returns a buffer of all device properties into Buffer.
///
/// @param[in]      This   A pointer to the protocol instance.
/// @param[out]     Buffer The buffer allocated by the caller to return the property buffer into.
/// @param[in, out] Size On input the size of the allocated Buffer.
///                                                   On output the size required to fill the buffer.
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
  ) // sub_893
{
  EFI_STATUS                           Status;

  EFI_LIST                             *Nodes;
  BOOLEAN                              Result;
  EFI_DEVICE_PATH_PROPERTY_NODE        *Node;
  UINTN                                BufferSize;
  EFI_DEVICE_PATH_PROPERTY             *Property;
  UINT32                               NoNodes;
  EFI_DEVICE_PATH_PROPERTY_BUFFER_NODE *BufferNode;
  VOID                                 *BufferPtr;

  Nodes  = &(PROPERTY_DATABASE_FROM_PROTOCOL (This))->Nodes;
  Result = IsListEmpty (Nodes);

  if (Result) {
    *Size = 0;
    Status  = EFI_SUCCESS;
  } else {
    CallProtocol ();

    Node       = PROPERTY_NODE_FROM_LIST_ENTRY (GetFirstNode (Nodes));
    Result     = IsNull (Nodes, &Node->Hdr.This);
    BufferSize = sizeof (Buffer->Hdr);
    NoNodes    = 0;

    while (!Result) {
      Property = PROPERTY_FROM_LIST_ENTRY (GetFirstNode (&Node->Hdr.Properties));
      Result   = IsNull (&Node->Hdr.Properties, &Property->This);

      while (!Result) {
        BufferSize += EFI_DEVICE_PATH_PROPERTY_SIZE (Property);
        Property    = PROPERTY_FROM_LIST_ENTRY (GetNextNode (&Node->Hdr.Properties, &Property->This));
        Result      = IsNull (&Node->Hdr.Properties, &Property->This);
      }

      Node         = PROPERTY_NODE_FROM_LIST_ENTRY (GetNextNode (Nodes, &Node->Hdr.This));
      Result       = IsNull (Nodes, &Node->Hdr.This);
      BufferSize  += EFI_DEVICE_PATH_PROPERTY_NODE_SIZE (Node);
      ++NoNodes;
    }

    Result = (*Size < BufferSize);
    *Size  = BufferSize;
    Status = EFI_BUFFER_TOO_SMALL;

    if (!Result) {
      Buffer->Hdr.Size    = (UINT32)BufferSize;
      Buffer->Hdr.MustBe1 = 1;
      Buffer->Hdr.NoNodes = NoNodes;
      Node                = PROPERTY_NODE_FROM_LIST_ENTRY (GetFirstNode (Nodes));
      Result              = IsNull (&Node->Hdr.This, &Node->Hdr.This);
      Status              = EFI_SUCCESS;

      if (!Result) {
        BufferNode = &Buffer->Node;

        do {
          BufferSize = EfiDevicePathSize (&Node->DevicePath);

          gBS->CopyMem ((VOID *)&BufferNode->DevicePath, (VOID *)&Node->DevicePath, BufferSize);

          BufferNode->Hdr.NoProperties = (UINT32)Node->Hdr.NoProperties;
          Property                     = PROPERTY_FROM_LIST_ENTRY (GetFirstNode (&Node->Hdr.Properties));
          Result                       = IsNull (&Node->Hdr.Properties, &Property->This);
          BufferSize                  += sizeof (BufferNode->Hdr);
          BufferPtr                    = (VOID *)((UINTN)Buffer + BufferSize);

          while (!Result) {
            gBS->CopyMem (BufferPtr, (VOID *)Property->Name, (UINTN)Property->Name->Hdr.Size);
            gBS->CopyMem ((VOID *)((UINTN)BufferPtr + (UINTN)Property->Name->Hdr.Size), Property->Value, (UINTN)Property->Value->Hdr.Size);

            BufferPtr   = (VOID *)((UINTN)BufferPtr + (Property->Name->Hdr.Size + Property->Value->Hdr.Size));
            BufferSize += EFI_DEVICE_PATH_PROPERTY_SIZE (Property);
            Property    = PROPERTY_FROM_LIST_ENTRY (GetNextNode (&Node->Hdr.Properties, &Property->This));
            Result      = IsNull (&Node->Hdr.Properties, &Property->This);
          }

          BufferNode->Hdr.Size = (UINT32)BufferSize;
          BufferNode           = (EFI_DEVICE_PATH_PROPERTY_BUFFER_NODE *)((UINTN)BufferNode + BufferSize);
          Node                 = PROPERTY_NODE_FROM_LIST_ENTRY (GetNextNode (Nodes, &Node->Hdr.This));
        } while (!IsNull (&Node->Hdr.This, &Node->Hdr.This));
      }
    }
  }

  return Status;
}
