/////////////////////////////////////////////////////////////////////////////
// Name:        ConsoleLog.h
// Project:     perfLib
// Purpose:     Class that saves log messages to console
// Author:      Piotr Likus
// Modified by:
// Created:     28/04/2010
/////////////////////////////////////////////////////////////////////////////

#ifndef _CONSOLELOG_H__
#define _CONSOLELOG_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/// \file ConsoleLog.h
///
/// File description

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
//sc
#include "perf/details/ptypes.h"
#include "perf/Log.h"

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
class ConsoleLog: public LogDevice {
  typedef LogDevice inherited;
public:
  ConsoleLog();
  ~ConsoleLog();
protected:
  virtual void intAddText(const dtpString &a_text);
  virtual void formatMessage(dtpString &output, const dtpString &a_text, LogMsgLevel level, const dtpString &msgCode);
};

}; // namespace perf

#endif // _CONSOLELOG_H__
