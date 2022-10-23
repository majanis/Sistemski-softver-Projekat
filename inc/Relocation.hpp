
#ifndef RELOCATION_HPP_
#define RELOCATION_HPP_

#include <string>
#include <sstream>
#include <vector>

class Relocation {
  std::string section;
  std::string symbol;
  std::string sectionWhereSymbolIsDefined;
  int offset;
  int relocType; //abs or pcrel
  bool isLittleEndian;
  bool isLinked;
  std::string filename = "";
  public:

  Relocation(std::string section, std::string symbol, int offset, int relocType, bool isLittleEndian, std::string
  sectionWhereSymbolIsDefined) {
    this->section = section;
    this->symbol = symbol;
    this->offset = offset;
    this->relocType = relocType;
    this->isLittleEndian = isLittleEndian;
    this->sectionWhereSymbolIsDefined = sectionWhereSymbolIsDefined;
    isLinked=false;
  }

  void printRelocation(std::ostream& outfile) {
    std::string typeRel = "ABS";
    if(relocType==1) typeRel = "PCREL";
    outfile<<"Symbol: "<<symbol<<" ";
    outfile<<"Section: "<<section<<" ";
    outfile<<"Offset: "<<decimalToHex(std::to_string(offset))<<" ";
    outfile<<"Type: "<<typeRel<<" ";
    outfile<<"isLittleEndian: "<<isLittleEndian<<" ";
    if(sectionWhereSymbolIsDefined!="initial") //17.8.
    outfile<<"DefinedIn: "<<sectionWhereSymbolIsDefined<<std::endl;
    else outfile<<"DefinedIn: "<<"/"<<std::endl;
  }

  std::string getFilename() {return filename;}

  void setFilename(std::string filename) {this->filename=filename;}

  void incrementOffset(int increment) {
    offset+=increment;
  }

  std::string getSectionWhereDef() {return sectionWhereSymbolIsDefined;}

  int getOffset() {return offset;}

  int getRelocType() {return relocType;}
  
  int getIsLittleEndian() {return isLittleEndian;}

  void setLinked() {isLinked=true;}

  bool getLinked() {return isLinked;}

  std::string getSymbolName() {return symbol;}

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
};

#endif