/////////////////////////////////////////////////////////////////////////////
// Name:        dnode_serializer.cpp
// Project:     dtpLib
// Purpose:     Serializer for dnode - JSON format
// Author:      Piotr Likus
// Modified by:
// Created:     10/05/2009
/////////////////////////////////////////////////////////////////////////////

#include "dtp/dnode_serializer.h"
#include "dtp/YawlIoClasses.h"

using namespace dtp;

// ----------------------------------------------------------------------------
// dnSerializer
// ----------------------------------------------------------------------------
dnSerializer::dnSerializer()
{
}

void dnSerializer::setCommentsEnabled(bool value)
{
  m_commentsEnabled = value;
}

bool dnSerializer::getCommentsEnabled()
{
  return m_commentsEnabled;
}

int dnSerializer::convToString(const dtp::dnode& input, dtpString &output)
{
#ifdef SC_TIMER_ENABLED
  scTimer::start("JSON.DN.Out.01.ToString");
#endif

  YawlWriter writer(false, DTP_NULL);

  writer.writeDataNode(input);

#ifdef SC_TIMER_ENABLED
  scTimer::start("JSON.DN.Out.02.exportString");
#endif
  writer.outputToString(output);
#ifdef SC_TIMER_ENABLED
  scTimer::stop("JSON.DN.Out.02.exportString");
#endif

#ifdef SC_TIMER_ENABLED
  scTimer::stop("JSON.DN.Out.01.ToString");
#endif

#ifdef SC_DNJSONY_LOG_ENABLED
  scLog::addText("JSON message send: "+output);
#endif

  return 0;
}

int dnSerializer::convFromString(const dtpString &input, dtp::dnode& output)
{
  YawlReaderForDataNode reader(false, m_commentsEnabled);
  int res = 1;

#ifdef SC_DNJSONY_LOG_ENABLED
  scLog::addText("JSON message recv: "+input);
#endif

#ifdef SC_TIMER_ENABLED
  scTimer::start("JSON.DN.In.01.FromString");
#endif

#ifdef SC_DNJSONY_LOG_ENABLED
  try {
#endif
#ifdef SC_TIMER_ENABLED
  scTimer::start("JSON.DN.In.02.parse");
#endif

  if (!reader.parseString(input, output))
  {
    res = 0;
  }
#ifdef SC_TIMER_ENABLED
  scTimer::stop("JSON.DN.In.02.parse");
#endif

#ifdef SC_TIMER_ENABLED
  scTimer::stop("JSON.DN.In.01.FromString");
#endif

#ifdef SC_DNJSONY_LOG_ENABLED
  } // try
  catch (const std::exception& e) {
    scLog::addError("DNSJ01: convFromString - exception: "+dtpString(e.what()));
    throw;
  }
#endif

  return res;
}

