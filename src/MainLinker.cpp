#include "../inc/linker.hpp"

int main(int argc, const char** argv) {

  try {

  if(argc<4) {
    throw MyException("Nedovoljno argumenata za pokretanje linkera");
  }
  std::string argv1 = argv[1];
  std::string argv2 = argv[2];
  std::vector<std::string> argv3 = std::vector<std::string>();
  if(argv1!="-o" && argv1!="-hex") { //args are either -o -hex or -hex -o
    throw MyException("Neispravni argumenti komandne linije");
  }

  if(argv1=="-o") {
    if(argc<4) {
    throw MyException("Nedovoljno argumenata za pokretanje linkera");      
    }
  if(argv2!="-hex") {
  throw MyException("Ne moze da se pokrene linker bez komande -hex");
  }
  argv2 = argv[3];
  std::string substringFormatCheck = argv2;
  substringFormatCheck = substringFormatCheck.substr(substringFormatCheck.size()-4); //remove file extension .hex
  if(substringFormatCheck!=".hex") throw MyException("los format fajla za linker " + argv2);

  for(int i=4; i<argc; i++) {
    argv3.push_back(argv[i]);
  std::string argvarr = argv[i];
  substringFormatCheck = argv[i];
  substringFormatCheck = substringFormatCheck.substr(substringFormatCheck.size()-2); //remove file extension .o
  if(substringFormatCheck!=".o") throw MyException("los format fajla za linker " + argvarr);
  }
  Linker linker = Linker(argv3, argv2);
  linker.startLinker();
  linker.hexCommandPrint();
  }
  else if(argv1=="-hex") {
    if(argv2!="-o")
    throw MyException("Ne moze da se pokrene linker bez komande -o");

    argv2 = argv[3];
    std::string substringFormatCheck = argv2;
    substringFormatCheck = substringFormatCheck.substr(substringFormatCheck.size()-4);
    if(substringFormatCheck!=".hex") throw MyException("los format fajla za linker " + argv2);

    for(int i=4; i<argc; i++) {
    argv3.push_back(argv[i]);
    std::string argvarr = argv[i];
    substringFormatCheck = argv[i];
    substringFormatCheck = substringFormatCheck.substr(substringFormatCheck.size()-2);
    if(substringFormatCheck!=".o") throw MyException("los format fajla za linker " + argvarr);
    }
    Linker linker = Linker(argv3, argv2);
    linker.startLinker();
    linker.hexCommandPrint();

  }
  }

  catch(MyException& ex) {
    std::cout<<ex.cause()<<std::endl;
  }
  return 0;
}