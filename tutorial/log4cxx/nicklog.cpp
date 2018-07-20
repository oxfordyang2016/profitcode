#include <string>
#include <sstream>
#include <log4cxx/logger.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/helpers/pool.h>
#include <log4cxx/fileappender.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

using namespace std;
using namespace log4cxx;

class nick_logger {
 public:
  nick_logger(std::string filename) {
    s.clear();
    m_logger = Logger::getLogger("nick_logger");
    PatternLayoutPtr layout = new PatternLayout();
    string conversionPattern = "[%p] %d %c %M - %m%n";
    conversionPattern = "[%p] %d %l:%m%n";
    layout->setConversionPattern(conversionPattern);
    FileAppenderPtr appender = new FileAppender();
    appender->setFile(filename);
    appender->setLayout(layout);
    helpers::Pool p;
    appender->activateOptions(p);
    appender->setAppend(true);
    m_logger->addAppender(appender);
  }
  void LOGE() {
    string str;
    s >> str;
    LOG4CXX_ERROR(m_logger, str);
    s.clear();
  }
  template <typename T, typename ...Args>
  void LOGE(T head, Args... rest) {
    s << head;
    LOGE(rest...);
  }
 private:
  LoggerPtr m_logger;
  stringstream s;
};

int i = 0;
pthread_mutex_t mutex;
void* Run(void *param) {
  nick_logger* n = reinterpret_cast<nick_logger*>(param);
  while (true) {
    pthread_mutex_lock(&mutex);
    n->LOGE(++i, '*', ++i, '*', ++i,'*', ++i);
    pthread_mutex_unlock(&mutex);
    sleep(1);
  }
}

int main() {
  nick_logger a("nick.log");
  pthread_mutex_init(&mutex, NULL);
  pthread_t listener_thread;
  if (pthread_create(&listener_thread,
                     NULL,
                     &Run,
                     &a) != 0) {
    exit(1);
    }
  while (true) {
    pthread_mutex_lock(&mutex);
    a.LOGE(++i);
    pthread_mutex_unlock(&mutex);
    sleep(1);
    }
}
