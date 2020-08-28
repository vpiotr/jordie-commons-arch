/////////////////////////////////////////////////////////////////////////////
// Name:        string.cpp
// Project:     dtpLib
// Purpose:     String type & manipulation functions
// Author:      Piotr Likus
// Modified by:
// Created:     25/04/2012
/////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <cassert>
#include <cstdio>

#include "base/string.h"

#ifndef BASE_CPP11
#define snprintf _snprintf
#endif

// scChar is a type of item inside dtpString
typedef char dtpChar;

// ----------------------------------------------------------------------------
// dtpString
// ----------------------------------------------------------------------------

dtpString strCharToStr(char c) {
  dtpString res(1, c);
  return res;
}

dtpString strRepeat(size_t count, const dtpString &pattern)
{
  dtpString res;
  for(size_t i=0, epos = count; i != epos; i++)
    res += pattern;
  return res;
}

size_t strReplaceThis(dtpString &subject, const dtpString &oldStr, const dtpString &newStr, bool replaceAll)
{
  size_t res = 0;

  dtpString::size_type pos = 0;
  dtpString::size_type oldSize = oldStr.length();
  dtpString::size_type moveBy = newStr.length();

  while ( (pos = subject.find(oldStr, pos)) != dtpString::npos ) {
      subject.replace( pos, oldSize, newStr );
      pos += moveBy;
      res++;
      if (!replaceAll)
        break;
  }

  return res;
}

dtpString strReplace(const dtpString &src, const dtpString &oldStr, const dtpString &newStr, bool replaceAll)
{
  dtpString res(src);
  strReplaceThis(res, oldStr, newStr, replaceAll);
  return res;
}

dtpString strPad(const dtpString &value, size_t count, char chPad, bool fromRight)
{
  dtpString res = value;
  dtpString::size_type len = res.length();
  if (len < count) {
    if (fromRight)
      res += dtpString(count - len, chPad);
    else
      res = dtpString(count - len, chPad) + res;
  }
  return res;
}

dtpString &strPadThis(dtpString &subject, size_t count, char chPad, bool fromRight)
{
  dtpString::size_type len = subject.length();
  if (len < count) {
    if (fromRight)
      subject += dtpString(count - len, chPad);
    else
      subject = dtpString(count - len, chPad) + subject;
  }
  return subject;
}

dtpString strPadLeft(const dtpString &value, size_t count, char chPad)
{
  dtpString res = value;
  dtpString::size_type len = res.length();
  if (len < count) {
    res = dtpString(count - len, chPad) + res;
  }
  return res;
}

dtpString &strPadLeftThis(dtpString &subject, size_t count, char chPad)
{
  dtpString::size_type len = subject.length();
  if (len < count) {
    subject = dtpString(count - len, chPad) + subject;
  }
  return subject;
}

dtpString strPadRight(const dtpString &value, size_t count, char chPad)
{
  dtpString res = value;
  dtpString::size_type len = res.length();
  if (len < count) {
    res += dtpString(count - len, chPad);
  }
  return res;
}

dtpString &strPadRightThis(dtpString &subject, size_t count, char chPad)
{
  dtpString::size_type len = subject.length();
  if (len < count) {
    subject += dtpString(count - len, chPad);
  }
  return subject;
}

dtpString strTrimLeft(const dtpString &src) {
  if(src.empty())
    return src;

  dtpString::size_type pos = src.find_first_not_of(" \t\n\r");
  if (pos != dtpString::npos)
    return src.substr(pos);
  else
    return dtpString();
}

dtpString strTrimRight(const dtpString &src) {
  if(src.empty())
    return src;

  dtpString::size_type pos = src.find_last_not_of(" \t\n\r");
  if (pos != dtpString::npos)
    return src.substr(0, pos + 1);
  else
    return dtpString();
}

dtpString strTrim(const dtpString &src) {
  if(src.empty())
    return src;

  const dtpString toTrim = " \t\n\r";
  dtpString::size_type headPos = src.find_first_not_of(toTrim);
  dtpString::size_type tailPos = src.find_last_not_of(toTrim);

  if (headPos != dtpString::npos) {
    return src.substr(headPos, tailPos - headPos + 1);
  }
  else {
    return dtpString();
  }
}

dtpString &strTrimLeftThis(dtpString &subject) {
  if(subject.empty())
    return subject;

  dtpString::size_type pos = subject.find_first_not_of(" \t\n\r");
  if (pos != dtpString::npos)
    subject = subject.substr(pos);
  else
    subject = dtpString();

  return subject;
}

dtpString &strTrimRightThis(dtpString &subject) {
  if(subject.empty())
    return subject;

  dtpString::size_type pos = subject.find_last_not_of(" \t\n\r");
  if (pos != dtpString::npos)
    subject = subject.substr(0, pos + 1);
  else
    subject = dtpString();

  return subject;
}

dtpString &strTrimThis(dtpString &subject) {
  if(subject.empty())
    return subject;

  const dtpString toTrim = " \t\n\r";
  dtpString::size_type headPos = subject.find_first_not_of(toTrim);
  dtpString::size_type tailPos = subject.find_last_not_of(toTrim);

  if (headPos != dtpString::npos) {
    subject = subject.substr(headPos, tailPos - headPos + 1);
  }
  else {
    subject = dtpString();
  }

  return subject;
}

dtpString &strTrimThis(dtpString &subject, const dtpString &toTrim, bool fromRight, bool fromLeft) {
  if(subject.empty())
    return subject;

  dtpString::size_type headPos;
  dtpString::size_type tailPos;

  if (fromLeft) {
     headPos = subject.find_first_not_of(toTrim);
     if (headPos == dtpString::npos)
       headPos = subject.length();
  }
  else
     headPos = 0;

  if (fromRight) {
     tailPos = subject.find_last_not_of(toTrim);
     if (tailPos == dtpString::npos)
       tailPos = headPos;
     else
       tailPos++;
  }
  else
     tailPos = subject.length();

  subject = subject.substr(headPos, tailPos - headPos);
  return subject;
}

dtpString strTrim(const dtpString &src, const dtpString &toTrim, bool fromRight, bool fromLeft) {
  dtpString res(src);
  strTrimThis(res, toTrim, fromRight, fromLeft);
  return res;
}

dtpString strToUpper(const dtpString &str)
{
  std::string upList;
  upList = str.c_str();
  std::transform(upList.begin(), upList.end(), upList.begin(), toupper);
  return upList.c_str();
}

const char *stringToCharPtr(const dtpString &a)
{
  //return a.char_str();
  return reinterpret_cast<char *>(const_cast<dtpChar *>((a.c_str())));
}

const std::string stringToStdString(const dtpString &a)
{
  return std::string(stringToCharPtr(a));
}

const dtpString stdStringToString(const std::string &a)
{
  return dtpString(a.c_str());
}

int stringToIntDef(const dtpString &value, int defValue)
{
  int res;
  std::string text = value;
  std::istringstream ss( text );
  if (!(ss >> res))
    res = defValue;
  return res;
}

int stringToInt(const dtpString &value)
{
  return atoi(stringToCharPtr(value));
}

unsigned int stringToUInt(const dtpString &value)
{
  return static_cast<uint>(strtoul(stringToCharPtr(value), NULL, 10));
}

uint stringToUIntDef(const dtpString &value, uint defValue)
{
  unsigned int res;
  std::string text = value;
  std::istringstream ss( text );
  if (!(ss >> res))
    res = defValue;
  return res;
}

int64 stringToInt64(const dtpString &value)
{
  int64 res;
  std::string text = value;
  std::istringstream ss( text );
  if (!(ss >> res))
    throw dtp::dtpBadConversion(std::string("stringToInt64("+value+")"));
  return res;
}

uint64 stringToUInt64(const dtpString &value)
{
  uint64 res;
  std::string text = value;
  std::istringstream ss( text );
  if (!(ss >> res))
    throw dtp::dtpBadConversion(std::string("stringToUInt64("+value+")"));
  return res;
}

int64 stringToInt64Def(const dtpString &value, int64 defValue)
{
  int64 res;
  std::string text = value;
  std::istringstream ss( text );
  if (!(ss >> res))
    res = defValue;
  return res;
}

uint64 stringToUInt64Def(const dtpString &value, uint64 defValue)
{
  uint64 res;
  std::string text = value;
  std::istringstream ss( text );
  if (!(ss >> res))
    res = defValue;
  return res;
}

float stringToFloat(const dtpString &value)
{
  float res;
  errno = 0;
  res = static_cast<float>(strtod(stringToCharPtr(value), NULL));
  if(errno == ERANGE)
    throw dtp::dtpBadConversion(std::string("stringToFloat("+value+")"));
  return res;
}

double stringToDouble(const dtpString &value)
{
  double res;
  errno = 0;
  res = strtod(stringToCharPtr(value), NULL);
  if(errno == ERANGE)
    throw dtp::dtpBadConversion(std::string("stringToDouble("+value+")"));
  return res;
}

xdouble stringToXDouble(const dtpString &value)
{
  xdouble res;
  std::string text = value;
  std::istringstream ss( text );
  if (!(ss >> res))
    throw dtp::dtpBadConversion(std::string("stringToXDouble("+value+")"));
  return res;
}

bool isDigit(char c)
{
  if ((c >= '0') && (c <= '9'))
    return true;
  else
    return false;
}

dtpString formatNumberInBase(uint64 value, long base)
{
	const char *digits = "0123456789abcdef";
        uint64 workVal = value;
	dtpString res;
	size_t index;
	assert((base >= 2) && (base <= 16));
	do {
		index = workVal % base;
		res = digits[index] + res;
		workVal /= base;
	}	while(workVal);

	return res;
}

dtpString toString(uint arg)
{
  const int BUFFER_SIZE = 30;
  char buffer[BUFFER_SIZE];
  snprintf(buffer, BUFFER_SIZE, "%u", arg);
  return buffer;
}

dtpString toString(int arg)
{
  const int BUFFER_SIZE = 30;
  char buffer[BUFFER_SIZE];
  snprintf(buffer, BUFFER_SIZE, "%d", arg);
  return buffer;
}

dtpString toString(double arg)
{
  const int BUFFER_SIZE = 30;
  char buffer[BUFFER_SIZE];
  snprintf(buffer, BUFFER_SIZE, "%f", arg);
  return buffer;
}

dtpString toString(float arg)
{
  const int BUFFER_SIZE = 30;
  char buffer[BUFFER_SIZE];
  snprintf(buffer, BUFFER_SIZE, "%f", arg);
  return buffer;
}

scString fillTemplateValues(const scString &tpl, const scDataNode &values)
{
  scString res;
  fillTemplateValues(tpl, values, res);
  return res;
}

void fillTemplateValues(const scString &tpl, const scDataNode &values, scString &output)
{
  scDataNode element;
  output = tpl;
  for(int i=0,epos=values.size();i!=epos;i++)
  {
    values.getElement(i, element);
    strReplaceThis(output, "{"+values.getElementName(i)+"}", element.getAsString(), true);
  }
}


