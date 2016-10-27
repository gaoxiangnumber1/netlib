#include <base/date.h>

#include <assert.h>
#include <stdio.h> // printf()
#include <time.h>

using netlib::Date;

const int kMonthsOfYear = 12;

int IsLeapYear(int year) // Return 0 if not a leap year, 1 if leap year.
{
	if (year % 400 == 0)
	{
		return 1;
	}
	else if (year % 100 == 0)
	{
		return 0;
	}
	else if (year % 4 == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int DaysOfMonth(int year, int month) // Return the days of year-month
{
	static int days[2][kMonthsOfYear + 1] =
	{
		{0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
		{0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
	};
	return days[IsLeapYear(year)][month];
}

void PassByConstReference(const Date &date)
{
	printf("%s\n", date.ToIsoString().c_str());
}

void PassByValue(Date date)
{
	printf("%s\n", date.ToIsoString().c_str());
}

int main()
{
	Date some_day(1996, 8, 2);
	printf("%s\n", some_day.ToIsoString().c_str());
	PassByValue(some_day);
	PassByConstReference(some_day);

	// returns the time as the number of seconds since the Epoch,
	// 1970-01-01 00:00:00 +0000 (UTC).
	time_t now = time(NULL);
	struct tm time1 = *gmtime(&now);
	struct tm time2 = *localtime(&now);
	Date today_utc(time1);
	printf("%s\n", today_utc.ToIsoString().c_str());
	Date today_local(time2);
	printf("%s\n", today_local.ToIsoString().c_str());

	int julian_day_number = 2415021;
	int week_day = 1; // Monday

	for(int year = 1900; year < 2500; ++year)
	{
		assert(Date(year, 3, 1).get_julian_day_number() - Date(year, 2, 29).get_julian_day_number()
		       == IsLeapYear(year));
		for(int month = 1; month <= kMonthsOfYear; ++month)
		{
			for(int day = 1; day <= DaysOfMonth(year, month); ++day)
			{
				Date date1(year, month, day);
				assert(year == date1.GetYear());
				assert(month == date1.GetMonth());
				assert(day == date1.GetDay());
				assert(week_day == date1.GetWeekDay());
				assert(julian_day_number == date1.get_julian_day_number());

				Date date2(julian_day_number);
				assert(year == date2.GetYear());
				assert(month == date2.GetMonth());
				assert(day == date2.GetDay());
				assert(week_day == date2.GetWeekDay());
				assert(julian_day_number == date2.get_julian_day_number());

				++julian_day_number;
				week_day = (week_day + 1) % 7;
			}
		}
	}
	printf("All passed.\n");
}
