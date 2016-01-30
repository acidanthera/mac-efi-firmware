/** @file
  Base Stack Check library for GCC/clang.

  Use -fstack-protector-all compiler flag to make the compiler insert the __stack_chk_guard "canary" value into the
  stack and check the value prior to exiting the function. If the "canary" is overwritten __stack_chk_fail() is called.
  This is GCC specific code.

  Copyright (C) 2012, Apple Inc. All rights reserved.<BR>

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include <EfiCommon.h>
#include <EfiDebug.h>

// __stack_chk_guard
/// "canary" value that is inserted by the compiler into the stack frame.
VOLATILE VOID *__stack_chk_guard = (VOID *)0x0AFF;

// __stack_chk_fail
/** Error path for compiler generated stack "canary" value check code. If the stack canary has been overwritten this
    function gets called on exit of the function.
**/
VOID
__stack_chk_fail (
 VOID
 )
{
  DEBUG ((EFI_D_ERROR, "STACK FAULT: Buffer Overflow in function %a.\n", __builtin_return_address(0)));
  EFI_DEADLOOP ();
}
