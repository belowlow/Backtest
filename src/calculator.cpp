#include "../include/calculator.h"
#include "../include/backtrader.h"
#include "../include/indicators.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <iostream>


struct DateTimeStruct{
    unsigned int date;
    Flag flag;
    int time;
};

queue<DateTimeStruct> datetimeQ;

class Ship : public DataShip{
public:
    vector<unsigned int> date;

    vector<int> time;

    vector<int> open;
    vector<int> high;
    vector<int> low;
    vector<int> close;
    vector<float> pct_change;

    vector<unsigned long long> volume;
    vector<unsigned long long> volume_value;
};

class MarketTiming : public BackTrader{
public:
    MarketTiming(int type, int* typeID);
    Ship ship;
private:
    int type;
    int now;
    void next();
};

MarketTiming::MarketTiming(int typeCount, int* typeID) : BackTrader(typeCount, typeID){
    this->type = typeID[0];

    ship.sailor(&ship.date, UInt);

    if (type == _30MIN || type == _5MIN || type == _3MIN || type == _1MIN){
        ship.sailor(&ship.time, Int);
    }

    ship.sailor(&ship.open, Int);
    ship.sailor(&ship.high, Int);
    ship.sailor(&ship.low, Int);
    ship.sailor(&ship.close, Int);
    ship.sailor(&ship.pct_change, Float);
    ship.sailor(&ship.volume, ULongLong);
    ship.sailor(&ship.volume_value, ULongLong);

    DataShip ships[1] = {ship};
    docking(ships);
}

void MarketTiming::next(){
    double pct_change_SD = get_sd(&ship.pct_change, Float);

    if (type == _30MIN || type == _5MIN || type == _3MIN || type == _1MIN){
        now = ship.time.size() - 1;

        if (ship.pct_change[now - 1] < -pct_change_SD){
            DateTimeStruct datetime;

            datetime.date = ship.date[now];
            datetime.time = ship.time[now];
            datetime.flag = Static;

            datetimeQ.push(datetime);
        }
        else if (ship.pct_change[now - 2] < -pct_change_SD){
            DateTimeStruct datetime;

            datetime.date = ship.date[now];
            datetime.time = ship.time[now];
            datetime.flag = Static;

            datetimeQ.push(datetime);
        }
        else{
            DateTimeStruct datetime;

            datetime.date = ship.date[now];
            datetime.time = ship.time[now];
            datetime.flag = Dynamic;

            datetimeQ.push(datetime);
        }
    }
    else{
        now = ship.date.size() - 1;

        if (ship.pct_change[now - 1] < -pct_change_SD){
            DateTimeStruct datetime;

            datetime.date = ship.date[now];
            datetime.time = 0;
            datetime.flag = Static;

            datetimeQ.push(datetime);
        }
        else if (ship.pct_change[now - 2] < -pct_change_SD){
            DateTimeStruct datetime;

            datetime.date = ship.date[now];
            datetime.time = 0;
            datetime.flag = Static;

            datetimeQ.push(datetime);
        }
        else{
            DateTimeStruct datetime;

            datetime.date = ship.date[now];
            datetime.time = 0;
            //datetime.flag = Static; // 잠깐 스태틱으로 바꿈
            datetime.flag = Dynamic;

            datetimeQ.push(datetime);
        }
    }
}

Calculator::Calculator(const char* tableID, int type, const char* indexPath)
{
    int typeID[1] = {type};
    MarketTiming flager(1, typeID);

    const char* path[1] = {indexPath};
    flager.feed(tableID, path);
    flager.set_startDate(20150625);

    flager.run();

    if (type == _30MIN || type == _5MIN || type == _3MIN || type == _1MIN){
        while(!datetimeQ.empty()){
            DateTimeStruct datetime = datetimeQ.front();
            datetimeQ.pop();

            make_CalNode(datetime.flag, datetime.date, datetime.time);
        }
    }
    else{
        while(!datetimeQ.empty()){
            DateTimeStruct datetime = datetimeQ.front();
            datetimeQ.pop();

            make_CalNode(datetime.flag, datetime.date);
        }
    }

}

void Calculator::make_CalNode(Flag flag, unsigned int date, int time){
    /* Make CalNode and put it to the linked list */
    CalNode* cal = new CalNode;

    cal->date = date;
    cal->time = time;
    cal->flag = flag;

    if (this->head == NULL){
        this->head = cal;
        this->tail = cal;
    }
    else{
        this->tail->next = cal;
        this->tail = cal;
    }
}

void Calculator::insert(PortManager* portManager){
    portManagers.push_back(*portManager);
}

void Calculator::calculate(){
    CalNode* target = head;
    while(target != NULL){
        /* Get the TradingRecords of all port managers and push it to the CalNode */
        int num = 0; // The Number of records a CalNode has
        if (target->flag == Dynamic){
            for(int i = 0; i < (int)portManagers.size(); i++){

                if (num == dynamicNum){
                    /* If CalNode has enough Trading records */
                    break;
                }

                /* If port manager has port about date */
                if (portManagers[i].get_dateSet().count(target->date) != 0){
                    /* Get the max priority queue */
                    priority_queue<TradingRecord> maxRecordQ;
                    if (target->time != 0 && portManagers[i].get_timeSet(target->date).count(target->time) != 0){
                        maxRecordQ = portManagers[i].get_recordQ(target->date, target->time);
                    }
                    else{
                        maxRecordQ = portManagers[i].get_recordQ(target->date);
                    }

                    /* Add TradingRecords to the CalNode */
                    while(!maxRecordQ.empty()){
                        TradingRecord rec = maxRecordQ.top();
                        maxRecordQ.pop();
                        num++; // The Number of records a CalNode has

                        target->recordQ.push(rec);
                        target->priceDiv.push_back( round(1 + dynamicRatio * (tax * ((double)rec.sellPrice / rec.buyPrice) - 1), 4) );

                        if (num == dynamicNum){
                            /* If CalNode has enough Trading records */
                            break;
                        }
                    }
                }
            }
        }
        else{
            for(int i = 0; i < (int)portManagers.size(); i++){

                if (num == staticNum){
                    /* If CalNode has enough Trading records */
                    break;
                }

                /* If port manager has port about date */
                if (portManagers[i].get_dateSet().count(target->date) != 0){
                    /* Get the max priority queue */
                    priority_queue<TradingRecord> maxRecordQ;
                    if (target->time != 0 && portManagers[i].get_timeSet(target->date).count(target->time) != 0){
                        maxRecordQ = portManagers[i].get_recordQ(target->date, target->time);
                    }
                    else{
                        maxRecordQ = portManagers[i].get_recordQ(target->date);
                    }

                    /* Add TradingRecords to the CalNode */
                    while(!maxRecordQ.empty()){
                        TradingRecord rec = maxRecordQ.top();
                        maxRecordQ.pop();
                        num++; // The Number of records a CalNode has

                        target->recordQ.push(rec);
                        target->priceDiv.push_back( round(1 + staticRatio * (tax * ((double)rec.sellPrice / rec.buyPrice) - 1), 4) );

                        if (num == staticNum){
                            /* If CalNode has enough Trading records */
                            break;
                        }
                    }
                }
            }
        }

        /* Get the average dividen */
        double sum = 0;
        int size = (int)target->priceDiv.size();
        if (target->flag == Dynamic){
            if (size <= dynamicNum){
                for(int i = 0; i < size; i++){
                    sum += target->priceDiv[i];
                }
                target->avgDiv = round(sum / target->priceDiv.size(), 4); // Get the average with dynamic size
            }
            else{
                for(int i = 0; i < dynamicNum; i++){
                    sum += target->priceDiv[i];
                }
                target->avgDiv = round(sum /dynamicNum, 4); // Get the average with dynamic size
            }

            if (size == 0){
                target->avgDiv = 1;
            }
        }
        else{
            if (size < staticNum){
                for(int i = 0; i < size; i++){
                    sum += target->priceDiv[i];
                }
                for(int i = 0; i < staticNum - size; i++){
                    sum += 1;
                }
                target->avgDiv = round(sum / staticNum, 4); // Get the average with static size
            }
            else{
                for(int i = 0; i < staticNum; i++){
                    sum += target->priceDiv[i];
                }
                target->avgDiv = round(sum / staticNum, 4); // Get the average with static size
            }

            if (size == 0){
                target->avgDiv = 1;
            }
        }

        target = target->next;
    }
}

void Calculator::clear(){

}

void Calculator::set_dynamic(int dynamicNum, double dynamicRatio){
    this->dynamicNum = dynamicNum;
    this->dynamicRatio = dynamicRatio;
}

void Calculator::set_static(int staticNum, double staticRatio){
    this->staticNum = staticNum;
    this->staticRatio = staticRatio;
}

CalNode* Calculator::get_head(){
    return head;
}
