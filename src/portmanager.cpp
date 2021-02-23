#include "../include/portmanager.h"

PortManager::PortManager()
{
    hashDate.rehash(NUM_DATE_ROW);
}

/* This operator will be used sorting trading records of port manager */
bool operator < (TradingRecord x, TradingRecord y){
    if(x.priority < y.priority){
        return true;
    }
    else if (x.priority > y.priority){
        return false;
    }
    else{
        if (x.priority_sub < y.priority_sub){
            return true;
        }
        else{
            return false;
        }
    }
}

void PortManager::publish_record(TradingRecord tradingRecord){
    unsigned int sellDate = tradingRecord.sellDate;
    int sellTime = tradingRecord.sellTime;

    /* Records are saved based on the selling */
    if (sellTime != 0){
        /* If record have time value */
        hashDate[sellDate].date = tradingRecord.sellDate;

        /* Save date and time */
        dateSet.insert(sellDate);
        hashDate[sellDate].timeSet.insert(sellTime);

        /* Use hash time's max queue. Not a hash date's max queue */
        hashDate[sellDate].hashTime[sellTime].time = sellTime;
        hashDate[sellDate].hashTime[sellTime].maxRecordQ.push(tradingRecord);
    }
    else{
        /* If record does not have time value */
        /* Use hash date's max queue */
        hashDate[sellDate].date = tradingRecord.sellDate;
        hashDate[sellDate].maxRecordQ.push(tradingRecord);

        /* Save date */
        dateSet.insert(sellDate);
    }
}

priority_queue<TradingRecord> PortManager::get_recordQ(unsigned int date, int time){
    if (time != 0){
        return hashDate[date].hashTime[time].maxRecordQ;
    }
    else{
        return hashDate[date].maxRecordQ;
    }
}

set<unsigned int> PortManager::get_dateSet(){
    return dateSet;
}

set<int> PortManager::get_timeSet(unsigned int date){
    return hashDate[date].timeSet;
}

void PortManager::clear(){

}
