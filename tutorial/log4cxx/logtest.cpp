#include <log4cxx/logger.h>  
//#include <log4cxx/basicconfigurator.h>  
//#include <log4cxx/propertyconfigurator.h>  
//#include <log4cxx/helpers/exception.h>  
#include <iostream>  
#include <cmath>
#include <math.h>
#include <sstream>
#include <string>
using namespace std;
int main()  
{  
//log4cxx::PropertyConfigurator::configureAndWatch("log4cxx.properties");  
//log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("lib"));  
//LOG4CXX_DEBUG(logger, "this is log4cxx test");  
int i = 0;
std::stringstream s;
s << 123;
s << "asd";
string c;
s >> c;
log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("com.foo"));
const char* region = "World";
LOG4CXX_INFO(logger, "Simple message text.")
LOG4CXX_INFO(logger, "Hello, "<<region)
LOG4CXX_DEBUG(logger, "Iteration " << i)
LOG4CXX_DEBUG(logger, "e^10 = " << std::scientific << exp(10.0))
//
// Use a wchar_t first operand to force use of wchar_t based stream.
//
LOG4CXX_WARN(logger, c)
}  
