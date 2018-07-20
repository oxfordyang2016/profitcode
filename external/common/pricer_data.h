#ifndef EXTERNAL_COMMON_PRICER_DATA_H_
#define EXTERNAL_COMMON_PRICER_DATA_H_

#include "define.h"

struct PricerData {
  char topic[MAX_TOPIC_LENGTH];
  char ticker[MAX_TICKER_LENGTH];
  int sequence_no;
  int time_sec;
  double data;

  void Show(FILE* stream) const {
    timeval time;
    gettimeofday(&time, NULL);
    fprintf(stream, "%ld %06ld PricerData %s |",
            time.tv_sec, time.tv_usec, ticker);

    fprintf(stream, "%s seq_no:%d %lf", topic, sequence_no, data);

  }
};

#endif  // EXTERNAL_COMMON_PRICER_DATA_H_
