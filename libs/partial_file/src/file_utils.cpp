/////////////////////////////////////////////////////////////////////////////
// Name:        file_utils.cpp
// Project:     scLib
// Purpose:     File utility functions
// Author:      Piotr Likus
// Modified by:
// Created:     20/06/2010
/////////////////////////////////////////////////////////////////////////////
#include "base/file_utils.h"

// std
#include <fstream>

// boost
#pragma warning(disable:4640) // construction of local static object is not thread-safe
#include <boost/filesystem/operations.hpp>
#pragma warning(default:4640) 

// boost
#include <boost/filesystem/convenience.hpp>

#ifdef DEBUG_MEM
#include "dbg/DebugMem.h"
#endif

bool createEmptyFile(const dtpString &fname)
{
  if (fileExists(fname))
    throw std::runtime_error(dtpString("File already exists: ")+fname);
  
  std::ofstream myfile;
  myfile.open(stringToStdString(fname), std::ios::out | std::ios::trunc);
  if (!myfile.is_open())
    throw std::runtime_error("File open (for write) error: ["+fname+"] // [FU]");  
  myfile.close();
  return true;
}

bool deleteFile(const dtpString &fname)
{
  boost::filesystem::remove(stringToStdString(fname));
  return true;
}

bool deleteFileIfExists(const dtpString &fname)
{
  if (fileExists(fname))
    return deleteFile(fname);
  else
    return false;  
}

bool fileExists(const dtpString &fname)
{
  return boost::filesystem::exists(stringToStdString(fname));
}

bool renameFile(const dtpString &oldFileName, const dtpString &newFileName)
{
  boost::filesystem::rename(stringToStdString(oldFileName), stringToStdString(newFileName));
  return true;
}

uint64 getFileSize(const dtpString &fname)
{
  return boost::filesystem::file_size(stringToStdString(fname));
}

size_t findDirSepPos(const dtpString &a_path)
{
  size_t idx1 = a_path.find_last_of("\\");
  size_t idx2 = a_path.find_last_of("/");
  size_t lastPos;

  lastPos = dtpString::npos;
  if (idx1 != dtpString::npos)
    lastPos = idx1;

  if (idx2 != dtpString::npos)
    if ((lastPos == dtpString::npos) || (lastPos < idx2))
      lastPos = idx2;

  return lastPos;
}

dtpString extractDir(const dtpString &a_path)
{
  size_t lastPos = findDirSepPos(a_path);

  if (lastPos != dtpString::npos)
    return a_path.substr(0, lastPos);
  else
    return a_path;
}

dtpString extractFileName(const dtpString &path)
{
  size_t lastPos = findDirSepPos(path);

  if (lastPos != dtpString::npos)
    return path.substr(lastPos+1);
  else
    return path;
}

bool preparePath(const dtpString &path)
{
  return boost::filesystem::create_directories(stringToStdString(path));
}
