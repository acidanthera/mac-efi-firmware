#include <AppleMacEfi.h>

#include <Protocol/AppleSmcIo.h>

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>

#include "SmcIoInternal.h"

#pragma pack (1)

// SMC_KEY_VALUE
typedef PACKED struct {
  SMC_KEY Key;
  UINT8   Size;
  UINT8   Data[23];
} SMC_KEY_VALUE;

#pragma pack ()

// gSoftwareSmcKeyValueList
STATIC SMC_KEY_VALUE gSoftwareSmcKeyValueList[] = {
  { 
    SMC_MAKE_KEY ('B', 'N', 'u', 'm'),
    1,
    {
      0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    }
  },
  {
    SMC_MAKE_KEY ('B', 'A', 'T', 'P'),
    1,
    {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    }
  },
  {
    SMC_MAKE_KEY ('B', 'R', 'S', 'C'),
    2,
    {
      0x64, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    }
  },
  {
    SMC_MAKE_KEY ('B', 'I', 'S', 'n'),
    1,
    {
      0x43, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    }
  },
  {
    SMC_MAKE_KEY ('M', 'S', 'L', 'D'),
    1,
    {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    }
  },
  {
    SMC_MAKE_KEY ('R', 'P', 'l', 't'),
    3,
    {
      0x7A, 0x6F, 0x65, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    }
  },
  {
    SMC_MAKE_KEY ('R', 'E', 'V', ' '),
    6,
    {
      0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    }
  },
  {
    0,
    0,
    {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    }
  }
};

// SmcIoVirtualSmcReadValue
EFI_STATUS
SmcIoVirtualSmcReadValue (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  IN  SMC_KEY                Key,
  IN  SMC_DATA_SIZE          Size,
  OUT SMC_DATA               *Value
  )
{
  EFI_STATUS    Status;

  SMC_KEY_VALUE *Walker;
  UINTN         Index;

  Status = EFI_INVALID_PARAMETER;

  if ((This != NULL) && (Value != NULL)) {
    Status = EFI_UNSUPPORTED;

    if (gSoftwareSmcKeyValueList != NULL) {
      Index = 1;

      Walker = &gSoftwareSmcKeyValueList[0];

      while (Walker->Key != 0) {
        if (Walker->Key == Key) {
          // BUG: Size may be smaller - Buffer overflow.

          Size = Walker->Size;

          CopyMem (
            (VOID *)Value,
            (VOID *)&Walker->Data[0],
            Walker->Size
            );

          Status = EFI_SUCCESS;

          break;
        }

        Walker = &gSoftwareSmcKeyValueList[Index];
      }
    }
  }

  return Status;
}

// SmcIoVirtualSmcWriteValue
EFI_STATUS
SmcIoVirtualSmcWriteValue (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  IN  SMC_KEY                Key,
  IN  SMC_DATA_SIZE          Size,
  OUT SMC_DATA               *Value
  )
{
  return EFI_SUCCESS;
}

// SmcIoVirtualSmcMakeKey
EFI_STATUS
SmcIoVirtualSmcMakeKey (
  IN  CHAR8    *Name,
  OUT SMC_KEY  *Key
  )
{
  // TODO: Implement.

  return EFI_SUCCESS;
}

// SmcGetyKeyCount
EFI_STATUS
SmcIoVirtualSmcGetKeyCount (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  OUT UINT32                 *Count
  )
{
  // TODO: Implement.

  return EFI_SUCCESS;
}

// SmcIoVirtualSmcGetKeyFromIndex
EFI_STATUS
SmcIoVirtualSmcGetKeyFromIndex (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  IN  SMC_KEY_INDEX          Index,
  OUT SMC_KEY                *Key
  )
{
  // TODO: Implement.

  return EFI_SUCCESS;
}

// SmcIoVirtualSmcGetKeyInfo
EFI_STATUS
SmcIoVirtualSmcGetKeyInfo (
  IN  APPLE_SMC_IO_PROTOCOL  *This,
  IN  SMC_KEY                Key,
  OUT SMC_DATA_SIZE          *Size,
  OUT SMC_KEY_TYPE           *Type,
  OUT SMC_KEY_ATTRIBUTES     *Attributes
  )
{
  // TODO: Implement.

  return EFI_SUCCESS;
}

// SmcIoVirtualSmcReset
EFI_STATUS
SmcIoVirtualSmcReset (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINT32                 Mode
  )
{
  return EFI_SUCCESS;
}

// SmcIoVirtualSmcFlashType
EFI_STATUS
SmcIoVirtualSmcFlashType (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN SMC_FLASH_TYPE         Type
  )
{
  return EFI_SUCCESS;
}

// SmcIoVirtualSmcFlashWrite
EFI_STATUS
SmcIoVirtualSmcFlashWrite (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN UINT32                 Unknown,
  IN SMC_FLASH_SIZE         Size,
  IN SMC_DATA               *Data
  )
{
  return EFI_SUCCESS;
}

// SmcIoVirtualSmcFlashAuth
EFI_STATUS
SmcIoVirtualSmcFlashAuth (
  IN APPLE_SMC_IO_PROTOCOL  *This,
  IN SMC_FLASH_SIZE         Size,
  IN SMC_DATA               *Data
  )
{
  return EFI_SUCCESS;
}
