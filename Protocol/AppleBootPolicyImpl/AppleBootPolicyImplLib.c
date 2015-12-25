/** @file
  Copyright (C) 2005 - 2015 Apple Inc.  All rights reserved.<BR>

  This program and the accompanying materials have not been licensed.
  Neither is its usage, its redistribution, in source or binary form,
  licensed, nor implicitely or explicitely permitted, except when
  required by applicable law.

  Unless required by applicable law or agreed to in writing, software
  distributed is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
  OR CONDITIONS OF ANY KIND, either express or implied.
**/

#include <AppleEfi.h>

#include EFI_PROTOCOL_CONSUMER (SimpleFileSystem)

#include "AppleBootPolicyImplInternal.h"

// BootPolicyFileExists
/** Checks whether the given file exists or not.

  @param[in] Root      The volume's opened root.
  @param[in] FileName  The path of the file to check.

  @return  Returned is whether the specified file exists or not.
**/
BOOLEAN
BootPolicyFileExists (
  IN EFI_FILE_HANDLE  Root,
  IN CHAR16           *FileName
  )
{
  BOOLEAN         Exists;

  EFI_STATUS      Status;
  EFI_FILE_HANDLE FileHandle;

  ASSERT (Root != NULL);
  ASSERT (FileName != NULL);

  Status = Root->Open (Root, &FileHandle, FileName, EFI_FILE_MODE_READ, 0);

  if (!EFI_ERROR (Status)) {
    FileHandle->Close (FileHandle);

    Exists = TRUE;
  } else {
    Exists = FALSE;
  }

  return Exists;
}
