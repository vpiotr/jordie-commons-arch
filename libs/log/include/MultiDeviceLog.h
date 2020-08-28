/////////////////////////////////////////////////////////////////////////////
// Name:        MultiDeviceLog.h
// Project:     perfLib
// Purpose:     Class that saves log messages to several other log devices.
// Author:      Piotr Likus
// Modified by:
// Created:     28/04/2010
/////////////////////////////////////////////////////////////////////////////

#ifndef _PERFMLTDEVLOG_H__
#define _PERFMLTDEVLOG_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/// \file MultiDeviceLog.h
///
/// Class that saves log messages to several other log devices.

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
// boost
#include "boost/ptr_container/ptr_list.hpp"
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
typedef boost::ptr_vector<LogDevice> LogDeviceColn;
typedef std::vector<uint> LogDeviceLevelMaskColn;

class MultiDeviceLog: public Log {
public:
  MultiDeviceLog();
  ~MultiDeviceLog();
  void addDevice(LogDevice *device, uint msgLevelMask = 0);
protected:
  virtual void intAddText(const dtpString &a_text);
  virtual void intAddText(const dtpString &a_text, LogMsgLevel level, uint msgCode);
  virtual void intAddText(const dtpString &a_text, LogMsgLevel level, const dtpString &msgCode);
  virtual void intFlush();
protected:
  LogDeviceColn m_devices;
  LogDeviceLevelMaskColn m_levels;
};

}; // namespace perf

#endif // _PERFMLTDEVLOG_H__
