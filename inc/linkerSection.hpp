#ifndef LINKERSECTION_HPP_
#define LINKERSECTION_HPP_

#include <string>
#include <vector>
#include <iostream>
#include "../inc/RelocationTable.hpp"

class LinkerSection { //sections from .o files that linker merges
//analogous to section class with added information

  private:
  std::string sectionName;
  std::string originFilename;
  std::vector<std::string> sectionHex= std::vector<std::string>();
  int locationCounter;
  RelocationTable relocationTable;
  int startAddress; //where this starts in merged section

  public:

  LinkerSection(std::string sectionName, std::string originFileName):relocationTable(sectionName) {
    locationCounter = 0;
    this->sectionName=sectionName;
    this->originFilename=originFileName;
    startAddress = 0;
  }

  void setStartAddress(int address) {
    startAddress = address;
    }

  int getStartAddress() {
    return startAddress;
    }

  std::string getFilename() {
    return originFilename;
    }

  void setFilename(std::string filename) {
    originFilename=filename;
    }

  std::string getSectionName() {
    return sectionName;
    }

  void incLocationCounter() {
    locationCounter++;
    }

  int getLocationCounter() {
    return locationCounter;
    }

  void addToSection(std::string hexcode){
    sectionHex.push_back(hexcode);
  }

  std::vector<std::string>* getSectionHex() {
    return &sectionHex;
    }

  int getSectionSize() {
    return sectionHex.size();
  }

  RelocationTable getRelocationTable() {
    return relocationTable;
  }

  void addToRelocationTable(Relocation reloc) {
    relocationTable.addRelocation(reloc);
  }

  void updateRelocationTable(int index, int increment){
    relocationTable.updateRelocationOffset(index, increment);
  }
};

class LinkerSectionTable {
  //analogous to section table class
  
  private:
  std::vector<LinkerSection*> sectionTable;

  public:
  LinkerSectionTable() {
    sectionTable = std::vector<LinkerSection*>();
  }

  void addToSectionTable(LinkerSection* section) {
    sectionTable.push_back(section);
  }

  std::vector<LinkerSection*> getSectionTable() {
    return sectionTable;
    }

  LinkerSection* getSectionByNameAndFile(std::string sectionName, std::string fileName) {
  std::vector<LinkerSection*>::iterator iterator;
  iterator = sectionTable.begin();
  while(iterator!=sectionTable.end()) {
    if((*iterator)->getSectionName()==sectionName && (*iterator)->getFilename()==fileName) return *iterator;
    iterator++;
  }
  return NULL;
  }

  bool sectionExists(std::string sectionName, std::string filename) {
  std::vector<LinkerSection*>::iterator iterator;
  iterator = sectionTable.begin();
  while(iterator!=sectionTable.end()) {
    if((*iterator)->getSectionName()==sectionName && (*iterator)->getFilename()==filename) return true;
    iterator++;
  }
  return false;
  }

  ~LinkerSectionTable() {}; //da li je neophodno ako nemam new? da li ove strukture treba da se dealociraju

};

#endif