/////////////////////////////////////////////////////////////////////////////
// Name:        file_utils.h
// Project:     scLib
// Purpose:     File utility functions
// Author:      Piotr Likus
// Modified by:
// Created:     20/06/2010
/////////////////////////////////////////////////////////////////////////////

#ifndef _FILEUTILS_H__
#define _FILEUTILS_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/// \file file_utils.h
///
/// File utility functions

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
//#include "sc/dtypes.h"
#include "base/string.h"

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

// ----------------------------------------------------------------------------
// Global functions
// ----------------------------------------------------------------------------
bool createEmptyFile(const dtpString &fname);
bool deleteFile(const dtpString &fname);
bool deleteFileIfExists(const dtpString &fname);
bool fileExists(const dtpString &fname);
bool renameFile(const dtpString &oldFileName, const dtpString &newFileName);
uint64 getFileSize(const dtpString &fname);


/// Extract directory part from file path
dtpString extractDir(const dtpString &path);
/// Extract file name part from file path
dtpString extractFileName(const dtpString &path);

/// Create all dirs in path (if necessary)
bool preparePath(const dtpString &path);

#endif // _FILEUTILS_H__
