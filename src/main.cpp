#include "../include/backtrader.h"
#include "../include/portmanager.h"
#include "../include/indicators.h"
#include "../include/calculator.h"
#include <QApplication>
#include <iostream>
#include <QAxWidget>
#include <QStringList>
#include <QVariant>
#include <pthread.h>
#include <string>

using namespace std;

FILE* fp1;

pthread_mutex_t portLatch = PTHREAD_MUTEX_INITIALIZER;

QStringList List;

PortManager* portA;
PortManager* portB;

struct code{
    QString name;
    int buyPrice;
    int sellPrice;
    double priority;
};

bool operator < (code a, code b){
    if (a.priority < b.priority){
        return true;
    }
    else{
        return false;
    }
}

priority_queue<code> q1;

class ShipDAY : public DataShip{
public:
    vector<unsigned int> date;

    vector<int> open;
    vector<int> high;
    vector<int> low;
    vector<int> close;

    vector<float> pct_change;

    vector<unsigned long long> volume;
    vector<unsigned long long> volume_value;
    vector<unsigned long long> share_quantity;
    vector<unsigned long long> company_price;

    vector<unsigned int> foreign_offer_limit;
    vector<unsigned int> foreign_offer_possible;
    vector<unsigned int> foreign_holding_quantity;

    vector<float> foreign_holding_ratio;

    vector<int> foreign_netbuy;

    vector<int> institution_netbuy;
    vector<int> institution_netbuy_accumulation;
};

class Ship30MIN : public DataShip{
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

    vector<unsigned long long> buy_accumulation;
    vector<unsigned long long> sell_accumulation;
};

class TestStrategy : public BackTrader{
public:
    TestStrategy(int typeCount, int* typeId);

    ShipDAY shipDAY;
    Ship30MIN ship30MIN;
    int nowDAY;
    int now30MIN;

private:
    void next();
};

TestStrategy::TestStrategy(int typeCount, int* typeId) : BackTrader(typeCount, typeId)
{
    shipDAY.sailor(&shipDAY.date, UInt);
    shipDAY.sailor(&shipDAY.open, Int);
    shipDAY.sailor(&shipDAY.high, Int);
    shipDAY.sailor(&shipDAY.low, Int);
    shipDAY.sailor(&shipDAY.close, Int);
    shipDAY.sailor(&shipDAY.pct_change, Float);
    shipDAY.sailor(&shipDAY.volume, ULongLong);
    shipDAY.sailor(&shipDAY.volume_value, ULongLong);
    shipDAY.sailor(&shipDAY.share_quantity, ULongLong);
    shipDAY.sailor(&shipDAY.company_price, ULongLong);
    shipDAY.sailor(&shipDAY.foreign_offer_limit, UInt);
    shipDAY.sailor(&shipDAY.foreign_offer_possible, UInt);
    shipDAY.sailor(&shipDAY.foreign_holding_quantity, UInt);
    shipDAY.sailor(&shipDAY.foreign_holding_ratio, Float);
    shipDAY.sailor(&shipDAY.foreign_netbuy, Int);
    shipDAY.sailor(&shipDAY.institution_netbuy, Int);
    shipDAY.sailor(&shipDAY.institution_netbuy_accumulation, Int);

    ship30MIN.sailor(&ship30MIN.date, UInt);
    ship30MIN.sailor(&ship30MIN.time, Int);
    ship30MIN.sailor(&ship30MIN.open, Int);
    ship30MIN.sailor(&ship30MIN.high, Int);
    ship30MIN.sailor(&ship30MIN.low, Int);
    ship30MIN.sailor(&ship30MIN.close, Int);
    ship30MIN.sailor(&ship30MIN.pct_change, Float);
    ship30MIN.sailor(&ship30MIN.volume, ULongLong);
    ship30MIN.sailor(&ship30MIN.volume_value, ULongLong);
    ship30MIN.sailor(&ship30MIN.buy_accumulation, ULongLong);
    ship30MIN.sailor(&ship30MIN.sell_accumulation, ULongLong);

    DataShip ships[2] = {shipDAY, ship30MIN};
    docking(ships);
}

void TestStrategy::next(){
    /* Strategy */
}

void* backtest(void* arg){
    int* index = (int*)arg;
    for(int i = index[0]; i < index[1]; i++){
        int typeId[2] = {DAY, _30MIN};
        const char* table = List[i].toStdString().c_str();
        cout << table << endl;
        if (table[6] != '0'){
            continue;
        }

        const char* path[] = {"KOSDAQ_DAY.db", "KOSDAQ_30MIN.db"};

        TestStrategy k(2, typeId);
        k.feed(table, path);
        k.set_startDate(20150625);
        k.set_endDate(20210222);
        k.run();

        cout << table << endl;
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    portA = new PortManager;
    portB = new PortManager;

    QAxWidget* codeManager = new QAxWidget("CpUtil.CpCodeMgr");
    //QVariant code = codeManager->dynamicCall("GetStockListByMarket(1)"); // kospi list
    QVariant code = codeManager->dynamicCall("GetStockListByMarket(2)"); // kosdaq list
    List = code.toStringList();
    fp1 = fopen("code.txt", "w");

    int size = List.size();
    int index1[2] = {0, size / 3};
    int index2[2] = {size / 3, 2*size / 3};
    int index3[2] = {2*size /3, size};

    int* indexes[3] = {index1, index2, index3};

    pthread_t threadID[3];
    for(int i = 0; i < 3; i++){
        pthread_create(&threadID[i], NULL, backtest, indexes[i]);
    }

    for(int i = 0; i < 3; i++){
        pthread_join(threadID[i], NULL);
    }

    Calculator* calculator = new Calculator("U201", DAY,
                                            "KOSDAQ_INDEX_DAY.db");
    calculator->insert(portA);

    calculator->set_dynamic(1, 1);
    calculator->set_static(1, 1);

    calculator->calculate();

    CalNode* target = calculator->get_head();
    FILE* fp = fopen("result.txt", "w");

    while(target != NULL){
        fprintf(fp, "%lf\n", target->avgDiv);
        target = target->next;
    }
    fclose(fp);

    return a.exec();
}
