/////////////////////////////////////////////////////////////////////////////
// Name:        Bion.h
// Project:     baseLib
// Purpose:     Binary Object Notation classes (binary JSON)
// Author:      Piotr Likus
// Modified by:
// Created:     22/06/2013
/////////////////////////////////////////////////////////////////////////////

#ifndef _BION_H__
#define _BION_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/** \file Bion.h
\brief Binary Object Notation classes (binary JSON)

Limited BION format implementation - just for JSON support.
*/

#define BION_TRACE_ENABLED

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
#include "base/btypes.h"
#include "base/string.h"
#include "base/varint.h"

#ifdef BION_TRACE_ENABLED
#include <istream>
#include "perf/Log.h"
#endif

// ----------------------------------------------------------------------------
// Simple type definitions
// ----------------------------------------------------------------------------
typedef unsigned char BionNullDataStorage;
typedef unsigned char BionBoolDataStorage;

enum BionValueType {
  bvt_undef,
  bvt_null,
  bvt_bool,
  bvt_bool_data,
  bvt_int,
  bvt_uint,
  bvt_float,
  bvt_zstring
};

enum BionInstrCode {
  bic_object_start = 0x02,
  //bic_object_end = 0x00,
  bic_array_start = 0x04,
  //bic_array_end = 0x00
  bic_struct_end = 0x00,
  bic_fixtype_array = 0x06
};

enum BionStdTypeCode {
  bst_undef = 0x00,
  bst_null = 0x01,
  bst_bool_true = 0x02,
  bst_bool_false = 0x03,
  bst_int = 0x01,
  bst_uint = 0x02,
  bst_float = 0x03,
  bst_bool_data = 0x04
};

enum BionOtherCodes {
  boc_dt_zstring = 0x0f
};

enum BionParseContext {
  bctx_null = 0,
  bctx_instr_code = 1,
  bctx_string_val = 2,
  bctx_string_tail = 3,
  bctx_name = 4,
  bctx_name_tail = 5
};


// ----------------------------------------------------------------------------
// Forward class definitions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------
// -- Error codes
const uint BEC_WrongIntSize = 2;
const uint BEC_WrongFloatSize = 3;
const uint BEC_UnknownValueType = 4;
const uint BEC_DataTooShort = 5;
const uint BEC_WrongHeaderContents = 6;
const uint BEC_StructNotOpen = 7;
const uint BEC_UnknownZeroLenValType = 8;
const uint BEC_UnknownFloatSize = 9;
const uint BEC_UnknownStdValType = 10;
const uint BEC_ValTooLong = 11;
const uint BEC_IntSizeInvalid = 12;
const uint BEC_UIntSizeInvalid = 13;
const uint BEC_FloatSizeInvalid = 14;
const uint BEC_UnkownStdTypeInProtProc = 15;
const uint BEC_MemBufferOverflowOnWr = 16;
const uint BEC_InvalidInputLimit = 17;

// ----------------------------------------------------------------------------
// Class definitions
// ----------------------------------------------------------------------------
class BionError: public std::runtime_error {
public:
  BionError(unsigned int errorCode): std::runtime_error(calcErrorMessage(errorCode)) {}
  BionError(unsigned int errorCode, const std::string &msgContext): std::runtime_error(calcErrorMessage(errorCode)+", context: "+msgContext) {}
protected:
  std::string calcErrorMessage(unsigned int errorCode) {
    switch(errorCode) {
      case BEC_WrongIntSize:          return "BION02: Wrong int size";
      case BEC_WrongFloatSize:        return "BION03: Wrong float size";
      case BEC_UnknownValueType:      return "BION04: Unknown value type";
      case BEC_DataTooShort:          return "BION05: Data too short";
      case BEC_WrongHeaderContents:   return "BION06: Wrong header contents";
      case BEC_StructNotOpen:         return "BION07: Structure not open";
      case BEC_UnknownZeroLenValType: return "BION08: Unknown zero-length value type";
      case BEC_UnknownFloatSize:      return "BION09: Unknown float size";
      case BEC_UnknownStdValType:     return "BION10: Unkown std value type";
      case BEC_ValTooLong:            return "BION11: Value too long";
      case BEC_IntSizeInvalid:        return "BION12: Int size invalid";
      case BEC_UIntSizeInvalid:       return "BION13: UInt size invalid";
      case BEC_FloatSizeInvalid:      return "BION14: Float size invalid";
      case BEC_UnkownStdTypeInProtProc: return "BION15: Unknown std type in prot proc";
      case BEC_MemBufferOverflowOnWr: return "BION16: Mem buffer overflow on write";
      case BEC_InvalidInputLimit:     return "BION17: Invalid input limit value";
      default: return "Unknown error";
    }
  }
  BionError(const std::string &text): std::runtime_error(text) {}
};

class BionWriterIntf {
public:
  virtual ~BionWriterIntf() {}
  //-----------------------------------------------------------------
  virtual void writeHeader() = 0;
  virtual void writeFooter() = 0;
  virtual void writeObjectBegin() = 0;
  virtual void writeObjectEnd() = 0;
  virtual void writeArrayBegin() = 0;
  virtual void writeArrayEnd() = 0;
  virtual void writeFixTypeArrayBegin(unsigned int arraySize) = 0;
  virtual void writeFixTypeArrayEnd() = 0;
  virtual void writeElementName(char *name) = 0;
  virtual void writeElementName(const std::string &name) = 0;
  //-----------------------------------------------------------------
  virtual void writeValueType(const std::string &tagValue) = 0;
  virtual void writeValueType(float tagValue) = 0;
  virtual void writeValueType(double tagValue) = 0;
  virtual void writeValueType(xdouble tagValue) = 0;
  virtual void writeValueType(byte tagValue) = 0;
  virtual void writeValueType(int tagValue) = 0;
  virtual void writeValueType(uint tagValue) = 0;
  virtual void writeValueType(int64 tagValue) = 0;
  virtual void writeValueType(uint64 tagValue) = 0;
  virtual void writeValueType(bool tagValue) = 0;
  virtual void writeNullValueType() = 0;
  //------------------------------------------
  virtual void writeValue(const std::string &value) = 0;
  virtual void writeValue(float value) = 0;
  virtual void writeValue(double value) = 0;
  virtual void writeValue(xdouble value) = 0;
  virtual void writeValue(byte value) = 0;
  virtual void writeValue(int value) = 0;
  virtual void writeValue(uint value) = 0;
  virtual void writeValue(int64 value) = 0;
  virtual void writeValue(uint64 value) = 0;
  virtual void writeValue(bool value) = 0;
  virtual void writeNullValue() = 0;
  //---------------------------------------------------------
  virtual void writeValueData(const std::string &value) = 0;
  virtual void writeValueData(float value) = 0;
  virtual void writeValueData(double value) = 0;
  virtual void writeValueData(xdouble value) = 0;
  virtual void writeValueData(byte value) = 0;
  virtual void writeValueData(int value) = 0;
  virtual void writeValueData(uint value) = 0;
  virtual void writeValueData(int64 value) = 0;
  virtual void writeValueData(uint64 value) = 0;
  virtual void writeValueData(bool value) = 0;
  virtual void writeNullValueData() = 0;
};

/// Bion data writer class
template<typename Output>
class BionWriter: public BionWriterIntf {
public:
  BionWriter(Output &output): m_output(&output) {}

  void writeHeader() {
    static char header[] = {0x21, 0x23};
    m_output->write(header, sizeof(header));
  }

  void writeFooter() {
    static char footer[] = {0x00};
    m_output->write(footer, sizeof(footer));
  }

  void writeObjectBegin() {
    const char instrCode = bic_object_start;
    m_output->write(&instrCode, sizeof(char));
  }

  void writeObjectEnd() {
    const char instrCode = bic_struct_end;
    m_output->write(&instrCode, sizeof(char));
  }

  void writeArrayBegin() {
    const char instrCode = bic_array_start;
    m_output->write(&instrCode, sizeof(char));
  }

  void writeArrayEnd() {
    const char instrCode = bic_struct_end;
    m_output->write(&instrCode, sizeof(char));
  }

  void writeFixTypeArrayBegin(unsigned int arraySize) {
    const char instrCode = bic_fixtype_array;
    m_output->write(&instrCode, sizeof(char));

    char buf[VARINT_MAX_SIZE_INT];
    unsigned int written = varint_encode(arraySize, buf, sizeof(buf));
    m_output->write(buf, written);
  }

  void writeFixTypeArrayEnd() {
    // empty
  }

  void writeElementName(char *name) {
    m_output->write(name, std::char_traits<char>::length(name) + 1);
  }

  void writeElementName(const std::string &name) {
    m_output->write(name.c_str(), name.length() + 1);
  }

  void writeFloat(float value) {
    writeStdValue(bvt_float, &value, sizeof(value));
  }

  void writeFloatType() {
    writeStdType(bvt_float, NULL, sizeof(float));
  }

  void writeFloatData(float value) {
    writeStdData(bvt_float, &value, sizeof(value));
  }

  void writeDouble(double value) {
    writeStdValue(bvt_float, &value, sizeof(value));
  }

  void writeDoubleType() {
    writeStdType(bvt_float, NULL, sizeof(double));
  }

  void writeDoubleData(double value) {
    writeStdData(bvt_float, &value, sizeof(value));
  }

  void writeXDouble(xdouble value) {
    writeStdValue(bvt_float, &value, sizeof(value));
  }

  void writeXDoubleType() {
    writeStdType(bvt_float, NULL, sizeof(xdouble));
  }

  void writeXDoubleData(xdouble value) {
    writeStdData(bvt_float, &value, sizeof(value));
  }

  void writeZString(const char *value) {
    writeStdValue(bvt_zstring, reinterpret_cast<void *>(const_cast<char *>(value)), std::char_traits<char>::length(value) + 1);
  }

  void writeZString(const std::string &value) {
    writeZString(value.c_str());
  }

  void writeZStringType() {
    writeStdType(bvt_zstring, NULL, 0);
  }

  void writeZStringData(const char *value) {
    writeStdData(bvt_zstring, reinterpret_cast<void *>(const_cast<char *>(value)), std::char_traits<char>::length(value) + 1);
  }

  void writeZStringData(const std::string &value) {
    writeZStringData(value.c_str());
  }

  void writeBool(bool value) {
    writeStdValue(bvt_bool, &value, sizeof(value));
  }

  void writeBoolType() {
    writeStdType(bvt_bool_data, NULL, 0);
  }

  void writeBoolData(bool value) {
    BionBoolDataStorage stValue = value?1:0;
    writeStdData(bvt_bool_data, &stValue, sizeof(BionBoolDataStorage));
  }

  void writeNull() {
    writeStdValue(bvt_null, NULL, 0);
  }

  void writeNullType() {
    writeStdType(bvt_null, NULL, 0);
  }

  void writeNullData() {
    BionNullDataStorage stValue = 0;
    writeStdData(bvt_null, &stValue, sizeof(BionNullDataStorage));
  }

  void writeByte(byte value) {
    writeStdValue(bvt_uint, &value, sizeof(value));
  }

  void writeByteType() {
    writeStdType(bvt_uint, NULL, sizeof(byte));
  }

  void writeByteData(byte value) {
    writeStdData(bvt_uint, &value, sizeof(value));
  }

  void writeInt(int value) {
    writeStdValue(bvt_int, &value, sizeof(value));
  }

  void writeIntType() {
    writeStdType(bvt_int, NULL, sizeof(int));
  }

  void writeIntData(int value) {
    writeStdData(bvt_int, &value, sizeof(value));
  }

  void writeUInt(uint value) {
    writeStdValue(bvt_uint, &value, sizeof(value));
  }

  void writeUIntType() {
    writeStdType(bvt_uint, NULL, sizeof(unsigned int));
  }

  void writeUIntData(uint value) {
    writeStdData(bvt_uint, &value, sizeof(value));
  }

  void writeInt64(int64 value) {
    writeStdValue(bvt_int, &value, sizeof(value));
  }

  void writeInt64Type() {
    writeStdType(bvt_int, NULL, sizeof(int64));
  }

  void writeInt64Data(int64 value) {
    writeStdData(bvt_int, &value, sizeof(value));
  }

  void writeUInt64(uint64 value) {
    writeStdValue(bvt_uint, &value, sizeof(value));
  }

  void writeUInt64Type() {
    writeStdType(bvt_uint, NULL, sizeof(uint64));
  }

  void writeUInt64Data(uint64 value) {
    writeStdData(bvt_uint, &value, sizeof(value));
  }

  void writeValueType(const std::string &tagValue)
  {
    writeZStringType();
  }

  void writeValueType(float tagValue)
  {
    writeFloatType();
  }

  void writeValueType(double tagValue)
  {
    writeDoubleType();
  }

  void writeValueType(xdouble tagValue)
  {
    writeXDoubleType();
  }

  void writeValueType(byte tagValue)
  {
    writeByteType();
  }

  void writeValueType(int tagValue)
  {
    writeIntType();
  }

  void writeValueType(uint tagValue)
  {
    writeUIntType();
  }

  void writeValueType(int64 tagValue)
  {
    writeInt64Type();
  }

  void writeValueType(uint64 tagValue)
  {
    writeUInt64Type();
  }

  void writeValueType(bool tagValue)
  {
    writeBoolType();
  }

  void writeNullValueType()
  {
    writeNullType();
  }


  //------------------------------------------
  void writeValue(const std::string &value)
  {
    writeZString(value);
  }

  void writeValue(float value)
  {
    writeFloat(value);
  }

  void writeValue(double value)
  {
    writeDouble(value);
  }

  void writeValue(xdouble value)
  {
    writeXDouble(value);
  }

  void writeValue(byte value)
  {
    writeByte(value);
  }

  void writeValue(int value)
  {
    writeInt(value);
  }

  void writeValue(uint value)
  {
    writeUInt(value);
  }

  void writeValue(int64 value)
  {
    writeInt64(value);
  }

  void writeValue(uint64 value)
  {
    writeUInt64(value);
  }

  void writeValue(bool value)
  {
    writeBool(value);
  }

  void writeNullValue()
  {
    writeNull();
  }

  //------------------------------------------
  void writeValueData(const std::string &value)
  {
    writeZStringData(value);
  }

  void writeValueData(float value)
  {
    writeFloatData(value);
  }

  void writeValueData(double value)
  {
    writeDoubleData(value);
  }

  void writeValueData(xdouble value)
  {
    writeXDoubleData(value);
  }

  void writeValueData(byte value)
  {
    writeByteData(value);
  }

  void writeValueData(int value)
  {
    writeIntData(value);
  }

  void writeValueData(uint value)
  {
    writeUIntData(value);
  }

  void writeValueData(int64 value)
  {
    writeInt64Data(value);
  }

  void writeValueData(uint64 value)
  {
    writeUInt64Data(value);
  }

  void writeValueData(bool value)
  {
    writeBoolData(value);
  }

  void writeNullValueData()
  {
    writeNullData();
  }

protected:
  void writeStdValue(BionValueType valueType, void *data, size_t dataSize) {
    if (writeStdType(valueType, data, dataSize))
      writeStdData(valueType, data, dataSize);
  }

  void writeStdData(BionValueType valueType, void *data, size_t dataSize) {
    m_output->write(reinterpret_cast<char *>(data), dataSize);
  }

  // returns true if data should be written
  bool writeStdType(BionValueType valueType, void *data, size_t dataSize) {
    unsigned char valueTypeCode;
    BionStdTypeCode stdType;
    int stdSize;
    bool hasData = true;

    valueTypeCode = 0;

    switch(valueType) {
    case bvt_zstring:
      valueTypeCode = boc_dt_zstring;
      break;
    default:
      ;
    } // switch (valueType)

    if (valueTypeCode == 0) {
      switch(valueType) {
      case bvt_null:
        // warning: reversed fields (type, size)
        stdSize = bst_null;
        stdType = bst_undef;
        hasData = false;
        break;
      case bvt_bool:
        // warning: reversed fields (type, size)
        stdSize = *(static_cast<bool *>(data))?bst_bool_true:bst_bool_false;
        stdType = bst_undef;
        hasData = false;
        break;
      case bvt_bool_data:
        stdType = bst_bool_data;
        stdSize = 1;
        break;
      case bvt_int:
      case bvt_uint:
        stdType = (valueType == bvt_uint)?bst_uint:bst_int;
        if (dataSize == 1)
          stdSize = 1;
        else if (dataSize == 2)
          stdSize = 2;
        else if (dataSize == 4)
          stdSize = 3;
        else if (dataSize == 8)
          stdSize = 4;
        else
          throw BionError(BEC_WrongIntSize);
        break;
      case bvt_float:
        stdType = bst_float;
        if (dataSize == sizeof(float))
          stdSize = 1;
        else if (dataSize == sizeof(double))
          stdSize = 2;
        else if (dataSize == sizeof(xdouble))
          stdSize = 3;
        else
          throw BionError(BEC_WrongFloatSize);
        break;
      default:
        throw BionError(BEC_UnknownValueType);
      } // switch (valueType)

      valueTypeCode = (stdSize << 4) | static_cast<unsigned int>(stdType);
    }

    m_output->write(reinterpret_cast<char *>(&valueTypeCode), sizeof(char));
    return hasData;
  } // function
private:
  Output *m_output;
};

/// Required interface for writer output
class BionOutputIntf {
public:
  virtual void write(const char *data, size_t size) = 0;
};

class BionMemoryOutputStream
#ifdef _DEBUG
  :public BionOutputIntf
#endif
{
public:
  BionMemoryOutputStream(void *data, size_t size): m_data(data), m_dataSize(size), m_offset(0) {
    m_ptr = reinterpret_cast<char *>(data);
  }

  void write(const char *data, size_t size) {
    if (m_offset + size <= m_dataSize)
    {
      std::memcpy(m_ptr, data, size);
      m_ptr += size;
      m_offset += size;
    } else {
      throw BionError(BEC_MemBufferOverflowOnWr, std::string("Buffer size: ")+toString(m_dataSize)+", offset: "+toString(m_offset)+", write size: "+toString(size));
    }
  }

  void reset() {
    m_offset = 0;
    m_ptr = reinterpret_cast<char *>(m_data);
  }

  size_t tellg() {
    return m_offset;
  }
private:
  void *m_data;
  char *m_ptr;
  size_t m_dataSize;
  size_t m_offset;
};

class BufferLog {
public:
  BufferLog(char *buffer, size_t bufferSize): m_buffer(buffer), m_bufferSize(bufferSize) {}

  std::string getContents(size_t filledSize, bool binary)
  {
    char *buffer = m_buffer;
    size_t bufferSize = m_bufferSize;

    std::stringstream resStream;
    size_t bytesLeft = filledSize;
    char *cptr = buffer;

    if (binary)
    {
      while((bytesLeft--) > 0)
      {
        resStream << "\\0x" << std::hex << (0xff & static_cast<uint>(*(cptr++)));
      }
    } else {
      while((bytesLeft--) > 0)
      {
        if (!*cptr)
          resStream << "\\0";
        else
          resStream << *(cptr++);
      }
    }

    if (filledSize)
      resStream << " (" << filledSize << " bytes)";

    return resStream.str();
  }

protected:
  char *m_buffer;
  size_t m_bufferSize;
};

/// Bion data reader class
template<typename InputStream, typename InputProcessor>
class BionReader {
  typedef std::vector<int> OpenStructsStack;
public:
  BionReader(InputStream &input, InputProcessor &aProc): m_input(&input), m_processor(&aProc), m_traceOn(false) {}

  void setTraceOn(bool value) { m_traceOn = value; }

  /// Read data from Bion stream and process it using provided processor
  void process() {
    static char header[] = {0x21, 0x23};

    bool ok;
    bool traceOn = m_traceOn;
    const unsigned int BUFFER_SIZE = 500;
    char buffer[BUFFER_SIZE];
    char *cptr;
    OpenStructsStack openStructs;

    std::memset(buffer, '\0', BUFFER_SIZE);

    cptr = buffer;
    ok = false;
    if (m_input->peek() != EOF) {
      m_input->read(buffer, 2 * sizeof(char));
      ok = (m_input->gcount() == 2 * sizeof(char));
      assert(ok);
    }

    if (!ok)
      throw BionError(BEC_DataTooShort);

    if ((buffer[0] != header[0]) || (buffer[1] != header[1]))
      throw BionError(BEC_WrongHeaderContents);

    m_processor->processHeader(buffer, 2*sizeof(char));

    if (m_input->peek() == EOF)
      return;

    size_t valueSize;
    unsigned char ch;
    BionValueType valueType;
    int stdType;
    size_t size, runSize;
    bool eofFound = false;
    BionParseContext context = bctx_instr_code;

    if (traceOn) logParseEvent("---- trace start ----", buffer, BUFFER_SIZE, 0);

    do {
      cptr = buffer;

      if ((m_input->peek() == EOF) || eofFound) {
        if (traceOn) logParseEvent("eof_found", buffer, BUFFER_SIZE, 0);
        break;
      }

      m_input->read(buffer, sizeof(char));
      assert(m_input->gcount() == sizeof(char));

      ch = static_cast<unsigned char>(*buffer);
      if (traceOn) logParseEvent("char_read", buffer, BUFFER_SIZE, sizeof(ch));

      if (ch == bic_struct_end) {
        if (traceOn) logParseEvent("struct_end", buffer, BUFFER_SIZE, sizeof(ch));

        processStructEnd(openStructs, eofFound, context);

        if (eofFound) {
          if (traceOn) logParseEvent("eof_struct_end", buffer, BUFFER_SIZE, sizeof(ch));
          break;
        }
        if (traceOn) logParseEvent(scString("new_ctx:")+toString(context), buffer, BUFFER_SIZE, 0);
      } else if (context == bctx_string_val)
      {
        size = parseZString(buffer, BUFFER_SIZE, sizeof(ch));
        if (traceOn) logParseEvent("string_val", buffer, BUFFER_SIZE, size, false);
        m_processor->processZString(buffer);
        handleValueParsed(openStructs, context);
        if (traceOn) logParseEvent(scString("new_ctx:")+toString(context), buffer, BUFFER_SIZE, 0);
      } else if (context == bctx_name)
      {
        size = parseZString(buffer, BUFFER_SIZE, sizeof(ch));
        if (traceOn) logParseEvent("name", buffer, BUFFER_SIZE, size, false);
        m_processor->processElementName(buffer);
        context = bctx_instr_code;
        if (traceOn) logParseEvent(scString("new_ctx(instr_code):")+toString(context), buffer, BUFFER_SIZE, 0);
      }
      else if (context == bctx_instr_code) 
      {
        switch(ch) {
        case bic_object_start:
          if (traceOn) logParseEvent("object_start", buffer, BUFFER_SIZE, 0);
          m_processor->processObjectBegin();
          openStructs.push_back(bic_object_start);
          context = bctx_name;
          if (traceOn) logParseEvent(scString("new_ctx(name):")+toString(context), buffer, BUFFER_SIZE, 0);
          break;
        case bic_array_start:
          if (traceOn) logParseEvent("array_start", buffer, BUFFER_SIZE, 0);
          m_processor->processArrayBegin();
          openStructs.push_back(bic_array_start);
          context = bctx_instr_code;
          if (traceOn) logParseEvent(scString("new_ctx(instr_code):")+toString(context), buffer, BUFFER_SIZE, 0);
          break;
        case bic_fixtype_array:
          if (traceOn) logParseEvent("fix_array", buffer, BUFFER_SIZE, 0);
          readFixTypeArray(buffer, BUFFER_SIZE);
          handleValueParsed(openStructs, context);
          break;
        case boc_dt_zstring:
          if (traceOn) logParseEvent("zstring_instr", buffer, BUFFER_SIZE, 0);
          context = bctx_string_val;
          if (traceOn) logParseEvent(scString("new_ctx(str_val):")+toString(context), buffer, BUFFER_SIZE, 0);
          break;
        default:
          // decode instruction
          decodeStdType(ch, size, stdType, valueType);
          if (traceOn) logParseEvent("instr_decoded", buffer, BUFFER_SIZE, size);

          if (size > 0) {
            m_input->read(buffer, size);
            ok = (m_input->gcount() == size);
          } else {
            switch(stdType) {
              case bst_bool_true:
                *(reinterpret_cast<bool *>(buffer)) = true;
                size = sizeof(bool);
                valueType = bvt_bool;
                break;
              case bst_bool_false:
                *(reinterpret_cast<bool *>(buffer)) = false;
                size = sizeof(bool);
                valueType = bvt_bool;
                break;
            }
          }
         
          if (traceOn) logParseEvent(scString("val_rdy_for_parse:")+toString(valueType), buffer, BUFFER_SIZE, size);

          // process value 
          processStdValue(valueType, buffer, size);
          handleValueParsed(openStructs, context);

          if (traceOn) logParseEvent(scString("new_ctx_after_val:")+toString(context), buffer, BUFFER_SIZE, 0);
        } // switch ch
      } // non-zero ch

      } while(true); 

      if (traceOn) logParseEvent(scString("---- footer ----"), buffer, BUFFER_SIZE, 0);
      m_processor->processFooter(buffer, 0);
  } // function
protected:
  void handleValueParsed(OpenStructsStack &openStructs, BionParseContext &context)
  {
    if (getOpenStructCode(openStructs) == bic_object_start) {
      context = bctx_name;
    } else {
      context = bctx_instr_code;
    }
  }

  void logParseEvent(const std::string &eventName, char *buffer, size_t bufferSize, size_t filledSize, bool binary = true)
  {
#ifdef BION_TRACE_ENABLED
    perf::Log::addDebug(scString("Bion trace - event: ")+eventName);
    if (filledSize > 0) {
      BufferLog bufLog(buffer, bufferSize);
      perf::Log::addDebug(scString("Bion trace - buffer: ")+bufLog.getContents(filledSize, binary));
    }
    perf::Log::addDebug(scString("Bion trace - input: ")+getInputLog());
#endif
  }

  std::string getInputLog() {
    return m_input->getContentsForLog();
  }

  size_t parseZString(char *buffer, size_t bufferSize, size_t insertedCount)
  {
    assert(insertedCount < bufferSize);

    char *insertPtr = buffer + insertedCount;
    size_t readCnt = insertedCount;

    readCnt += readString(insertPtr, bufferSize - 1 - readCnt);
    buffer[readCnt] = '\0';

    return readCnt;
  }

  void processStdValue(BionValueType valueType, void *data, size_t dataSize)
  {
    switch(valueType) {
    case bvt_null:
      m_processor->processNull();
      break;
    case bvt_bool:
      m_processor->processBool(*(static_cast<bool *>(data)));
      break;
    case bvt_bool_data: {
      BionBoolDataStorage rawData = *(static_cast<BionBoolDataStorage *>(data));
      m_processor->processBool(rawData > 0);
      break;
                        }
    case bvt_int:
      if (dataSize == sizeof(int))
        m_processor->processInt(*(static_cast<int *>(data)));
      else if (dataSize == sizeof(int64))
        m_processor->processInt64(*(static_cast<int64 *>(data)));
      else 
        throw BionError(BEC_IntSizeInvalid);
      break;
    case bvt_uint:
      if (dataSize == sizeof(unsigned int))
        m_processor->processUInt(*(static_cast<unsigned int *>(data)));
      else if (dataSize == sizeof(uint64))
        m_processor->processUInt64(*(static_cast<uint64 *>(data)));
      else 
        throw BionError(BEC_UIntSizeInvalid);
      break;
    case bvt_float:
      if (dataSize == sizeof(float))
        m_processor->processFloat(*(static_cast<float *>(data)));
      else if (dataSize == sizeof(double))
        m_processor->processDouble(*(static_cast<double *>(data)));
      else if (dataSize == sizeof(xdouble))
        m_processor->processXDouble(*(static_cast<xdouble *>(data)));
      else 
        throw BionError(BEC_FloatSizeInvalid);
      break;
    case bvt_zstring:
        m_processor->processZString(static_cast<char *>(data));
        break;
    default:
      throw BionError(BEC_UnkownStdTypeInProtProc);
    }
  }

  void decodeStdType(unsigned char aType, unsigned int &readSize, int &stdType, BionValueType &valueType)
  {
    unsigned char size;
    if ((aType >= 0x10) && (aType <= 0x8f))
    {
      stdType = aType & 0x0f;
      size = (aType & 0xf0) >> 4;
      if (stdType == 0) {
        stdType = size;
        size = 0;
        switch(stdType) {
          case bst_null: 
            valueType = bvt_null;
            break;
          case bst_bool_true:
            valueType = bvt_bool;
            break;
          case bst_bool_false:
            valueType = bvt_bool;
            break;
          default:
            throw BionError(BEC_UnknownZeroLenValType);
        } // switch
      } else {
        switch(stdType) {
        case bst_int:
          if (size > 1)
            size = 1 << (size - 1);  
          valueType = bvt_int;
          break;
        case bst_uint:
          if (size > 1)
            size = 1 << (size - 1); 
          valueType = bvt_uint;
          break;
        case bst_float:
          if (size == 1)
            size = sizeof(float);
          else if (size == 2)
            size = sizeof(double);
          else if (size == 3)
            size = sizeof(xdouble);
          else
            throw BionError(BEC_UnknownFloatSize);
          valueType = bvt_float;
          break;
        case bst_bool_data:
          size = sizeof(BionBoolDataStorage);
          valueType = bvt_bool;
          break;
        default:
            throw BionError(BEC_UnknownStdValType);
        } // switch stdType

      } // std-type not zero

      readSize = size;
    } else {
      readSize = 0;
    }
  }

  void processStructEnd(OpenStructsStack &openStructs, bool &eofFound, BionParseContext &context)
  {
      OpenStructsStack::value_type openStructCode = getOpenStructCode(openStructs);
      
      if (openStructCode == bic_array_start) {
        m_processor->processArrayEnd();
        openStructs.pop_back();
      } else if (openStructCode == bic_object_start) {
        m_processor->processObjectEnd();
        openStructs.pop_back();
      } else {
        eofFound = true;
        //processString = false;
        //throw BionError(BEC_StructNotOpen);
      }

      openStructCode = getOpenStructCode(openStructs);

      if (openStructCode == bic_object_start) {
        context = bctx_name;
        //nameAdded = false;
        //processString = true;
        //context = bctx_string_val;
      } else {
        context = bctx_instr_code;
        //processString = false;
      }
  }

  OpenStructsStack::value_type getOpenStructCode(OpenStructsStack &openStructs)
  {
      OpenStructsStack::value_type res = 0;

      if (!openStructs.empty()) {
        res = openStructs.back();
      }

      return res;
  }

  size_t readString(char *output, size_t outputSize)
  {
    size_t readSize = 0;
    while ((m_input->peek() != EOF) && (readSize < outputSize))
    {
      m_input->read(output, sizeof(char));
      assert(m_input->gcount() == sizeof(char));
      readSize++;
      if(*output == '\0')
        break;
      output++;
    } 

    if (readSize >= outputSize)
      throw BionError(BEC_ValTooLong);

    if (*output != '\0')
      *output = '\0';

    return readSize;
  }


  void readFixTypeArray(char *buffer, unsigned int bufferSize) {
    size_t elementCount;
    unsigned int elementSize;
    char *cptr;
    bool ok;
    unsigned char sizeLength;
    char rawType;
    BionValueType valueType;
    int stdType;

    if (m_traceOn) logParseEvent(std::string("fix_type_array.begin"), buffer, bufferSize, 0);

    m_input->read(buffer, sizeof(char));
    ok = (m_input->gcount() == sizeof(char));
    assert(ok);

    sizeLength = varint_get_size(buffer, sizeof(char));
    assert(sizeLength > 0);

    if (ok && (sizeLength > sizeof(char))) {
      cptr = buffer + sizeof(char);
      m_input->read(cptr, sizeLength - sizeof(char));
      ok = (m_input->gcount() == sizeLength - sizeof(char));
    }

    if (m_traceOn) logParseEvent(std::string("fix_type_array.len_size: ")+toString(static_cast<uint>(sizeLength)), buffer, bufferSize, 0);

    if (ok && (varint_decode(buffer, sizeLength, elementCount) == sizeLength))
    {
      m_input->read(&rawType, sizeof(char));
      ok = (m_input->gcount() == sizeof(char));
      assert(ok);
    } else {
      elementCount = 0;
      rawType = bvt_undef;
    }

    if (m_traceOn) logParseEvent(std::string("fix_type_array.val_cnt: ")+toString(elementCount), buffer, bufferSize, 0);
    if (m_traceOn) logParseEvent(std::string("fix_type_array.val_type: ")+toString(static_cast<uint>(rawType)), buffer, bufferSize, 0);

    if (ok)
    {
      decodeStdType(rawType, elementSize, stdType, valueType);
      if (valueType == bvt_bool) 
        elementSize = sizeof(BionBoolDataStorage);
      else if (valueType == bvt_null)
        elementSize = sizeof(BionNullDataStorage);

      assert(elementSize <= bufferSize);

      if (m_traceOn) logParseEvent(std::string("fix_type_array.val_size: ")+toString(static_cast<uint>(elementSize)), buffer, bufferSize, 0);

      m_processor->processFixTypeArrayBegin(valueType, elementSize, elementCount);

      int dbgIdx = 0;

      while(elementCount > 0)
      {
        m_input->read(buffer, elementSize);
        assert(m_input->gcount() == elementSize);
        if (m_traceOn) logParseEvent(std::string("fix_type_array.val_data[")+toString(dbgIdx++)+"]", buffer, bufferSize, elementSize);
        processStdValue(valueType, buffer, elementSize);
        elementCount--;
      }

      if (m_traceOn) logParseEvent(std::string("fix_type_array.end"), buffer, bufferSize, 0);
      m_processor->processFixTypeArrayEnd();
    }
  }
private:
  InputStream *m_input;
  InputProcessor *m_processor;
  bool m_traceOn;
};

/// Required interface for reader input source
class BionInputStreamIntf {
public:
  /// read bytes, up to limit
  virtual void read(void *output, size_t limit) = 0;
  /// check if EOF
  virtual int peek() = 0;
  /// returns number of bytes read by "read" method
  virtual size_t gcount() const = 0;
};

class BionInputMemoryStream
#ifdef _DEBUG
  : public BionInputStreamIntf 
#endif
{
public:
  BionInputMemoryStream(void *buffer, size_t size): m_buffer(buffer), m_size(size), m_offset(0), m_readCnt(0), m_limit(size) {
    m_ptr = reinterpret_cast<char *>(buffer);
  } 

  void reset() {
    m_ptr = reinterpret_cast<char *>(m_buffer);
    m_offset = 0;
    m_readCnt = 0;
  }

  void read(void *output, size_t limit) {
    if (m_offset < m_limit)
    {
      size_t leftCnt = m_limit - m_offset;
      if (limit <= leftCnt)
        m_readCnt = limit;
      else
        m_readCnt = leftCnt;
    }
    std::memcpy(output, m_ptr, m_readCnt);
    m_ptr += m_readCnt;
    m_offset += m_readCnt;
  }

  /// check if EOF
  int peek() {
    if (m_offset < m_limit)
      return *m_ptr;
    else
      return EOF;
  }

  void setLimit(size_t limit) {
    if (limit <= m_size)
      m_limit = limit;
    else
      throw BionError(BEC_InvalidInputLimit);
  }

  /// returns number of bytes read by "read" method
  size_t gcount() const {
    return m_readCnt;
  }

  std::string getContentsForLog() {
    size_t leftCnt = m_limit - m_offset;
    if (leftCnt > 100)
      leftCnt = 100;
    BufferLog bufLog(m_ptr, m_size);
    return bufLog.getContents(leftCnt, true);
  }
private:
  void *m_buffer;
  char *m_ptr;
  size_t m_offset;
  size_t m_size;
  size_t m_readCnt;
  size_t m_limit;
};

/// Required interface for input processor
class BionReaderProcessorIntf {
public:
  virtual void processHeader(void *data, size_t dataSize) = 0;
  virtual void processFooter(void *data, size_t dataSize) = 0;
  virtual void processObjectBegin() = 0;
  virtual void processObjectEnd() = 0;
  virtual void processArrayBegin() = 0;
  virtual void processArrayEnd() = 0;
  virtual void processFixTypeArrayBegin(BionValueType valueType, unsigned int elementSize, size_t arraySize) = 0;
  virtual void processFixTypeArrayEnd() = 0;
  virtual void processElementName(char *name) = 0;
  virtual void processFloat(float value) = 0;
  virtual void processDouble(double value) = 0;
  virtual void processXDouble(xdouble value) = 0;
  virtual void processZString(char *value) = 0;
  virtual void processBool(bool value) = 0;
  virtual void processNull() = 0;
  virtual void processInt(int value) = 0;
  virtual void processInt64(int64 value) = 0;
  virtual void processUInt(uint value) = 0;
  virtual void processUInt64(uint64 value) = 0;
};

#endif // _BION_H__
