#include "../inc/linker.hpp"

Linker::Linker(std::vector<std::string> inputFiles, std::string outputFile) {
  this->inputFiles = inputFiles;
  this->outputFile = outputFile;
  out.open(outputFile, std::ios::out);
  try {
  if(out.fail()) throw MyException("GRESKA: Ne moze da otvori fajl " + outputFile);
  }
  catch(MyException& ex) {
  std::cout<<ex.cause()<<std::endl; //file exception
  }
  hexContent = std::vector<std::string>(); 
  linesVector = std::vector<std::string>();
  globalSymbolTable = SymbolTable();
  sectionTable = SectionTable();
  linkerSectionTable = LinkerSectionTable();
  globalLocationCounter = 0;
}

void Linker::startLinker() {
  for(int i=0; i<inputFiles.size(); i++) { //loads files in order given by command
  currentFile = inputFiles.at(i);
  fileLocalLocationCounter = 0;
  emptyVector(); //empties vector for each file
  loadLinesIntoVectorByFile(inputFiles.at(i));

  for(int i=0; i<linesVector.size(); i++) { //.o files have a format we load according to

    while(isspace(linesVector.at(i).at(0))) linesVector.at(i)=linesVector.at(i).erase(0,1);

    if(linesVector.at(i)==symbolTableBeginning) { //first, the symbol table
        i++;
        while(linesVector.at(i)!=sectionTableBeginning) {
        fillSymbolTable(i);
        i++;
    }
    }

    if(linesVector.at(i)==sectionTableBeginning) { //second, the section table
        i++;
        while(linesVector.at(i).find(relocationTableBeginning) == std::string::npos) {
        fillSectionTable(i, currentFile);
        i++;
      }
    }

    if(linesVector.at(i).find(relocationTableBeginning) != std::string::npos) { //third, the relocation tables
    std::string sectionName = linesVector.at(i);
    sectionName = sectionName.erase(linesVector.at(i).find(relocationTableBeginning), relocationTableBeginning.size());
    sectionName = sectionName.erase(sectionName.find("******")); //format of .o file
    currentSection = sectionTable.getSectionByName(sectionName);
    if(i==linesVector.size()-1) continue; //end of file
    if(linesVector.at(i+1).find(relocationTableBeginning) != std::string::npos) continue; //end of current relocation table
    i++;
      while(linesVector.at(i).find("Symbol") != std::string::npos) { //contents of relocation table
        fillRelocationTable(i, currentSection, currentFile);
        if(i==linesVector.size()-1) break;
        if(linesVector.at(i+1).find(relocationTableBeginning) != std::string::npos) break;
        i++;
      }
    }
  }
  }
  for(int i=0; i<sectionTable.getSectionTable().size(); i++) {
    addToHexCode(*sectionTable.getSectionTable().at(i)->getSectionHex());
  }
  updateOffsetsForLinkerSections();
  updateOffsetsForSymbolTable();
  solveRelocations();
try{
  noDefinitionOfExternSymbol(); //linker error can be an unresolved symbol
}
catch(MyException& ex){
  std::cout<<ex.cause()<<std::endl;
  const char* filename = outputFile.c_str();
  std::remove(filename); //in case of file error, delete hex file
}
}

void Linker::fillSymbolTable(int index) { //index of array that contains the symbol
     std::vector<std::string> symbolList = std::vector<std::string>();
      std::stringstream sstream(linesVector.at(index));
      while(sstream.good()) {
      std::string substr;
      getline(sstream, substr, ' '); //get first string delimited by comma
      while(isspace(substr.at(0))) substr.erase(0,1);
      while(isspace(substr.at(substr.size()-1))) substr.erase(substr.size()-1, 1);
      if(substr.find("Name:") != std::string::npos) continue; //index 0
      if(substr.find("Section:") != std::string::npos) continue; //1
      if(substr.find("Defined:") != std::string::npos) continue; //2
      if(substr.find("Type:") != std::string::npos) continue; //3
      if(substr.find("Value:") != std::string::npos) continue;//4
      symbolList.push_back(substr);
      } 
    bool defined = false;
    if(symbolList.at(2)=="true") defined=true;
    int type = 0;
    if(symbolList.at(3)=="global")  type=1;
    else if(symbolList.at(3)=="extern") type=2;
    Symbol symbol(symbolList.at(0), symbolList.at(1), hexToDec(symbolList.at(4)), defined, type);
    symbol.setFilename(currentFile);
    if(symbolList.at(3)=="global") {
    try {
    multipleDefinitionOfGlobalSymbol(symbolList.at(0)); //linker error can be multiple definitions of a global symbol in different object files
    }
    catch(MyException& ex) {
      std::cout<<ex.cause()<<std::endl;
      exit(-1);
    }
    }
    if(type!=0) { //only global symbols
    if(!globalSymbolTable.symbolExistsInTable(symbolList.at(0))) { //add symbol to table
    symbol.setFilename(currentFile);
    globalSymbolTable.addToSymbolTable(symbol);
    }
    else { //symbol with this name is in table
      if(symbol.isDefined() && symbol.getSymbolTypeDef()==1) { //only a defined global symbol can replace in table
      globalSymbolTable.removeSymbolByName(symbolList.at(0));
      globalSymbolTable.addToSymbolTable(symbol);
      }
    }
    }
}

void Linker::fillSectionTable(int index, std::string currentFile) {
if(linesVector.at(index).find("end of") != std::string::npos
|| linesVector.at(index).find("Content") != std::string::npos) return;

if(linesVector.at(index).find("Name: ") != std::string::npos) {
      std::vector<std::string> symbolList = std::vector<std::string>();
      std::stringstream sstream(linesVector.at(index));
      while(sstream.good()) {
        std::string substr;
        getline(sstream, substr, ' '); //get first string delimited by comma
        while(isspace(substr.at(0))) substr.erase(0,1);
        while(isspace(substr.at(substr.size()-1))) substr.erase(substr.size()-1, 1);
        symbolList.push_back(substr);
        }
      std::string sectionName = symbolList.at(1);
      int sectionSize = stoi(symbolList.at(3));
      if(sectionTable.sectionExists(sectionName)) { //we are adding to an existing section
        currentSection = sectionTable.getSectionByName(sectionName);
        LinkerSection* linkerSection = new LinkerSection(sectionName, currentFile); //linker section is only a piece of
        //a whole executable section, that comes from a specific object file
        linkerSection->setStartAddress(currentSection->getStartAddress() + currentSection->getLocationCounter());
        linkerSectionTable.addToSectionTable(linkerSection);
      }
      else {//we need a new section
        Section* section = new Section(sectionName);
        section->setStartAddress(globalLocationCounter);
        sectionTable.addToSectionTable(section);
        currentSection=section;

        LinkerSection* linkerSection = new LinkerSection(sectionName, currentFile);
        linkerSection->setStartAddress(section->getStartAddress());
        linkerSectionTable.addToSectionTable(linkerSection);
      }
}
else {//hex code we are adding to section
      std::vector<std::string> symbolList = std::vector<std::string>();
      std::stringstream sstream(linesVector.at(index));
      while(sstream.good()) {
        std::string substr;
        getline(sstream, substr, ' '); //get first string delimited by space
        if(substr.empty()) break;
        if(substr.size()==0) break;
        while(isspace(substr.at(0))) substr.erase(0,1);
        while(isspace(substr.at(substr.size()-1))) substr.erase(substr.size()-1, 1);
        if(std::regex_match(substr, blankLine)) break;
        symbolList.push_back(substr); //creates a hex code vector
      }
      for(int i=0; i<symbolList.size(); i++) {
        currentSection->addToSection(symbolList.at(i));
        currentSection->incLocationCounter();
        globalLocationCounter++;
        fileLocalLocationCounter++;
      }
      updateSectionStartAddresses(currentSection, symbolList.size()); //sections following should be moved the length
      //of this hex code
}
}

void Linker::fillRelocationTable(int index, Section* section, std::string currentFile) {
  std::vector<std::string> symbolList = std::vector<std::string>();
      std::stringstream sstream(linesVector.at(index));
      while(sstream.good()) {
        std::string substr;
        getline(sstream, substr, ' '); //get first string delimited by comma
        while(isspace(substr.at(0))) substr.erase(0,1);
        while(isspace(substr.at(substr.size()-1))) substr.erase(substr.size()-1, 1);
        symbolList.push_back(substr);
        }
      std::string symbolName = symbolList.at(1);
      std::string sectionName = symbolList.at(3);
      int offset = hexToDec(symbolList.at(5));
      int type = 1;
      if(symbolList.at(7)=="ABS") type = 0;
      int isLittleEndian = stoi(symbolList.at(9));
      std::string sectionWhereSymbolIsDefined = symbolList.at(11);
      Relocation reloc = Relocation(sectionName, symbolName, offset, type, isLittleEndian, sectionWhereSymbolIsDefined);
      try {
      if(!linkerSectionTable.sectionExists(sectionName, currentFile)) {
      throw MyException("GRESKA pronalaska linker sekcije");
      }
      linkerSectionTable.getSectionByNameAndFile(sectionName, currentFile)->addToRelocationTable(reloc);
      }
      catch(MyException& ex) {
        std::cout<<ex.cause()<<std::endl;
      }
}

void Linker::loadLinesIntoVectorByFile(std::string inputFile) {
  try {
  in.open(inputFile, std::ios::in);
  if(in.fail()) throw MyException("GRESKA: Ne moze da otvori fajl " + inputFile);
  std::string line;
  while(getline(in, line)) {
    if(regex_match(line, blankLine)) continue;
    linesVector.push_back(line);
  }
  in.close();
  in.clear();
  }
  catch(MyException& ex) {
  std::cout<<ex.cause()<<std::endl;
  }

}

void Linker::multipleDefinitionOfGlobalSymbol(std::string symbolName) {
  Symbol* symbol = globalSymbolTable.getSymbolByName(symbolName);
  if(symbol!=NULL) {
    if(symbol->getSymbolTypeDef()==1) { //globalni
      if(symbol->isDefined()==true) throw MyException("GRESKA: visestruka definicija globalnog simbola " + symbolName);
    }
  }
}

void Linker::noDefinitionOfExternSymbol() {
  for(int i=0; i<globalSymbolTable.getSymbolTable().size(); i++) {
    Symbol symbol = globalSymbolTable.getSymbolTable().at(i);
    if(symbol.isDefined()==false) throw MyException("GRESKA: simbol " 
    + symbol.getSymbolName() + " nije definisan ni u jednom fajlu");
  }
}

void Linker::updateSectionStartAddresses(Section* currentSection, int increment){
  int startIndex = 0;
  for(int i=0; i<sectionTable.getSectionTable().size(); i++) {
    if(sectionTable.getSectionTable().at(i)==currentSection) {
      startIndex = i;
      break;
    }
  }
  if(startIndex==sectionTable.getSectionTable().size()-1) {
    return;
  }
  for(int j=startIndex+1; j<sectionTable.getSectionTable().size(); j++) { //updates all following sections
    Section* section = sectionTable.getSectionTable().at(j);
    section->setStartAddress(section->getStartAddress()+increment);
    for(int k =0; k<linkerSectionTable.getSectionTable().size(); k++) { //and linker-sections
      if(linkerSectionTable.getSectionTable().at(k)->getSectionName()!=section->getSectionName()) continue;
      LinkerSection* linkerSection = linkerSectionTable.getSectionTable().at(k);
      linkerSection->setStartAddress(linkerSection->getStartAddress()+increment);
    }
  }
}

void Linker::addToHexCode(std::vector<std::string> hexCodeArray) {
  for(int i=0; i<hexCodeArray.size(); i++) {
    hexContent.push_back(hexCodeArray.at(i));
  }
}

void Linker::emptyVector() {
  linesVector.clear();
}

void Linker::hexCommandPrint() {
  for(int i=0; i<globalLocationCounter; i++) {
    if(i%8==0) out<<formatHexOutput(decimalToHex(std::to_string(i)))<<": ";
    out<<hexContent.at(i)<<" ";
    if((i+1)%8==0) out<<std::endl; //formatting
  }
  out<<std::endl;
}

std::string Linker::decimalToHex(std::string hexVal){
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

std::string Linker::formatHexOutput(std::string input) {
  std::string output="";
  if(input.size()==1) output = "000" + input;
  else if(input.size()==2) output = "00" + input;
  else if(input.size()==3) output="0" + input;
  else output=""+input;
  return output;
}

void Linker::updateOffsetsForLinkerSections() {
  for(int i=0; i<linkerSectionTable.getSectionTable().size(); i++) {
    LinkerSection* linkerSection = linkerSectionTable.getSectionTable().at(i);
    for(int j=0; j<linkerSection->getRelocationTable().getRelocationTable().size(); j++) {
      linkerSection->updateRelocationTable(j, linkerSection->getStartAddress());
    }
  }
}

void Linker::updateOffsetsForSymbolTable() {
  for(int i=0; i<globalSymbolTable.getSymbolTable().size(); i++) {
    LinkerSection* linkerSection=NULL;
    std::string symbolName=globalSymbolTable.getSymbolTable().at(i).getSymbolName();
    std::string sectionName=globalSymbolTable.getSymbolTable().at(i).getSymbolSection();
    std::string fileName=globalSymbolTable.getSymbolTable().at(i).getFilename();
    for(int j=0; j<linkerSectionTable.getSectionTable().size(); j++) {
      linkerSection = linkerSectionTable.getSectionTable().at(j);
      if(sectionName!=linkerSection->getSectionName()) continue;
      if(fileName!=linkerSection->getFilename()) continue;
      int value = globalSymbolTable.getSymbolTable().at(i).getSymbolValue();
      globalSymbolTable.updateSymbolOffset(i, value + linkerSection->getStartAddress()); //new value is old value + offset of linker section
    }
  }
}

void Linker::solveRelocations() {
  for(int i=0; i<linkerSectionTable.getSectionTable().size(); i++) { //iterate through all relocations in each linker-section
      int linkerSectionStartAddress = linkerSectionTable.getSectionTable().at(i)->getStartAddress();
      currentFile = linkerSectionTable.getSectionTable().at(i)->getFilename();
      std::string linkerSectionName = linkerSectionTable.getSectionTable().at(i)->getSectionName();
    for(int j=0; j<linkerSectionTable.getSectionTable().at(i)->getRelocationTable().getRelocationTable().size(); j++) {
      std::string sectionNameDef = linkerSectionTable.getSectionTable().at(i)->
      getRelocationTable().getRelocationTable().at(j).getSectionWhereDef();
      Section* sectionWhereSymbDefined = sectionTable.getSectionByName(sectionNameDef);
      int relocationOffset =linkerSectionTable.getSectionTable().at(i)->
      getRelocationTable().getRelocationTable().at(j).getOffset();
      int isRelocLittleEndian = linkerSectionTable.getSectionTable().at(i)->
      getRelocationTable().getRelocationTable().at(j).getIsLittleEndian();
      int relocRelocType = linkerSectionTable.getSectionTable().at(i)->
      getRelocationTable().getRelocationTable().at(j).getRelocType();
      std::string symbolName = linkerSectionTable.getSectionTable().at(i)->
      getRelocationTable().getRelocationTable().at(j).getSymbolName();
      if(globalSymbolTable.symbolExistsInTable(symbolName)) { //in case of global symbol, replace the hex zeroes with value
        std::string symbolVal = formatHexOutput(decimalToHex(std::to_string(globalSymbolTable.getSymbolByName(symbolName)->getSymbolValue())));
        std::vector<std::string> hexLC = std::vector<std::string>(); //symbol value is always 2 bytes long
        std::string firstHalf = symbolVal.substr(0,2);
        std::string secondHalf = symbolVal.substr(2,2);
        hexLC.push_back(firstHalf);
        hexLC.push_back(secondHalf);
        if(relocRelocType==0) patchAbs(hexLC, relocationOffset, isRelocLittleEndian);
        else patchRel(globalSymbolTable.getSymbolByName(symbolName)->getSymbolValue(), relocationOffset,
        isRelocLittleEndian); //patch according to relocation type
      }
      else { //symbol is local, the value is hex value in code + start address of linker section where the relocation is referenced
        std::vector<std::string> content = std::vector<std::string>();
        content.push_back(hexContent.at(relocationOffset));
        content.push_back(hexContent.at(relocationOffset+1));
        int value = hexToDec("" + content.at(0) + content.at(1));
        value += linkerSectionTable.getSectionByNameAndFile(sectionNameDef, currentFile)->getStartAddress();
        std::string valueString = formatHexOutput(decimalToHex(std::to_string(value)));
        std::vector<std::string> hexLC = std::vector<std::string>();
        std::string firstHalf = valueString.substr(0,2);
        std::string secondHalf = valueString.substr(2,2);
        hexLC.push_back(firstHalf);
        hexLC.push_back(secondHalf);
        if(relocRelocType==0) patchAbs(hexLC, relocationOffset, isRelocLittleEndian);
        else patchRel(value, relocationOffset, isRelocLittleEndian); //patch according to relocation type
    }
    }
  }
}

void Linker::patchAbs(std::vector<std::string> code, int startAddress, int isLittleEndian) {
    if(!isLittleEndian) {
    for(int i=0; i<code.size(); i++) {
    if(!hexContent.size()>=startAddress+i-1) hexContent.push_back("00");
    hexContent.at(startAddress+i) = code.at(i);
    }
    }
    else { //patching a word directive
    std::vector<std::string> newValueForPatching = std::vector<std::string>();
    newValueForPatching.insert(newValueForPatching.begin(),"00");
    newValueForPatching.insert(newValueForPatching.begin(),"00");
    newValueForPatching.at(0) = code.at(1);
    newValueForPatching.at(1) = code.at(0); //swap order of bytes
    for(int i=0; i<code.size(); i++) {
    if(!hexContent.size()>=startAddress+i-1) hexContent.push_back("00");
    hexContent.at(startAddress+i) = newValueForPatching.at(i);
    }
    }
}

void Linker::patchRel(int symbolValue, int startAddress, int isLittleEndian) {
  int codeToAdd = symbolValue - startAddress - 2; //-2 is addend
  std::string valueString = decimalToHex(std::to_string(codeToAdd));
  std::vector<std::string> hexLC = std::vector<std::string>();
  std::string firstHalf = valueString.substr(0,2);
  std::string secondHalf = valueString.substr(2,2);
  hexLC.push_back(firstHalf);
  hexLC.push_back(secondHalf);
  patchAbs(hexLC, startAddress, isLittleEndian); //the rest is the same
}

int Linker::hexToDec(std::string ourString) {
if(ourString=="0") return 0;
short s = 0;
s=std::stoi(ourString, NULL, 16);
if(s>32767) { //we want a signed decimal value
  s = ~s + 1;
  s = -s;
}
return s;
}
