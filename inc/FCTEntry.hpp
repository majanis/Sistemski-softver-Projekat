#ifndef FCTENTRY_HPP_
#define FCTENTRY_HPP_


#include "../inc/Section.hpp"
#include "../inc/Exception.hpp"
class FCTEntry {
  private:
  int patchAddress; //location counter of section where we are doing backpatching
  Section* section;
  int length; //number of bytes
  bool patched;
  int relocType; //0 absolute, 1 pcrel
  bool isLittleEndian; //true for word directive only

  public:
  
  FCTEntry(int patchAddress, Section* section, int length, int relocType, bool isLittleEndian) {
    this->section = section;
    this->patchAddress=patchAddress;
    this->length=length;
    this->relocType = relocType;
    this->isLittleEndian = isLittleEndian;
    patched=false;
  }
  int getPatchAddress() {
    return patchAddress;
  }

  bool getPatched() {
    return patched;
  }
  
  void patchAddressAbs(std::vector<std::string> valueForPatching, std::string symbolName,
  std::string sectionWhereSymbolIsDefined) {
    if(valueForPatching.size()>length) { //not allowed
      throw MyException("Greska u velicini patching koda");
      return;
    }
    else if (valueForPatching.size()<length) {
      for(int i=0; i<valueForPatching.size()-length; i++) {
        valueForPatching.insert(valueForPatching.begin(), "00");
      }
    }
    if(!isLittleEndian) {
    for(int i=0; i<length; i++) {
    if(!section->getSectionHex()->size()>=patchAddress+i-1) section->getSectionHex()->push_back("00");
    section->getSectionHex()->at(patchAddress+i) = valueForPatching.at(i);
    }
    }
    else {
    std::vector<std::string> newValueForPatching = std::vector<std::string>();
    newValueForPatching.insert(newValueForPatching.begin(),"00");
    newValueForPatching.insert(newValueForPatching.begin(),"00");
    newValueForPatching.at(0) = valueForPatching.at(1);
    newValueForPatching.at(1) = valueForPatching.at(0);
    for(int i=0; i<length; i++) {
    if(!section->getSectionHex()->size()>=patchAddress+i-1) section->getSectionHex()->push_back("00");
    section->getSectionHex()->at(patchAddress+i) = newValueForPatching.at(i);
    }
    }
    createRelocationEntry(symbolName, patchAddress, relocType, isLittleEndian, sectionWhereSymbolIsDefined);
    //all absolute symbols need a relocation entry
    patched=true;
  }

  void patchAddressRel(std::vector<std::string> valueForPatching, std::string symbolName, bool makeRelocEntry,
  std::string sectionWhereSymbolIsDefined){
    for(int i=0; i<valueForPatching.size(); i++)
    if(valueForPatching.size()>length) { //not allowed
      throw MyException("Greska u velicini patching koda");
      return;
    }
    else if (valueForPatching.size()<length) {
      for(int i=0; i<valueForPatching.size()-length; i++) {
        valueForPatching.insert(valueForPatching.begin(), "00");
      }
    }
    std::string hexString = "";
    hexString = valueForPatching.at(0) + "" + valueForPatching.at(1);
    int offset = hexToDecimal(hexString) - patchAddress -2; //-2 because of addend
    hexString = decimalToHex(std::to_string(offset));
    if(hexString.size()==1) hexString = "0" + hexString;
    std::vector<std::string> hexLC = std::vector<std::string>();
    int i=0;
    while(i<hexString.size()) {
    std::string myString(hexString, i, 2);
    hexLC.push_back(myString);
    i=i+2;
  }

    if(hexLC.size()>length) {
      throw MyException("greska u velicini patching koda");
      return;
    }
    else if (hexLC.size()<length) {
      for(int i=0; i<hexLC.size()-length; i++) {
        hexLC.insert(hexLC.begin(), "00");
      }
    }
    if(!isLittleEndian) {
    for(int i=0; i<length; i++) {
    if(!section->getSectionHex()->size()>=patchAddress+i-1) section->getSectionHex()->push_back("00");
    section->getSectionHex()->at(patchAddress+i) = hexLC.at(i);
    }
    }
    else { //treba da se patchuje word direktiva
    std::vector<std::string> newValueForPatching = std::vector<std::string>();
    newValueForPatching.insert(newValueForPatching.begin(),"00");
    newValueForPatching.insert(newValueForPatching.begin(),"00");
    newValueForPatching.at(0) = hexLC.at(1);
    newValueForPatching.at(1) = hexLC.at(0);
    for(int i=0; i<length; i++) {
    if(!section->getSectionHex()->size()>=patchAddress+i-1) section->getSectionHex()->push_back("00");
    section->getSectionHex()->at(patchAddress+i) = newValueForPatching.at(i);
    }
    }
    if(makeRelocEntry) //no relocation needed if symbol definition and symbol call are in same section
    createRelocationEntry(symbolName, patchAddress, relocType, isLittleEndian, sectionWhereSymbolIsDefined);
    patched=true;    
  }

  void createRelocationEntry(std::string symbolName, int offset, int relocType, bool isLittleEndian,
  std::string sectionWhereSymbolIsDefined) {
    Relocation reloc = Relocation(section->getSectionName(), symbolName, offset, relocType, isLittleEndian,
    sectionWhereSymbolIsDefined);
    section->addToRelocationTable(reloc);
  }

  int getRelocType() {
    return relocType;
  }

  std::string decimalToHex(std::string hexVal){
  std::stringstream iss(hexVal);
  std::stringstream hss;
	int num;
	iss >> num;
	hss <<std::uppercase << std::hex << num;
	hexVal = hss.str();
	
    if (hexVal.length() > 4) {
        hexVal = hexVal.substr(hexVal.length() - 4);
    }
    return hexVal;
}

  int hexToDecimal(std::string simb) {
  std::stringstream hss(simb);
  int decNum;
  hss << std::hex << simb;
  hss >> decNum;
  return decNum;
}

void createFinalRelocation(std::string symbolName, std::string sectionWhereSymbolIsDefined) {
    Relocation reloc = Relocation(section->getSectionName(), symbolName, patchAddress, relocType, isLittleEndian,
    sectionWhereSymbolIsDefined);
    section->addToRelocationTable(reloc);
}

};

#endif