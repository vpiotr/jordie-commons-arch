/////////////////////////////////////////////////////////////////////////////
// Name:        MultiDeviceLog.h
// Project:     perfLib
// Purpose:     Class that saves log messages to several other log devices.
// Author:      Piotr Likus
// Modified by:
// Created:     28/04/2010
/////////////////////////////////////////////////////////////////////////////

#include "perf/MultiDeviceLog.h"

#ifdef DEBUG_MEM
#include "dbg/DebugMem.h"
#endif

using namespace perf;

MultiDeviceLog::MultiDeviceLog(): Log()
{
}
MultiDeviceLog::~MultiDeviceLog()
{
}

void MultiDeviceLog::addDevice(LogDevice *device, uint msgLevelMask)
{
  m_devices.push_back(device);
  m_levels.push_back(msgLevelMask);
}

void MultiDeviceLog::intAddText(const dtpString &a_text)
{
  intAddText(a_text, lmlInfo, 0);
}

void MultiDeviceLog::intAddText(const dtpString &a_text, LogMsgLevel level, uint msgCode)
{
  for(uint i=0, epos = m_devices.size(); i != epos; i++)
  {
    if ((m_levels[i] == 0) || ((m_levels[i] & level) != 0))
      m_devices[i].intAddText(a_text, level, msgCode);
  }
}

void MultiDeviceLog::intAddText(const dtpString &a_text, LogMsgLevel level, const dtpString &msgCode)
{
  for(uint i=0, epos = m_devices.size(); i != epos; i++)
  {
    if ((m_levels[i] == 0) || ((m_levels[i] & level) != 0))
      m_devices[i].intAddText(a_text, level, msgCode);
  }
}

void MultiDeviceLog::intFlush()
{
  for(uint i=0, epos = m_devices.size(); i != epos; i++)
    m_devices[i].intFlush();
}

