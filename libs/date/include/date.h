/////////////////////////////////////////////////////////////////////////////
// Name:        date.h
// Project:     dtpLib
// Purpose:     Date-related functions
// Author:      Piotr Likus
// Modified by:
// Created:     20/05/2012
/////////////////////////////////////////////////////////////////////////////

#ifndef _DTPDATE_H__
#define _DTPDATE_H__

// ----------------------------------------------------------------------------
// Description
// ----------------------------------------------------------------------------
/// \file date.h
///
/// Date-related functions

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------
//std

//sc
//#include "dtp/defs.h"
#include "base/string.h"
#include "base/btypes.h"

namespace dtp {

typedef double fdatetime_t; // days with time as fraction of day

// --- formatting --- 

// ISO string (YYYY-MM-DD HH:NN:SS) to fdatetime_t
fdatetime_t isoStrToDateTime(const dtpString &value);
// fdatetime_t to ISO string YYYY-MM-DD HH:NN:SS
dtpString dateTimeToIsoStr(fdatetime_t value);
// fdatetime_t to ISO string YYYY-MM-DD
dtpString dateToIsoStr(fdatetime_t value);
// time to ISO string HH:NN:SS
dtpString timeToIsoStr(fdatetime_t value);
// fdatetime_t to string YYYYMMDDHHNNSS
dtpString dateTimeToNoSepStr(fdatetime_t value);
// fdatetime_t to string YYYYMMDD
dtpString dateToNoSepStr(fdatetime_t value);

// --- calculations --- 

fdatetime_t dateTimeDiff(fdatetime_t endValue, fdatetime_t startValue);
fdatetime_t dateTimeToDate(fdatetime_t value);
fdatetime_t dateTimeToTime(fdatetime_t value);
fdatetime_t currentDateTime();
fdatetime_t currentDate();
fdatetime_t currentTime();
uint64 currentTimeInSecs();
bool isNullDateTime(fdatetime_t value);

// conversions
bool encodeDateTime(uint iyear, uint imon, uint iday, uint ihour, uint imin, uint isecs, fdatetime_t &ovalue);
bool decodeDateTime(fdatetime_t value, uint &oyear, uint &omon, uint &oday, uint &ohour, uint &omin, uint &osecs);
uint64 dateTimeToMSecs(fdatetime_t value);
fdatetime_t msecsToDateTime(uint64 value);
void extractDateTimeMSecs(fdatetime_t value, uint msecs);

} // namespace

#endif // _DTPDATE_H__
