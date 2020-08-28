/////////////////////////////////////////////////////////////////////////////
// Name:        varint.h
// Project:     baseLib
// Purpose:     Encode & decode variable-size integer values.
// Author:      Piotr Likus
// Modified by:
// Created:     24/06/2013
/////////////////////////////////////////////////////////////////////////////

#ifndef _VARINT_H__
#define _VARINT_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/** \file varint.h
\brief Encode & decode variable-size integer values.

Long description
*/

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
#include <cassert>

// ----------------------------------------------------------------------------
// Simple type definitions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Forward class definitions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------
const unsigned int VARINT_MAX_SIZE_INT = 6;

// ----------------------------------------------------------------------------
// Functions
// ----------------------------------------------------------------------------

/// Encode varint and store it in given buffer
/// \param value value to be encoded (unsigned)
/// \param output pointer to destination buffer
/// \param outputSize size of destination buffer
/// \return returns number of bytes written
template <typename T>
unsigned int varint_encode(T value, void *output, size_t outputSize)
{
  assert(value >= 0);
  unsigned int writtenSize;
  unsigned char writeByte = value & 0x7f;
  unsigned char *destPtr = reinterpret_cast<unsigned char *>(output);
  if (value == writeByte) {
    writtenSize = 1;
    assert(writtenSize <= outputSize);
    *destPtr = writeByte;
  } else if (value <= 0x7FF)
  {
    writtenSize = 2;
    assert(writtenSize <= outputSize);
    writeByte = 0xC0 | ((value >> 6) & 0x1f);
    *destPtr = writeByte;
    destPtr++;
    writeByte = 0x80 | (value & 0x3f);
    *destPtr = writeByte;
  } else if (value <= 0xFFFF)
  {
    writtenSize = 3;
    assert(writtenSize <= outputSize);
    writeByte = 0xE0 | ((value >> 12) & 0x0f);
    *destPtr = writeByte;
    destPtr++;
    writeByte = 0x80 | ((value >> 6) & 0x3f);
    *destPtr = writeByte;
    destPtr++;
    writeByte = 0x80 | (value & 0x3f);
    *destPtr = writeByte;
  } else if (value <= 0x1FFFFF)
  {
    writtenSize = 4;
    assert(writtenSize <= outputSize);
    writeByte = 0xF0 | ((value >> 18) & 0x07);
    *destPtr = writeByte;
    destPtr++;
    writeByte = 0x80 | ((value >> 12) & 0x3f);
    *destPtr = writeByte;
    destPtr++;
    writeByte = 0x80 | ((value >> 6) & 0x3f);
    *destPtr = writeByte;
    destPtr++;
    writeByte = 0x80 | (value & 0x3f);
    *destPtr = writeByte;
  } else if (value <= 0x3FFFFFF)
  {
    writtenSize = 5;
    assert(writtenSize <= outputSize);
    writeByte = 0xF8 | ((value >> 24) & 0x03);
    *destPtr = writeByte;
    destPtr++;
    writeByte = 0x80 | ((value >> 18) & 0x3f);
    *destPtr = writeByte;
    destPtr++;
    writeByte = 0x80 | ((value >> 12) & 0x3f);
    *destPtr = writeByte;
    destPtr++;
    writeByte = 0x80 | ((value >> 6) & 0x3f);
    *destPtr = writeByte;
    destPtr++;
    writeByte = 0x80 | (value & 0x3f);
    *destPtr = writeByte;
  } else if (value <= 0x7FFFFFFF)
  {
    writtenSize = 6;
    assert(writtenSize <= outputSize);
    writeByte = 0xFC | ((value >> 30) & 0x01);
    *destPtr = writeByte;
    destPtr++;
    writeByte = 0x80 | ((value >> 24) & 0x3f);
    *destPtr = writeByte;
    destPtr++;
    writeByte = 0x80 | ((value >> 18) & 0x3f);
    *destPtr = writeByte;
    destPtr++;
    writeByte = 0x80 | ((value >> 12) & 0x3f);
    *destPtr = writeByte;
    destPtr++;
    writeByte = 0x80 | ((value >> 6) & 0x3f);
    *destPtr = writeByte;
    destPtr++;
    writeByte = 0x80 | (value & 0x3f);
    *destPtr = writeByte;
  } else {
    assert(false);
    writtenSize = 0;
  }

  assert(writtenSize > 0);
  return writtenSize;
}

/// decode varint from buffer
/// \param input pointer to data buffer
/// \param inputSize size of buffer
/// \param output decoded value
/// \return returns number of bytes read
template <typename T>
unsigned int varint_decode(void *input, size_t inputSize, T &output)
{
  assert(inputSize >= 0);
  output = 0;
  unsigned char readByte;
  unsigned int srcSize;
  unsigned char *srcPtr = reinterpret_cast<unsigned char *>(input);
  if (*srcPtr <= 0x7f)
  {
    assert(inputSize >= 1);
    srcSize = 1;
    output = *srcPtr;
  } else if (*srcPtr < 0xE0) {
    srcSize = 2;
    assert(inputSize >= 2);
    output = ((*srcPtr) & 0x1f) << 6;
    srcPtr++;
    output += ((*srcPtr) & 0x3f);
  } else if (*srcPtr < 0xF0) {
    srcSize = 3;
    assert(inputSize >= 3);
    output =  ((*srcPtr) & 0x0f) << 12;
    srcPtr++;
    output += ((*srcPtr) & 0x3f) << 6;
    srcPtr++;
    output += ((*srcPtr) & 0x3f);
  } else if (*srcPtr < 0xF8) {
    srcSize = 4;
    assert(inputSize >= 4);
    output =  ((*srcPtr) & 0x07) << 18;
    srcPtr++;
    output =  ((*srcPtr) & 0x3f) << 12;
    srcPtr++;
    output += ((*srcPtr) & 0x3f) << 6;
    srcPtr++;
    output += ((*srcPtr) & 0x3f);
  } else if (*srcPtr < 0xFC) {
    srcSize = 5;
    assert(inputSize >= 5);
    output =  ((*srcPtr) & 0x03) << 24;
    srcPtr++;
    output =  ((*srcPtr) & 0x3f) << 18;
    srcPtr++;
    output =  ((*srcPtr) & 0x3f) << 12;
    srcPtr++;
    output += ((*srcPtr) & 0x3f) << 6;
    srcPtr++;
    output += ((*srcPtr) & 0x3f);
  } else if (*srcPtr < 0xFE) {
    srcSize = 6;
    assert(inputSize >= 6);
    output =  ((*srcPtr) & 0x01) << 30;
    srcPtr++;
    output =  ((*srcPtr) & 0x3f) << 24;
    srcPtr++;
    output =  ((*srcPtr) & 0x3f) << 18;
    srcPtr++;
    output =  ((*srcPtr) & 0x3f) << 12;
    srcPtr++;
    output += ((*srcPtr) & 0x3f) << 6;
    srcPtr++;
    output += ((*srcPtr) & 0x3f);
  } else {
    assert(false);
    srcSize = 0;
  }

  assert(srcSize > 0);
  return srcSize;
}

/// calculate varint size
/// \param input pointer to data buffer
/// \param inputSize size of buffer
/// \return returns number of bytes 
inline unsigned int varint_get_size(void *input, size_t inputSize)
{
  assert(inputSize >= 0);
  //unsigned char readByte;
  unsigned int srcSize;
  unsigned char *srcPtr = reinterpret_cast<unsigned char *>(input);
  if (*srcPtr <= 0x7f)
  {
    srcSize = 1;
  } else if (*srcPtr < 0xE0) {
    srcSize = 2;
  } else if (*srcPtr < 0xF0) {
    srcSize = 3;
  } else if (*srcPtr < 0xF8) {
    srcSize = 4;
  } else if (*srcPtr < 0xFC) {
    srcSize = 5;
  } else if (*srcPtr < 0xFE) {
    srcSize = 6;
  } else {
    assert(false);
    srcSize = 0;
  }

  assert(srcSize > 0);
  return srcSize;
}


/// encode signed integer into varint
template <typename T>
unsigned int varint_encode_signed(T value, void *output, size_t outputSize)
{
  T newVal = value;

  if (value < 0)
    newVal = ((-value) << 1) | 1;
  else
    newVal = value << 1;
  
  return varint_encode(newVal, output, outputSize);
}

/// decode signed integer from varint
template <typename T>
unsigned int varint_decode_signed(void *input, size_t inputSize, T &output)
{
  T rawVal;
  unsigned int res = varint_decode(input, inputSize, rawVal);

  if ((rawVal & 1) != 0)
    output = -(rawVal >> 1);
  else
    output = rawVal >> 1;

  return res;
}

// ----------------------------------------------------------------------------
// Class definitions
// ----------------------------------------------------------------------------


#endif // _VARINT_H__
