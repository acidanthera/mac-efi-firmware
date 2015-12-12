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
/// @file      Protocol/OsIdentificationImpl/OsIdentificationImpl.c
///
///            
///
/// @author    Download-Fritz
/// @date      18/07/2015: Initial version
/// @copyright Copyright (C) 2005 - 2015 Apple Inc. All rights reserved.
///

#include <AppleEfi.h>

#include <Library/AppleDriverLib.h>

#include <Guid/AppleOsLoaded.h>

#include "OsIdentificationImplInternal.h"

// OsIdentificationOSNameImpl
/// 
///
/// @param 
///
/// @return 
/// @retval 
VOID
EFIAPI
OsIdentificationOSNameImpl (
	IN CHAR8  *OSName
	)
{
	return;
}

// OsIdentificationOSVendorImpl
/// 
///
/// @param 
///
/// @return 
/// @retval 
VOID
EFIAPI
OsIdentificationOSVendorImpl (
	IN CHAR8  *OSVendor
	)
{
	INTN Result;

	Result = EfiAsciiStrCmp (OSVendor, OS_IDENTIFICATION_VENDOR_NAME);

	if (Result == 0) {
		EfiLibNamedEventSignal (&gAppleOsLoadedNamedEventGuid);
	}
}
