#ifndef PORTMANAGER_H
#define PORTMANAGER_H

#include <queue>
#include <string>
#include <unordered_map>
#include <set>

#define NUM_DATE_ROW (2000) // It will be used rehashing at first.

using namespace std;

struct TradingRecord{
    string code;

    int buyPrice = 0;
    int sellPrice = 0;

    unsigned int buyDate = 0;
    unsigned int sellDate = 0;

    int buyTime = 0;
    int sellTime = 0;

    double priority = 1; // The priority determines whether to enter the trading list
    double priority_sub = 1;
};

/* Used making max priority_queue */
bool operator < (TradingRecord x, TradingRecord y);

struct TimePort{
    int time = 0;
    priority_queue<TradingRecord> maxRecordQ; // Port has trading records with priority queue
};

struct DatePort{
    unsigned int date = 0;
    set<int> timeSet; // If timeSet has elements, DatePort's maxRecordQ is not used. Go to the hashTime and use TimePort's maxRecordQ
    priority_queue<TradingRecord> maxRecordQ; // Port has trading records with priority queue
    unordered_map<int, TimePort> hashTime; // Mapping PortNode to time
};

class PortManager
{
public:
    PortManager();

    void publish_record(TradingRecord tradingRecord);
    priority_queue<TradingRecord> get_recordQ(unsigned int date, int time = 0);

    set<unsigned int> get_dateSet();
    set<int> get_timeSet(unsigned int date);

    void clear();

private:
    unordered_map<unsigned int, DatePort> hashDate; // Mapping PortNode to date
    set<unsigned int> dateSet; // PortManager save the used dates here
};

// what is the red black tree?

#endif // PORTMANAGER_H
