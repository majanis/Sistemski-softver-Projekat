#ifndef SYMBOL_HPP_
#define SYMBOL_HPP_

#include <stdlib.h>
#include <string>
#include "../inc/FCTEntry.hpp"
#include <vector>

class Symbol {
  private:
  std::string symbolName;
  std::string symbolSection;
  int symbolValue;
  bool symbolDefined;
  int symbolTypeDef; //0 local, 1 global, 2 extern
  std::vector <FCTEntry> symbolFCT = std::vector<FCTEntry>();
  std::string symbolFileName="";
  
  public:

  bool isDefined() {
    return symbolDefined;
  }

  void setDefined(bool symbolDefined) {
    this->symbolDefined=symbolDefined;
  }

  Symbol(std::string symbolName, std::string symbolSection, int symbolValue, bool symbolDefined, int symbolTypeDef){
    this->symbolName=symbolName;
    this->symbolSection=symbolSection;
    this->symbolValue=symbolValue;
    this->symbolDefined=symbolDefined;
    this->symbolTypeDef=symbolTypeDef;
    this->symbolFCT = std::vector<FCTEntry>();
  }

  Symbol(std::string symbolName, std::string symbolSection, int symbolTypeDef) {
    this->symbolName=symbolName;
    this->symbolSection=symbolSection;
    this->symbolTypeDef=symbolTypeDef;
    this->symbolDefined=false;
    this->symbolValue=0;
    this->symbolFCT = std::vector<FCTEntry>();
  }

  std::string getSymbolName() {
    return symbolName;
  }

  std::string getSymbolSection() {
    return symbolSection;
  }

  void setSymbolSection(std::string sectionName) {
    symbolSection = sectionName;
  }

  int getSymbolTypeDef() {
    return symbolTypeDef;
  }

  Symbol* getObject() {
    return this;
  }

  void setSymbolValue(int symbolValue) {
    this->symbolValue=symbolValue;
  }

  int getSymbolValue() {
    return symbolValue;
  }

  void addFCTEntry(int patchAddress, Section* section, int length, int relocType, bool isLittleEndian) { 
  symbolFCT.push_back(FCTEntry(patchAddress, section, length, relocType, isLittleEndian));
  }

  void addFCTEntry(FCTEntry entry) {
  symbolFCT.push_back(entry);
  }

  void backpatching(std::vector<std::string> valueForPatching, std::string backpatchSection) {
  std::vector<FCTEntry>::iterator iterator;
  iterator = symbolFCT.begin();
  
  while (!(iterator==symbolFCT.end())) {
    if(iterator->getRelocType()==0) //absolute
    iterator->patchAddressAbs(valueForPatching, this->getSymbolName(), symbolSection);
    else { //pc relative
    bool makeRelocEntry = false;
    if(backpatchSection!=symbolSection) makeRelocEntry=true;
    iterator->patchAddressRel(valueForPatching, this->getSymbolName(), makeRelocEntry, symbolSection);
    }
    iterator++;
  }
  setDefined(true);
  }

  int getFCTSize() {
    return symbolFCT.size();
  }

  void setFilename(std::string filename) {
    symbolFileName=filename;
  }

  std::string getFilename() {return symbolFileName;}

  void createRelocations() {
    for(int i=0; i<symbolFCT.size(); i++) {
      if(symbolFCT.at(i).getPatched()==false){
        symbolFCT.at(i).createFinalRelocation(symbolName, symbolSection);
      }
    }
  }

};

#endif