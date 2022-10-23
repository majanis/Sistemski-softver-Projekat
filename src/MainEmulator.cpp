#ifndef MAINEMULATOR_HPP_
#define MAINEMULATOR_HPP_

#include "../inc/emulator.hpp"
#include <iostream>

int main(int argc, const char** argv) {

try{
    if (argc != 2){
    throw MyException("Neadekvatan broj argumenata za pokretanje emulatora");
  }

  std::string filename=argv[1];
  std::string substringFormatCheck = filename;
  substringFormatCheck = substringFormatCheck.substr(substringFormatCheck.size()-4);
  if(substringFormatCheck!=".hex") throw MyException("los format fajla za emulator " + filename);
  Emulator emulator = Emulator(filename);
  emulator.startEmulator();
}
  catch(MyException& ex) {
    std::cout<<ex.cause()<<std::endl;
  }
  return 0;
}

#endif