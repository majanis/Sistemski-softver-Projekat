#ifndef SECTIONTABLE_HPP_
#define SECTIONTABLE_HPP_

#include "../inc/Symbol.hpp"
#include "../inc/Section.hpp"
#include <vector>
#include <string>
#include <sstream>


class SectionTable {
  private:
  std::vector<Section*> sectionTable;
  public:
  SectionTable() {
    sectionTable = std::vector<Section*>();
  }

  void addToSectionTable(Section* section) {
    sectionTable.push_back(section);
  }

  std::vector<Section*> getSectionTable() {return sectionTable;}

  Section* getSectionByName(std::string sectionName) {
  std::vector<Section*>::iterator iterator;
  iterator = sectionTable.begin();
  while(iterator!=sectionTable.end()) {
    if((*iterator)->getSectionName()==sectionName) return *iterator;
    iterator++;
  }
  return NULL;
  }

  bool sectionExists(std::string sectionName) {
  std::vector<Section*>::iterator iterator;
  iterator = sectionTable.begin();
  while(iterator!=sectionTable.end()) {
    if((*iterator)->getSectionName()==sectionName) return true;
    iterator++;
  }
  return false;
  }

  void printSectionTable(std::ostream& outfile) {
    outfile<<"******SECTION TABLE******"<<std::endl;
    for(int i=0; i<sectionTable.size(); i++) {
      if(sectionTable.at(i)->getSectionName()=="initial") continue;
      outfile<<"Name: "<<sectionTable.at(i)->getSectionName()<<" ";
      outfile<<"Size: "<<sectionTable.at(i)->getLocationCounter()<<std::endl;
      outfile<<"Content: "<<std::endl;
      sectionTable.at(i)->printSectionData(outfile);
      outfile<<std::endl;
    }
    outfile<<std::endl;
    for(int i=0; i<sectionTable.size(); i++) {
    if(sectionTable.at(i)->getSectionName()=="initial") continue;
    sectionTable.at(i)->printRelocationTable(outfile);
    }
    outfile<<std::endl;
  }

  ~SectionTable() {}; 
};

#endif