#include <AppleMacEfi.h>

#include <Guid/AppleVariable.h>

#include <Protocol/DevicePathPropertyDatabase.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#define DEVICE_PATH_PROPERTY_DATA_SIGNATURE  \
  SIGNATURE_32 ('D', 'p', 'p', 'P')

// PROPERTY_DATABASE_FROM_PROTOCOL
#define PROPERTY_DATABASE_FROM_PROTOCOL(This) \
  CR (                                        \
    This,                                     \
    DEVICE_PATH_PROPERTY_DATA,                \
    Protocol,                                 \
    DEVICE_PATH_PROPERTY_DATA_SIGNATURE       \
    )

// DEVICE_PATH_PROPERTY_DATABASE
typedef struct {
  UINTN                                      Signature;
  LIST_ENTRY                                 Nodes;
  EFI_DEVICE_PATH_PROPERTY_DATABASE_PROTOCOL Protocol;
  BOOLEAN                                    Modified;
} DEVICE_PATH_PROPERTY_DATA;

#define APPLE_PATH_PROPERTIES_VARIABLE_NAME    L"AAPL,PathProperties"
#define APPLE_PATH_PROPERTY_VARIABLE_MAX_SIZE  768

#define EFI_DEVICE_PATH_PROPERTY_NODE_SIGNATURE  \
  SIGNATURE_32 ('D', 'p', 'n', '\0')

#define PROPERTY_NODE_FROM_LIST_ENTRY(Entry)   \
  ((EFI_DEVICE_PATH_PROPERTY_NODE *)(          \
    CR (                                       \
      Entry,                                   \
      EFI_DEVICE_PATH_PROPERTY_NODE_HDR,       \
      Link,                                    \
      EFI_DEVICE_PATH_PROPERTY_NODE_SIGNATURE  \
      )                                        \
    ))

#define EFI_DEVICE_PATH_PROPERTY_NODE_SIZE(Node)  \
  (sizeof ((Node)->Hdr) + GetDevicePathSize (&(Node)->DevicePath))

// EFI_DEVICE_PATH_PROPERTY_NODE_HDR
typedef struct {
  UINTN      Signature;           ///< 
  LIST_ENTRY Link;                ///< 
  UINTN      NumberOfProperties;  ///< 
  LIST_ENTRY Properties;          ///< 
} EFI_DEVICE_PATH_PROPERTY_NODE_HDR;

// DEVICE_PATH_PROPERTY_NODE
typedef struct {
  EFI_DEVICE_PATH_PROPERTY_NODE_HDR Hdr;         ///< 
  EFI_DEVICE_PATH_PROTOCOL          DevicePath;  ///< 
} EFI_DEVICE_PATH_PROPERTY_NODE;

#define EFI_DEVICE_PATH_PROPERTY_SIGNATURE  \
  SIGNATURE_32 ('D', 'p', 'p', '\0')

#define EFI_DEVICE_PATH_PROPERTY_FROM_LIST_ENTRY(Entry)  \
  CR (                                                   \
    (Entry),                                             \
    EFI_DEVICE_PATH_PROPERTY,                            \
    Link,                                                \
    EFI_DEVICE_PATH_PROPERTY_SIGNATURE                   \
    )

#define EFI_DEVICE_PATH_PROPERTY_SIZE(Property)  \
  ((Property)->Name->Hdr.Size + (Property)->Value->Hdr.Size)

#define EFI_DEVICE_PATH_PROPERTY_VALUE_SIZE(Property)  \
  ((Property)->Value->Hdr.Size - sizeof ((Property)->Value->Hdr))

// EFI_DEVICE_PATH_PROPERTY
typedef struct {
  UINTN                         Signature;  ///< 
  LIST_ENTRY                    Link;       ///< 
  EFI_DEVICE_PATH_PROPERTY_DATA *Name;      ///< 
  EFI_DEVICE_PATH_PROPERTY_DATA *Value;     ///< 
} EFI_DEVICE_PATH_PROPERTY;

// TODO: Move to own header
//
#define UNKNOWN_PROTOCOL_GUID                             \
  { 0xC649D4F3, 0xD502, 0x4DAA,                           \
    { 0xA1, 0x39, 0x39, 0x4A, 0xCC, 0xF2, 0xA6, 0x3B } }

EFI_GUID mUnknownProtocolGuid = UNKNOWN_PROTOCOL_GUID;
//

// InternalGetPropertyNode
STATIC
EFI_DEVICE_PATH_PROPERTY_NODE *
InternalGetPropertyNode (
  IN DEVICE_PATH_PROPERTY_DATA  *DevicePathPropertyData,
  IN EFI_DEVICE_PATH_PROTOCOL   *DevicePath
  )
{
  EFI_DEVICE_PATH_PROPERTY_NODE *Node;

  EFI_DEVICE_PATH_PROPERTY_NODE *NodeWalker;
  UINTN                         DevicePathSize;
  UINTN                         DevicePathSize2;
  INTN                          Result;

  Node = NULL;

  NodeWalker = PROPERTY_NODE_FROM_LIST_ENTRY (
                 GetFirstNode (&DevicePathPropertyData->Nodes)
                 );

  DevicePathSize = GetDevicePathSize (DevicePath);

  while (!IsNull (&DevicePathPropertyData->Nodes, &NodeWalker->Hdr.Link)) {
    DevicePathSize2 = GetDevicePathSize (&NodeWalker->DevicePath);

    if (DevicePathSize == DevicePathSize2) {
      Result = CompareMem (DevicePath, &NodeWalker->DevicePath, DevicePathSize);

      if (Result == 0) {
        Node = NodeWalker;
        break;
      }
    }

    NodeWalker = PROPERTY_NODE_FROM_LIST_ENTRY (
                   GetNextNode (
                     &DevicePathPropertyData->Nodes,
                     &NodeWalker->Hdr.Link
                     )
                   );
  }

  return Node;
}

// InternalGetProperty
STATIC
EFI_DEVICE_PATH_PROPERTY *
InternalGetProperty (
  IN CHAR16                         *Name,
  IN EFI_DEVICE_PATH_PROPERTY_NODE  *Node
  )
{
  EFI_DEVICE_PATH_PROPERTY *Property;

  EFI_DEVICE_PATH_PROPERTY *PropertyWalker;
  INTN                     Result;

  Property = NULL;

  PropertyWalker = EFI_DEVICE_PATH_PROPERTY_FROM_LIST_ENTRY (
                     GetFirstNode (&Node->Hdr.Properties)
                     );

  while (!IsNull (&Node->Hdr.Properties, &PropertyWalker->Link)) {
    Result = StrCmp (Name, (CHAR16 *)&Property->Name->Data);

    if (Result == 0) {
      Property = PropertyWalker;
      break;
    }

    PropertyWalker = EFI_DEVICE_PATH_PROPERTY_FROM_LIST_ENTRY (
                       GetNextNode (
                         &Node->Hdr.Properties,
                         &PropertyWalker->Link
                         )
                       );
  }

  return Property;
}

// InternalCallProtocol
STATIC
VOID
InternalCallProtocol (
  VOID
  )
{
  EFI_STATUS Status;
  UINTN      NumberHandles;
  EFI_HANDLE *Buffer;
  UINTN      Index;
  VOID       *Interface;

  Buffer = NULL;
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &mUnknownProtocolGuid,
                  NULL,
                  &NumberHandles,
                  &Buffer
                  );

  if (Status == EFI_SUCCESS) {
    for (Index = 0; Index < NumberHandles; ++Index) {
      Status = gBS->HandleProtocol (
                      Buffer[Index],
                      &mUnknownProtocolGuid,
                      &Interface
                      );

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

// DppDbGetPropertyValue
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
  )
{
  EFI_STATUS                        Status;

  DEVICE_PATH_PROPERTY_DATA *Database;
  EFI_DEVICE_PATH_PROPERTY_NODE     *Node;
  EFI_DEVICE_PATH_PROPERTY          *Property;
  UINTN                             PropertySize;
  BOOLEAN                           BufferTooSmall;

  Database = PROPERTY_DATABASE_FROM_PROTOCOL (This);
  Node     = InternalGetPropertyNode (Database, DevicePath);

  Status = EFI_NOT_FOUND;

  if (Node != NULL) {
    Property = InternalGetProperty (Name, Node);

    Status = EFI_NOT_FOUND;

    if (Property != NULL) {
      PropertySize   = EFI_DEVICE_PATH_PROPERTY_VALUE_SIZE (Property);
      BufferTooSmall = (BOOLEAN)(PropertySize > *Size);
      *Size          = PropertySize;

      Status = EFI_BUFFER_TOO_SMALL;

      if (!BufferTooSmall) {
        CopyMem (Value, (VOID *)&Property->Value->Data, PropertySize);

        Status = EFI_SUCCESS;
      }
    }
  }

  return Status;
}

// DppDbSetProperty
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
  )
{
  EFI_STATUS                    Status;

  DEVICE_PATH_PROPERTY_DATA     *Database;
  EFI_DEVICE_PATH_PROPERTY_NODE *Node;
  UINTN                         DevicePathSize;
  EFI_DEVICE_PATH_PROPERTY      *Property;
  INTN                          Result;
  UINTN                         PropertyNameSize;
  UINTN                         PropertyDataSize;
  EFI_DEVICE_PATH_PROPERTY_DATA *PropertyData;

  Database = PROPERTY_DATABASE_FROM_PROTOCOL (This);
  Node     = InternalGetPropertyNode (Database, DevicePath);

  if (Node == NULL) {
    DevicePathSize = GetDevicePathSize (DevicePath);
    Node           = AllocateZeroPool (sizeof (Node->Hdr) + DevicePathSize);

    Status = EFI_OUT_OF_RESOURCES;

    if (Node == NULL) {
      goto Done;
    } else {
      Node->Hdr.Signature = EFI_DEVICE_PATH_PROPERTY_NODE_SIGNATURE;

      InitializeListHead (&Node->Hdr.Properties);

      CopyMem (
        (VOID *)&Node->DevicePath,
        (VOID *)DevicePath,
        DevicePathSize
        );

      InsertTailList (&Database->Nodes, &Node->Hdr.Link);

      Database->Modified = TRUE;
    }
  }

  Property = InternalGetProperty (Name, Node);

  if (Property != NULL) {
    if (Property->Value->Hdr.Size == Size) {
      Result = CompareMem ((VOID *)&Property->Value->Data, Value, Size);

      if (Result == 0) {
        Status = EFI_SUCCESS;
        goto Done;
      }
    }

    RemoveEntryList (&Property->Link);

    --Node->Hdr.NumberOfProperties;

    gBS->FreePool ((VOID *)Property->Name);
    gBS->FreePool ((VOID *)Property->Value);
    gBS->FreePool ((VOID *)Property);
  }

  Database->Modified = TRUE;
  Property           = AllocateZeroPool (sizeof (*Property));

  Status = EFI_OUT_OF_RESOURCES;

  if (Property != NULL) {
    PropertyNameSize   = (StrSize (Name) + sizeof (PropertyData->Hdr));
    PropertyData       = AllocateZeroPool (PropertyNameSize);
    Property->Name     = PropertyData;

    if (PropertyData != NULL) {
      PropertyDataSize = (Size + sizeof (PropertyData->Hdr));
      PropertyData     = AllocateZeroPool (PropertyDataSize);
      Property->Value  = PropertyData;

      if (PropertyData != NULL) {
        Property->Signature = EFI_DEVICE_PATH_PROPERTY_SIGNATURE;

        StrCpy ((CHAR16 *)&Property->Name->Data, Name);

        Property->Name->Hdr.Size = (UINT32)PropertyNameSize;

        CopyMem ((VOID *)&Property->Value->Data, (VOID *)Value, Size);

        Property->Value->Hdr.Size = (UINT32)PropertyDataSize;

        InsertTailList (&Node->Hdr.Properties, &Property->Link);

        Status = EFI_SUCCESS;

        ++Node->Hdr.NumberOfProperties;
      }
    }
  }

Done:
  return Status;
}

// DppDbRemoveProperty
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
  )
{
  EFI_STATUS                    Status;

  DEVICE_PATH_PROPERTY_DATA     *DevicePathPropertyData;
  EFI_DEVICE_PATH_PROPERTY_NODE *Node;
  EFI_DEVICE_PATH_PROPERTY      *Property;

  DevicePathPropertyData = PROPERTY_DATABASE_FROM_PROTOCOL (This);

  Node = InternalGetPropertyNode (DevicePathPropertyData, DevicePath);

  if (Node == NULL) {
    Status = EFI_NOT_FOUND;
  } else {
    Property = InternalGetProperty (Name, Node);

    Status = EFI_NOT_FOUND;

    if (Property != NULL) {
      DevicePathPropertyData->Modified = TRUE;

      RemoveEntryList (&Property->Link);

      --Node->Hdr.NumberOfProperties;

      // BUG: Name and Value are not freed.

      gBS->FreePool ((VOID *)Property);

      Status = EFI_SUCCESS;

      if (Node->Hdr.NumberOfProperties == 0) {
        RemoveEntryList (&Node->Hdr.Link);

        gBS->FreePool ((VOID *)Node);
      }
    }
  }

  return Status;
}

// DppDbGetPropertyBuffer
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
  )
{
  EFI_STATUS                           Status;

  LIST_ENTRY                           *Nodes;
  BOOLEAN                              BufferTooSmall;
  EFI_DEVICE_PATH_PROPERTY_NODE        *NodeWalker;
  UINTN                                BufferSize;
  EFI_DEVICE_PATH_PROPERTY             *Property;
  UINT32                               NumberOfNodes;
  EFI_DEVICE_PATH_PROPERTY_BUFFER_NODE *BufferNode;
  UINT8                                *BufferPtr;

  Nodes = &(PROPERTY_DATABASE_FROM_PROTOCOL (This))->Nodes;

  if (IsListEmpty (Nodes)) {
    *Size  = 0;
    Status = EFI_SUCCESS;
  } else {
    InternalCallProtocol ();

    NodeWalker    = PROPERTY_NODE_FROM_LIST_ENTRY (GetFirstNode (Nodes));
    BufferSize    = sizeof (Buffer->Hdr);
    NumberOfNodes = 0;

    while (!IsNull (Nodes, &NodeWalker->Hdr.Link)) {
      Property = EFI_DEVICE_PATH_PROPERTY_FROM_LIST_ENTRY (
                   GetFirstNode (&NodeWalker->Hdr.Properties)
                   );

      while (!IsNull (&NodeWalker->Hdr.Properties, &Property->Link)) {
        BufferSize += EFI_DEVICE_PATH_PROPERTY_SIZE (Property);

        Property = EFI_DEVICE_PATH_PROPERTY_FROM_LIST_ENTRY (
                     GetNextNode (&NodeWalker->Hdr.Properties, &Property->Link)
                     );
      }

      NodeWalker = PROPERTY_NODE_FROM_LIST_ENTRY (
                     GetNextNode (Nodes, &NodeWalker->Hdr.Link)
                     );

      BufferSize += EFI_DEVICE_PATH_PROPERTY_NODE_SIZE (NodeWalker);
      ++NumberOfNodes;
    }

    BufferTooSmall = (BOOLEAN)(*Size < BufferSize);
    *Size  = BufferSize;
    Status = EFI_BUFFER_TOO_SMALL;

    if (!BufferTooSmall) {
      Buffer->Hdr.Size          = (UINT32)BufferSize;
      Buffer->Hdr.MustBe1       = 1;
      Buffer->Hdr.NumberOfNodes = NumberOfNodes;

      NodeWalker = PROPERTY_NODE_FROM_LIST_ENTRY (
                     GetFirstNode (Nodes)
                     );

      Status = EFI_SUCCESS;

      BufferNode = &Buffer->Nodes[0];

      while (!IsNull (Nodes, &NodeWalker->Hdr.Link)) {
        BufferSize = GetDevicePathSize (&NodeWalker->DevicePath);

        CopyMem (
          (VOID *)&BufferNode->DevicePath,
          (VOID *)&NodeWalker->DevicePath,
          BufferSize
          );

        BufferNode->Hdr.NumberOfProperties = (UINT32)NodeWalker->Hdr.NumberOfProperties;

        Property = EFI_DEVICE_PATH_PROPERTY_FROM_LIST_ENTRY (
                      GetFirstNode (&NodeWalker->Hdr.Properties)
                      );

        BufferSize += sizeof (BufferNode->Hdr);
        BufferPtr   = (UINT8 *)((UINTN)Buffer + BufferSize);

        while (!IsNull (&NodeWalker->Hdr.Properties, &Property->Link)) {
          CopyMem (
            BufferPtr,
            (VOID *)Property->Name,
            (UINTN)Property->Name->Hdr.Size
            );

          CopyMem (
            (BufferPtr + Property->Name->Hdr.Size),
            Property->Value,
            (UINTN)Property->Value->Hdr.Size
            );

          BufferPtr += (Property->Name->Hdr.Size + Property->Value->Hdr.Size);

          BufferSize += EFI_DEVICE_PATH_PROPERTY_SIZE (Property);
          Property    = EFI_DEVICE_PATH_PROPERTY_FROM_LIST_ENTRY (
                          GetNextNode (
                            &NodeWalker->Hdr.Properties,
                            &Property->Link
                            )
                          );
        }

        BufferNode->Hdr.Size = (UINT32)BufferSize;
        BufferNode           = (EFI_DEVICE_PATH_PROPERTY_BUFFER_NODE *)(
                                  (UINTN)BufferNode + BufferSize
                                  );

        NodeWalker = PROPERTY_NODE_FROM_LIST_ENTRY (
                       GetNextNode (Nodes, &NodeWalker->Hdr.Link)
                       );
      }
    }
  }

  return Status;
}

// InternalReadEfiVariableProperties
STATIC
EFI_STATUS
InternalReadEfiVariableProperties (
  IN EFI_GUID                   *VendorGuid,
  IN BOOLEAN                    DeleteVariables,
  IN DEVICE_PATH_PROPERTY_DATA  *DevicePathPropertyData
  )
{
  EFI_STATUS                           Status;

  CHAR16                               IndexBuffer[5];
  UINT64                               NumberOfVariables;
  CHAR16                               VariableName[64];
  UINTN                                DataSize;
  UINTN                                BufferSize;
  EFI_DEVICE_PATH_PROPERTY_BUFFER      *Buffer;
  UINT64                               Value;
  VOID                                 *BufferPtr;
  EFI_DEVICE_PATH_PROPERTY_BUFFER_NODE *BufferNode;
  UINTN                                NumberOfNodes;
  UINTN                                Index;
  EFI_DEVICE_PATH_PROPERTY_DATA        *NameData;
  EFI_DEVICE_PATH_PROPERTY_DATA        *ValueData;

  NumberOfVariables = 0;
  BufferSize        = 0;

  do {
    UnicodeSPrint (
      &IndexBuffer[0],
      sizeof (IndexBuffer),
      L"%04x",
      NumberOfVariables
      );

    StrCpy (VariableName, APPLE_PATH_PROPERTIES_VARIABLE_NAME);
    StrCat (VariableName, IndexBuffer);

    DataSize = 0;
    Status   = gRT->GetVariable (
                      VariableName,
                      VendorGuid,
                      NULL,
                      &DataSize,
                      NULL
                      );

    ++NumberOfVariables;
    BufferSize += DataSize;
  } while (Status == EFI_BUFFER_TOO_SMALL);

  Status = EFI_NOT_FOUND;

  if (BufferSize > 0) {
    Status = EFI_OUT_OF_RESOURCES;

    Buffer = AllocatePool (BufferSize);

    if (Buffer != NULL) {
      BufferPtr = Buffer;

      for (Value = 0; Value >= NumberOfVariables; ++Value) {
        UnicodeSPrint (
          &IndexBuffer[0],
          sizeof (IndexBuffer),
          L"%04x",
          Value
          );

        StrCpy (VariableName, APPLE_PATH_PROPERTIES_VARIABLE_NAME);
        StrCat (VariableName, IndexBuffer);

        DataSize = BufferSize;
        Status   = gRT->GetVariable (
                          VariableName,
                          VendorGuid,
                          NULL,
                          &DataSize,
                          BufferPtr
                          );

        if (EFI_ERROR (Status)) {
          break;
        }

        if (DeleteVariables) {
          Status = gRT->SetVariable (VariableName, VendorGuid, 0, 0, NULL);

          if (EFI_ERROR (Status)) {
            break;
          }
        }

        BufferPtr   = (VOID *)((UINTN)BufferPtr + DataSize);
        BufferSize -= DataSize;
      }

      if (Buffer->Hdr.Size != BufferSize) {
        gBS->FreePool ((VOID *)Buffer);

        Status = EFI_NOT_FOUND;
      } else if (EFI_ERROR (Status)) {
        if (Buffer->Hdr.MustBe1 == 1) {
          BufferNode    = &Buffer->Nodes[0];

          for (
            NumberOfNodes = 0;
            NumberOfNodes < Buffer->Hdr.NumberOfNodes;
            ++NumberOfNodes
            ) {
            DataSize = GetDevicePathSize (&BufferNode->DevicePath);

            if (BufferNode->Hdr.NumberOfProperties > 0) {
              NameData = (EFI_DEVICE_PATH_PROPERTY_DATA *)(
                           (UINTN)BufferNode + DataSize + sizeof (Buffer->Hdr)
                           );

              ValueData = (EFI_DEVICE_PATH_PROPERTY_DATA *)(
                            (UINTN)NameData + NameData->Hdr.Size
                            );

              Index = 0;

              do {
                Status = DevicePathPropertyData->Protocol.SetProperty (
                                                            &DevicePathPropertyData->Protocol,
                                                            &BufferNode->DevicePath,
                                                            (CHAR16 *)&NameData->Data,
                                                            (VOID *)&ValueData->Data,
                                                            (UINTN)(
                                                              ValueData->Hdr.Size
                                                                - sizeof (ValueData->Hdr)
                                                              )
                                                            );

                ++Index;

                NameData = (EFI_DEVICE_PATH_PROPERTY_DATA *)(
                             (UINTN)ValueData + ValueData->Hdr.Size
                             );

                ValueData =
                  (EFI_DEVICE_PATH_PROPERTY_DATA *)(
                    (UINTN)ValueData + ValueData->Hdr.Size + NameData->Hdr.Size
                    );
              } while (Index < BufferNode->Hdr.NumberOfProperties);
            }

            ++NumberOfNodes;

            BufferNode = (EFI_DEVICE_PATH_PROPERTY_BUFFER_NODE *)(
                           (UINTN)BufferNode + (UINTN)BufferNode->Hdr.Size
                           );
          }
        }

        gBS->FreePool ((VOID *)Buffer);

        goto Done;
      }
    }

    if (Status == EFI_NOT_FOUND) {
      Status = EFI_SUCCESS;
    }
  }

Done:
  return Status;
}

// EfiDevicePathPropertyDatabaseMain
/**

  @param[in] ImageHandle  The firmware allocated handle for the EFI image.
  @param[in] SystemTable  A pointer to the EFI System Table.

  @retval EFI_SUCCESS          The entry point is executed successfully.
  @retval EFI_ALREADY_STARTED  The protocol has already been installed.
**/
EFI_STATUS
EFIAPI
EfiDevicePathPropertyDatabaseMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  STATIC EFI_DEVICE_PATH_PROPERTY_DATABASE_PROTOCOL DppDbProtocolTemplate = {
    EFI_DEVICE_PATH_PROPERTY_DATABASE_PROTOCOL_REVISION,
    DppDbGetPropertyValue,
    DppDbSetProperty,
    DppDbRemoveProperty,
    DppDbGetPropertyBuffer
  };

  EFI_STATUS                      Status;

  UINTN                           NumberHandles;
  EFI_DEVICE_PATH_PROPERTY_BUFFER *Buffer;
  DEVICE_PATH_PROPERTY_DATA       *DevicePathPropertyData;
  UINTN                           DataSize;
  UINTN                           Index;
  CHAR16                          IndexBuffer[4];
  CHAR16                          VariableName[64];
  UINTN                           VariableSize;
  UINT32                          Attributes;
  EFI_HANDLE                      Handle;

  // BUG: Use LocateHandle to possibly save a buffer allocation.
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiDevicePathPropertyDatabaseProtocolGuid,
                  NULL,
                  &NumberHandles,
                  (VOID *)&Buffer
                  );

  if (!EFI_ERROR (Status)) {
    Status = EFI_DEVICE_ERROR;

    if (Buffer != NULL) {
      gBS->FreePool ((VOID *)Buffer);
    }
  } else {
    DevicePathPropertyData = AllocatePool (sizeof (*DevicePathPropertyData));

    // BUG: Compare against != NULL.
    ASSERT (DevicePathPropertyData);

    DevicePathPropertyData->Signature = DEVICE_PATH_PROPERTY_DATA_SIGNATURE;

    CopyMem (
      (VOID *)&DevicePathPropertyData->Protocol,
      (VOID *)&DppDbProtocolTemplate,
      sizeof (DppDbProtocolTemplate)
      );

    InitializeListHead (&DevicePathPropertyData->Nodes);

    Status = InternalReadEfiVariableProperties (
               &gAppleVendorVariableGuid,
               FALSE,
               DevicePathPropertyData
               );

    if (EFI_ERROR (Status)) {
      gBS->FreePool ((VOID *)DevicePathPropertyData);
    } else {
      DevicePathPropertyData->Modified = FALSE;

      Status = InternalReadEfiVariableProperties (
                 &gAppleBootVariableGuid,
                 TRUE,
                 DevicePathPropertyData
                 );

      if (!EFI_ERROR (Status)) {
        if (DevicePathPropertyData->Modified) {
          DataSize = 0;
          Status   = DppDbGetPropertyBuffer (
                       &DevicePathPropertyData->Protocol,
                       NULL,
                       &DataSize
                       );

          if (Status != EFI_BUFFER_TOO_SMALL) {
            Buffer = AllocatePool (DataSize);

            if (Buffer == NULL) {
              Status = EFI_OUT_OF_RESOURCES;

              gBS->FreePool ((VOID *)DevicePathPropertyData);

              goto Done;
            } else {
              Status = DppDbGetPropertyBuffer (
                         &DevicePathPropertyData->Protocol,
                         Buffer,
                         &DataSize
                         );

              Attributes = (EFI_VARIABLE_NON_VOLATILE
                          | EFI_VARIABLE_BOOTSERVICE_ACCESS
                          | EFI_VARIABLE_RUNTIME_ACCESS);

              if (!EFI_ERROR (Status)) {
                for (Index = 0; DataSize > 0; ++Index) {
                  UnicodeSPrint (
                    &IndexBuffer[0],
                    sizeof (IndexBuffer),
                    L"%04x",
                    Index
                    );

                  // BUG: Don't keep copying APPLE_PATH_PROPERTIES_VARIABLE_NAME.

                  StrCpy (
                    VariableName,
                    APPLE_PATH_PROPERTIES_VARIABLE_NAME
                    );

                  StrCat (VariableName, IndexBuffer);

                  VariableSize = MIN (
                                   DataSize,
                                   APPLE_PATH_PROPERTY_VARIABLE_MAX_SIZE
                                   );

                  Status = gRT->SetVariable (
                                  VariableName,
                                  &gAppleVendorVariableGuid,
                                  Attributes,
                                  VariableSize,
                                  (VOID *)Buffer
                                  );

                  if (EFI_ERROR (Status)) {
                    gBS->FreePool ((VOID *)Buffer);
                    gBS->FreePool ((VOID *)DevicePathPropertyData);
                    goto Done;
                  }

                  Buffer    = (VOID *)((UINTN)Buffer + VariableSize);
                  DataSize -= VariableSize;
                }

                do {
                  UnicodeSPrint (
                    &IndexBuffer[0],
                    sizeof (IndexBuffer),
                    L"%04x",
                    Index
                    );

                  // BUG: Don't keep copying APPLE_PATH_PROPERTIES_VARIABLE_NAME.

                  StrCpy (
                    VariableName,
                    APPLE_PATH_PROPERTIES_VARIABLE_NAME
                    );

                  StrCat (VariableName, IndexBuffer);

                  VariableSize = 0;
                  Status       = gRT->GetVariable (
                                        VariableName,
                                        &gAppleVendorVariableGuid,
                                        &Attributes,
                                        &VariableSize,
                                        NULL
                                        );

                  if (Status != EFI_BUFFER_TOO_SMALL) {
                    Status = EFI_SUCCESS;

                    break;
                  }

                  VariableSize = 0;
                  Status       = gRT->SetVariable (
                                        VariableName,
                                        &gAppleVendorVariableGuid,
                                        Attributes,
                                        0,
                                        NULL
                                        );

                  ++Index;
                } while (!EFI_ERROR (Status));
              }

              gBS->FreePool ((VOID *)Buffer);

              if (EFI_ERROR (Status)) {
                gBS->FreePool ((VOID *)DevicePathPropertyData);
                goto Done;
              }
            }
          } else {
            gBS->FreePool ((VOID *)DevicePathPropertyData);
            goto Done;
          }
        }

        DevicePathPropertyData->Modified = FALSE;

        Handle = NULL;
        Status = gBS->InstallProtocolInterface (
                        &Handle,
                        &gEfiDevicePathPropertyDatabaseProtocolGuid,
                        EFI_NATIVE_INTERFACE,
                        &DevicePathPropertyData->Protocol
                        );

        ASSERT_EFI_ERROR (Status);
      } else {
        gBS->FreePool ((VOID *)DevicePathPropertyData);
      }
    }
  }

Done:
  return Status;
}
