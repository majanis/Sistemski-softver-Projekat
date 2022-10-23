#ifndef RELOCATIONTABLE_HPP_
#define RELOCATIONTABLE_HPP_

#include <string>
#include "../inc/Relocation.hpp"
#include <sstream>
#include <iostream>

class RelocationTable{
  std::string section;
  std::vector<Relocation> relocationTable;
  public:

  RelocationTable(std::string section){
    this->section = section;
    relocationTable = std::vector<Relocation>();
  }
  std::vector<Relocation> getRelocationTable(){
    return relocationTable;
  }
  void addRelocation(Relocation relocation){
    relocationTable.push_back(relocation);
  }

  void printRelocationTable(std::ostream& outfile) {
    outfile<<"******RELOCATION TABLE FOR "<<section<<"******"<<std::endl;
    for(int i=0; i<relocationTable.size(); i++) {
      relocationTable.at(i).printRelocation(outfile);
    }
    outfile<<std::endl;
  }

  void updateRelocationOffset(int indexOfRelocation, int offset) {
    relocationTable.at(indexOfRelocation).incrementOffset(offset);
  }

};

#endif
