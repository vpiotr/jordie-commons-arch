/////////////////////////////////////////////////////////////////////////////
// Name:        Log.h
// Project:     perfLib
// Purpose:     Base log class for scLib and sgpLib
// Author:      Piotr Likus
// Modified by:
// Created:     29/12/2009
/////////////////////////////////////////////////////////////////////////////


#ifndef _PERFLOG_H__
#define _PERFLOG_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/// \file Log.h
///
/// Log message types:
/// - info    - low volume, info-only
/// - warning - to be verified
/// - error   - process error
/// - trace   - high volume info, release & debug versions, cli switch
/// - debug   - info for debug-only release

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
#include "perf/details/ptypes.h"

namespace perf {

// ----------------------------------------------------------------------------
// Simple type definitions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Forward class definitions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Class definitions
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// Log
// ----------------------------------------------------------------------------
enum LogMsgLevel {
  lmlTrace = 1,
  lmlDebug = 2,
  lmlInfo = 4,
  lmlWarning = 8,
  lmlError = 16,
  lmlCriticalError = 32
};

enum LogFormat {
  lfTimestamp = 1,
  lfMsgType = 2,
  lfHeader = 4
};

const uint lfDefault = lfTimestamp + lfMsgType + lfHeader;
const uint lfRawMsg = 0;
const uint lmlAll = 0xffffffff;

// generates list of lines
class LogLinesGeneratorIntf {
public:
  virtual ~LogLinesGeneratorIntf() {};
  virtual bool hasNext() = 0;
  virtual void getNext(dtpString &output) = 0;
};

// log class
class Log {
public:
  Log();
  virtual ~Log();
  void setFormatOptions(uint options);
  uint getFormatOptions();
  // universal methods
  static void addText(const dtpString &a_text);
  static void addText(const dtpString &a_text, LogMsgLevel level, uint msgCode = 0);
  static void addText(const dtpString &a_text, LogMsgLevel level, const dtpString &msgCode);
  static void addLines(LogLinesGeneratorIntf *generator);
  /// debug information
  static void addDebug(const dtpString &a_text);
  /// general status information
  static void addInfo(const dtpString &a_text);
  /// warning - something incorrect happend, but process continues
  static void addWarning(const dtpString &a_text);
  /// trace - mass log, can be generated in release version on request
  static void addTrace(const dtpString &a_text);
  /// error - task/call/program processing aborted
  static void addError(const dtpString &a_text);
  static void addError(int errorCode, const dtpString &a_text);
  static void flush();
protected:
  virtual void formatMessage(dtpString &output, const dtpString &a_text, LogMsgLevel level, uint msgCode);
  virtual void formatMessage(dtpString &output, const dtpString &a_text, LogMsgLevel level, const dtpString &msgCode);
  virtual void intAddText(const dtpString &a_text) = 0;
  virtual void intAddText(const dtpString &a_text, LogMsgLevel level, uint msgCode);
  virtual void intAddText(const dtpString &a_text, LogMsgLevel level, const dtpString &msgCode);
  virtual void intAddLines(LogLinesGeneratorIntf *generator);
  virtual void intAddError(const dtpString &a_text, int errorCode = 0);
  virtual void intAddInfo(const dtpString &a_text);
  virtual void intAddWarning(const dtpString &a_text);
  virtual void intAddDebug(const dtpString &a_text);
  virtual void intAddTrace(const dtpString &a_text);
  static bool checkLog();
  virtual void intFlush();
private:
  static Log *m_activeLog;
  uint m_formatOptions;
};

class LogDevice: public Log {
public:
  using Log::intAddText;
  using Log::intFlush;
  using Log::intAddLines;
};

}; // namespace perf

#endif // _PERFLOG_H__
