#include <AppleEfi.h>

#include <Library/AppleDriverLib.h>

STATIC EFI_LIST mListEntry = INITIALIZE_LIST_HEAD_VARIABLE (mListEntry);

EFI_STATUS
EFIAPI
UserInterfaceThemeDriverMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
)
{
  AppleInitializeDriverLib (ImageHandle, SystemTable);

  if (!IsListEmpty (&mListEntry)) {
    do {

    } while (!IsNull (&mListEntry, NULL));
  }
}
