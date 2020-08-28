/////////////////////////////////////////////////////////////////////////////
// Name:        FileLog.h
// Project:     scLib
// Purpose:     Log which writes output to file
// Author:      Piotr Likus
// Modified by:
// Created:     19/09/2009
/////////////////////////////////////////////////////////////////////////////

#ifndef _FILELOG_H__
#define _FILELOG_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/// \file FileLog.h
///
/// Log which writes output to file

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
//sc
//#include "sc/dtypes.h"
//#include "sc/log.h"
#include "base/string.h"
#include "perf/log.h"

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
class FileLog: public LogDevice {
  typedef LogDevice inherited;
public:
  FileLog(const dtpString &fileName);
  ~FileLog();
  void setSizeLimit(uint64 value);
  uint64 getSizeLimit();
  virtual bool init();
protected:
  virtual void intAddText(const dtpString &a_text);
  virtual void intAddLines(LogLinesGeneratorIntf *generator);
  virtual void formatMessage(dtpString &output, const dtpString &a_text, LogMsgLevel level, const dtpString &msgCode);
  void checkSizeLimitReached();
  void performArchiveFile();
  void *openFile();
protected:
  dtpString m_fileName;
  uint64 m_sizeLimit;
};

}; // namespace perf

#endif // _FILELOG_H__
