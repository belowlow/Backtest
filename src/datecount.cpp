#include "../include/datecount.h"
#include <iostream>

using namespace std;

/* Last day of month */
int lastDay_default[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
int lastDay_forth[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; // forth year's month 2 has 29 day

int total_default = 365; // total sum of days in 1 year
int total_forth = 366; // total sum of day in 1 year of forth year

/* Date on which calculation is based */
unsigned int std_date = 20150101;
unsigned int std_weekday = THURSDAY;
unsigned int std_year = 2015;
unsigned int std_month = 1;
unsigned int std_day = 1;

unsigned int get_year(unsigned int date){
    return date / 10000;
}

unsigned int get_month(unsigned int date){
    return (date % 10000) / 100;
}

unsigned int get_day(unsigned int date){
    return (date % 10000 % 100);
}

unsigned int get_weekDay(unsigned int date){
    /* Get the difference between date and std_date, then divide 7 for acquaring week day of date */
    int diff_day = 0;

    int year = get_year(date);
    int month = get_month(date);

    if (year == 2015){
        /* e.g) date : 2015.06.25, add days until 2015.5.31 */
        for(int i = 0; i < month - 1; i++){
            diff_day += lastDay_default[i];
        }
        diff_day -= 1; // Except 2015.01.01

        /* e.g) date : 2015.06.25, add 25 */
        diff_day += get_day(date);
    }
    else{
        if (year % 4 == 0){
            /* If year of date has 29 days at month 2*/
            for (int i = 0; i < month - 1; i++){
                diff_day += lastDay_forth[i];
            }
        }
        else{
            /* Default year */
            for(int i = 0; i < month - 1; i++){
                diff_day += lastDay_default[i];
            }
        }

        int diff_year = year - std_year;
        int forthCount = diff_year % 4 >= 2 ? diff_year / 4 + 1 : diff_year / 4; // Count of forth year between date and 2015

        diff_day += (diff_year - forthCount) * total_default + forthCount * total_forth;
        diff_day -= 1; // Except 2015.01.01

        diff_day += get_day(date);
    }

    unsigned int weekDay = (diff_day % 7 + std_weekday) % 7;
    return weekDay;
}

unsigned int get_weekCount(unsigned int date){
    unsigned int weekDay = get_weekDay(date);
    unsigned int day = get_day(date);

    /* To get the sunday's date */
    unsigned int diff_weekDay = weekDay - SUNDAY;
    int sunday = day - diff_weekDay;

    if (day < 1 + diff_weekDay){ // It means that sunday < 1
        /* For example, if date is 20210101, year is 2021 and month is 01 and day is 01.
         * But in terms of week data(week type db), the date 20210101 is 4th week of month 12 of year 2020.
         * Because 20210101 is Friday and this week's sunday is 20201227.
         * So week db says that the date 20210101 is in the 20201240 of week db's date.
         * It means that 20210101's year_for_weekCount is 2020, month_for_weekCount is 12, weekCount is 4
         * cf) year can be different with year_for_weekCount. month can be different with month_for_weekCount */
        unsigned int year = get_year(date);
        unsigned int month = get_month(date);
        unsigned int month_sunday = month == 1 ? 12 : month - 1;
        unsigned int year_sunday = month == 1 ? year - 1 : year;
        month_sunday -= 1; // month data start at 0. e.g month is 1, than make it 0

        unsigned int lastDay;
        if (month_sunday == 0 || month_sunday == 2 || month_sunday == 4 || month_sunday == 6 || month_sunday == 7 ||
                month_sunday == 9 || month_sunday == 11){
            lastDay = 31;
        }
        else if (month_sunday == 3 || month_sunday == 5 || month_sunday == 8 || month_sunday == 10){
            lastDay = 30;
        }
        else if (year % 4 == 0){
            lastDay = 29;
        }
        else{
            lastDay = 28;
        }

        unsigned int date_sunday = make_date(year_sunday, month_sunday + 1, lastDay);
        weekDay = get_weekDay(date_sunday);
        diff_weekDay = weekDay - SUNDAY;
        sunday = lastDay - diff_weekDay;
    }

    if (sunday % 7 == 0){
        return sunday / 7;
    }
    else{
        return sunday / 7 + 1;
    }
}

unsigned int get_weekCount_from_weekData(unsigned int date){
    /* If date is 20201150, it means that date is 5th week of month 11 */
    /* So change it to 5 */
    return (date % 10000 % 100) / 10;
}

int compare_date(unsigned int date1, unsigned int date2){
   if (date1 > date2){
       return 1;
   }
   else if (date1 == date2){
       return 0;
   }
   else{
       return -1;
   }
}

unsigned int make_date(unsigned int year, unsigned int month, unsigned int day){
    return (year * 10000 + month * 100 + day);
}

unsigned int get_year_for_weekCount(unsigned int date){
    unsigned int weekDay = get_weekDay(date);
    unsigned int diff_weekDay = weekDay - SUNDAY;

    unsigned int year = get_year(date);
    unsigned int month = get_month(date);
    unsigned int day = get_day(date);

    if (day < 1 + diff_weekDay && month == 1){ // It means that sunday < 1 and month is 1
        year -= 1;
    }

    return year;
}

unsigned int get_month_for_weekCount(unsigned int date){
    unsigned int weekDay = get_weekDay(date);
    unsigned int diff_weekDay = weekDay - SUNDAY;

    unsigned int month = get_month(date);
    unsigned int day = get_day(date);

    unsigned int ret = 0;

    if (day < 1 + diff_weekDay){ // It means that sunday < 1
        if (month == 1){
            ret = 12;
        }
        else{
            ret = month - 1;
        }
    }
    else{
        ret = month;
    }

    return ret;
}
