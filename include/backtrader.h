#ifndef BACKTRADER_H
#define BACKTRADER_H

#include "dbtype.h"
#include <vector>
#include <queue>

using namespace std;

/* Data type of column */
enum ColumnType
{
    Int,
    UInt,
    LongLong,
    ULongLong,
    Float
};

struct Column{
    void* columnName;
    ColumnType columnType;

    Column(void* columnName, ColumnType columnType);
};

class DataShip
{
public:
    void push(Column col);
    void pop();
    Column front();
    int size();

    void set_at(int at);
    int get_at();

    void sailor(void* sailorName, ColumnType type);

private:
    queue<Column> columnQ;
    int at = -1; // The location of row
};

class BackTrader
{
public:
    BackTrader(int typeCount, int typeId[]); // User must input what kind of type will be used
    ~BackTrader();

    void feed(const char* tableId, const char* path[]); // User must input DataBase's path and table name
    void docking(DataShip* userShips); // Connect db's record with user's data sturcture
    void run();

    void set_startDate(unsigned int date); // User can set the start date
    void set_endDate(unsigned int date); // User can set the end date

    int len();

    const char* get_tableID();

protected:
    virtual void next() = 0; // User must implement the strategy in the next() at the child class

private:
    unsigned int startDate = 0; // Date to start analysis
    unsigned int endDate = 0; // Date to end analysis

    int* typeId; // Array of type id
    int typeCount; // Count of type id

    /* Index of typeId */
    /* If a type is not used, the index of that type will be kept at -1 */
    int monthIndex = -1;
    int weekIndex = -1;
    int dayIndex = -1;
    int _30MinIndex = -1;
    int _5MinIndex = -1;
    int _3MinIndex = -1;
    int _1MinIndex = -1;

    /* Index of databse who has shortest time frame */
    int centerIndex = -1;

    const char** path; // Array of database's path
    const char* tableId; // Name of target table

    int length = 0;

    /* Array of DataShip */
    DataShip* ships;
};

#endif // BACKTRADER_H
