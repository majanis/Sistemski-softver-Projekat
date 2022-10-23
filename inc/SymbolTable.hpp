#ifndef SYMBOLTABLE_HPP_
#define SYMBOLTABLE_HPP_

#include "../inc/Symbol.hpp"
#include <vector>
#include <sstream>

class SymbolTable {
  private:
  std::vector<Symbol> symbolTable;
  public:
  SymbolTable() {
    symbolTable = std::vector<Symbol>();
  }
  void addToSymbolTable(Symbol symbol) {
    symbolTable.push_back(symbol);
  }

  bool symbolExistsInTable(std::string symbolName) {
  std::vector<Symbol>::iterator iterator;
  iterator = symbolTable.begin();
  while(iterator!=symbolTable.end()) {
    if(iterator->getSymbolName()==symbolName) return true;
    iterator++;
  }
  return false;
  }

  Symbol* getSymbolByName(std::string symbolName) {
  std::vector<Symbol>::iterator iterator;
  iterator = symbolTable.begin();
  while(iterator!=symbolTable.end()) {
    if(iterator->getSymbolName()==symbolName) return iterator->getObject();
    iterator++;
  }
  return NULL;

  }

  std::vector<Symbol> getSymbolTable() {
    return symbolTable;
  }

  void removeSymbolByName(std::string symbolName) {
  int index = 0;
  for(int i=0; i<symbolTable.size(); i++) {
    if(symbolTable.at(i).getSymbolName()==symbolName) {
      index = i;
      break;
    }
  }
  symbolTable.erase(symbolTable.begin() + index);
  }

  void printSymbolTable(std::ostream& outfile) {
    outfile<<"******SYMBOL TABLE******"<<std::endl;
    for(int i=0; i<symbolTable.size(); i++) {
      outfile<<"Name: "<<symbolTable.at(i).getSymbolName();
      outfile<<" Section:";
      if(symbolTable.at(i).getSymbolSection()=="initial") outfile<<" /";
      else outfile<<" "<<symbolTable.at(i).getSymbolSection();
      outfile<<" Defined: ";
      if(symbolTable.at(i).isDefined()==1)
      outfile<<"true ";
      else outfile<<"false ";
      outfile<<"Type: ";
      if(symbolTable.at(i).getSymbolTypeDef()==0)
      outfile<<"local ";
      else if(symbolTable.at(i).getSymbolTypeDef()==1)
      outfile<<"global ";
      else outfile<<"extern ";
      outfile<<"Value:";
      outfile<<" "<<decimalToHex(std::to_string(symbolTable.at(i).getSymbolValue()));
      outfile<<std::endl;
    }
    outfile<<std::endl;
  }
  ~SymbolTable() {};

  std::string decimalToHex(std::string simb){
  std::stringstream iss(simb);
	int br;
	iss >> br;
	std::stringstream hss;
	hss <<std::uppercase << std::hex << br;
	simb = hss.str();
    if (simb.length() > 4) {
        simb = simb.substr(simb.length() - 4);
    }
    return simb;
  }

  void updateSymbolOffset(int indexOfSymbol, int newValue) {
    symbolTable.at(indexOfSymbol).setSymbolValue(newValue);
  }

  void createFinalRelocationsForAssembler() {
  for(int i=0; i<symbolTable.size(); i++) {
    symbolTable.at(i).createRelocations();
  }
  }
};

#endif