#ifndef DATECOUNT_H
#define DATECOUNT_H

#define SUNDAY 0
#define MONDAY 1
#define TUESDAY 2
#define WEDNESDAY 3
#define THURSDAY 4
#define FRIDAY 5
#define SATURDAY 6

unsigned int get_weekCount(unsigned int date);
unsigned int get_weekDay(unsigned int date);
unsigned int get_year(unsigned int date);
unsigned int get_month(unsigned int date);
unsigned int get_day(unsigned int date);

unsigned int get_weekCount_from_weekData(unsigned int date); // If type of db is week, form of date is different from the other types

unsigned int get_year_for_weekCount(unsigned int date);
unsigned int get_month_for_weekCount(unsigned int date);

int compare_date(unsigned int date1, unsigned int date2);

unsigned int make_date(unsigned int year, unsigned int month, unsigned int day);

#endif // DATECOUNT_H
