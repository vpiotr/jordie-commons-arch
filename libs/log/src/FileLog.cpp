/////////////////////////////////////////////////////////////////////////////
// Name:        FileLog.cpp
// Project:     perfLib
// Purpose:     Log which writes output to file
// Author:      Piotr Likus
// Modified by:
// Created:     19/09/2009
/////////////////////////////////////////////////////////////////////////////

// std
#include <fstream>
#include <sstream>

// boost
#include <boost/filesystem.hpp>

// base
#include "base/date.h"

// sc
//#include "sc/FileLog.h"
//#include "sc/utils.h"
#include "base/file_utils.h"

// perf
#include "perf/FileLog.h"

#ifdef DEBUG_MEM
#include "dbg/DebugMem.h"
#endif

using namespace dtp;
using namespace perf;

FileLog::FileLog(const dtpString &fileName): LogDevice(), m_fileName(fileName), m_sizeLimit(0)
{
}

FileLog::~FileLog()
{
}

void FileLog::setSizeLimit(uint64 value)
{
  m_sizeLimit = value;
}

uint64 FileLog::getSizeLimit()
{
  return m_sizeLimit;
}

bool FileLog::init()
{
    bool res = true;

    try {
      intAddText("Log started");
      res = fileExists(m_fileName);
    } catch(...) {
      res = false;
    }
    return res;
}

void *FileLog::openFile()
{
  std::auto_ptr<std::ofstream> logFile(new std::ofstream());

  if (m_sizeLimit > 0)
    checkSizeLimitReached();

  logFile->open(stringToStdString(m_fileName), std::ios::out | std::ios::app);

  if (!logFile->is_open())
    throw std::runtime_error("File open (for write) error: ["+m_fileName+"]");

  return logFile.release();
}

void FileLog::intAddText(const dtpString &a_text)
{
  std::auto_ptr<std::ofstream> logFile(static_cast<std::ofstream *>(openFile()));
  (*logFile) << a_text.c_str() << "\n";
}

void FileLog::intAddLines(LogLinesGeneratorIntf *generator)
{
  std::auto_ptr<std::ofstream> logFile(static_cast<std::ofstream *>(openFile()));
  dtpString str;
  while(generator->hasNext())
  {
    generator->getNext(str);
    (*logFile) << str << "\n";
  }
}

void FileLog::formatMessage(dtpString &output, const dtpString &a_text, LogMsgLevel level, const dtpString &msgCode)
{
  using namespace dtp;
  inherited::formatMessage(output, a_text, level, msgCode);

  if ((getFormatOptions() & lfTimestamp) != 0)
    output = dateTimeToIsoStr(currentDateTime()) + ": " + output;
}

void FileLog::checkSizeLimitReached()
{
  if (m_sizeLimit > 0)
  if (boost::filesystem::exists(stringToStdString(m_fileName)))
  if (boost::filesystem::file_size(stringToStdString(m_fileName)) >= m_sizeLimit)
    performArchiveFile();
}

void FileLog::performArchiveFile()
{
  boost::filesystem::path p(m_fileName.c_str());
  std::string newPart;
  std::string archFileName;
  std::string fname;
  bool archived;

  archived = false;

  for(uint i=0, epos = 1000; i != epos; i++) {
    p = boost::filesystem::path(m_fileName.c_str());
    newPart = std::string(dateTimeToNoSepStr(currentDateTime()));
    if (i > 0) {
      newPart += toString(i);
    }
    newPart += p.extension().string();
    fname = p.stem().string();
    fname += "." + newPart;
    p.remove_filename();
    p /= fname;
    if (!boost::filesystem::exists(p)) {
      boost::filesystem::rename(stringToStdString(m_fileName), p);
      archived = true;
      break;
    }
  }

  if (!archived)
    throw std::runtime_error("Log file overflow");
}
