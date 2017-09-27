#include <AppleMacEfi.h>

#include <Protocol/ApplePlatformInfoDatabase.h>

#include <Library/UefiBootServicesTableLib.h>

UINT32 mData;

Main (

)
{
  EFI_STATUS                            Status;

  EFI_GUID                              NameGuid; // Todo
  VOID                                  *Interface;
  APPLE_PLATFORM_INFO_DATABASE_PROTOCOL *PlatformInfoDb;
  UINT32 Data;
  UINT32 DataSize;

  Status = gBS->LocateProtocol (
                  NULL, // Todo,
                  NULL,
                  &Interface
                  );

  if (!EFI_ERROR (Status)) {
    Status = EFI_ALREADY_STARTED;
  } else {
    Status = gBS->LocateProtocol (
                    &gApplePlatformInfoDatabaseProtocolGuid,
                    NULL,
                    (VOID **)&PlatformInfoDb
                    );

    if (!EFI_ERROR (Status)) {
      DataSize = sizeof (Data);
      Status   = PlatformInfoDb->GetFirstData (
                                   PlatformInfoDb,
                                   &NameGuid,
                                   (VOID *)&Data,
                                   &DataSize
                                   );

      if (!EFI_ERROR (Status)) {
        mData = Data | 1;
      }
    }

    // EVEnt shit
  }

  return Status;
}
