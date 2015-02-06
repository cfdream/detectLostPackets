#ifndef GLOBAL_DATA_H
#define GLOBAL_DATA_H

#include <stdint.h>
#include <stdio.h>
#include <set>
#include <queue>
#include <unistd.h>
#include <pthread.h>

using namespace std;

const uint32_t MAX_PACKETS_IN_BUFFER = 100000;

class Packet{
public:    
    uint32_t srcip;
    uint32_t dstip;
    uint16_t srcport;
    uint16_t dstport;
    uint32_t seqid;
    uint32_t len;
    uint64_t timestamp;

    Packet(){}

    Packet(uint32_t srcip, uint32_t dstip, uint16_t srcport, uint16_t dstport, uint32_t seqid, uint32_t len, uint64_t timestamp) {
        this->srcip = srcip;
        this->dstip = dstip;
        this->srcport = srcport;
        this->dstport = dstport;
        this->seqid = seqid;
        this->len = len;
        this->timestamp = timestamp;
    }
};

class Flow{
public:    
    uint32_t srcip;
    float lossRate;
    uint64_t AllVolume;
    /*
    uint32_t dstip;
    uint16_t srcport;
    uint16_t dstport;
    */

    /*
    Flow(){}
    Flow(uint32_t srcip, uint32_t dstip, uint16_t srcport, uint16_t dstport) {
        this->srcip = srcip;
        this->dstip = dstip;
        this->srcport = srcport;
        this->dstport = dstport;
    }
    */
    Flow(Packet &pkt) {
        this->srcip = pkt.srcip;
        /*
        this->dstip = pkt.dstip;
        this->srcport = pkt.srcport;
        this->dstport = pkt.dstport;
        */
    }
/*
    bool operator<(const Flow &flow) const {
        return !(this->srcip == flow.srcip &&
                this->dstip == flow.dstip &&
                this->srcport == flow.srcport &&
                this->dstport == flow.dstport);
    }
    bool operator()(const Flow &pkt1, const Flow &pkt2) const {
        return !(pkt2.srcip == pkt1.srcip &&
                pkt2.dstip == pkt1.dstip &&
                pkt2.srcport == pkt1.srcport &&
                pkt2.dstport == pkt1.dstport);
    }
*/
};

//thread safe set
class SafeSet{
    pthread_mutex_t mtx;
    set<uint64_t> dataSet;
    
public:
    SafeSet() {
        //mtx = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_init(&mtx, NULL);
        dataSet.clear();
    }
    bool findAndErase (const uint64_t &val) {
        pthread_mutex_lock(&mtx);
        bool exist = false;
        if(dataSet.find(val) != dataSet.end()) {
            dataSet.erase(val);
            exist = true;
        }
        pthread_mutex_unlock(&mtx);
        return exist;
    }

    void insert (const uint64_t &val) {
        while(true) {
            pthread_mutex_lock(&mtx);
            if (dataSet.size() < MAX_PACKETS_IN_BUFFER) {
                dataSet.insert(val);
                pthread_mutex_unlock(&mtx);
                break;
            }
            pthread_mutex_unlock(&mtx);
            usleep(1);
        }
    }
};

//thread safe queue
template <class T>
class SafeQueue {
    pthread_mutex_t mtx;
    queue<T> dataQueue;

public:
    SafeQueue() {
        //mtx = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_init(&mtx, NULL);
    }
    void push (const T& val) {
        while(true) {
            pthread_mutex_lock(&mtx);
            if (dataQueue.size() < MAX_PACKETS_IN_BUFFER) {
                dataQueue.push(val);
                pthread_mutex_unlock(&mtx);
                break;
            }
            pthread_mutex_unlock(&mtx);
            usleep(1);
        }
    }
    bool frontAndPop(T &val) {
        /*DEBUG*/
        //usleep(100);
        /*DEBUG*/
        pthread_mutex_lock(&mtx);
        bool succ = false;
        if (dataQueue.size() > 0) {
            val = dataQueue.front();
            dataQueue.pop();
            succ = true;
        }
        pthread_mutex_unlock(&mtx);
        return succ;
    }
};

/*Start GlobalData Definition*/
/*Start GlobalData Definition*/
SafeSet g_receivedPacketSet;

SafeQueue<Packet> g_groundTruthPacketQueue;

uint32_t g_maxSeqidReceived = 0L;

pthread_mutex_t g_seqidMtx = PTHREAD_MUTEX_INITIALIZER;;

uint32_t g_ith_interval = 0;
bool g_first_pkt = true;

//interval information
//about time interval
const uint64_t FIRST_INTERVAL_START_USECOND = 21600000000ULL;
const uint64_t SECOND_2_USECOND = 1000000;
const int INTERVAL_SECONDS = 30;
const uint64_t USECONDS_IN_ONE_INTERVAL = 30000000;  //SECOND_2_USECOND * INTERVAL_SECONDS;

/*End GlobalData Definition*/
/*End GlobalData Definition*/

//pthread_mutex_init(&g_seqidMtx, NULL);
uint64_t getMaxSeqidReceived() {
    pthread_mutex_lock(&g_seqidMtx);
    uint64_t val = g_maxSeqidReceived;
    pthread_mutex_unlock(&g_seqidMtx);
    return val;
}

void setMaxSeqidReceived(uint32_t val) {
    pthread_mutex_lock(&g_seqidMtx);
    if (val > g_maxSeqidReceived) {
        g_maxSeqidReceived = val;
    }
    pthread_mutex_unlock(&g_seqidMtx);
}

int get_interval_id(uint64_t timestamp) {
    return (timestamp - FIRST_INTERVAL_START_USECOND) / USECONDS_IN_ONE_INTERVAL + 1;
}
#endif
