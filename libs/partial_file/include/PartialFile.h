/////////////////////////////////////////////////////////////////////////////
// Name:        PartialFile.h
// Project:     scLib
// Purpose:     Class for handling partial files - files with commitment 
//              control (*.part). 
// Author:      Piotr Likus
// Modified by:
// Created:     20/06/2010
/////////////////////////////////////////////////////////////////////////////

#ifndef _PARTIALFILE_H__
#define _PARTIALFILE_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/// \file PartialFile.h
///
/// File with commitment control.

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
#include "sc/dtypes.h"

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
class scPartialFile {
public:
  // create
  scPartialFile(const scString &fname);
  virtual ~scPartialFile();
  
  // properties
  scString getCommitedFileName() const;
  scString getUncommitedFileName() const;
  scString getActiveFileName() const;
  void setWorkFileSuffix(const scString &value);

  // --- run

  /// Check if file is uncommited, if yes, return back to commited state
  void resync();

  void start();
  bool isCommited() const;
  void commit();
  void rollback();
protected:  
  scString getOldFileName() const;
protected:
  scString m_fileName; 
  scString m_workFileSuffix;
};  
  
#endif // _PARTIALFILE_H__
