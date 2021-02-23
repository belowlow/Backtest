#include <QSqlQuery>
#include <QSqlDatabase>
#include <QString>
#include <QVariant>
#include <iostream>
#include "../include/backtrader.h"
#include "../include/datecount.h"

/* Used in run() for synchronize between database */
void move_until_catch_year(QSqlQuery& query, unsigned int targetYear);
void move_until_catch_month(QSqlQuery& query, unsigned int targetMonth);
void move_until_catch_weekCount(QSqlQuery& query, unsigned int targetweekCount, bool weekIndex = false);
void move_until_catch_day(QSqlQuery& query, unsigned int targetDay);
void move_until_catch_time(QSqlQuery& query, unsigned int targetTime);

/* Convey data of records to the ships */
void convey(DataShip* ships, QSqlQuery* query, int shipCount);

BackTrader::BackTrader(int typeCount, int typeId[])
{
    /* Set the type id and type count */
    this->typeCount = typeCount;

    this->typeId = new int[typeCount];
    path = new const char*[typeCount];

    for(int i = 0; i < typeCount; i++){
        this->typeId[i] = typeId[i];

        /* If type id is larger than type id of center db, change center db */
        if (centerIndex == -1){
            centerIndex = i; // index type 제외해야함
        }
        else{
            centerIndex = typeId[centerIndex] < typeId[i] ? i : centerIndex;
        }
    }

    ships = new DataShip[typeCount];
}

BackTrader::~BackTrader(){
    delete[] typeId;
    delete[] path;
    delete[] ships;
}

void BackTrader::set_startDate(unsigned int date){
    this->startDate = date;
}

void BackTrader::set_endDate(unsigned int date){
    this->endDate = date;
}

void BackTrader::feed(const char* tableId, const char **path){
    /* Set the table id and path of db */
    for(int i = 0; i < typeCount; i++){
        this->path[i] = path[i];
    }

    this->tableId = tableId;
}

void BackTrader::run(){
    /* Query object corresponding to database */
    QSqlQuery* query = new QSqlQuery[typeCount];
    QSqlDatabase* db = new QSqlDatabase[typeCount];

    for(int i = 0; i < typeCount; i++){
        QString dbID;
        switch (typeId[i])
        {
        case MONTH:
            dbID = QString("MONTH_%1").arg(tableId);
            monthIndex = i;
            break;
        case WEEK:
            dbID = QString("WEEK_%1").arg(tableId);
            weekIndex = i;
            break;
        case DAY:
            dbID = QString("DAY_%1").arg(tableId);
            dayIndex = i;
            break;
        case _30MIN:
            dbID = QString("_30MIN_%1").arg(tableId);
            _30MinIndex = i;
            break;
        case _5MIN:
            dbID = QString("_5MIN_%1").arg(tableId);
            _5MinIndex = i;
            break;
        case _3MIN:
            dbID = QString("_3MIN_%1").arg(tableId);
            _3MinIndex = i;
            break;
        case _1MIN:
            dbID = QString("_1MIN_%1").arg(tableId);
            _1MinIndex = i;
            break;
        }

        /* SQLite3 is used */
        db[i] = QSqlDatabase::addDatabase("QSQLITE", dbID);

        db[i].setDatabaseName(path[i]);
        db[i].open();

        query[i] = QSqlQuery(db[i]);
    }

    QString selectSQL = QString("SELECT * FROM %1").arg(tableId);

    /* Set all database to the same date */
    /* target date will be start analysing date of all databases */
    unsigned int targetYear = 0;
    unsigned int targetMonth = 0;
    unsigned int targetDay = 0;

    /* year and month in terms of week count are different from normal year and month
     * For example, if date is 20210101, year is 2021 and month is 01 and day is 01.
     * But in terms of week data(week type db), the date 20210101 is 4th week of month 12 of year 2020.
     * Because 20210101 is Friday and this week's sunday is 20201227.
     * So week db says that the date 20210101 is in the 20201240 of week db's date.
     * It means that 20210101's year_for_weekCount is 2020, month_for_weekCount is 12, weekCount is 4
     * cf) year can be different with year_for_weekCount. month can be different with month_for_weekCount */
    unsigned int targetYear_for_weekCount = 0;
    unsigned int targetMonth_for_weekCount = 0;
    unsigned int targetWeekCount = 0;

    /* The index of the db which has target date as date of first record */
    int targetIndex = -1;

    /* Check what is the target db */
    unsigned int year, month, weekCount, day, date;
    unsigned int year_for_weekCount, month_for_weekCount;
    for(int i = 0; i < typeCount; i++){
        /* Move to the first record */
        query[i].prepare(selectSQL);
        query[i].exec();
        query[i].next();

        /* Database's first date */
        date = query[i].value(0).toUInt();
        year = get_year(date);
        month = get_month(date);

        if (i == monthIndex){
            /* If month db's fisrt date is more recent than target date, change target to the db's first date */
            /* Also set this db to the target db */
            if (targetYear < year || (targetYear == year && targetMonth < month)){
                targetYear = year;
                targetMonth = month;

                /* In month db, day is unknown. So make it 0 */
                targetDay = 0;

                /* Set targetMonth, targetYear in terms of weekCount */
                /* It will be used in comparing date with week db's date */
                /* Use date + 1. It means that first day of the month */
                targetYear_for_weekCount = get_year_for_weekCount(date + 1);
                targetMonth_for_weekCount = get_month_for_weekCount(date + 1);
                targetWeekCount = get_weekCount(date + 1);

                targetIndex = i; // Set this db to the target db
            }
        }
        else if (i == weekIndex){
            year_for_weekCount = get_year(date);
            month_for_weekCount = get_month(date);
            weekCount = get_weekCount_from_weekData(date);

            /* If week db's fisrt date is more recent than target date, change target to the db's first date */
            /* Also set this db to the target db */
            /* About week db, just year and month are not used. Instead year for week count, month for week count is used */
            if (targetYear_for_weekCount < year_for_weekCount ||
                    (targetYear_for_weekCount == year_for_weekCount && targetMonth_for_weekCount < month_for_weekCount) ||
                    (targetYear_for_weekCount == year_for_weekCount && targetMonth_for_weekCount == month_for_weekCount &&
                     targetWeekCount < weekCount)){
                targetYear = year;
                targetMonth = month;

                /* In week db, day is unknown. So make it -1 */
                targetDay = 0;

                /* Set targetMonth, targetYear in terms of weekCount */
                targetYear_for_weekCount = year_for_weekCount;
                targetMonth_for_weekCount = month_for_weekCount;
                targetWeekCount = weekCount;

                targetIndex = i; // Set this db to the target db
            }
        }
        else{
            year_for_weekCount = get_year_for_weekCount(date);
            month_for_weekCount = get_month_for_weekCount(date);
            weekCount = get_weekCount(date);

            day = get_day(date);

            /* If day db's fisrt date is more recent than target date, change target to the db's first date */
            /* Also set this db to the target db */
            /* If target db is week db, than compare dates in terms of week count */
            if (targetIndex == weekIndex &&
                    (targetYear_for_weekCount < year_for_weekCount ||
                    (targetYear_for_weekCount == year_for_weekCount && targetMonth_for_weekCount < month_for_weekCount) ||
                    (targetYear_for_weekCount == year_for_weekCount && targetMonth_for_weekCount == month_for_weekCount && targetWeekCount < weekCount))){
                targetYear = year;
                targetMonth = month;
                targetDay = day;

                targetYear_for_weekCount = year_for_weekCount;
                targetMonth_for_weekCount = month_for_weekCount;
                targetWeekCount = weekCount;

                targetIndex = i; // Set this db to the target db
            }
            else if (targetYear < year ||
                    (targetYear == year && targetMonth < month) ||
                    (targetYear == year && targetMonth && month && targetDay < day)){
                targetYear = year;
                targetMonth = month;
                targetDay = day;

                targetYear_for_weekCount = year_for_weekCount;
                targetMonth_for_weekCount = month_for_weekCount;
                targetWeekCount = weekCount;

                targetIndex = i; // Set this db to the target db
            }
        }
    }


    /* If users set the start date */
    if (startDate != 0){
        unsigned int startYear = get_year(startDate);
        unsigned int startMonth = get_month(startDate);
        unsigned int startDay = get_day(startDate);

        unsigned int startYear_for_weekCount = get_year_for_weekCount(startDate);
        unsigned int startMonth_for_weekCount = get_month_for_weekCount(startDate);
        unsigned int startWeekCount= get_weekCount(startDate);

        /* If start date is more recent than target db's first date, change target to the start date */
        if (targetIndex == monthIndex){
            if (targetYear < startYear || (targetYear == startYear && targetMonth < startMonth)){
                targetYear = startYear;
                targetMonth = startMonth;
                targetDay = startDay;

                targetYear_for_weekCount = startYear_for_weekCount;
                targetMonth_for_weekCount = startMonth_for_weekCount;
                targetWeekCount = startWeekCount;

                targetIndex = -1;
            }
        }
        else if (targetIndex == weekIndex){
            if (targetYear_for_weekCount < startYear_for_weekCount ||
                    (targetYear_for_weekCount == startYear_for_weekCount && targetMonth_for_weekCount < startMonth_for_weekCount) ||
                    (targetYear_for_weekCount == startYear_for_weekCount && targetMonth_for_weekCount == startMonth_for_weekCount &&
                     targetWeekCount < startWeekCount)){
                targetYear = startYear;
                targetMonth = startMonth;
                targetDay = startDay;

                targetYear_for_weekCount = startYear_for_weekCount;
                targetMonth_for_weekCount = startMonth_for_weekCount;
                targetWeekCount = startWeekCount;

                targetIndex = -1;
            }
        }
        else{
            if (targetIndex == weekIndex &&
                    (targetYear_for_weekCount < startYear_for_weekCount ||
                    (targetYear_for_weekCount == startYear_for_weekCount && targetMonth_for_weekCount < startMonth_for_weekCount) ||
                    (targetYear_for_weekCount == startYear_for_weekCount && targetMonth_for_weekCount == startMonth_for_weekCount && targetWeekCount < startWeekCount))){
                targetYear = startYear;
                targetMonth = startMonth;
                targetDay = startDay;

                targetYear_for_weekCount = startYear_for_weekCount;
                targetMonth_for_weekCount = startMonth_for_weekCount;
                targetWeekCount = startWeekCount;

                targetIndex = -1;
            }
            else if (targetYear < startYear ||
                    (targetYear == startYear && targetMonth < startMonth) ||
                    (targetYear == startYear && targetMonth && startMonth && targetDay < startDay)){
                targetYear = startYear;
                targetMonth = startMonth;
                targetDay = startDay;

                targetYear_for_weekCount = startYear_for_weekCount;
                targetMonth_for_weekCount = startMonth_for_weekCount;
                targetWeekCount = startWeekCount;

                targetIndex = -1;
            }
        }
    }

    /* Synchronize databases about target date */
    for (int i = 0; i < typeCount; i++){
        if (i == monthIndex){
            if(weekIndex != -1 && targetIndex == weekIndex){
                /* If week db is a target db, the way of moving is different from other type of center db */
                /* Move until year >= targetYear */
                move_until_catch_year(query[i], targetYear_for_weekCount);

                /* Move until month >= targetMonth */
                move_until_catch_month(query[i], targetMonth_for_weekCount);
            }
            else{
                /* Move until year >= targetYear */
                move_until_catch_year(query[i], targetYear);

                /* Move until month >= targetMonth */
                move_until_catch_month(query[i], targetMonth);
            }
        }
        else if (i == weekIndex){
            /* Move until year >= targetYear */
            move_until_catch_year(query[i], targetYear_for_weekCount);

            /* Move until month >= targetMonth */
            move_until_catch_month(query[i], targetMonth_for_weekCount);

            /* Move until weekCount >= targetWeekCount */
            move_until_catch_weekCount(query[i], targetWeekCount, true);
        }
        else{
            if (weekIndex != -1 && targetIndex == weekIndex){
                /* Move until year >= targetYear */
                move_until_catch_year(query[i], targetYear_for_weekCount);

                /* Move until month >= targetMonth */
                move_until_catch_month(query[i], targetMonth_for_weekCount);

                /* Move until weekCount >= targetWeekCount */
                move_until_catch_weekCount(query[i], targetWeekCount);
            }
            else{
                /* Move until year >= targetYear */
                move_until_catch_year(query[i], targetYear);

                /* Move until month >= targetMonth */
                move_until_catch_month(query[i], targetMonth);

                /* Move until day >= targetDay */
                move_until_catch_day(query[i], targetDay);
            }
        }
    }


    /* Loop the user's strategy which is implemented in next() */
    unsigned int targetDate = 0;
    unsigned int targetTime = 0;
    do{
        length++; // Number of passed records

        /* Convey datas */
        convey(ships, query, typeCount);

        /* Strategy */
        next();

        /* If meet the end date or end time, finish analysis. */
        if (endDate != 0 && targetDate >= endDate){
            if (targetTime == 0){
                /* If minute type not used */
                break;
            }
            else if ((endDate >= 20160801 && targetTime == 1530) || (endDate < 20160801 && targetTime == 1500)){
                /* From 20160801 market close at 1530 */
                break;
            }
        }

        /* Move records */
        query[centerIndex].next();
        if(!query[centerIndex].isValid()){
            break;
        }

        /* Set new target date to the center db's date */
        targetDate = query[centerIndex].value(0).toUInt();
        targetYear = get_year(targetDate);
        targetMonth = get_month(targetDate);
        targetDay = get_day(targetDate);

        targetYear_for_weekCount = get_year_for_weekCount(targetDate);
        targetMonth_for_weekCount = get_month_for_weekCount(targetDate);
        targetWeekCount = get_weekCount(targetDate);

        /* If center db is minute db */
        if (centerIndex != monthIndex && centerIndex != weekIndex && centerIndex != dayIndex){
            targetTime = query[centerIndex].value(1).toUInt();
        }

        /* Move records of other databases */
        for(int i = 0; i < typeCount; i++){
            if (i == centerIndex){
                continue;
            }

            if (i == monthIndex){
                if(centerIndex != weekIndex){
                    /* Move until year >= targetYear */
                    move_until_catch_year(query[i], targetYear);

                    /* Move until month >= targetMonth */
                    move_until_catch_month(query[i], targetMonth);
                }
                else{
                    /* If week db is a center db, the way of moving is different from other type of center db */
                    /* Move until year >= targetYear */
                    move_until_catch_year(query[i], targetYear_for_weekCount);

                    /* Move until month >= targetMonth */
                    move_until_catch_month(query[i], targetMonth_for_weekCount);
                }

            }
            else if (i == weekIndex){
                /* Move until year >= targetYear */
                move_until_catch_year(query[i], targetYear_for_weekCount);

                /* Move until month >= targetMonth */
                move_until_catch_month(query[i], targetMonth_for_weekCount);

                /* Move until weekCount >= targetWeekCount */
                move_until_catch_weekCount(query[i], targetWeekCount, true);
            }
            else{
                /* If day db exists, center db is not the week db */
                /* So there is no reason to divide the way of moving and to using weekCount */

                /* Move until year >= targetYear */
                move_until_catch_year(query[i], targetYear);

                /* Move until month >= targetMonth */
                move_until_catch_month(query[i], targetMonth);

                /* Move until day >= targetDay */
                move_until_catch_day(query[i], targetDay);

                /* If db is minute type */
                if (i != dayIndex){
                    move_until_catch_time(query[i], targetTime);
                }
            }
        }
    }while(query[centerIndex].isValid());

    for(int i = 0; i <typeCount; i++){
        db[i].close();
    }
    delete[] db;
    delete[] query;
}

int BackTrader::len(){
    return length;
}

void BackTrader::docking(DataShip* userShips){
    for(int i = 0; i < typeCount; i++){
        ships[i] = userShips[i];
    }
}

const char* BackTrader::get_tableID(){
    return tableId;
}

/* Convey data of records to the ships */
void convey(DataShip* ships, QSqlQuery* query, int shipCount){
    vector<int>* colInt;
    vector<unsigned int>* colUInt;
    vector<long long>* colLongLong;
    vector<unsigned long long>* colULongLong;
    vector<float>* colFloat;

    for(int i = 0; i < shipCount; i++){
        if (ships[i].get_at() == query[i].at()){
            continue;
        }

        for(int j = 0; j < ships[i].size(); j++){
            Column col = ships[i].front();
            ColumnType columnType = col.columnType;

            switch (columnType)
            {
            case Int:
                colInt = (vector<int>*)col.columnName;
                colInt->push_back(query[i].value(j).toInt());
                ships[i].set_at(query[i].at());
                break;
            case UInt:
                colUInt = (vector<unsigned int>*)col.columnName;
                colUInt->push_back(query[i].value(j).toUInt());
                ships[i].set_at(query[i].at());
                break;
            case LongLong:
                colLongLong = (vector<long long>*)col.columnName;
                colLongLong->push_back(query[i].value(j).toLongLong());
                ships[i].set_at(query[i].at());
                break;
            case ULongLong:
                colULongLong = (vector<unsigned long long>*)col.columnName;
                colULongLong->push_back(query[i].value(j).toULongLong());
                ships[i].set_at(query[i].at());
                break;
            case Float:
                colFloat = (vector<float>*)col.columnName;
                colFloat->push_back(query[i].value(j).toFloat());
                ships[i].set_at(query[i].at());
                break;
            }

            ships[i].pop();
            ships[i].push(col);
        }
    }
}

void move_until_catch_year(QSqlQuery& query, unsigned int targetYear){
    /* Move until year >= targetYear */
    unsigned int date = query.value(0).toUInt();
    unsigned int year = get_year(date);

    while (year < targetYear){
        query.next();

        date = query.value(0).toUInt();
        year = get_year(date);
    }
}

void move_until_catch_month(QSqlQuery& query, unsigned int targetMonth){
    /* Move until month >= targetMonth */
    unsigned int date = query.value(0).toUInt();
    unsigned int month = get_month(date);

    while (month < targetMonth){
        query.next();

        date = query.value(0).toUInt();
        month = get_month(date);
    }
}

void move_until_catch_weekCount(QSqlQuery& query, unsigned int targetWeekCount, bool weekIndex){
    /* Move until weekCount >= targetWeekCount */
    unsigned int date = query.value(0).toUInt();
    unsigned int weekCount;

    if (weekIndex){
        /* If type of db is week, form of date is different from the other types */
        weekCount = get_weekCount_from_weekData(date);
    }
    else{
        weekCount = get_weekCount(date);
    }

    while (weekCount < targetWeekCount){
        query.next();

        date = query.value(0).toUInt();
        if (weekIndex){
            /* If type of db is week, form of date is different from the other types */
            weekCount = get_weekCount_from_weekData(date);
        }
        else{
            weekCount = get_weekCount(date);
        }
    }
}

void move_until_catch_day(QSqlQuery& query, unsigned int targetDay){
    /* Move until day >= targetDay */
    unsigned int date = query.value(0).toUInt();
    unsigned int day = get_day(date);

    while (day < targetDay){
        query.next();

        date = query.value(0).toUInt();
        day = get_day(date);
    }
}

void move_until_catch_time(QSqlQuery& query, unsigned int targetTime){
    /* Move until time >= targetTime */
    unsigned int time = query.value(1).toUInt();

    while (time < targetTime){
        query.next();

        time = query.value(1).toUInt();
    }
}

Column::Column(void* columnName, ColumnType columnType){
    this->columnName = columnName;
    this->columnType = columnType;
}

void DataShip::push(Column col){
    columnQ.push(col);
}

void DataShip::pop(){
    columnQ.pop();
}

Column DataShip::front(){
    return columnQ.front();
}

int DataShip::size(){
    return columnQ.size();
}

void DataShip::set_at(int at){
    this->at = at;
}

int DataShip::get_at(){
    return at;
}

void DataShip::sailor(void *sailorName, ColumnType type){
    Column col(sailorName, type);
    push(col);
}
