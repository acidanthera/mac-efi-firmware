/** @file
  Boot service DXE BIOS ID library implementation.

  These functions in this file can be called during DXE and cannot be called during runtime
  or in SMM which should use a RT or SMM library.

Copyright (c) 2011 - 2014, Intel Corporation. All rights reserved.<BR>
Portions Copyright (c) 2005 - 2017, Apple Inc.  All rights revserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiDxe.h>
#include <Guid/BiosId.h>
#include <Library/BaseLib.h>
#include <Library/HobLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>

#include <Library/BiosIdLib.h>
#include <Protocol/FirmwareVolume.h>
#include <Protocol/LoadedImage.h>

STATIC
VOID *
InternalGetImage (
  IN  EFI_GUID           *NameGuid,
  IN  EFI_SECTION_TYPE   SectionType,
  OUT UINTN              *Size
  )
{
  VOID                         *Buffer;
  EFI_STATUS                   Status;
  EFI_HANDLE                   *HandleBuffer;
  UINTN                        NumberOfHandles;
  UINTN                        Index;
  EFI_FIRMWARE_VOLUME_PROTOCOL *Fv;
  UINT32                       AuthenticationStatus;

  Buffer = NULL;

  HandleBuffer = NULL;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiFirmwareVolumeProtocolGuid,
                  NULL,
                  &NumberOfHandles,
                  &HandleBuffer
                  );
  if (!EFI_ERROR (Status)) {
    //
    // Find desired image in all Fvs
    //
    for (Index = 0; Index < NumberOfHandles; ++Index) {
      Status = gBS->HandleProtocol (
                      HandleBuffer[Index],
                      &gEfiFirmwareVolumeProtocolGuid,
                      (VOID**)&Fv
                      );

      if (EFI_ERROR (Status)) {
        break;
      }

      //
      // Read desired section content in NameGuid file
      //
      Buffer = NULL;
      *Size  = 0;
      Status = Fv->ReadSection (
                     Fv,
                     NameGuid,
                     SectionType,
                     0,
                     &Buffer,
                     Size,
                     &AuthenticationStatus
                     );

      if (!EFI_ERROR (Status)) {
        break;
      }
    }

    FreePool(HandleBuffer);
  }

  return Buffer;
}

// InternalGetBiosId
/** This function returns BIOS ID by searching FV.

  @return BIOS ID
**/
STATIC
BIOS_ID_IMAGE *
InternalGetBiosId (
  VOID
  )
{
  BIOS_ID_IMAGE *BiosId;

  UINTN         Size;

  DEBUG ((EFI_D_INFO, "Get BIOS ID from FV\n"));

  BiosId = InternalGetImage (
             &gEfiBiosIdGuid,
             EFI_SECTION_RAW,
             &Size
             );

  //
  // BiosId image is present in FV
  //
  if (BiosId != NULL) {
    DEBUG ((EFI_D_INFO, "Get BIOS ID from FV successfully\n"));
    DEBUG ((EFI_D_INFO, "BIOS ID: %s\n", (CHAR16 *)&BiosId->BiosIdString));
  }

  return BiosId;
}

// GetRomInfo
/** This function returns ROM Info by searching FV.

  @param[out] RomInfo  ROM Info
**/
VOID
GetRomInfo (
  OUT APPLE_ROM_INFO_STRING  *RomInfo
  )
{
  APPLE_ROM_INFO_STRING RomInfoString;

  UINTN                 Size;

  RomInfoString = InternalGetImage (
                    &gAppleRomInfoGuid,
                    EFI_SECTION_RAW,
                    &Size
                    );

  //
  // RomInfo image is present in FV
  //
  if (RomInfoString != NULL) {
    DEBUG ((EFI_D_INFO, "Get ROM Info from FV successfully\n"));
    DEBUG ((EFI_D_INFO, "ROM Info: %s\n", RomInfoString));
  }

  *RomInfo = RomInfoString;
}

// GetBiosVersionDateTime
/** This function returns the Version & Release Date by getting and converting
    BIOS ID.

  @param[out] BiosVersion      The Bios Version out of the conversion.
  @param[out] BiosReleaseDate  The Bios Release Date out of the conversion.
**/
VOID
GetBiosVersionDateTime (
  OUT CHAR8  **BiosVersion,
  OUT CHAR8  **BiosReleaseDate
  )
{
  BIOS_ID_IMAGE *BiosIdImage;
  UINTN         Size;
  CHAR16        *BiosId;
  CHAR8         *Destination;

  *BiosVersion     = NULL;
  *BiosReleaseDate = NULL;

  BiosIdImage = InternalGetBiosId ();

  if (BiosIdImage != NULL) {
    //
    // Fill the BiosVersion data from the BIOS ID.
    //
    BiosId = (CHAR16 *)&BiosIdImage->BiosIdString.BoardId[0];

    while (*BiosId == L' ') {
      ++BiosId;
    }

    // BUG: Use ASCII size.
    Size         = StrSize (BiosId);
    *BiosVersion = AllocateZeroPool (Size);

    if (*BiosVersion != NULL) {
      Destination = *BiosVersion;

      while (*BiosId != L'\0') {
        *Destination = (CHAR8)*BiosId;

        ++Destination;
        ++BiosId;
      }
      
      // BUG: Use ASCII size.
      Size             = StrSize (L"MM/DD/20YY");
      *BiosReleaseDate = AllocateZeroPool (Size);

      if (*BiosReleaseDate != NULL) {
        //
        // Fill the build timestamp date from the BIOS ID in the "MM/DD/YY" format.
        //
        (*BiosReleaseDate)[0] = (CHAR8)BiosIdImage->BiosIdString.TimeStamp[2];
        (*BiosReleaseDate)[1] = (CHAR8)BiosIdImage->BiosIdString.TimeStamp[3];
        (*BiosReleaseDate)[2] = '/';

        (*BiosReleaseDate)[3] = (CHAR8)BiosIdImage->BiosIdString.TimeStamp[4];
        (*BiosReleaseDate)[4] = (CHAR8)BiosIdImage->BiosIdString.TimeStamp[5];
        (*BiosReleaseDate)[5] = '/';

        //
        // Add 20 for SMBIOS table
        // Current Linux kernel will misjudge 09 as year 0, so using 2009 for SMBIOS table
        //
        (*BiosReleaseDate)[6] = '2';
        (*BiosReleaseDate)[7] = '0';
        (*BiosReleaseDate)[8] = (CHAR8)BiosIdImage->BiosIdString.TimeStamp[0];
        (*BiosReleaseDate)[9] = (CHAR8)BiosIdImage->BiosIdString.TimeStamp[1];
      } else {
        FreePool ((VOID *)*BiosVersion);

        *BiosVersion = NULL;
      }
    }

    if (BiosIdImage != NULL) {
      FreePool ((VOID *)BiosIdImage);
    }
  }
}
