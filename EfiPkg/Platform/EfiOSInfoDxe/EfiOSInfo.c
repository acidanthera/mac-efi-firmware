#include <AppleMacEfi.h>

#include <Guid/AppleOSLoaded.h>

#include <Protocol/OSInfo.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

// OS_INFO_VENDOR_NAME
#define OS_INFO_VENDOR_NAME  "Apple Inc."

// mOSName
STATIC CHAR8 *mOSName = NULL;

// mOSVendor
STATIC CHAR8 *mOSVendor = NULL;

// mBootVTdEnabled
STATIC UINTN mBootVTdEnabled = 0;

// mAppleOSLoadedSignaled
STATIC BOOLEAN mAppleOSLoadedSignaled = FALSE;

// InternalOSInfoSet
STATIC
VOID
InternalOSInfoSet (
  VOID
  )
{
  INTN Result;

  if (mOSVendor != NULL) {
    Result = AsciiStrCmp (mOSVendor, OS_INFO_VENDOR_NAME);

    if (Result == 0) {
      if (!mAppleOSLoadedSignaled) {
        EfiNamedEventSignal (&gAppleOSLoadedNamedEventGuid);

        mAppleOSLoadedSignaled = TRUE;
      }

      // TODO: OSName logic.
    }
  }
}

// OSInfoOSName
VOID
EFIAPI
OSInfoOSName (
  IN CHAR8  *OSName
  )
{
  UINTN Size;

  CHAR8 *Buffer;

  Size   = AsciiStrSize (OSName);
  Buffer = AllocateCopyPool (Size, (CONST VOID *)OSName);

  mOSName = Buffer;

  InternalOSInfoSet ();
}

// OSInfoOSVendor
VOID
EFIAPI
OSInfoOSVendor (
  IN CHAR8  *OSVendor
  )
{
  UINTN Size;

  CHAR8 *Buffer;

  Size   = AsciiStrSize (OSVendor);
  Buffer = AllocateCopyPool (Size, (CONST VOID *)OSVendor);

  mOSVendor = Buffer;

  InternalOSInfoSet ();
}

// OSInfoSetBootVTdEnabled
VOID
EFIAPI
OSInfoSetBootVTdEnabled (
  IN UINTN  *BootVTdEnabled
  )
{
  // BUG: Check BootVTdEnabled for NULL.

  mBootVTdEnabled = *BootVTdEnabled;
}

// OSInfoGetBootVTdEnabled
VOID
EFIAPI
OSInfoGetBootVTdEnabled (
  OUT UINTN  *BootVTdEnabled
  )
{
  // BUG: Check BootVTdEnabled for NULL.

  *BootVTdEnabled = mBootVTdEnabled;
}

// EfiOSInfoMain
/**

  @param[in] ImageHandle  The firmware allocated handle for the EFI image.
  @param[in] SystemTable  A pointer to the EFI System Table.

  @retval EFI_SUCCESS          The entry point is executed successfully.
  @retval EFI_ALREADY_STARTED  The protocol has already been installed.
**/
EFI_STATUS
EFIAPI
EfiOSInfoMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  STATIC EFI_OS_INFO_PROTOCOL OSInfoProtocol = {
    EFI_OS_INFO_PROTOCOL_REVISION,
    OSInfoOSName,
    OSInfoOSVendor,
    OSInfoSetBootVTdEnabled,
    OSInfoGetBootVTdEnabled
  };

  return gBS->InstallProtocolInterface (
                ImageHandle,
                &gEfiOSInfoProtocolGuid,
                EFI_NATIVE_INTERFACE,
                (VOID *)&OSInfoProtocol
                );
}
