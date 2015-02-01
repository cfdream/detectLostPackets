#include <stdio.h>
#include <unistd.h>
#include <cstdlib>
#include <pthread.h>
#include "GlobalData.h"
#include "readPcapFile.h"
#include "checkThread.h"

//const char TCPDUMP_FNAME[] = "tcpdump.pcap";
//const char TRUTH_FNAME[] = "truth.pcap";

void* readTcpdumpPackets(void* fileName) {
    ReadPcapFile reader;
    reader.readReceivedSeqid((char*)fileName);
    return NULL;
}

void* readGroundTruthPackets(void* fileName) {
    ReadPcapFile reader;
    reader.readGroundTruthPackets((char*)fileName);
    return NULL;
}

int main(int argc, char* argv[]) {

    if (argc < 3) {
        printf("detect tcpdump.pcap, truth.pcap");
        exit(1);
    }

    char* TCPDUMP_FNAME = argv[1];
    char* TRUTH_FNAME = argv[2];

    pthread_t tcpdumpReadThread, truthReadThread;
    pthread_t checkAndInformThread;

    int ret = pthread_create(&tcpdumpReadThread, NULL, readTcpdumpPackets, (void*)TCPDUMP_FNAME);
    if (ret) {
        printf("create thread tcpdumpReadThread failed");
        return -1;
    }
    ret = pthread_create(&truthReadThread, NULL, readGroundTruthPackets, (void*)TRUTH_FNAME);
    if (ret) {
        printf("create thread truthReadThread failed");
        return -1;
    }
    ret = pthread_create(&checkAndInformThread, NULL, checkAndSend, NULL);
    if (ret) {
        printf("create thread checkAndInformThread failed");
        return -1;
    }

    pthread_join(tcpdumpReadThread, NULL);
    pthread_join(truthReadThread, NULL);
    pthread_join(checkAndInformThread, NULL);
}
