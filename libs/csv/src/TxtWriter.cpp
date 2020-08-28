/////////////////////////////////////////////////////////////////////////////
// Name:        TxtWriter.cpp
// Project:     scLib
// Purpose:     Writer class for text files 
// Author:      Piotr Likus
// Modified by:
// Created:     01/12/2008
/////////////////////////////////////////////////////////////////////////////
// std
#include <fstream>
#include <sstream>
// boost
#include <boost/filesystem/operations.hpp>

// base
#include "base/file_utils.h"

// perf
#include "perf\Timer.h"
#include "perf\Counter.h"

// sc
#include "sc/utils.h"
#include "sc/txtf/TxtWriter.h"

#ifdef DEBUG_MEM
#include "sc/DebugMem.h"
#endif

scTxtWriter::scTxtWriter()
{
  m_formatter = SC_NULL;
  m_fileHandle = SC_NULL;
  setTruncateFileFlag(true);
}

scTxtWriter::~scTxtWriter()
{
  checkClosed();
}

void scTxtWriter::setTruncateFileFlag(bool value)
{
  m_truncateFileFlag = value;
}
  
bool scTxtWriter::getTruncateFileFlag()
{
  return m_truncateFileFlag;
}

void scTxtWriter::setFormatter(const scTxtWriterValueFormatter *formatter)
{
  m_formatter = const_cast<scTxtWriterValueFormatter *>(formatter);
}

void scTxtWriter::openFile(const scString &fname)
{
  using namespace perf;
  bool dumpHdr1Running = Timer::isRunning("io-dump-work-hdr-1");

  if (m_fileHandle) {
    if (dumpHdr1Running) 
      Counter::inc("io-dump-work-hdr-1-2-2-1");
    if (dumpHdr1Running) Counter::inc("io-dump-work-hdr-1-2-2-"+fname);

    if (dumpHdr1Running) Timer::start("io-dump-work-hdr-1-2-2-1");
    m_fileEmpty = isFileEmpty(fname);
    if (dumpHdr1Running) Timer::stop("io-dump-work-hdr-1-2-2-1");

    if (dumpHdr1Running) {
      if (m_fileEmpty) Counter::inc("io-dump-work-hdr-1-2-2-1-empty");
      else Counter::inc("io-dump-work-hdr-1-2-2-1-nonempty");
    }

    std::ofstream *myfile = (std::ofstream *)m_fileHandle; 

    if (m_truncateFileFlag) {
      if (dumpHdr1Running) Timer::start("io-dump-work-hdr-1-2-2-2");
      myfile->open(stringToStdString(fname), std::ios::out | std::ios::trunc);
      if (dumpHdr1Running) Timer::stop("io-dump-work-hdr-1-2-2-2");
      m_fileEmpty = true;
    } else {   
      if (dumpHdr1Running) Timer::start("io-dump-work-hdr-1-2-2-3");
      myfile->open(stringToStdString(fname), std::ios::out | std::ios::app);
      if (dumpHdr1Running) Timer::stop("io-dump-work-hdr-1-2-2-3");
    }
    
    if (dumpHdr1Running) Timer::start("io-dump-work-hdr-1-2-2-4");
    if (!myfile->is_open())
      throw scError("File open (for write) error: ["+fname+"]");
    if (dumpHdr1Running) Timer::stop("io-dump-work-hdr-1-2-2-4");
  } else {
    throw scError("File handle not assigned!");
  }    
}

bool scTxtWriter::isFileEmpty(const scString &fname)
{
  if (!fileExists(fname))
    return true;
  else  
    return (getFileSize(fname) <= 0);
}

void scTxtWriter::checkClosed()
{
  if (m_fileHandle != SC_NULL)
  {
    std::ofstream *fileHandle = static_cast<std::ofstream *>(m_fileHandle);
    fileHandle->close();
    m_fileHandle = SC_NULL;
    delete fileHandle;
  }
}

bool scTxtWriter::isFileOpen()
{
  return (m_fileHandle != SC_NULL);
}

void scTxtWriter::beginWrite(const scString &fname, const scDataNode &input)
{
  using namespace perf;
  bool dumpHdr1Running = Timer::isRunning("io-dump-work-hdr-1");

  if (dumpHdr1Running) Timer::start("io-dump-work-hdr-1-2-all");
  if (dumpHdr1Running) Timer::start("io-dump-work-hdr-1-2-1");
  checkClosed();
  if (dumpHdr1Running) Timer::stop("io-dump-work-hdr-1-2-1");

  m_textHandle = SC_NULL;
  m_fileHandle = new std::ofstream();

  if (dumpHdr1Running) {
    if (m_truncateFileFlag) Counter::inc("io-dump-work-hdr-1-2-trunc");
    else Counter::inc("io-dump-work-hdr-1-2-notrunc");
  }

  if (dumpHdr1Running) Timer::start("io-dump-work-hdr-1-2-2");
  openFile(fname);
  if (dumpHdr1Running) Timer::stop("io-dump-work-hdr-1-2-2");

  if (dumpHdr1Running) Timer::start("io-dump-work-hdr-1-2-3");
  checkWriteHeader(input);
  if (dumpHdr1Running) Timer::stop("io-dump-work-hdr-1-2-3");
  if (dumpHdr1Running) Timer::stop("io-dump-work-hdr-1-2-all");
}

void scTxtWriter::endWrite(const scDataNode &input)
{
  if (!isFileOpen())
    return;

  std::ofstream *fileHandle = static_cast<std::ofstream *>(m_fileHandle);
  writeFooter(input);
  fileHandle->close();
  delete fileHandle;
  m_fileHandle = SC_NULL;
}

void scTxtWriter::writeFile(const scDataNode &input, const scString &fname)
{
  using namespace perf;
  bool dumpHdr1Running = Timer::isRunning("io-dump-work-hdr-1");

  if (dumpHdr1Running) Counter::inc("io-dump-work-hdr-1-writeFile");

  checkClosed();
  std::ofstream myfile;

  m_textHandle = SC_NULL;
  m_fileHandle = &myfile;
  try {
    openFile(fname);
    checkWriteHeader(input);
    writeData(input);      
    writeFooter(input);
    myfile.close();
    m_fileHandle = SC_NULL;
  }
  catch(...) {
    m_fileHandle = SC_NULL;
    throw;
  }            
}

void scTxtWriter::writeText(const scDataNode &input, scString &output)
{
  checkClosed();
	std::ostringstream out;
	m_textHandle = &out;
	m_fileHandle = SC_NULL;
	m_fileEmpty = true;
  try {	
	  checkWriteHeader(input);
	  writeData(input);
	  writeFooter(input);
    output = out.str();
	  
  	m_textHandle = SC_NULL;
	}
	catch(...) {
	  m_fileHandle = SC_NULL;
	  throw;
	}            
}

void scTxtWriter::writeData(const scDataNode &input)
{
  if (input.isContainer())
  {
    scDataNode row; 
    for(scDataNode::const_iterator p = input.begin(), epos = input.end(); p != epos; ++p) {
        writeRow(p->getAsNode(row));
    }
  } else {
    writeRow(input);
  }  
}

void scTxtWriter::writeRow(const scDataNode &input)
{
  scString line;
  formatLine(input, line);
  writeLine(line);
}

void scTxtWriter::writeLine(const scString &a_line)
{
  using namespace perf;
  bool dumpHdr1Running = Timer::isRunning("io-dump-work-hdr-1");
  if (dumpHdr1Running) Counter::inc("io-dump-work-hdr-1-writeLine");

  if (m_fileHandle) {
    std::ofstream *mystream = (std::ofstream *)m_fileHandle;  
    (*mystream) << a_line << "\n";  
  }  
  else {
  	std::ostringstream *out = (std::ostringstream *)m_textHandle;
    (*out) << a_line.c_str() << "\n";
  }  
}

void scTxtWriter::checkWriteHeader(const scDataNode &input)
{
  if (m_fileEmpty)
    writeHeader(input);
}

void scTxtWriter::writeHeader(const scDataNode &input)
{ // empty
}

void scTxtWriter::writeFooter(const scDataNode &input)
{ // empty
}

void scTxtWriter::formatLine(const scDataNode &input, scString &output)
{
  output = const_cast<scDataNode &>(input).getAsString();
}

void scTxtWriter::formatValue(scString &value)
{
  if (m_formatter != SC_NULL)
    m_formatter->formatValue(value);
}
