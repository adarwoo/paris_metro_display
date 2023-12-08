#include "stdafx.h"
#include <stdint.h>
#include "logger.h"
#include "lib/tz.h"

static uint32_t timeoffset = 0;


extern "C"
{
   #include "calendar.h"
   #include <stdio.h>

	//! Unix epoch year
   #define TZ_EPOCH_YEAR 1970

	//! Number of seconds in an hour
   #define SECS_PER_HOUR 3600UL

	//! Number of seconds in a minute
   #define SECS_PER_MINUTE 60UL

	//! Number of days in a specified month. Index 1 for leap year, else 0.
	const uint8_t month[2][12] = {
		{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
		{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
	};

   void rtc_init(void)
   {
      timeoffset = 0;
   }

   // TODO -> Allow override for simulation
   uint32_t rtc_get_time(void)
   {
      // the system time
      SYSTEMTIME systemTime;
      GetSystemTime(&systemTime);

      // the current file time
      FILETIME fileTime;
      SystemTimeToFileTime(&systemTime, &fileTime);

      // filetime in 100 nanosecond resolution
      ULONGLONG fileTimeNano100;
      fileTimeNano100 = (((ULONGLONG)fileTime.dwHighDateTime) << 32) + fileTime.dwLowDateTime;

      //to milliseconds and unix windows epoche offset removed
      uint32_t posixTime = static_cast<uint32_t>(fileTimeNano100 / 10000000 - 11644473600);

      return posixTime + timeoffset;
   }

   /**
	* \internal
	* \brief Check if a year is a leap year
	*
	* Returns true or false depending if the input year is a leap year or not.
	*
	* \param year the year in format YYYY to check for leap year or not
	*
	* \retval true  if the year is a leap year
	* \retval false if the year is not a leap year
	*/
	static bool calendar_leapyear(uint16_t year)
	{
		if (!((year) % 4) && (((year) % 100) || !((year) % 400))) {
			return true;
		}
		else {
			return false;
		}
	}

	/**
	* \internal
	* \brief Find number of days in a year
	*
	* Returns the number of days in a year, depending if the input is leap year
	* or not.
	*
	* \param year the year in format YYYY to check number of days
	*
	* \retval 366 if the year is a leap year
	* \retval 365 if the year is not a leap year
	*/
	static uint16_t calendar_yearsize(uint16_t year)
	{
		if (calendar_leapyear(year)) {
			return 366;
		}
		else {
			return 365;
		}
	}

	/**
	* \internal
	* \brief Add a year to a date
	*
	* Adds one year to specified date as long as the current year is before 2105.
	*
	* \param *date the date to add a year to
	*
	*/
	static void calendar_add_year_to_date(struct calendar_date *date)
	{
		if (date->year < 2105) {
			date->year++;
		}
	}

	/**
	* \internal
	* \brief Add a month to a date
	*
	* Adds one month to specified date. If month is december, increment year.
	*
	* \param *date the date to add a month to
	*
	*/
	static void calendar_add_month_to_date(struct calendar_date *date)
	{
		uint8_t months = date->month;
		months++;
		if (months == 12) {
			months = 0;
			calendar_add_year_to_date(date);
		}
		date->month = months;
	}

	/**
	* \internal
	* \brief Add a day to a date
	*
	* Adds one day to specified date. If day is the last of the month, increment
	* month.
	*
	* \param *date the date to add a day to
	*
	*/
	static void calendar_add_day_to_date(struct calendar_date *date)
	{
		uint8_t dates = date->date;
		uint8_t months = date->month;
		uint16_t year = date->year;

		dates++;
		if (dates == month[calendar_leapyear(year)][months]) {
			dates = 0;
			calendar_add_month_to_date(date);
		}
		date->dayofweek++;
		if (date->dayofweek == 7) {
			date->dayofweek = 0;
		}
		date->date = dates;
	}

	/**
	* \internal
	* \brief Add an hour to a date
	*
	* Adds one hour to specified date. If hour is 23, increment day.
	*
	* \param *date the date to add an hour to
	*
	*/
	static void calendar_add_hour_to_date(struct calendar_date *date)
	{
		int8_t hour = date->hour;
		hour++;
		if (hour == 24) {
			hour = 0;
			calendar_add_day_to_date(date);
		}
		date->hour = hour;
	}

	/**
	* \internal
	* \brief Add a minute to a date
	*
	* Adds one minute to specified date. If minute is 59, increment hour.
	*
	* \param *date the date to add a minute to
	*
	*/
	static void calendar_add_minute_to_date(struct calendar_date *date)
	{
		uint8_t minute = date->minute;
		minute++;
		if (minute == 60) {
			minute = 0;
			calendar_add_hour_to_date(date);
		}
		date->minute = minute;
	}

	/**
	* \brief Check if a date is valid
	*
	* Checks that number of seconds, minutes and hours is a valid value.
	* Checks that number of days does not exceed number of days in current month.
	* Checks that number of months is a valid value, and checks that year is
	* between 1970 (epoch year) and 2106 (overflow year).
	*
	* \param *date the date to check if valid
	*
	*/
	bool calendar_is_date_valid(struct calendar_date *date)
	{
		// Make sure time is valid
		if ((date->second >= 60) || (date->minute >= 60) || (date->hour >= 24)) {
			return false;
		}

		// Make sure month and date is valid
		if ((date->month >= 12) || (date->date >= 31)) {
			return false;
		}

		// Make sure days in month are not more than it should be
		if (date->date >= month[calendar_leapyear(date->year)][date->month]) {
			return false;
		}

		// Make sure year is not earlier than 1970 and before 2106
		if ((date->year < TZ_EPOCH_YEAR) || (date->year >= 2106)) {
			return false;
		}
		else {
			return true;
		}
	}

	/**
	* \brief Convert a UNIX timestamp to a date
	*
	* Finds the corresponding date and time for a UNIX timestamp.
	*
	* \param timestamp  UNIX timestamp
	* \param date_out   Date to store result
	*
	*/
	void calendar_timestamp_to_date(uint32_t timestamp,
	struct calendar_date *date_out)
	{
		uint32_t day_number;
		uint32_t day_clock;

		date_out->year = TZ_EPOCH_YEAR;
		date_out->month = 0;

		day_clock = timestamp % TZ_SECS_PER_DAY;
		day_number = timestamp / TZ_SECS_PER_DAY;

		date_out->second = day_clock % SECS_PER_MINUTE;
		date_out->minute = static_cast<uint8_t>((day_clock % SECS_PER_HOUR) / SECS_PER_MINUTE);
		date_out->hour = static_cast<uint8_t>(day_clock / SECS_PER_HOUR);
		date_out->dayofweek = (day_number + 4) % 7;

		while (day_number >= calendar_yearsize(date_out->year)) {
			day_number -= calendar_yearsize(date_out->year);
			date_out->year++;
		}

		while (day_number >=
			month[calendar_leapyear(date_out->year)][date_out->month]) {
			day_number -= month[calendar_leapyear(date_out->year)][date_out->month];
			date_out->month++;
		}
		date_out->date = day_number;
	}

	/**
	* \brief Convert a date to a UNIX timestamp.
	*
	* \note
	* If date is invalid, timestamp 0 will be returned.
	*
	* \param date  Date
	*
	* \return The corresponding UNIX timestamp
	* \retval 0 if date is not valid
	*/
	uint32_t calendar_date_to_timestamp(struct calendar_date *date)
	{

		// Make sure date is valid
		if (!calendar_is_date_valid(date))
			return 0;

		uint32_t timestamp = 0;
		uint8_t date_month;
		uint16_t date_year;

		date_month = date->month;
		date_year = date->year;

		// Add number of seconds elapsed in current month
		timestamp += (date->date * TZ_SECS_PER_DAY) + (date->hour * SECS_PER_HOUR) +
			(date->minute * SECS_PER_MINUTE) + date->second;

		while (date_month != 0) {
			date_month--;
			// Add number of seconds in months of current year
			timestamp += month[calendar_leapyear(date_year)][date_month]
				* TZ_SECS_PER_DAY;
		}
		while (date_year > TZ_EPOCH_YEAR) {
			date_year--;
			// Add number of seconds in all years since epoch year
			timestamp += calendar_yearsize(date_year) * TZ_SECS_PER_DAY;
		}

		return timestamp;
	}

}

//
// For simulation
//
#include "lib/tz.h"

// Compute a seconds offset from the time now and store
void rtc_force_timedate(calendar_date &NewDateTime)
{
   // Convert the date to epoch seconds
   uint32_t epochOfGivenTimer = calendar_date_to_timestamp(&NewDateTime);

   // Adjust the offset
   timeoffset = epochOfGivenTimer - rtc_get_time();

   tz_datetime_t localAdjustedTime;
   tz_datetime_t *pNow = tz_now(&localAdjustedTime);

   if (!pNow)
   {
      LOG_WARN("RTC", "Local time is now invalid!");
   }

   LOG_MILE("RTC", "Local time is now: %.4d/%.2d/%.2d %.2d:%.2d:%.2d ",
      localAdjustedTime.year, localAdjustedTime.month + 1, localAdjustedTime.date + 1,
      localAdjustedTime.hour, localAdjustedTime.minute, localAdjustedTime.second);
}
