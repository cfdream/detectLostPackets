#ifndef GLOBAL_DATA_H
#define GLOBAL_DATA_H

#include <stdint.h>
#include <stdio.h>
#include <set>
#include <queue>
#include <unistd.h>
#include <pthread.h>

using namespace std;

typedef unsigned long long uint64;
typedef unsigned int uint32;
typedef unsigned short uint16;

const uint32 MAX_PACKETS_IN_BUFFER = 100000;

class Packet{
public:    
    uint32 srcip;
    uint32 dstip;
    uint16 srcport;
    uint16 dstport;
    uint32 seqid;
    uint32 len;

    Packet(){}

    Packet(uint32 srcip, uint32 dstip, uint16 srcport, uint16 dstport, uint32 seqid, uint32 len) {
        this->srcip = srcip;
        this->dstip = dstip;
        this->srcport = srcport;
        this->dstport = dstport;
        this->seqid = seqid;
        this->len = len;
    }
};

class Flow{
public:    
    uint32 srcip;
    float lossRate;
    uint64_t AllVolume;
    /*
    uint32 dstip;
    uint16 srcport;
    uint16 dstport;
    */

    /*
    Flow(){}
    Flow(uint32 srcip, uint32 dstip, uint16 srcport, uint16 dstport) {
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
    set<uint64> dataSet;
    
public:
    SafeSet() {
        //mtx = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_init(&mtx, NULL);
        dataSet.clear();
    }
    bool findAndErase (const uint64& val) {
        pthread_mutex_lock(&mtx);
        bool exist = false;
        if(dataSet.find(val) != dataSet.end()) {
            dataSet.erase(val);
            exist = true;
        }
        pthread_mutex_unlock(&mtx);
        return exist;
    }

    void insert (const uint64& val) {
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

SafeSet receivedPacketSet;

SafeQueue<Packet> groundTruthPacketQueue;

uint32 maxSeqidReceived = 0L;

pthread_mutex_t seqidMtx = PTHREAD_MUTEX_INITIALIZER;;
//pthread_mutex_init(&seqidMtx, NULL);
uint64 getMaxSeqidReceived() {
    pthread_mutex_lock(&seqidMtx);
    uint64 val = maxSeqidReceived;
    pthread_mutex_unlock(&seqidMtx);
    return val;
}

void setMaxSeqidReceived(uint32 val) {
    pthread_mutex_lock(&seqidMtx);
    if (val > maxSeqidReceived) {
        maxSeqidReceived = val;
    }
    pthread_mutex_unlock(&seqidMtx);
}

#endif
