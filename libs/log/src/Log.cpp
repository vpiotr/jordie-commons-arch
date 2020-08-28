/////////////////////////////////////////////////////////////////////////////
// Name:        Log.cpp
// Project:     perfLib
// Purpose:     Base log class for scLib and sgpLib
// Author:      Piotr Likus
// Modified by:
// Created:     29/12/2009
/////////////////////////////////////////////////////////////////////////////

//#include "sc/utils.h"
#include "base/string.h"

#include "perf/Log.h"

#ifdef DEBUG_MEM
#include "dbg/DebugMem.h"
#endif

using namespace perf;

// ----------------------------------------------------------------------------
// Log
// ----------------------------------------------------------------------------
Log* Log::m_activeLog = DTP_NULL;

Log::Log()
{
  if (m_activeLog == DTP_NULL)
    m_activeLog = this;
  m_formatOptions = lfDefault;
}

Log::~Log()
{
  if (m_activeLog == this)
    m_activeLog = DTP_NULL;
}

uint Log::getFormatOptions()
{
  return m_formatOptions;
}

void Log::setFormatOptions(uint options)
{
  m_formatOptions = options;
}

bool Log::checkLog()
{
  if (m_activeLog == DTP_NULL)
    return false;
  else
    return true;
}

void Log::flush()
{
  if (checkLog())
    m_activeLog->intFlush();
}

void Log::addText(const dtpString &a_text)
{
  if (checkLog())
    m_activeLog->intAddText(a_text);
}

void Log::addText(const dtpString &a_text, LogMsgLevel level, uint msgCode)
{
  if (checkLog())
    m_activeLog->intAddText(a_text, level, msgCode);
}

void Log::addText(const dtpString &a_text, LogMsgLevel level, const dtpString &msgCode)
{
  if (checkLog())
    m_activeLog->intAddText(a_text, level, msgCode);
}

void Log::addLines(LogLinesGeneratorIntf *generator)
{
  if (checkLog())
    m_activeLog->intAddLines(generator);
}

void Log::addError(const dtpString &a_text)
{
  if (checkLog())
    m_activeLog->intAddError(a_text);
}

void Log::addError(int errorCode, const dtpString &a_text)
{
  if (checkLog())
    m_activeLog->intAddError(a_text, errorCode);
}

void Log::formatMessage(dtpString &output, const dtpString &a_text, LogMsgLevel level, uint msgCode)
{
  dtpString msgCodeText;

  if (msgCode != 0)
    msgCodeText = toString(msgCode);

  formatMessage(output, a_text, level, msgCodeText);
}

void Log::formatMessage(dtpString &output, const dtpString &a_text, LogMsgLevel level, const dtpString &msgCode)
{
  output = "";
  dtpString msgContext;

  if ((m_formatOptions & lfMsgType) != 0)
  {
    dtpString txtLevel;
    switch (level) {
      case lmlError:
        txtLevel = "error";
        break;
      case lmlWarning:
        txtLevel = "warning";
        break;
      case lmlInfo:
        txtLevel = "info";
        break;
      case lmlCriticalError:
        txtLevel = "critical error";
        break;
      case lmlDebug:
        txtLevel = "debug";
        break;
      default:
        txtLevel = "uknown";
    }
    msgContext = "type='"+txtLevel+"'";
  }

  if (!msgCode.empty())
  {
    if (!msgContext.empty())
      msgContext += ", ";
    msgContext += "code='"+msgCode+"'";
  }

  if ((lfHeader & m_formatOptions))
    output += "Message";

  if (!msgContext.empty())
    output += "["+msgContext+"]: ";

  output += a_text;
}

void Log::intAddText(const dtpString &a_text, LogMsgLevel level, uint msgCode)
{
  dtpString msg;
  formatMessage(msg, a_text, level, msgCode);
  intAddText(msg);
}

void Log::intAddText(const dtpString &a_text, LogMsgLevel level, const dtpString &msgCode)
{
  dtpString msg;
  formatMessage(msg, a_text, level, msgCode);
  intAddText(msg);
}

void Log::intAddLines(LogLinesGeneratorIntf *generator)
{
  dtpString str;
  while(generator->hasNext())
  {
    generator->getNext(str);
    intAddText(str);
  }
}

void Log::intAddError(const dtpString &a_text, int errorCode)
{
  intAddText(a_text, lmlError, errorCode);
}

void Log::addInfo(const dtpString &a_text)
{
  if (checkLog())
    m_activeLog->intAddInfo(a_text);
}

void Log::addWarning(const dtpString &a_text)
{
  if (checkLog())
    m_activeLog->intAddWarning(a_text);
}

void Log::addTrace(const dtpString &a_text)
{
  if (checkLog())
    m_activeLog->intAddTrace(a_text);
}

void Log::addDebug(const dtpString &a_text)
{
  if (checkLog())
    m_activeLog->intAddDebug(a_text);
}

void Log::intAddInfo(const dtpString &a_text)
{
  intAddText(a_text, lmlInfo, 0);
}

void Log::intAddWarning(const dtpString &a_text)
{
  intAddText(a_text, lmlWarning, 0);
}

void Log::intAddDebug(const dtpString &a_text)
{
  intAddText(a_text, lmlDebug, 0);
}

void Log::intAddTrace(const dtpString &a_text)
{
  intAddText(a_text, lmlTrace, 0);
}

void Log::intFlush()
{ // empty here
}


