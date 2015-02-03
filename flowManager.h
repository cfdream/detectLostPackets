#ifndef FLOW_MANAGER_H
#define FLOW_MANAGER_H

#include <map>
#include "GlobalData.h"

struct FlowValue {
    uint64 allBytes;
    uint64 lostBytes;
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

class FlowManager{
public:
    map<Flow, uint64, classcomp> flowAllMap;
    map<Flow, uint64, classcomp> flowLostMap;

    void addAllBytes(Packet& pkt, uint32 len) {
        Flow flow(pkt);
        if (flowAllMap.find(flow) != flowAllMap.end()) {
            flowAllMap[flow] = flowAllMap[flow] + len;
        } else {
            flowAllMap[flow] = len;
        }
    }

    void addLostBytes(Packet& pkt, uint32 len) {
        Flow flow(pkt);
        if (flowLostMap.find(flow) != flowLostMap.end()) {
            flowLostMap[flow] = flowLostMap[flow] + len;
        } else {
            flowLostMap[flow] = len;
        }
    }

    uint64_t getAllVolume(Packet &pkt) {
        Flow flow(pkt);
        uint64 allLen = 0;
        if (flowAllMap.find(flow) != flowAllMap.end()) {
            allLen = flowAllMap[flow];
        }
        return allLen;
    }

    double getLossRate(Packet& pkt) {
        Flow flow(pkt);
        uint64 allLen = 0;
        if (flowAllMap.find(flow) != flowAllMap.end()) {
            allLen = flowAllMap[flow];
        }
        if (allLen == 0) {
            printf("zero\n");
            return 0;
        }

        uint64 lostLen = 0;
        if (flowLostMap.find(flow) != flowLostMap.end()) {
            lostLen = flowLostMap[flow];
        }
        return 1.0 * lostLen / allLen;
    }
};

#endif
