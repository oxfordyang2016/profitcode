#include <boost/property_tree/ptree.hpp>  
#include <boost/property_tree/xml_parser.hpp>
#include <iostream>
#include <stdio.h>
#include <string>

using namespace std;

int main() {
  boost::property_tree::ptree pt;
  boost::property_tree::read_xml("test.xml", pt);
  int filenum = pt.get<int>("root.delfile.filenum");
  cout << filenum;
}
