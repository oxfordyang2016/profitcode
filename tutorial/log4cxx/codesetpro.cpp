#include <log4cxx/logger.h>  
#include <log4cxx/basicconfigurator.h>  
//#include <log4cxx/propertyconfigurator.h>  
#include <log4cxx/helpers/pool.h>  
#include <log4cxx/dailyrollingfileappender.h>
#include <log4cxx/patternlayout.h>
#include <iostream>  
#include <cmath>
#include <math.h>
#include <string>

using namespace log4cxx;
using namespace std;
 
int main() {
  // LoggerPtr logger(Logger::getRootLogger());
  log4cxx::LoggerPtr  logger(log4cxx::Logger::getLogger("com.foo"));
  // Now set its level. Normally you do not need to set the
  // level of a logger programmatically. This is usually done
  // in configuration files.
  // logger->setLevel(log4cxx::Level::getInfo());

  PatternLayoutPtr layout = new PatternLayout();
  string conversionPattern = "[%p] %d %c %M - %m%n";  
  layout->setConversionPattern(conversionPattern);
  DailyRollingFileAppenderPtr rollingAppenderptr = new DailyRollingFileAppender();  
  rollingAppenderptr->setFile("rolling.log");  
  rollingAppenderptr->setDatePattern("'.'yyyy-MM-dd");
  rollingAppenderptr->setLayout(layout);
  log4cxx::helpers::Pool p;  
  rollingAppenderptr->activateOptions(p); 
  rollingAppenderptr->setAppend(true);

  logger->addAppender(rollingAppenderptr);
   
  //BasicConfigurator::configure(AppenderPtr(rollingAppenderptr));
  LOG4CXX_ERROR(logger, "Starting search for nearest gas station.")
  LOG4CXX_INFO(logger, "Starting search for nearest gas station.")
  LOG4CXX_WARN(logger, "Starting search for nearest gas station.")
}
