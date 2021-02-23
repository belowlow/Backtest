#include "../include/indicators.h"

double round(double x, int num){
    int p = pow(10, num);
    double ret = round(x * p);
    return ret / p;
}

double get_sd(void* vectorName, ColumnType colType, int period, int start){
    vector<int>* vecInt;
    vector<unsigned int>* vecUInt;
    vector<long long>* vecLongLong;
    vector<unsigned long long>* vecULongLong;
    vector<float>* vecFloat;

    double sum = 0;
    double squaredSum = 0;

    double avgSum;
    double avgSquaredSum;

    int i;

    double ret;

    int last;

    int size;

    switch (colType) {
    case Int:
        vecInt = (vector<int>*)vectorName;

        if (period != 0){
            last = (int)vecInt->size() - 2 - period;
            size = period;
        }
        else{
            last = -1;
            size = (int)vecInt->size();
        }

        for(i = (int)vecInt->size() - 2 + start; i > last; i--){
            sum += (double)(*vecInt)[i];
            squaredSum += pow((double)(*vecInt)[i], 2);
        }
        avgSum = round(sum / size, 2);
        avgSquaredSum = round(squaredSum / size, 2);
        ret = round(avgSquaredSum - pow(avgSum, 2), 2);
        break;
    case UInt:
        vecUInt = (vector<unsigned int>*)vectorName;

        if (period != 0){
            last = (int)vecUInt->size() - 2 - period;
            size = period;
        }
        else{
            last = -1;
            size = (int)vecUInt->size();
        }

        for(i = (int)vecUInt->size() - 2 + start; i > last; i--){
            sum += (double)(*vecUInt)[i];
            squaredSum += pow((double)(*vecUInt)[i], 2);
        }
        avgSum = round(sum / size, 2);
        avgSquaredSum = round(squaredSum / size, 2);
        ret = round(avgSquaredSum - pow(avgSum, 2), 2);
        break;
    case LongLong:
        vecLongLong = (vector<long long>*)vectorName;

        if (period != 0){
            last = (int)vecLongLong->size() - 2 - period;
            size = period;
        }
        else{
            last = -1;
            size = (int)vecLongLong->size();
        }

        for(i = (int)vecLongLong->size() - 2 + start; i > last; i--){
            sum += (double)(*vecLongLong)[i];
            squaredSum += pow((double)(*vecLongLong)[i], 2);
        }
        avgSum = round(sum / size, 2);
        avgSquaredSum = round(squaredSum / size, 2);
        ret = round(avgSquaredSum - pow(avgSum, 2), 2);
        break;
    case ULongLong:
        vecULongLong = (vector<unsigned long long>*)vectorName;

        if (period != 0){
            last = (int)vecULongLong->size() - 2 - period;
            size = period;
        }
        else{
            last = -1;
            size = (int)vecULongLong->size();
        }

        for(i = (int)vecULongLong->size() - 2 + start; i > last; i--){
            sum += (double)(*vecULongLong)[i];
            squaredSum += pow((double)(*vecULongLong)[i], 2);
        }
        avgSum = round(sum / size, 2);
        avgSquaredSum = round(squaredSum / size, 2);
        ret = round(avgSquaredSum - pow(avgSum, 2), 2);
        break;
    case Float:
        vecFloat = (vector<float>*)vectorName;

        if (period != 0){
            last = (int)vecFloat->size() - 2 - period;
            size = period;
        }
        else{
            last = -1;
            size = (int)vecFloat->size();
        }

        for(i = (int)vecFloat->size() - 2 + start; i > last; i--){
            sum += (double)(*vecFloat)[i];
            squaredSum += pow((double)(*vecFloat)[i], 2);
        }
        avgSum = round(sum / size, 2);
        avgSquaredSum = round(squaredSum / size, 2);
        ret = round(avgSquaredSum - pow(avgSum, 2), 2);
        break;
    }

    return ret;
}

double get_movingAVG(void* vectorName, ColumnType colType, int period){
    vector<int>* vecInt;
    vector<unsigned int>* vecUInt;
    vector<long long>* vecLongLong;
    vector<unsigned long long>* vecULongLong;
    vector<float>* vecFloat;

    double sum = 0;

    double avgSum;

    int i;

    switch (colType) {
    case Int:
        vecInt = (vector<int>*)vectorName;
        for(i = (int)vecInt->size() - 2; i > (int)vecInt->size() - 2 - period; i--){
            sum += (double)(*vecInt)[i];
        }
        avgSum = sum / period;
        break;
    case UInt:
        vecUInt = (vector<unsigned int>*)vectorName;
        for(i = (int)vecUInt->size() - 2; i > (int)vecUInt->size() - 2 - period; i--){
            sum += (double)(*vecUInt)[i];
        }
        avgSum = sum / period;
        break;
    case LongLong:
        vecLongLong = (vector<long long>*)vectorName;
        for(i = (int)vecLongLong->size() - 2; i > (int)vecLongLong->size() - 2 - period; i--){
            sum += (double)(*vecLongLong)[i];
        }
        avgSum = sum / period;
        break;
    case ULongLong:
        vecULongLong = (vector<unsigned long long>*)vectorName;
        for(i = (int)vecULongLong->size() - 2; i > (int)vecULongLong->size() - 2 - period; i--){
            sum += (double)(*vecULongLong)[i];
        }
        avgSum = sum / period;
        break;
    case Float:
        vecFloat = (vector<float>*)vectorName;
        for(i = (int)vecFloat->size() - 2; i > (int)vecFloat->size() - 2 - period; i--){
            sum += (double)(*vecFloat)[i];
        }
        avgSum = sum / period;
        break;
    }

    return avgSum;
}
