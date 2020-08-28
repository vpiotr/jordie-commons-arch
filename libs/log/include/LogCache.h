/////////////////////////////////////////////////////////////////////////////
// Name:        LogCache.h
// Project:     perfLib
// Purpose:     Caching device for log
// Author:      Piotr Likus
// Modified by:
// Created:     12/10/2013
/////////////////////////////////////////////////////////////////////////////

#ifndef _PERFLOGCACHE_H__
#define _PERFLOGCACHE_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/** \file LogCache.h
\brief Caching device for log

*/

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
#include <list>

#include "perf\Log.h"

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
class LogCache: public LogDevice {
public:
  typedef std::list<std::string> LineList;
  LogCache(perf::LogDevice &logDevice);
  virtual ~LogCache();
  void setCacheLimit(uint64 value);
  uint64 getCacheLimit();
private:
  virtual void intFlush();
  virtual void intAddText(const dtpString &a_text);
private:
  uint64 m_cacheLimit;
  uint64 m_size;
  LineList m_lines;
  perf::LogDevice *m_logDevice;
};

}; // namespace perf

#endif // _PERFLOGCACHE_H__
