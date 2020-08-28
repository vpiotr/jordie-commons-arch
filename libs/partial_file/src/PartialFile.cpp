/////////////////////////////////////////////////////////////////////////////
// Name:        PartialFile.cpp
// Project:     scLib
// Purpose:     Class for handling partial files - files with commitment 
//              control (*.part). 
// Author:      Piotr Likus
// Modified by:
// Created:     20/06/2010
/////////////////////////////////////////////////////////////////////////////

#include "base/file_utils.h"

#include "sc/dtypes.h"
#include "sc/file/PartialFile.h"

#ifdef DEBUG_MEM
#include "sc/DebugMem.h"
#endif

scPartialFile::scPartialFile(const scString &fname)
{
  m_fileName = fname;
}

scPartialFile::~scPartialFile()
{
}
  
scString scPartialFile::getCommitedFileName() const
{
  return m_fileName;
}
  
scString scPartialFile::getUncommitedFileName() const
{
  const scString partSfx = ".part";
  scString res(m_fileName);
  res += m_workFileSuffix;
  res += partSfx;
  return res;
}

scString scPartialFile::getOldFileName() const
{
  const scString oldSfx = ".old";
  scString res(m_fileName);
  res += m_workFileSuffix;
  res += oldSfx;
  return res;
}

scString scPartialFile::getActiveFileName() const
{
  scString res = getUncommitedFileName();
  if (!fileExists(res))
    res = getCommitedFileName();
  return res;  
}

void scPartialFile::setWorkFileSuffix(const scString &value)
{
  m_workFileSuffix = value;
}

/// - if *.old found - commit failed:
///   - if *.txt found - delete *.txt
///   - rename *.old to *.txt
/// - delete *.part
void scPartialFile::resync()
{
  if (fileExists(getOldFileName()))
  {
    scString commitedFName(getCommitedFileName());
    if (fileExists(commitedFName))
      deleteFile(commitedFName);
    renameFile(getOldFileName(), commitedFName);  
  } 
  if (fileExists(getUncommitedFileName()))
    deleteFile(getUncommitedFileName());
}

/// - run resync
/// - creates *.txt.part (truncate)
void scPartialFile::start()
{
  if (!isCommited())
    resync();
  createEmptyFile(getUncommitedFileName());  
}

/// - returns <true> if no partial/old files exists
bool scPartialFile::isCommited() const
{
  return (!fileExists(getUncommitedFileName()) && !fileExists(getOldFileName()));
}

/// - renames from *.txt to *.old (if exists)
/// - renames from *.txt.part to *.txt
/// - deletes *.old (if exists)
void scPartialFile::commit()
{
  scString commitedName = getCommitedFileName();
  if (fileExists(getUncommitedFileName()))  
  {
    if (fileExists(commitedName))
      renameFile(commitedName, getOldFileName());
    renameFile(getUncommitedFileName(), getCommitedFileName());
  }  
  scString oldFName(getOldFileName());
  if (fileExists(oldFName))
    deleteFile(oldFName);
}

void scPartialFile::rollback()
{
  resync();
}
