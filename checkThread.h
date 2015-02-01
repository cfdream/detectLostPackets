#ifndef CHECK_THREAD_H
#define CHECK_THREAD_H

#include <stdio.h>
#include <unistd.h>
#include "GlobalData.h"
#include "flowManager.h"
#include "sendUdpPacket.h"

const int MAX_CHECK_DELTA = 200;

FlowManager flowManager;

void* checkAndSend(void* param) {
    int lostPktNum = 0;
    while (true) {
        Packet pkt;
        //get one packet from the queue and delete it.
        while (!groundTruthPacketQueue.frontAndPop(pkt)) {
            usleep(1);
        }
        flowManager.addAllBytes(pkt, pkt.len);

        while (true) {
            if (receivedPacketSet.findAndErase(pkt.seqid) == true) {
                //packet received
                //check next packet;
                //printf("seqid:%d - received\n", pkt.seqid);
                break;
            }
            if ((int)getMaxSeqidReceived() - (int)pkt.seqid > MAX_CHECK_DELTA) {
                //packet is lost
                //record
                flowManager.addLostBytes(pkt, pkt.len);
                printf("lostPktNum:%d, seqid:%d srcip:%u - lost rate:%f\n", ++lostPktNum, pkt.seqid, pkt.srcip, flowManager.getLossRate(pkt));
                Flow flow(pkt);
                flow.lossRate = flowManager.getLossRate(pkt);
                //TODO:send the infor to the network
                udpSender.sendMessage((char*)(&flow), sizeof(flow));
                break;
            }
            usleep(10);
        }
    }
    return NULL;
}

#endif
