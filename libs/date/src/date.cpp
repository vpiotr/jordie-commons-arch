/////////////////////////////////////////////////////////////////////////////
// Name:        date.cpp
// Project:     dtpLib
// Purpose:     Date functions
// Author:      Piotr Likus
// Modified by:
// Created:     20/05/2012
/////////////////////////////////////////////////////////////////////////////

//#include <sys/timeb.h>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "base/date.h"
#include "base/string.h"
#include "base/bmath.h"

namespace dtp {

// YYYY-MM-DD HH:NN:SS to double datetime since 1900-01-01
fdatetime_t isoStrToDateTime(const dtpString &value)
{
  if ((value.empty()) || (value == "0"))
    return 0.0;
  else {
    uint year = stringToUInt(value.substr(0,4));
    uint month = stringToUInt(value.substr(5,2));
    uint day = stringToUInt(value.substr(8,2));
    uint hour = stringToUInt(value.substr(11,2));
    uint minute = stringToUInt(value.substr(14,2));
    uint secs = stringToUInt(value.substr(17,2));

    struct std::tm in_time;

    in_time.tm_hour = hour;
    in_time.tm_min = minute;
    in_time.tm_sec = secs;
    in_time.tm_year = year-1900;
    in_time.tm_mon = month-1;
    in_time.tm_mday = day;

    std::time_t y = std::mktime(&in_time);
    fdatetime_t res;

    if ( y != (std::time_t)(-1) )
    {
      res = y / (60.0 * 60.0 * 24.0);
    } else {
      res = 0.0;
    }
    return res;
  }
}

// YYYY-MM-DD HH:NN:SS
dtpString dateTimeToIsoStr(fdatetime_t value)
{
  if (isNullDateTime(value))
    return "";
  else {
    using std::setfill;
    using std::setw;

    uint y,m,d,h,min,sec;
    decodeDateTime(value, y,m,d,h,min,sec);

    std::ostringstream	out;
    out << setfill('0')
	<< setw(4) << y << "-"
	<< setw(2) << m  << "-"
	<< setw(2) << d << " "
	<< setw(2) << h << ":"
	<< setw(2) << min << ":"
	<< setw(2) << sec;

    return(out.str());
  }
}

// YYYY-MM-DD
dtpString dateToIsoStr(fdatetime_t value)
{
  if (isNullDateTime(value))
    return "";
  else {
    using std::setfill;
    using std::setw;

    uint y,m,d,h,min,sec;
    decodeDateTime(value, y,m,d,h,min,sec);

    std::ostringstream	out;
    out << setfill('0')
	<< setw(4) << y << "-"
	<< setw(2) << m  << "-"
	<< setw(2) << d;

    return(out.str());
  }
}

dtpString timeToIsoStr(fdatetime_t value)
{
  if (isNullDateTime(value))
    return "";
  else {
    using std::setfill;
    using std::setw;

    uint y,m,d,h,min,sec;
    decodeDateTime(value, y,m,d,h,min,sec);

    std::ostringstream	out;
    out << setfill('0')
	<< setw(2) << h << ":"
	<< setw(2) << min << ":"
	<< setw(2) << sec;

    return(out.str());
  }
}

dtpString dateTimeToNoSepStr(fdatetime_t value)
{
  uint oyear, omon, oday, ohour, omin, osecs;
  dtpString res, datePart;

  decodeDateTime(value, oyear, omon, oday, ohour, omin, osecs);

  res = toString(oyear);
  datePart = strPadLeft(toString(omon), 2, '0');
  res += datePart;
  datePart = strPadLeft(toString(oday), 2, '0');
  res += datePart;
  datePart = strPadLeft(toString(ohour), 2, '0');
  res += datePart;
  datePart = strPadLeft(toString(omin), 2, '0');
  res += datePart;
  datePart = strPadLeft(toString(osecs), 2, '0');
  res += datePart;

  return res;
}

dtpString dateToNoSepStr(fdatetime_t value)
{
  uint oyear, omon, oday, ohour, omin, osecs;
  dtpString res, datePart;

  decodeDateTime(value, oyear, omon, oday, ohour, omin, osecs);

  res = toString(oyear);
  datePart = strPadLeft(toString(omon), 2);
  res += datePart;
  datePart = strPadLeft(toString(oday), 2);
  res += datePart;
  return res;
}

bool encodeDateTime(uint iyear, uint imon, uint iday, uint ihour, uint imin, uint isecs, fdatetime_t &ovalue)
{
  bool res;
  struct std::tm in_time;

  in_time.tm_hour = ihour;
  in_time.tm_min = imin;
  in_time.tm_sec = isecs;
  in_time.tm_year = iyear-1900;
  in_time.tm_mon = imon-1;
  in_time.tm_mday = iday;

  in_time.tm_isdst = 0;

#ifdef DTP_COMP_VS
  std::time_t y = _mkgmtime(&in_time);
#else
  time_t y0 = mktime ( &in_time );
  struct std::tm *y1 = gmtime(&y0);
  std::time_t y = mktime ( y1 );
#endif

  if ( y != (std::time_t)(-1) )
  {
    ovalue = y / (60.0 * 60.0 * 24.0);
    res = true;
  } else {
    ovalue = 0.0;
    res = false;
  }
  return res;
}

bool decodeDateTime(fdatetime_t value, uint &oyear, uint &omon, uint &oday, uint &ohour, uint &omin, uint &osecs)
{
  std::time_t x = round<std::time_t>(value * (60.0 * 60.0 * 24.0));
  tm * ptm;
  ptm = gmtime ( &x );

  oyear = ptm->tm_year+1900;
  omon = ptm->tm_mon+1;
  oday = ptm->tm_mday;
  ohour = ptm->tm_hour;
  omin = ptm->tm_min;
  osecs = ptm->tm_sec;

  return true;
}

fdatetime_t dateTimeDiff(fdatetime_t endValue, fdatetime_t startValue)
{
  return endValue - startValue;
}

fdatetime_t dateTimeToDate(fdatetime_t value)
{
  return static_cast<fdatetime_t>(static_cast<uint64>(value));
}

fdatetime_t dateTimeToTime(fdatetime_t value)
{
  double diff;
  diff = value - dtp::dateTimeToDate(value);
  return static_cast<fdatetime_t>(diff);
}

fdatetime_t currentDateTime()
{
  std::time_t x;
  time(&x);

  fdatetime_t res;
//-- UTC -> local time correction
  struct tm *loctimp;
#ifdef DTP_COMP_VS
  struct tm loctim;
  localtime_s(&loctim, &x);
  loctimp = &loctim;
#else
  loctimp = localtime(&x);
#endif
  dtp::encodeDateTime(1900+loctimp->tm_year, 1+loctimp->tm_mon, loctimp->tm_mday,
    loctimp->tm_hour, loctimp->tm_min, loctimp->tm_sec, res);
//--

  return res;
}

fdatetime_t currentDate()
{
  return dtp::dateTimeToDate(dtp::currentDateTime());
}

fdatetime_t currentTime()
{
  return dtp::dateTimeToTime(dtp::currentDateTime());
}

uint64 currentTimeInSecs()
{
  std::time_t x;
  time(&x);
  return x;
}

uint64 dateTimeToMSecs(fdatetime_t value)
{
  uint64 res = uint64(double((value * (60.0 * 60.0 * 24.0))*1000.0));
  return res;
}

fdatetime_t msecsToDateTime(uint64 value)
{
  fdatetime_t res = double(value) / (60.0 * 60.0 * 24.0 * 1000.0);
  return res;
}

bool isNullDateTime(fdatetime_t value)
{
  return (dtp::dateTimeToMSecs(value) == 0);
}

cpu_ticks dateTimeToTicks(scDateTime value)
{
  ulong64 res = ulong64(double((value * (60.0 * 60.0 * 24.0))*1000.0));
  return res;
}

scDateTime ticksToDateTime(cpu_ticks value)
{
  scDateTime res = double(value) / (60.0 * 60.0 * 24.0 * 1000.0);
  return res;
}



} // namespace
