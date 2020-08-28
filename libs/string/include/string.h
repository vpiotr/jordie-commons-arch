/////////////////////////////////////////////////////////////////////////////
// Name:        string.h
// Project:     dtpLib
// Purpose:     String type & manipulation functions
// Author:      Piotr Likus
// Modified by:
// Created:     25/04/2012
/////////////////////////////////////////////////////////////////////////////

#ifndef _DTPSTRING_H__
#define _DTPSTRING_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/** \file string.h
\brief String type & manipulation functions

*/

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>
#include <iomanip>
#include <memory>

#include "base/btypes.h"

// ----------------------------------------------------------------------------
// Configuration
// ----------------------------------------------------------------------------
// define symbol below when using something different than std::string as a base for dtpString
//#define SC_NON_STD_STRING

// ----------------------------------------------------------------------------
// Simple type definitions
// ----------------------------------------------------------------------------
typedef std::string dtpString;

// ----------------------------------------------------------------------------
// dtpStringList: list of strings
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
namespace dtp {

class dtpBadConversion : public std::runtime_error {
 public:
   dtpBadConversion(const std::string& s)
     : std::runtime_error(s)
     { }
 };

} // namespace dtp

// ----------------------------------------------------------------------------
// Functions
// ----------------------------------------------------------------------------
dtpString strCharToStr(char c);
dtpString strRepeat(size_t count, const dtpString &pattern);
dtpString strReplace(const dtpString &src, const dtpString &oldStr, const dtpString &newStr, bool replaceAll = true);
size_t strReplaceThis(dtpString &subject, const dtpString &oldStr, const dtpString &newStr, bool replaceAll = true);
// pad
/// Make sure given string is count characters long, if not - add some characters before or after contents
dtpString strPad(const dtpString &value, size_t count, char chPad = ' ', bool fromRight = true);
dtpString &strPadThis(dtpString &subject, size_t count, char chPad = ' ', bool fromRight = true);
dtpString strPadLeft(const dtpString &value, size_t count, char chPad = ' ');
dtpString &strPadLeftThis(dtpString &subject, size_t count, char chPad = ' ');
dtpString strPadRight(const dtpString &value, size_t count, char chPad = ' ');
dtpString &strPadRightThis(dtpString &subject, size_t count, char chPad = ' ');
// trim
dtpString strTrimLeft(const dtpString &src);
dtpString strTrimRight(const dtpString &src);
dtpString strTrim(const dtpString &src);
dtpString &strTrimThis(dtpString &subject);
dtpString &strTrimLeftThis(dtpString &subject);
dtpString &strTrimRightThis(dtpString &subject);
dtpString strTrim(const dtpString &src, const dtpString &toTrim, bool fromRight, bool fromLeft);
dtpString &strTrimThis(dtpString &subject, const dtpString &toTrim, bool fromRight, bool fromLeft);
dtpString strToUpper(const dtpString &str);

template < class T >
dtpString toString(const T &arg)
{
  std::ostringstream out;
  out << arg;
  return(out.str());
}

/// Re-usable version of toString
namespace dtp {
class StringBuilder {
public:

  template < class T >
  void write(const T &arg)
  {
    m_output << arg;
  }

  template < class T >
  void rewrite(const T &arg)
  {
    m_output.str("");
    m_output << arg;
  }

  dtpString get() { 
    return m_output.str(); 
  }

  void get(dtpString &output) { 
    output = m_output.str(); 
  }

  void set(const dtpString &value) {
    m_output.str(value);
  }

  void clear() { 
    m_output.str(""); 
  }

private:
  std::ostringstream m_output;
};

} // namespace dtp

dtpString toString(uint arg);
dtpString toString(int arg);
dtpString toString(double arg);
dtpString toString(float arg);

template < class T >
dtpString &toString(const T &arg, dtpString &output)
{
  std::ostringstream	out;
  out << arg;
  output = dtpString(out.str());
  return output;
}

template < class T >
dtpString toStringPrec(const T &arg, int prec)
{
  using namespace std;
  std::ostringstream	out;
  out.setf(ios::fixed, ios::floatfield);
  out.setf(ios::showpoint);
  out << setprecision(prec) << arg;
  return(out.str());
}

template < class T >
dtpString toHexString(const T &arg)
{
  std::ostringstream	out;
  out << std::hex << arg;
  return(out.str());
}

template <typename T>
T fromString(const dtpString &str) {
    std::istringstream is(str);
    T t;
    is >> t;
    return t;
}

template <typename T>
T fromStringDef(const dtpString &str, const T &defValue) {
    std::istringstream is(str);
    T res;
    if (!(is >> res))
      res = defValue;
    return res;
}


int stringToIntDef(const dtpString &value, int defValue = 0);
int stringToInt(const dtpString &value);
unsigned int stringToUInt(const dtpString &value);
uint stringToUIntDef(const dtpString &value, uint defValue = 0);
int64 stringToInt64(const dtpString &value);
uint64 stringToUInt64(const dtpString &value);
int64 stringToInt64Def(const dtpString &value, int64 defValue);
uint64 stringToUInt64Def(const dtpString &value, uint64 defValue);

float stringToFloat(const dtpString &value);
double stringToDouble(const dtpString &value);
xdouble stringToXDouble(const dtpString &value);

const char *stringToCharPtr(const dtpString &a);
const std::string stringToStdString(const dtpString &a);
const dtpString stdStringToString(const std::string &a);

bool isDigit(char c);
dtpString formatNumberInBase(uint64 value, long base);

scString fillTemplateValues(const scString &tpl, const scDataNode &values);
void fillTemplateValues(const scString &tpl, const scDataNode &values, scString &output);

#endif // _DTPSTRING_H__
