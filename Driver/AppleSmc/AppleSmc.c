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

#include EFI_PROTOCOL_CONSUMER (CpuIo)
#include APPLE_PROTOCOL_PRODUCER (AppleSmcIoImpl)

#include <Library/AppleDriverLib.h>

// NEXT_SMC_ADDRESS
#define NEXT_SMC_ADDRESS(Address)                         \
  ((((Address) & 0xFF0000) | (((Address) >> 16) >> 8))    \
    | (((Address) & 0xFF00) | (((Address) << 16) << 8)))

// mAppleSmcIoProtocolTemplate
STATIC APPLE_SMC_IO_PROTOCOL mAppleSmcIoProtocolTemplate = {
  APPLE_SMC_IO_PROTOCOL_REVISION,
  SmcIoSmcReadValueImpl,
  SmcIoSmcWriteValueImpl,
  SmcIoSmcGetKeyCountImpl,
  SmcIoSmcMakeKeyImpl,
  SmcIoSmcGetKeyFromIndexImpl,
  SmcIoSmcGetKeyInfoImpl,
  SmcIoSmcResetImpl,
  SmcIoSmcFlashTypeImpl,
  SmcIoSmcUnsupportedImpl,
  SmcIoSmcFlashWriteImpl,
  SmcIoSmcFlashAuthImpl,
  0,
  SMC_PORT_BASE,
  FALSE,
  SmcIoSmcUnknown1Impl,
  SmcIoSmcUnknown2Impl,
  SmcIoSmcUnknown3Impl,
  SmcIoSmcUnknown4Impl,
  SmcIoSmcUnknown5Impl
};

EFI_DRIVER_ENTRY_POINT (AppleSmcMain);

// AppleSmcIoMain
EFI_STATUS
EFIAPI
AppleSmcMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  ) // start
{
  EFI_STATUS          Status;

  EFI_CPU_IO_PROTOCOL *CpuIo;
  SMC_DEV             *SmcDev;
  UINT8               NumberOfSmcDevices;
  UINT8               Index;
  SMC_INDEX           SmcIndex;
  SMC_ADDRESS         SmcAddress;
  SMC_DEV             *SmcDevChild;

  AppleInitializeDriverLib (ImageHandle, SystemTable);

  ASSERT_PROTOCOL_ALREADY_INSTALLED (NULL, &gAppleSmcIoProtocolGuid);

  Status = gBS->LocateProtocol (&gEfiCpuIoProtocolGuid, NULL, (VOID **)&CpuIo);

  if (!EFI_ERROR (Status)) {
    SmcDev = EfiLibAllocateZeroPool (sizeof (*SmcDev));

    ASSERT (SmcDev != NULL);

    if (SmcDev == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
    } else {
      SmcDev->Signature = SMC_DEV_SIGNATURE;
      SmcDev->CpuIo     = CpuIo;

      EfiInitializeLock (&SmcDev->Lock, EFI_TPL_NOTIFY);

      EfiCopyMem (
        (VOID *)&SmcDev->SmcIo,
        (VOID *)&mAppleSmcIoProtocolTemplate,
        sizeof (mAppleSmcIoProtocolTemplate)
        );

      Status = gBS->InstallProtocolInterface (
                      &SmcDev->Handle,
                      &gAppleSmcIoProtocolGuid,
                      EFI_NATIVE_INTERFACE,
                      (VOID *)&SmcDev->SmcIo
                      );

      if (EFI_ERROR (Status)) {
        gBS->FreePool ((VOID *)SmcDev);
      } else {
        NumberOfSmcDevices = 1;

        SmcIoSmcReadValueImpl (
          &SmcDev->SmcIo,
          SMC_KEY_NUM,
          sizeof (NumberOfSmcDevices),
          (VOID *)&NumberOfSmcDevices
          );

        Status = EFI_SUCCESS;

        for (Index = 1; Index < NumberOfSmcDevices; ++Index) {
          SmcIndex = Index;
          Status   = SmcIoSmcWriteValueImpl (
                       &SmcDev->SmcIo,
                       SMC_KEY_NUM,
                       sizeof (SmcIndex),
                       (VOID *)&SmcIndex
                       );

          if (!EFI_ERROR (Status)) {
            Status = SmcIoSmcReadValueImpl (
                       &SmcDev->SmcIo,
                       SMC_KEY_ADR,
                       sizeof (SmcAddress),
                       (VOID *)&SmcAddress
                       );

            if (!EFI_ERROR (Status)) {
              SmcDevChild = EfiLibAllocateZeroPool (sizeof (*SmcDevChild));

              if (SmcDevChild != NULL) {
                SmcDevChild->Signature = SMC_DEV_SIGNATURE;
                SmcDevChild->CpuIo     = CpuIo;

                EfiInitializeLock (&SmcDevChild->Lock, EFI_TPL_NOTIFY);

                EfiCopyMem (
                  (VOID *)&SmcDevChild->SmcIo,
                  (VOID *)&mAppleSmcIoProtocolTemplate,
                  sizeof (mAppleSmcIoProtocolTemplate)
                  );

                SmcDevChild->SmcIo.Index   = Index;
                SmcDevChild->SmcIo.Address = NEXT_SMC_ADDRESS (SmcAddress);

                Status = gBS->InstallProtocolInterface (
                                &SmcDevChild->Handle,
                                &gAppleSmcIoProtocolGuid,
                                EFI_NATIVE_INTERFACE,
                                (VOID *)&SmcDevChild->SmcIo
                                );

                if (EFI_ERROR (Status)) {
                  gBS->FreePool ((VOID *)SmcDevChild);
                }
              }
            }
          }
        }
      }
    }
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}
