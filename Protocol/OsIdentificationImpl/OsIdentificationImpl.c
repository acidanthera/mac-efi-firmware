// 18/07/2015

#include <AppleEfi.h>
#include <EfiDriverLib.h>

#include <Protocol/OsIdentificationImpl.h>
#include <Protocol/OsIdentification.h>

// AppleOsIdentificationOSName
VOID
EFIAPI
AppleOsIdentificationOSName (
	IN CHAR8  *OSName
	)
{
	return;
}

// AppleOsIdentificationOSVendor
VOID
EFIAPI
AppleOsIdentificationOSVendor (
	IN CHAR8  *OSVendor
	)
{
	INTN Result;

	Result = EfiAsciiStrCmp (OSVendor, OS_IDENTIFICATION_VENDOR_NAME);

	if (Result == 0) {
		EfiLibNamedEventSignal (&gAppleOsLoadedNamedEventGuid);
	}
}
