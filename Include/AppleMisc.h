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

#ifndef APPLE_MISC_H_
#define APPLE_MISC_H_

#ifndef CPU_IA32
  #define MULT_U64_X32(Multiplicand, Multiplier) ((Multiplicand) * (Multiplier))
  #define MULT_U64_X64(Multiplicand, Multiplier) ((Multiplicand) * (Multiplier))
  #define MULT_S64_X64(Multiplicand, Multiplier) ((Multiplicand) * (Multiplier))
  #define DIV_U64_X64(Dividend, Divisor)         ((Dividend) / (Divisor))
  #define DIV_S64_X64(Dividend, Divisor)         ((Dividend) / (Divisor))
#else
  #define MULT_U64_X32(Multiplicand, Multiplier) MultU64x32 ((Multiplicand), (Multiplier))
  #define MULT_U64_X64(Multiplicand, Multiplier) MultU64x64 ((Multiplicand), (Multiplier))
  #define MULT_S64_X64(Multiplicand, Multiplier) MultS64x64 ((Multiplicand), (Multiplier))
  #define DIV_U64_X64(Dividend, Divisor)         MathLibDivU64x64 ((Dividend), (Divisor))
  #define DIV_S64_X64(Dividend, Divisor)         MathLibDivS64x64 ((Dividend), (Divisor))
#endif // CPU_IA32

#define HZ  (1000 * 1000 * 10)

// ARRAY_LENGTH
#define ARRAY_LENGTH(Array) (sizeof (Array) / sizeof (*(Array)))

// CONVERT_LENGTH
#define CONVERT_LENGTH(Size, SourceType, DestinationType) (((Size) / sizeof (SourceType)) * sizeof (DestinationType))

/// @{
#define OFFSET_PTR(Ptr, Index, Type) ((Type *)((UINTN)(Ptr) + (UINTN)((Index) * sizeof (Type))))
#define OFFSET(Value, Index, Type)   OFFSET_PTR (&(Value), (Index), Type)
#define BYTE_PTR(Ptr, Index)         (*(OFFSET_PTR ((Ptr), (Index), UINT8)))
#define BYTE(Value, Index)           (*(OFFSET ((Value), (Index), UINT8)))
#define WORD_PTR(Ptr, Index)         (*(OFFSET_PTR ((Ptr), (Index), UINT16)))
#define WORD(Value, Index)           (*(OFFSET ((Value), (Index), UINT16)))
#define DWORD_PTR(Ptr, Index)        (*(OFFSET ((Ptr), (Index), UINT32)))
#define DWORD(Value, Index)          (*(OFFSET ((Value), (Index), UINT32)))
#define QWORD_PTR(Ptr, Index)        (*(OFFSET ((Ptr), (Index), UINT64)))
#define QWORD(Value, Index)          (*(OFFSET ((Value), (Index), UINT64)))
/// @}

/// @{
#define BIT(Index)                (1 << (Index))
#define JOIN_MASKS(Mask1, Mask2)  ((Mask1) | (Mask2))
#define JOIN_BITS(Mask1, Mask2)   JOIN_MASKS ((Mask1), (Mask2))
#define SET_BIT(Mask1, Mask2)     JOIN_MASKS ((Mask1), (Mask2))
#define SET_BITS(Mask1, Mask2)    SET_BIT ((Mask1), (Mask2))
#define UNSET_BITS(Mask1, Mask2)  ((Mask1) & ~(Mask2))
#define UNSET_BIT(Mask1, Mask2)   UNSET_BITS ((Mask1), (Mask2))
#define DIFF_BITS(Mask1, Mask2)   ((Mask1) ^ (Mask2))
#define TOGGLE_BITS(Mask1, Mask2) DIFF_BITS ((Mask1), (Mask2))
#define TOGGLE_BIT(Mask1, Mask2)  TOGGLE_BITS ((Mask1), (Mask2))
#define APPLY_MASK(Mask1, Mask2)  ((Mask1) & (Mask2))
#define SELECT_BITS(Mask1, Mask2) APPLY_MASK ((Mask1), (Mask2))
#define BIT_SET(Mask1, Mask2)     ((BOOLEAN)(SELECT_BITS ((Mask1), (Mask2)) != 0))
#define BITS_SET(Mask1, Mask2)    ((BOOLEAN)(SELECT_BITS ((Mask1), (Mask2)) == (Mask2)))
/// @}

#endif // APPLE_MISC_H_
