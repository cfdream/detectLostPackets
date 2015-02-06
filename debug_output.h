#ifndef DEBUG_OUTPUT_H
#define DEBUG_OUTPUT_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

const bool OPEN_DEBUG = true;
const bool OPEN_NOTICE = true;
const bool OPEN_WARNING = true;
const bool OPEN_ERROR = true;

const char DEBUG_FNAME[] =  "log/debug.txt";
const char NOTICE_FNAME[] =  "log/notice.txt";
const char WARNING_FNAME[] = "log/warning.txt";
const char ERROR_FNAME[] = "log/error.txt";

void DEBUG(const char* buffer);
void NOTICE(const char* buffer);
void WARNING(const char* buffer);
void ERROR(const char* buffer);

void DEBUG(const char* buffer) {
    FILE * fp;
    if (!OPEN_DEBUG) {
        return;
    }
    fp = fopen(DEBUG_FNAME, "a+");
    if (fp == NULL) {
        printf("open file failed");
        return;
    }
    fputs(buffer, fp);
    fputc('\n', fp);
    fclose(fp);
}

void NOTICE(const char* buffer) {
    FILE * fp;
    if (!OPEN_NOTICE) {
        return;
    }
    fp = fopen(NOTICE_FNAME, "a+");
    if (fp == NULL) {
        printf("open file failed");
        return;
    }
    fputs(buffer, fp);
    fputc('\n', fp);
    fclose(fp);
}

void WARNING(const char* buffer) {
    FILE * fp;
    if (!OPEN_WARNING) {
        return;
    }
    fp = fopen(WARNING_FNAME, "a+");
    if (fp == NULL) {
        printf("open file failed");
        return;
    }
    fputs(buffer, fp);
    fputc('\n', fp);
    fclose(fp);
}

void ERROR(const char* buffer) {
    FILE * fp;
    if (!OPEN_ERROR) {
        return;
    }
    fp = fopen(ERROR_FNAME, "a+");
    if (fp == NULL) {
        printf("open file failed");
        return;
    }
    fputs(buffer, fp);
    fputc('\n', fp);
    fclose(fp);
}

#endif
