//
// Copyright (C) 2005 - 2015 Apple Inc. All rights reserved.
//
// This program and the accompanying materials have not been licensed.
// Neither is its usage, its redistribution, in source or binary form,
// licensed, nor implicitely or explicitely permitted, except when
// required by applicable law.
//
// Unless required by applicable law or agreed to in writing, software
// distributed is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
// OR CONDITIONS OF ANY KIND, either express or implied.
//

///
/// @file      Include/Protocol/OsIdentificationImpl.h
///
///            
///
/// @author    Download-Fritz
/// @date      18/07/2015: Initial version
/// @copyright Copyright (C) 2005 - 2015 Apple Inc. All rights reserved.
///

#ifndef __OS_IDENTIFICATION_IMPL_H__
#define __OS_IDENTIFICATION_IMPL_H__

#include <Protocol/OsIdentification.h>

// OS_IDENTIFICATION_PROTOCOL_REVISION
#define OS_IDENTIFICATION_PROTOCOL_REVISION  0x01

// OS_IDENTIFICATION_VENDOR_NAME
#define OS_IDENTIFICATION_VENDOR_NAME  "Apple Inc."

// OSName
/// 
///
/// @param 
///
/// @return 
/// @retval 
VOID
EFIAPI
AppleOsIdentificationOSName (
	IN CHAR8 *OSName
	);

// OSVendor
/// 
///
/// @param 
///
/// @return 
/// @retval 
VOID
EFIAPI
AppleOsIdentificationOSVendor (
	IN CHAR8 *OSVendor
	);

#endif // ifndef __OS_IDENTIFICATION_IMPL_H__
