#ifndef FLOW_MANAGER_H
#define FLOW_MANAGER_H

#include <stdint.h>
#include <map>
#include "GlobalData.h"

struct FlowValue {
    uint64_t allBytes;
    uint64_t lostBytes;
};

struct classcomp {
/*
    A binary predicate that takes two element keys as arguments and returns a bool. The expression comp(a,b), where comp is an object of this type and a and b are key values, shall return true if a is considered to go before b in the strict weak ordering the function defines. 
    The map object uses this expression to determine both the order the elements follow in the container and whether two element keys are equivalent (by comparing them reflexively: they are equivalent if !comp(a,b) && !comp(b,a)). No two elements in a map container can have equivalent keys.
    This can be a function pointer or a function object (see constructor for an example). This defaults to less<T>, which returns the same as applying the less-than operator (a<b).
*/
    bool operator() (const Flow& flow1, const Flow& flow2) const
    {
        /*
        if (flow1.srcip == flow2.srcip) {
            if (flow1.dstip == flow2.dstip) {
                if (flow1.srcport == flow2.srcport) {
                    if (flow1.dstport == flow2.dstport) {
                    }
                    return flow1.dstport < flow2.dstport;
                }
                return flow1.srcport < flow2.srcport;
            }
            return flow1.dstip < flow2.dstip;
        }
        */
        return flow1.srcip < flow2.srcip;
    }
};


//TODO: dual buffer here
class FlowManager{
public:
    map<Flow, uint64_t, classcomp> flowAllMap[2];
    map<Flow, uint64_t, classcomp> flowLostMap[2];
    int idx;

    FlowManager() {
        idx = 0;
    }

    void switchBuffer() {
        idx = 1 - idx;
    }

    void addAllBytes(Packet& pkt, uint32_t len) {
        Flow flow(pkt);
        if (flowAllMap[idx].find(flow) != flowAllMap[idx].end()) {
            flowAllMap[idx][flow] = flowAllMap[idx][flow] + len;
        } else {
            flowAllMap[idx][flow] = len;
        }
    }

    void addLostBytes(Packet& pkt, uint32_t len) {
        Flow flow(pkt);
        if (flowLostMap[idx].find(flow) != flowLostMap[idx].end()) {
            flowLostMap[idx][flow] = flowLostMap[idx][flow] + len;
        } else {
            flowLostMap[idx][flow] = len;
        }
    }

    uint64_t getAllVolume(Packet &pkt) {
        Flow flow(pkt);
        uint64_t allLen = 0;
        if (flowAllMap[idx].find(flow) != flowAllMap[idx].end()) {
            allLen = flowAllMap[idx][flow];
        }
        return allLen;
    }

    double getLossRate(Packet& pkt) {
        Flow flow(pkt);
        uint64_t allLen = 0;
        if (flowAllMap[idx].find(flow) != flowAllMap[idx].end()) {
            allLen = flowAllMap[idx][flow];
        }
        if (allLen == 0) {
            printf("zero\n");
            return 0;
        }

        uint64_t lostLen = 0;
        if (flowLostMap[idx].find(flow) != flowLostMap[idx].end()) {
            lostLen = flowLostMap[idx][flow];
        }
        return 1.0 * lostLen / allLen;
    }
};


#endif
