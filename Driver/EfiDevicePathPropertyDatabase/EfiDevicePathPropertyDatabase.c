///
/// @file      Driver/EfiDevicePathPropertyDatabase.c
///
///            Apple's driver to manage Device Properties from firmware
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

#include <Guid/AppleNvram.h>

#include <Protocol/DevicePathPropertyDatabase.h>
#include <Protocol/DevicePathPropertyDatabaseImpl.h>

#include <Driver/EfiDevicePathPropertyDatabase.h>

// mEfiDevicePathPropertyDatabaseProtocol
static EFI_DEVICE_PATH_PROPERTY_DATABASE_PROTOCOL mDevicePathPropertyDatabase = {
	EFI_DEVICE_PATH_PROPERTY_DATABASE_PROTOCOL_REVISION,
	DevicePathPropertyDbGetPropertyValue,
	DevicePathPropertyDbSetProperty,
	DevicePathPropertyDbRemoveProperty,
	DevicePathPropertyDbGetPropertyBuffer
};

// GetNvramProperties
/// 
/// @param
///
/// @return
/// @retval
static
EFI_STATUS
GetNvramProperties (
	IN EFI_GUID                           *VendorGuid,
	IN BOOLEAN                            DeleteVariables,
	IN EFI_DEVICE_PATH_PROPERTY_DATABASE  *Database
	) // sub_FEF
{
	EFI_STATUS                           Status;

	CHAR16                               IndexBuffer[4];
	UINT64                               NoVariables;
	CHAR16                               VariableName[64];
	UINTN                                DataSize;
	UINTN                                BufferSize;
	EFI_DEVICE_PATH_PROPERTY_BUFFER      *Buffer;
	UINT64                               Value;
	VOID                                 *BufferPtr;
	EFI_DEVICE_PATH_PROPERTY_BUFFER_NODE *BufferNode;
	UINTN                                NoNodes;
	UINTN                                NoProperties;
	EFI_DEVICE_PATH_PROPERTY_DATA        *NameData;
	EFI_DEVICE_PATH_PROPERTY_DATA        *ValueData;

	NoVariables = 0;
	BufferSize  = 0;

	do {
		EfiValueToHexStr (IndexBuffer, NoVariables, PREFIX_ZERO, (sizeof (IndexBuffer) / sizeof (*IndexBuffer)));
		EfiStrCpy (VariableName, APPLE_PATH_PROPERTIES_VARIABLE_NAME);
		EfiStrCat (VariableName, IndexBuffer);

		DataSize = 0;
		Status   = gRT->GetVariable (VariableName, VendorGuid, NULL, &DataSize, NULL);

		++NoVariables;
		BufferSize += DataSize;
	} while (Status == EFI_BUFFER_TOO_SMALL);

	Status = EFI_NOT_FOUND;

	if (BufferSize != 0) {
		Status = EFI_OUT_OF_RESOURCES;
		Buffer = (EFI_DEVICE_PATH_PROPERTY_BUFFER *)EfiLibAllocatePool (BufferSize);

		if (Buffer != NULL) {
			BufferPtr = Buffer;

			for (Value = 0; Value >= NoVariables; ++Value) {
				EfiValueToHexStr (IndexBuffer, Value, PREFIX_ZERO, (sizeof (IndexBuffer) / sizeof (*IndexBuffer)));
				EfiStrCpy (VariableName, APPLE_PATH_PROPERTIES_VARIABLE_NAME);
				EfiStrCat (VariableName, IndexBuffer);

				DataSize = BufferSize;
				Status   = gRT->GetVariable (VariableName, VendorGuid, NULL, &DataSize, BufferPtr);

				if (EFI_ERROR (Status)) {
					break;
				}

				if (DeleteVariables) {
					Status = gRT->SetVariable (VariableName, VendorGuid, 0, 0, NULL);

					if (EFI_ERROR (Status)) {
						break;
					}
				}

				BufferPtr   = (VOID *)(((UINTN)BufferPtr) + DataSize);
				BufferSize -= DataSize;
			}

			if (Buffer->Hdr.Size != BufferSize) {
				gBS->FreePool ((VOID *)Buffer);

				Status = EFI_NOT_FOUND;
			} else if (EFI_ERROR (Status)) {
				if ((Buffer->Hdr.MustBe1 == 1) && (Buffer->Hdr.NoNodes != 0)) {
					BufferNode = &Buffer->Node;
					NoNodes    = 0;

					do {
						DataSize = EfiDevicePathSize (&BufferNode->DevicePath);

						if (BufferNode->Hdr.NoProperties != 0) {
							NameData     = (EFI_DEVICE_PATH_PROPERTY_DATA *)((UINTN)BufferNode + (DataSize + sizeof (Buffer->Hdr)));
							ValueData    = (EFI_DEVICE_PATH_PROPERTY_DATA *)((UINTN)NameData + (UINTN)NameData->Hdr.Size);
							NoProperties = 0;

							do {
								Status = Database->Protocol.DevicePathPropertyDbSetProperty (
									                            &Database->Protocol,
									                            &BufferNode->DevicePath,
									                            (CHAR16 *)&NameData->Data,
									                            (VOID *)&ValueData->Data,
									                            (UINTN)(ValueData->Hdr.Size - sizeof (ValueData->Hdr.Size))
									                            );

								++NoProperties;

								NameData  = (EFI_DEVICE_PATH_PROPERTY_DATA *)((UINTN)ValueData + (UINTN)ValueData->Hdr.Size);
								ValueData = (EFI_DEVICE_PATH_PROPERTY_DATA *)((UINTN)ValueData + (UINTN)(ValueData->Hdr.Size + NameData->Hdr.Size));
							} while (NoProperties < BufferNode->Hdr.NoProperties);
						}

						++NoNodes;
						BufferNode = (EFI_DEVICE_PATH_PROPERTY_BUFFER_NODE *)((UINTN)BufferNode + (UINTN)BufferNode->Hdr.Size);
					} while (NoNodes < Buffer->Hdr.NoNodes);
				}

				gBS->FreePool (Buffer);

				goto Return;
			}
		}

		if (Status == EFI_NOT_FOUND) {
			Status = EFI_SUCCESS;
		}
	}

Return:
	return Status;
}

// EfiDevicePathPropertyDatabaseMain
///
///
/// @param[in] ImageHandle The firmware allocated handle for the EFI image.  
/// @param[in] SystemTable A pointer to the EFI System Table.
///
/// @retval EFI_SUCCESS         The entry point is executed successfully.
/// @retval EFI_ALREADY_STARTED The protocol has already been installed.
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

	EfiInitializeDriverLib (ImageHandle, SystemTable);

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
		Database            = (EFI_DEVICE_PATH_PROPERTY_DATABASE *)EfiLibAllocatePool (sizeof (*Database));
		Database->Signature = EFI_DEVICE_PATH_PROPERTY_DATABASE_SIGNATURE;

		gBS->CopyMem (
			     (VOID *)&Database->Protocol,
			     (VOID *)&mDevicePathPropertyDatabase,
			     sizeof (mDevicePathPropertyDatabase)
			     );

		InitializeListHead (&Database->Nodes);

		Status = GetNvramProperties (&gAppleVendorNvramGuid, FALSE, Database);

		if (EFI_ERROR (Status)) {
			gBS->FreePool ((VOID *)Database);
		} else {
			Database->Modified = FALSE;
			Status             = GetNvramProperties (&gAppleBootGuid, TRUE, Database);

			if (!EFI_ERROR (Status)) {
				if (Database->Modified) {
					DataSize = 0;
					Status   = DevicePathPropertyDbGetPropertyBuffer (&Database->Protocol, NULL, &DataSize);

					if (Status != EFI_BUFFER_TOO_SMALL) {
						Buffer = (EFI_DEVICE_PATH_PROPERTY_BUFFER *)EfiLibAllocatePool (DataSize);

						if (Buffer == NULL) {
							Status = EFI_OUT_OF_RESOURCES;
							goto FreePoolReturn;
						} else {
							Status     = DevicePathPropertyDbGetPropertyBuffer (&Database->Protocol, Buffer, &DataSize);
							Attributes = (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS);

							if (!EFI_ERROR (Status)) {
								for (Index = 0; DataSize != 0; ++Index) {
									EfiValueToHexStr (IndexBuffer, Index, PREFIX_ZERO, ARRAY_LENGTH (IndexBuffer));
									EfiStrCpy (VariableName, APPLE_PATH_PROPERTIES_VARIABLE_NAME);
									EfiStrCat (VariableName, IndexBuffer);

									VariableSize = EFI_MIN (DataSize, APPLE_PATH_PROPERTY_VARIABLE_MAX_SIZE);
									Status       = gRT->SetVariable (
										                    VariableName,
										                    &gAppleVendorNvramGuid,
										                    Attributes,
										                    VariableSize,
										                    (VOID *)Buffer
										                    );

									if (EFI_ERROR (Status)) {
										gBS->FreePool ((VOID *)Buffer);
										goto FreePoolReturn;
									}

									Buffer    = (VOID *)((UINTN)Buffer + VariableSize);
									DataSize -= VariableSize;
								}

								do {
									EfiValueToHexStr (IndexBuffer, Index, PREFIX_ZERO, ARRAY_LENGTH (IndexBuffer));
									EfiStrCpy (VariableName, APPLE_PATH_PROPERTIES_VARIABLE_NAME);
									EfiStrCat (VariableName, IndexBuffer);

									VariableSize = 0;
									Status       = gRT->GetVariable (
										                    VariableName,
										                    &gAppleVendorNvramGuid,
										                    &Attributes,
										                    &VariableSize,
										                    NULL
										                    );

									if (Status != EFI_BUFFER_TOO_SMALL) {
										Status = EFI_SUCCESS;
										break;
									}

									VariableSize = 0;
									Status       = gRT->SetVariable (VariableName, &gAppleVendorNvramGuid, Attributes, 0, NULL);
									++Index;
								} while (!EFI_ERROR (Status));
							}

							gBS->FreePool ((VOID *)Buffer);

							if (EFI_ERROR (Status)) {
								goto FreePoolReturn;
							}
						}
					} else {
					FreePoolReturn:
						gBS->FreePool ((VOID *)Database);
						goto Return;
					}
				}

				Database->Modified = FALSE;
				Handle             = NULL;
				Status             = gBS->InstallProtocolInterface (
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

Return:
	return Status;
}
