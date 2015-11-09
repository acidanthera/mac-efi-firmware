#include <AppleEfi.h>
#include <EfiDriverLib.h>

#include <ArchProtocol/VariableWrite/VariableWrite.h>

// mProtocol
EFI_GUID mProtocolGuid = { 0xBD7B48F2, 0x15EE, 0x48F4, { 0x8F, 0xCD, 0x0A, 0x9B, 0xF0, 0x2D, 0x4D, 0x92 } };

// mRegistration
VOID *mRegistration;

// mProtocol
struct {

} mProtocol;

// mVariableWritePresent
BOOLEAN mVariableWritePresent;

// NotifyFunction
VOID
EFIAPI
NotifyFunction (
	IN EFI_EVENT  Event,
	IN VOID       *Context
	)
{

}

// NotifyFunction
VOID
EFIAPI
VariableWriteNotifyFunction (
	IN EFI_EVENT  Event,
	IN VOID       *Context
	)
{

}

// PlatformInfoVariableMain
EFI_STATUS
EFIAPI
PlatformInfoVariableMain (
	IN EFI_HANDLE				 ImageHandle,
	IN EFI_SYSTEM_TABLE  *SystemTable
	)
{
	EFI_STATUS Status;

	EFI_EVENT ProtocolInstallEvent;
	VOID      *VariableWriteArchProtocol;
	EFI_EVENT VariableWriteInstallEvent;

	ProtocolInstallEvent = NULL;

	Status = gBS->LocateProtocol (&mProtocolGuid, NULL, (VOID **)&mProtocol);

	if (EFI_ERROR (Status)) {
		Status = gBS->CreateEvent (EFI_EVENT_NOTIFY_SIGNAL, EFI_TPL_CALLBACK, NotifyFunction, NULL, &ProtocolInstallEvent);

		if (!EFI_ERROR (Status)) {
			Status               = gBS->RegisterProtocolNotify (&mProtocolGuid, ProtocolInstallEvent, &mRegistration);
			ProtocolInstallEvent = NULL;

			if (EFI_ERROR (Status)) {
				// RETURN
			}
		}
	} else {
		NotifyFunction (NULL, NULL);
	}

	Status = gBS->LocateProtocol (&gEfiVariableWriteArchProtocolGuid, NULL, &VariableWriteArchProtocol);

	if (EFI_ERROR (Status)) {
		Status = gBS->CreateEvent (EFI_EVENT_NOTIFY_SIGNAL, EFI_TPL_CALLBACK, VariableWriteNotifyFunction, NULL, &VariableWriteInstallEvent);

		if (!EFI_ERROR (Status)) {
			Status                    = gBS->RegisterProtocolNotify (&gEfiVariableWriteArchProtocolGuid, VariableWriteInstallEvent, &mRegistration);
			VariableWriteInstallEvent = NULL;

			// RETURN
		}
	} else {
		mVariableWritePresent = TRUE;

		VariableWriteNotifyFunction (NULL, NULL);
	}

	if (ProtocolInstallEvent != NULL) {
		gBS->CloseEvent (ProtocolInstallEvent);
	}

	return Status;
}
