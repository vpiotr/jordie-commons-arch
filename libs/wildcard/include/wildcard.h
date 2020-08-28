/////////////////////////////////////////////////////////////////////////////
// Name:        wildcard.h
// Project:     scLib
// Purpose:     Wildcard supporting functions (strings like "*.htm?")
// Author:      Piotr Likus
// Modified by:
// Created:     18/04/2013
/////////////////////////////////////////////////////////////////////////////

#ifndef _SCWILDCARD_H__
#define _SCWILDCARD_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/// \file wildcard.h
/// \brief Wildcard supporting functions (strings like "*.htm?")
/// 

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
#include "base/string.h"

// ----------------------------------------------------------------------------
// Classes
// ----------------------------------------------------------------------------
/// Version of "wildcardMatch" suitable for testing more then one string
/// in a row with the same pattern.
class WildcardMatcher {
public:
  WildcardMatcher(const std::string &pattern);
  ~WildcardMatcher();
  bool isMatching(const std::string &testStr);
private:
  class WildcardMatcherImpl;
  WildcardMatcherImpl *m_body;
};

// ----------------------------------------------------------------------------
// Functions
// ----------------------------------------------------------------------------

/// Returns true if testStr matches the given pattern 
bool wildcardMatch(const std::string &testStr, const std::string &pattern);

#endif // _SCWILDCARD_H__
