#ifndef CHECK_THREAD_H
#define CHECK_THREAD_H

#include <stdio.h>
#include <unistd.h>
#include "GlobalData.h"
#include "debug_output.h"
#include "flowManager.h"
#include "sendUdpPacket.h"

const int MAX_CHECK_DELTA = 200;

FlowManager g_flowManager;

void* saveRestBufferToFile(void* temp);

void* checkAndSend(void* param) {
    int lostPktNum = 0;
    pthread_t  thread_id;
    while (true) {
        Packet pkt;
        //get one packet from the queue and delete it.
        while (!g_groundTruthPacketQueue.frontAndPop(pkt)) {
            usleep(1);
        }

        /*check with buffer switch and store data in file*/
        uint32_t ith_interval = get_interval_id(pkt.timestamp);
        if(g_first_pkt) {
            //the first pkt
            g_first_pkt = false;
            g_ith_interval = ith_interval;
        } else {
            //not the first pkt, check whether ith_interval is a new interval
            if (ith_interval == g_ith_interval) {
                //the same interval, nothing to do
            } else if (ith_interval < g_ith_interval) {
                //packet disorder, makes ith_interval < g_ith_interval;
                //
            } else {
                //ith_interval > g_ith_interval;
                g_ith_interval = ith_interval;
                //switch the g_flowManager buffer
                g_flowManager.switchBuffer();

                //start a thread to store the rest buffer to file;
                if(pthread_create(&thread_id, NULL,  saveRestBufferToFile, NULL) < 0) {
                    ERROR("failed to create thread to controller_communicator");
                }
            }
        }
        /*end check*/
        
        //add pkt.len to the total volume of the flow
        g_flowManager.addAllBytes(pkt, pkt.len);

        while (true) {
            if (g_receivedPacketSet.findAndErase(pkt.seqid) == true) {
                //packet received
                //check next packet;
                //printf("seqid:%d - received\n", pkt.seqid);
                break;
            }
            if ((int)getMaxSeqidReceived() - (int)pkt.seqid > MAX_CHECK_DELTA) {
                //packet is lost
                //record
                g_flowManager.addLostBytes(pkt, pkt.len);
                Flow flow(pkt);
                flow.lossRate = g_flowManager.getLossRate(pkt);
                flow.AllVolume = g_flowManager.getAllVolume(pkt);
                printf("lostPktNum:%d, seqid:%d srcip:%u - lost rate:%f, volume:%llu\n", ++lostPktNum, pkt.seqid, pkt.srcip, flow.lossRate, flow.AllVolume);
                //TODO:send the infor to the network
                //TODO:send data every several ms, this will make the overhead proportional
                udpSender.sendMessage((char*)(&flow), sizeof(flow));
                break;
            }
            usleep(10);
        }
    }
    return NULL;
}

void* saveRestBufferToFile(void* temp) {
    char fname[100];
    char buffer[100];
    //g_ith_interval is the current interval, the rest buffer stores the previous interval data
    snprintf(fname, 100, "data/interval_%d.txt", g_ith_interval-1);
    FILE* fp = fopen(fname, "w");
    //write data in the rest buffer
    int restIdx = 1 - g_flowManager.idx;
    for (map<Flow, uint64_t, classcomp>::iterator iter = g_flowManager.flowAllMap[restIdx].begin();
        iter != g_flowManager.flowAllMap[restIdx].end();
        ++iter) {
        snprintf(buffer, 100, "allVolume: %d\t%llu\n", iter->first.srcip, iter->second);
        fputs(buffer, fp);
    }
    for (map<Flow, uint64_t, classcomp>::iterator iter = g_flowManager.flowLostMap[restIdx].begin();
        iter != g_flowManager.flowLostMap[restIdx].end();
        ++iter) {
        snprintf(buffer, 100, "lostVolume: %d\t%llu\n", iter->first.srcip, iter->second);
        fputs(buffer, fp);
    }
    fclose(fp);

    //clear the rest buffer
    g_flowManager.flowAllMap[restIdx].clear();
    g_flowManager.flowLostMap[restIdx].clear();
    return NULL;
}

#endif
