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
void* sendSignalToNetwork(void* temp);

void* checkAndSend(void* param) {
    //int lostPktNum = 0;
    int allPktNum = 0;
    pthread_t thread_id;
    pthread_t signal_thread_id;
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
                    ERROR("failed to create thread to saveRestBufferToFile");
                }
                //clear the rest buffer
                g_flowManager.clearRestBuffer();
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

                allPktNum++;
                if (allPktNum % NUM_PKTS_TO_SEND_SIGNAL == 0) {
                    if(pthread_create(&signal_thread_id, NULL, sendSignalToNetwork, NULL) < 0) {
                        ERROR("failed to create thread to sendSignalToNetwork");
                    }
                    
                }

                break;
            }
            if ((int)getMaxSeqidReceived() - (int)pkt.seqid > MAX_CHECK_DELTA) {
                //packet is lost
                //record
                g_flowManager.addLostBytes(pkt, pkt.len);
                /*
                Flow flow(pkt);
                flow.lossRate = g_flowManager.getLossRate(pkt);
                flow.AllVolume = g_flowManager.getAllVolume(pkt);
                printf("lostPktNum:%d, seqid:%d srcip:%u - lost rate:%f, volume:%llu\n", ++lostPktNum, pkt.seqid, pkt.srcip, flow.lossRate, flow.AllVolume);
                //TODO:send the infor to the network
                //TODO:send data every several ms, this will make the overhead proportional
                udpSender.sendMessage((char*)(&flow), sizeof(flow));
                */
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
    double lossRate;
    map<Flow, uint64_t, classcomp>::iterator lossIter;
    //g_ith_interval is the current interval, the rest buffer stores the previous interval data
    snprintf(fname, 100, "data/interval_%d.txt", g_ith_interval-1);
    FILE* fp = fopen(fname, "w");
    //write general information
    snprintf(buffer, 100, "target flows, interval_seconds:%d, volume:%llu, lossRate:%f\n", 
        INTERVAL_SECONDS, 
        TARGET_VOLUME_THRESHOLD, 
        TARGET_LOSS_RATE_THRESHOLD
    );
    fputs(buffer, fp);
    //write data in the rest buffer
    int restIdx = 1 - g_flowManager.idx;
    for (map<Flow, uint64_t, classcomp>::iterator iter = g_flowManager.flowAllMap[restIdx].begin();
        iter != g_flowManager.flowAllMap[restIdx].end();
        ++iter) {

        lossIter = g_flowManager.flowLostMap[restIdx].find(iter->first);
        if (lossIter != g_flowManager.flowLostMap[restIdx].end()) {
            //loss volume exist
            lossRate = 1.0 * lossIter->second / iter->second;
        } else {
            lossRate = 0;
        }

        snprintf(buffer, 100, "%d\t%llu\t%f\n", iter->first.srcip, iter->second, lossRate);
        fputs(buffer, fp);
    }
    fclose(fp);

    //clear the rest buffer
    g_flowManager.flowAllMap[restIdx].clear();
    g_flowManager.flowLostMap[restIdx].clear();
    return NULL;
}

//send signal to switches
void* sendSignalToNetwork(void* temp) {
    int idx = g_flowManager.idx;
    char buffer[100];
    double lossRate = 0;
    uint64_t totalVolume;
    map<Flow, uint64_t, classcomp>::iterator lossIter;

    for (map<Flow, uint64_t, classcomp>::iterator iter = g_flowManager.flowAllMap[idx].begin();
        iter != g_flowManager.flowAllMap[idx].end();
        ++iter) {
        totalVolume = iter->second;
        if (totalVolume < TARGET_VOLUME_THRESHOLD ) {
            continue;
        }

        lossIter = g_flowManager.flowLostMap[idx].find(iter->first);
        if (lossIter != g_flowManager.flowLostMap[idx].end()) {
            //loss volume exist
            lossRate = 1.0 * lossIter->second / totalVolume;
            if (lossRate < TARGET_LOSS_RATE_THRESHOLD) {
                continue;
            }
            //send the flow to the network
            Flow flow(iter->first);
            flow.lossRate = lossRate;
            flow.AllVolume = totalVolume;
            snprintf(buffer, 100, "target flow, srcip:%u - lost rate:%f, volume:%llu\n", iter->first.srcip, lossRate, totalVolume);
            DEBUG(buffer);
            //TODO:send the infor to the network
            //TODO:send data every several ms, this will make the overhead proportional
            udpSender.sendMessage((char*)(&flow), sizeof(flow));
        }//if
    }//for 
    return NULL;
}

#endif
