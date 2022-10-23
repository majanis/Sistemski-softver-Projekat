#ifndef SECTION_HPP_
#define SECTION_HPP_

#include <string>
#include <vector>
#include "../inc/RelocationTable.hpp"
#include <sstream>

class Section {

  private:
  std::string sectionName;
  std::vector<std::string> sectionHex= std::vector<std::string>();
  int locationCounter;
  RelocationTable relocationTable;
  int startAddress;

  public:
  Section(std::string sectionName):relocationTable(sectionName) {
    locationCounter = 0; //counts bytes
    this->sectionName=sectionName;
    startAddress = 0; //each section starts at zero
  }

  void setStartAddress(int address) {startAddress = address;}

  int getStartAddress() {return startAddress;}

  std::string getSectionName() {return sectionName;}

  void incLocationCounter() {locationCounter++;}
  
  int getLocationCounter() {return locationCounter;}
  //Section* getObject() {return this;}

  void addToSection(std::string hexcode){
    sectionHex.push_back(hexcode);
  }

  std::vector<std::string>* getSectionHex() {return &sectionHex;}

  void printSectionData(std::ostream& outFile) {
    for(int i=0; i<sectionHex.size(); i++) {
      outFile<<sectionHex.at(i)<<" ";
      if((i+1)%20==0 && i!=0) outFile<<std::endl;
      }
    if(sectionHex.size()!=0) outFile<<std::endl;
    outFile<<"end of "<<sectionName<<std::endl;
  }

  void printRelocationTable(std::ostream& outfile) {
    relocationTable.printRelocationTable(outfile);
  }

  RelocationTable getRelocationTable() {
    return relocationTable;
  }

  void addToRelocationTable(Relocation reloc) {
    relocationTable.addRelocation(reloc);
  }

  void printSection() {
    std::cout<<"SEKCIJA"<<std::endl;
    std::cout<<sectionName<<std::endl;
    std::cout<<locationCounter<<std::endl;
    for(int i=0; i<sectionHex.size(); i++) std::cout<<sectionHex.at(i)<<" ";
    std::cout<<std::endl;
  }
};

#endif