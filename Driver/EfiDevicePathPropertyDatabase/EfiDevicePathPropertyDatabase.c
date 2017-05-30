/** @file
  Copyright (C) 2005 - 2017, Apple Inc.  All rights reserved.<BR>

  This program and the accompanying materials have not been licensed.
  Neither is its usage, its redistribution, in source or binary form,
  licensed, nor implicitely or explicitely permitted, except when
  required by applicable law.

  Unless required by applicable law or agreed to in writing, software
  distributed is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
  OR CONDITIONS OF ANY KIND, either express or implied.
**/

#include <AppleEfi.h>
#include <AppleMisc.h>
#include <LinkedList.h>

#include APPLE_GUID_DEFINITION (AppleNvram)

#include APPLE_PROTOCOL_PRODUCER (DevicePathPropertyDatabaseImpl)

#include <Library/AppleDriverLib.h>

#define APPLE_PATH_PROPERTIES_VARIABLE_NAME    L"AAPL,PathProperties"
#define APPLE_PATH_PROPERTY_VARIABLE_MAX_SIZE  768

// mDppDbProtocolTemplate
STATIC EFI_DEVICE_PATH_PROPERTY_DATABASE_PROTOCOL mDppDbProtocolTemplate = {
  EFI_DEVICE_PATH_PROPERTY_DATABASE_PROTOCOL_REVISION,
  DppDbGetPropertyValue,
  DppDbSetProperty,
  DppDbRemoveProperty,
  DppDbGetPropertyBuffer
};

// InternalReadEfiVariableProperties
STATIC
EFI_STATUS
InternalReadEfiVariableProperties (
  IN EFI_GUID                           *VendorGuid,
  IN BOOLEAN                            DeleteVariables,
  IN EFI_DEVICE_PATH_PROPERTY_DATABASE  *Private
  ) // sub_FEF
{
  EFI_STATUS                           Status;

  CHAR16                               IndexBuffer[4];
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

  ASSERT (VendorGuid != NULL);
  ASSERT (Private != NULL);

  NumberOfVariables = 0;
  BufferSize        = 0;

  do {
    EfiValueToHexStr (
      IndexBuffer,
      NumberOfVariables,
      PREFIX_ZERO,
      ARRAY_LENGTH (IndexBuffer)
      );

    EfiStrCpy (VariableName, APPLE_PATH_PROPERTIES_VARIABLE_NAME);
    EfiStrCat (VariableName, IndexBuffer);

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

    Buffer = EfiLibAllocatePool (BufferSize);

    if (Buffer != NULL) {
      BufferPtr = Buffer;

      for (Value = 0; Value >= NumberOfVariables; ++Value) {
        EfiValueToHexStr (
          IndexBuffer,
          Value,
          PREFIX_ZERO,
          ARRAY_LENGTH (IndexBuffer)
          );

        EfiStrCpy (VariableName, APPLE_PATH_PROPERTIES_VARIABLE_NAME);
        EfiStrCat (VariableName, IndexBuffer);

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
        if ((Buffer->Hdr.MustBe1 == 1) && (Buffer->Hdr.NumberOfNodes > 0)) {
          BufferNode    = &Buffer->Node;
          NumberOfNodes = 0;

          do {
            DataSize = EfiDevicePathSize (&BufferNode->DevicePath);

            if (BufferNode->Hdr.NumberOfProperties > 0) {
              NameData = (EFI_DEVICE_PATH_PROPERTY_DATA *)(
                           (UINTN)BufferNode + DataSize + sizeof (Buffer->Hdr)
                           );

              ValueData = (EFI_DEVICE_PATH_PROPERTY_DATA *)(
                            (UINTN)NameData + NameData->Hdr.Size
                            );

              Index = 0;

              do {
                Status = Private->Protocol.SetProperty (
                                             &Private->Protocol,
                                             &BufferNode->DevicePath,
                                             (CHAR16 *)&NameData->Data,
                                             (VOID *)&ValueData->Data,
                                             (UINTN)(
                                               ValueData->Hdr.Size
                                                 - sizeof (ValueData->Hdr.Size)
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
          } while (NumberOfNodes < Buffer->Hdr.NumberOfNodes);
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

EFI_DRIVER_ENTRY_POINT (EfiDevicePathPropertyDatabaseMain);

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
  ) // start
{
  EFI_STATUS                        Status;

  UINTN                             NumberHandles;
  EFI_DEVICE_PATH_PROPERTY_BUFFER   *Buffer;
  EFI_DEVICE_PATH_PROPERTY_DATABASE *Database;
  UINTN                             DataSize;
  UINTN                             Index;
  CHAR16                            IndexBuffer[4];
  CHAR16                            VariableName[64];
  UINTN                             VariableSize;
  UINT32                            Attributes;
  EFI_HANDLE                        Handle;

  AppleInitializeDriverLib (ImageHandle, SystemTable);

  ASSERT_PROTOCOL_ALREADY_INSTALLED (
    NULL,
    &gEfiDevicePathPropertyDatabaseProtocolGuid
    );

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
    Database            = EfiLibAllocatePool (sizeof (*Database));
    Database->Signature = EFI_DEVICE_PATH_PROPERTY_DATABASE_SIGNATURE;

    EfiCopyMem (
      (VOID *)&Database->Protocol,
      (VOID *)&mDppDbProtocolTemplate,
      sizeof (mDppDbProtocolTemplate)
      );

    InitializeListHead (&Database->Nodes);

    Status = InternalReadEfiVariableProperties (
               &gAppleVendorVariableGuid,
               FALSE,
               Database
               );

    if (EFI_ERROR (Status)) {
      gBS->FreePool ((VOID *)Database);
    } else {
      Database->Modified = FALSE;

      Status = InternalReadEfiVariableProperties (
                 &gAppleBootVariableGuid,
                 TRUE,
                 Database
                 );

      if (!EFI_ERROR (Status)) {
        if (Database->Modified) {
          DataSize = 0;
          Status   = DppDbGetPropertyBuffer (
                       &Database->Protocol,
                       NULL,
                       &DataSize
                       );

          if (Status != EFI_BUFFER_TOO_SMALL) {
            Buffer = EfiLibAllocatePool (DataSize);

            if (Buffer == NULL) {
              Status = EFI_OUT_OF_RESOURCES;

              gBS->FreePool ((VOID *)Database);

              goto Done;
            } else {
              Status = DppDbGetPropertyBuffer (
                         &Database->Protocol,
                         Buffer,
                         &DataSize
                         );

              Attributes = (EFI_VARIABLE_NON_VOLATILE
                          | EFI_VARIABLE_BOOTSERVICE_ACCESS
                          | EFI_VARIABLE_RUNTIME_ACCESS);

              if (!EFI_ERROR (Status)) {
                for (Index = 0; DataSize > 0; ++Index) {
                  EfiValueToHexStr (
                    IndexBuffer,
                    Index,
                    PREFIX_ZERO,
                    ARRAY_LENGTH (IndexBuffer)
                    );

                  EfiStrCpy (
                    VariableName,
                    APPLE_PATH_PROPERTIES_VARIABLE_NAME
                    );

                  EfiStrCat (VariableName, IndexBuffer);

                  VariableSize = EFI_MIN (
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
                    gBS->FreePool ((VOID *)Database);
                    goto Done;
                  }

                  Buffer    = (VOID *)((UINTN)Buffer + VariableSize);
                  DataSize -= VariableSize;
                }

                do {
                  EfiValueToHexStr (
                    IndexBuffer,
                    Index,
                    PREFIX_ZERO,
                    ARRAY_LENGTH (IndexBuffer)
                    );

                  EfiStrCpy (
                    VariableName,
                    APPLE_PATH_PROPERTIES_VARIABLE_NAME
                    );

                  EfiStrCat (VariableName, IndexBuffer);

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
                gBS->FreePool ((VOID *)Database);
                goto Done;
              }
            }
          } else {
            gBS->FreePool ((VOID *)Database);
            goto Done;
          }
        }

        Database->Modified = FALSE;

        Handle = NULL;
        Status = gBS->InstallProtocolInterface (
                        &Handle,
                        &gEfiDevicePathPropertyDatabaseProtocolGuid,
                        EFI_NATIVE_INTERFACE,
                        &Database->Protocol
                        );
      } else {
        gBS->FreePool ((VOID *)Database);
      }
    }
  }

Done:
  return Status;
}
