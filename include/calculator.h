#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <portmanager.h>
#include "dbtype.h"

enum Flag
{
    Static,
    Dynamic
};

struct CalNode{
    /* CalNode is linked list */
    CalNode* next = NULL;

    /* Information about date, time */
    unsigned int date = 0;
    int time = 0;

    /* CalNode has TradingRecords */
    queue<TradingRecord> recordQ;

    /* CalNode has TradingRecord's sellPrice/buyPrice as vector. At last, get the average sellPrice/buyPrice */
    vector<double> priceDiv;
    double avgDiv;

    /* Whether to trade aggressively(Dynamic) or conservatively(Static) */
    Flag flag;
};

class Calculator
{
public:
    Calculator(const char* tableID, int type, const char* indexPath);

    void insert(PortManager* portManager);

    void calculate();
    void clear();

    void set_dynamic(int dynamicNum, double dynamicRatio);
    void set_static(int staticNum, double staticRatio);

    CalNode* get_head();

private:
    void make_CalNode(Flag flag, unsigned int date, int time = 0);

    double tax = (1 - 0.0028);
    double dynamicRatio;
    double staticRatio;

    /* Calculator has CalNodes as linked list */
    CalNode* head = NULL;
    CalNode* tail = NULL;

    /* PortManagers */
    vector<PortManager> portManagers;

    int dynamicNum;
    int staticNum;
};

#endif // CALCULATOR_H
