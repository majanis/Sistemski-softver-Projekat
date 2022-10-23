#include "../inc/assembler.hpp"

int main(int argc, const char** argv) {

try{
    if (argc < 4){
    throw MyException("Nedovoljno argumenata za pokretanje asemblera");
  }
  std::string filename=argv[3];
  std::string substringFormatCheck = argv[3];
  substringFormatCheck = substringFormatCheck.substr(substringFormatCheck.size()-2);
  if(substringFormatCheck!=".s") throw MyException("los format fajla za asembler " + filename);
  filename = argv[2];
  substringFormatCheck = argv[2];
  substringFormatCheck = substringFormatCheck.substr(substringFormatCheck.size()-2);
  if(substringFormatCheck!=".o") throw MyException("los format fajla za asembler " + filename);

  Assembler ass(argv[3], argv[2]);
  ass.analyzeAssemblyCode(); 
  }
  catch(MyException& ex) {
    std::cout<<ex.cause()<<std::endl;
  }
  return 0;
}