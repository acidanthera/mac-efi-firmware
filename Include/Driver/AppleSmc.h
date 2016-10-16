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

#ifndef APPLE_SMC_DRV_H_
#define APPLE_SMC_DRV_H_

// AppleSmcIoMain
EFI_STATUS
EFIAPI
AppleSmcMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  );

#endif // APPLE_SMC_IO_DRV_H_
