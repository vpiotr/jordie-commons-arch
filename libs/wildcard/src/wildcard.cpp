/////////////////////////////////////////////////////////////////////////////
// Name:        wildcard.cpp
// Project:     scLib
// Purpose:     Wildcard supporting functions (strings like "*.htm?")
// Author:      Piotr Likus
// Modified by:
// Created:     18/04/2013
/////////////////////////////////////////////////////////////////////////////

#include "base/wildcard.h"

#ifdef WIN32
#define WILDMODE_WIN32
#else
#define WILDMODE_REGEXP
#endif

#ifdef WILDMODE_REGEXP
#define REGEXP_XPRESSIVE
#endif

//std
#include <cstdlib>
#include <ctime>
#include <sstream>

#ifdef REGEXP_BOOST
#include <boost/regex.hpp>
#endif

#ifdef REGEXP_XPRESSIVE
#pragma warning( disable : 4121 ) 
#include <boost/xpressive/xpressive.hpp>
#pragma warning( default : 4121 )
#endif //REGEXP_XPRESSIVE

#ifdef REGEXP_GRETA
//greta
#include "regexpr2.h"
#endif

#ifdef WILDMODE_WIN32
#ifdef WILD_WIN32_DBG
#include "DbgHelp.h"
#else
#include "Shlwapi.h"
#pragma comment(lib,"shlwapi.lib")
#endif
#endif

#ifdef REGEXP_BOOST
bool wildcard_match_boost(const std::string &testStr, const std::string &pattern)
{
  static const std::string escapeChars = ".^$|()[]+\\/-";
  std::string regexp_pattern;
  std::string stdTestStr = testStr.c_str();
  bool res;

  for(int i=0, cnt=pattern.length(); i < cnt; i++)
  {
    if (pattern[i] == '*')
      regexp_pattern += ".*";
    else if (pattern[i] == '?')
      regexp_pattern += ".{1}";
    else if (escapeChars.find_first_of(pattern[i]) != std::string::npos)
      regexp_pattern += std::string("\\")+pattern[i];
    else
      regexp_pattern += pattern[i];
  }

  boost::regex expression(regexp_pattern.c_str());

  boost::cmatch what;

  res = boost::regex_match(stdTestStr.c_str(), what, expression);

  return res;
}
#endif

#ifdef REGEXP_XPRESSIVE
bool wildcard_match_xpressive(const std::string &testStr, const std::string &pattern)
{
  using namespace boost::xpressive;

  const std::string escapeChars(".^$|()[]+\\/-");
  std::string regexp_pattern;
  std::string stdTestStr = testStr.c_str();
  bool res;

  for(int i=0, cnt=pattern.length(); i < cnt; i++)
  {
    if (pattern[i] == '*')
      regexp_pattern += ".*";
    else if (pattern[i] == '?')
      regexp_pattern += ".{1}";
    else if (escapeChars.find_first_of(pattern[i]) != std::string::npos)
      regexp_pattern += std::string("\\")+pattern[i];
    else
      regexp_pattern += pattern[i];
  }

  sregex rex = sregex::compile(regexp_pattern.c_str());

  smatch what;

  res = regex_match(stdTestStr, what, rex);

  return res;
}
#endif

#ifdef WILDMODE_WIN32
bool wildcard_match_win32api(const std::string &testStr, const std::string &pattern)
{
  // compare first char - to skip API call (works with empty strings)
  char c1 = testStr[0];
  char c2 = pattern[0];
  if (c1 != c2)
  {
    if ((c2 != '?') && (c2 != '*'))
      return false;
  }

  if (c1 == '\0')
  { 
    if (c2 == '\0')
      return true;
    else
      return false;
  }

#ifdef WILD_WIN32_DBG
  return (SymMatchString(testStr.c_str(), pattern.c_str(), TRUE) == TRUE);
#else
  return (PathMatchSpec(testStr.c_str(), pattern.c_str()) == TRUE);
#endif
}
#endif

bool wildcardMatch(const std::string &testStr, const std::string &pattern)
{
#ifdef REGEXP_GRETA
  return wildcard_match_greta(testStr, pattern);
#endif
#ifdef REGEXP_BOOST
  return wildcard_match_boost(testStr, pattern);
#endif
#ifdef REGEXP_XPRESSIVE
  return wildcard_match_xpressive(testStr, pattern);
#endif
#ifdef WILDMODE_WIN32
  return wildcard_match_win32api(testStr, pattern);
#endif
}

// ----------------------------------------------------------------------------
// WildcardMatcherImpl
// ----------------------------------------------------------------------------
#ifdef REGEXP_XPRESSIVE
class WildcardMatcher::WildcardMatcherImpl {
public:
  WildcardMatcherImpl(const std::string &pattern) 
  { 
    using namespace boost::xpressive;

    const std::string escapeChars(".^$|()[]+\\/-");
    std::string regexp_pattern;

    for(int i=0, cnt=pattern.length(); i < cnt; i++)
    {
      if (pattern[i] == '*')
        regexp_pattern += ".*";
      else if (pattern[i] == '?')
        regexp_pattern += ".{1}";
      else if (escapeChars.find_first_of(pattern[i]) != std::string::npos)
        regexp_pattern += std::string("\\")+pattern[i];
      else
        regexp_pattern += pattern[i];
    }

    m_rex = sregex::compile(regexp_pattern.c_str());
  }

  bool isMatching(const std::string &testStr) {
    using namespace boost::xpressive;
    smatch what;
    return regex_match(testStr, what, m_rex);
  }

private:
  boost::xpressive::sregex m_rex;
};
#endif

#ifdef WILDMODE_WIN32
class WildcardMatcher::WildcardMatcherImpl {
public:
  WildcardMatcherImpl(const std::string &pattern): m_pattern(pattern)
  { }

  bool isMatching(const std::string &testStr) {
    return wildcard_match_win32api(testStr, m_pattern);
  }

private:
  std::string m_pattern;
};
#endif

// ----------------------------------------------------------------------------
// WildcardMatcher
// ----------------------------------------------------------------------------
WildcardMatcher::WildcardMatcher(const std::string &pattern)
{
  m_body = new WildcardMatcherImpl(pattern);
}

WildcardMatcher::~WildcardMatcher()
{
  delete m_body;
}

bool WildcardMatcher::isMatching(const std::string &testStr)
{
  return m_body->isMatching(testStr);
}

