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

#ifndef APPLE_PLATFORM_INFO_DATABASE_IMPL_INTERNAL_H_
#define APPLE_PLATFORM_INFO_DATABASE_IMPL_INTERNAL_H_

#include <Protocol/ApplePlatformInfoDatabaseImpl.h>

#pragma pack (1)

// EFI_APPLE_SECTION_IDENTIFIER
typedef struct {
  EFI_RAW_SECTION Hdr;    ///< 
  UINT32          Ukn_4;  ///< 
} EFI_APPLE_SECTION_IDENTIFIER;

// EFI_APPLE_SECTION
typedef struct {
  EFI_APPLE_SECTION_IDENTIFIER Hdr;    ///< 
  UINT64                       int_8;  ///< 
  UINT32                       Size;   ///< 
  UINT8                        Data;   ///< 
} EFI_APPLE_SECTION;

#pragma pack ()

#endif // APPLE_PLATFORM_INFO_DATABASE_IMPL_INTERNAL_H_
