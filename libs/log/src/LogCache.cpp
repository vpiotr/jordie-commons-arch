/////////////////////////////////////////////////////////////////////////////
// Name:        LogCache.h
// Project:     perfLib
// Purpose:     Caching device for log
// Author:      Piotr Likus
// Modified by:
// Created:     12/10/2013
/////////////////////////////////////////////////////////////////////////////

#include "perf\LogCache.h"

using namespace perf;

LogCache::LogCache(perf::LogDevice &logDevice): m_logDevice(&logDevice), m_size(0), m_cacheLimit(0)
{
}

LogCache::~LogCache()
{
}

void LogCache::setCacheLimit(uint64 value)
{
  m_cacheLimit = value;
}

uint64 LogCache::getCacheLimit()
{
  return m_cacheLimit;
}

class LinesGeneratorForList: public LogLinesGeneratorIntf {
public:
  LinesGeneratorForList(LogCache::LineList *list): m_list(list), m_pos(list->begin()) {}

  virtual bool hasNext() { 
    return (m_pos != m_list->end());
  }

  virtual void getNext(dtpString &output) {
    output = *m_pos;
    ++m_pos;
  }

private:
  LogCache::LineList *m_list;
  LogCache::LineList::iterator m_pos;
};

void LogCache::intFlush()
{
  LinesGeneratorForList gen(&m_lines);
  m_logDevice->intAddLines(&gen);
  
  m_lines.clear();
  m_size = 0;
}

void LogCache::intAddText(const dtpString &a_text)
{
  if ((m_cacheLimit > 0) && (m_size + 1 >= m_cacheLimit))
  { 
    // write now
    intFlush();
    m_logDevice->intAddText(a_text);
  } else {
    // cache
    m_lines.push_back(a_text);
    m_size++;
  }
}

