/////////////////////////////////////////////////////////////////////////////
// Name:        ConsoleLog.cpp
// Project:     perfLib
// Purpose:     Class that saves log messages to console
// Author:      Piotr Likus
// Modified by:
// Created:     28/04/2010
/////////////////////////////////////////////////////////////////////////////
//std
#include <iostream>

//base
#include "base/date.h"

//sc
#include "perf/ConsoleLog.h"
//#include "sc/utils.h"

#ifdef DEBUG_MEM
#include "dbg/DebugMem.h"
#endif

using namespace std;
using namespace perf;

ConsoleLog::ConsoleLog(): LogDevice()
{
}

ConsoleLog::~ConsoleLog()
{
}

void ConsoleLog::intAddText(const dtpString &a_text)
{
  cout << a_text << "\n";
}

void ConsoleLog::formatMessage(dtpString &output, const dtpString &a_text, LogMsgLevel level, const dtpString &msgCode)
{
  using namespace dtp;
  inherited::formatMessage(output, a_text, level, msgCode);

  if ((getFormatOptions() & lfTimestamp) != 0)
    output = dateTimeToIsoStr(currentDateTime()) + dtpString(": ") + output;
}

