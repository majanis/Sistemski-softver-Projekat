#include "../inc/assembler.hpp"

Assembler::Assembler(std::string inputFileName, std::string outputFileName) {
  symbolTable = SymbolTable();
  sectionTable = SectionTable();
  currentSection = new Section("initial");
  sectionTable.addToSectionTable(currentSection);
  try{
  infile.open(inputFileName, std::ios::in);
  outfile.open(outputFileName, std::ios::out);
  if(outfile.fail()) throw MyException("Nije uspesno otvorio fajl " + outputFileName);
  if(infile.fail()) throw MyException("Nije uspesno otvorio fajl " + inputFileName);
  }
  catch(MyException& ex) {
      std::cout<<ex.cause()<<std::endl;
        const char* filename = outputFileName.c_str();
        std::remove(filename);
  }
}

Assembler::~Assembler() {
  currentSection = NULL;
  infile.close();
  outfile.close();
}

bool Assembler::isDirective(std::string string) {
  if(regex_match(string, global_directive)) return true;
  if(regex_match(string, extern_directive)) return true;
  if(regex_match(string, word_directive)) return true;
  if(regex_match(string, skip_directive)) return true;
  if(regex_match(string, section_directive)) return true;
  if(string.find(".end")!=std::string::npos) return true;
  return false;
}

void Assembler::loadLinesIntoVector() {
    std::string line;
    while(getline(infile, line)) {
    if(regex_match(line, blankLine)) continue;
    while(line.at(0)==' ') line = line.erase(0, 1);
    if(line.at(0)!='#') {
      if(line.find('#') != std::string::npos) { //removes same-line comment and adds only valid assembly code
        std::string substring = line.substr(0, line.find('#'));
        while(substring.at(substring.length()-1)==' ') substring = substring.substr(0, substring.length()-1);
        linesFromFile.push_back(substring);
      }
      else if(regex_search(line, isLabelDeclaration)) {
        while(line.at(0)==' ') line = line.erase(0, 1);
        while(line.at(line.length()-1)==' ') line = line.substr(0, line.length()-1);
        std::string substring = line.substr(0, line.find(":")+1);
        linesFromFile.push_back(substring);
        if(line!=substring) { //contains both label declaration and instruction
        substring = line.substr(line.find(":")+1, line.size());
        if(isspace(substring.at(0))) substring=substring.erase(0,1);
        linesFromFile.push_back(substring);
        }
      }
    else {
        while(line.at(line.length()-1)==' ') line = line.substr(0, line.length()-1);
      linesFromFile.push_back(line);
    }
    }
    if(!line.compare(".end")) break; //reached end of file
  }
}

void Assembler::globalDirective(std::string ourString) {
      std::string myList = ourString.erase(0, 7); //deletes 'global'
      std::vector<std::string> symbolList = std::vector<std::string>();
      std::stringstream sstream(myList);
      while(sstream.good()) {
      std::string substr;
      getline(sstream, substr, ','); //get first string delimited by comma
      if(substr.at(0)==' ') substr.erase(0,1);
      symbolList.push_back(substr);
      }
      for(int i=0; i<symbolList.size(); i++) {
        if(symbolTable.getSymbolByName(symbolList.at(i))!=NULL) {
          throw MyException("visestruka definicija simbola " + symbolList.at(i)); //multiple definitions of same symbol
        }
        createSymbol(symbolList.at(i), currentSection->getSectionName(), 1);
      }
}

void Assembler::externDirective(std::string ourString) {
      std::string myList = ourString.erase(0, 7); //deletes 'extern'
      std::vector<std::string> symbolList = std::vector<std::string>();
      std::stringstream sstream(myList);
      while(sstream.good()) {
      std::string substr;
      getline(sstream, substr, ','); //get first string delimited by comma
      if(substr.at(0)==' ') substr.erase(0,1);
      symbolList.push_back(substr);
      }
      for(int i=0; i<symbolList.size(); i++) {
          if(symbolTable.getSymbolByName(symbolList.at(i))!=NULL) {
          throw MyException("visestruka definicija simbola " + symbolList.at(i)); //multiple definitions of same symbol
          }
        createSymbol(symbolList.at(i), currentSection->getSectionName(), 2);
      }
}

void Assembler::skipDirective(Section* section, std::string ourString) {
      std::string myList = ourString.erase(0, 5); //deletes 'skip'
      int number;
      number=stoi(myList);
      if(number<0) { //number of skips cannot be less than zero
        throw MyException("Ne moze da se pozove skip direktiva sa negativnim brojem!");
      }
      for(int i=0; i<number; i++) {
      section->addToSection("00");
      section->incLocationCounter();
      }
}

void Assembler::wordDirective(Section* section, std::string ourString) {//word directive is little endian
      std::string myList = ourString.erase(0, 5); //deletes 'word'
      std::vector<std::string> symbolList = std::vector<std::string>();
      std::stringstream sstream(myList);
      while(sstream.good()) {
      std::string substr;
      getline(sstream, substr, ','); //get first string delimited by comma
      if(substr.at(0)==' ') substr.erase(0,1);
      symbolList.push_back(substr);
      }
      for(int i=0; i<symbolList.size(); i++) { //iterates through word directive list
      if(symbolList.at(i)=="0") {
        addHexCodeToSection(section, "0000");
      }
      else if(isHexNumber(symbolList.at(i))) {
          std::string myHexNumber = symbolList.at(i).erase(0, 2); //deletes '0x'
          if(myHexNumber.size()>4) { //argument represents 2 bytes
          throw MyException("predugacak arg za .word direktivu");
          continue;
          }
          addHexCodeLittleEndian(section, myHexNumber);
        }
        else if (isDecimalNumber(symbolList.at(i))) {
          std::string myHexNumber = decimalToHex(symbolList.at(i));
          if(myHexNumber.size()>4) { //argument can only be represented by 2 bytes
          throw MyException("predugacak arg za .word direktivu");
          continue;
        }
        addHexCodeLittleEndian(section, myHexNumber);
        }

      else {
        if(!symbolTable.symbolExistsInTable(symbolList.at(i))) {
          createSymbol(symbolList.at(i), section->getSectionName(), 0); //by default, the symbol is local
        }
        Symbol* symbolPointer = symbolTable.getSymbolByName(symbolList.at(i));
        if(!symbolPointer->isDefined()) {
          symbolPointer->addFCTEntry(section->getLocationCounter(), section, 2, 0, true);
          addHexCodeToSection(currentSection, "0000");
        }
        else {
          if(symbolPointer->getSymbolTypeDef()==0) { //local symbol
          std::string decimalValue = std::to_string((symbolPointer->getSymbolValue()));
          std::string myHexNumber = decimalToHex(decimalValue);
          Relocation reloc = Relocation(currentSection->getSectionName(), symbolPointer->getSymbolName(),
          currentSection->getLocationCounter(), 0, true, symbolPointer->getSymbolSection());
          currentSection->addToRelocationTable(reloc);
          if(myHexNumber.size()>4) { //argument can only be represented by 2 bytes
          throw MyException("predugacak arg za .word direktivu");
          continue;
          }
          addHexCodeLittleEndian(section, myHexNumber);
        }
        else { //global or extern symbol
          Relocation reloc = Relocation(currentSection->getSectionName(), symbolPointer->getSymbolName(),
          currentSection->getLocationCounter(), 0, true, symbolPointer->getSymbolSection());
          currentSection->addToRelocationTable(reloc);
          addHexCodeToSection(currentSection, "0000");
        }
        }
      }
      }
}

void Assembler::analyzeAssemblyCode() {
  loadLinesIntoVector();
  std::vector<std::string>::iterator iterator;
  iterator = linesFromFile.begin();
  while(iterator!=linesFromFile.end()) {
    std::string ourString = *iterator;
    if(isDirective(*iterator)) {
    if(ourString.find(".global")!=std::string::npos) globalDirective(ourString);
    if(ourString.find(".extern")!=std::string::npos) externDirective(ourString);
    if(ourString.find(".section")!=std::string::npos) createSection(ourString);
    if(ourString.find(".skip")!=std::string::npos) skipDirective(currentSection, ourString);
    if(ourString.find(".word")!=std::string::npos) wordDirective(currentSection, ourString);
    }
    else if (isInstruction(*iterator)) {
      if(ourString.find("halt")!=std::string::npos) haltInstruction("00");
      if(ourString.find("ret")!=std::string::npos 
      && ourString.find("iret")==std::string::npos) haltInstruction("40");
      if(ourString.find("iret")!=std::string::npos) haltInstruction("20");
      if(ourString.find("int")!=std::string::npos) intInstruction(ourString);
      if(ourString.find("not")!=std::string::npos) notInstruction(ourString);
      if(ourString.find("and")!=std::string::npos) andInstruction(ourString, "81");
      if(ourString.find("or")!=std::string::npos 
      && ourString.find("xor")==std::string::npos) orInstruction(ourString);
      if(ourString.find("xor")!=std::string::npos) andInstruction(ourString, "83");
      if(ourString.find("test")!=std::string::npos) testInstruction(ourString);
      if(ourString.find("add")!=std::string::npos) addInstruction(ourString, "70");
      if(ourString.find("sub")!=std::string::npos) addInstruction(ourString, "71");
      if(ourString.find("mul")!=std::string::npos) addInstruction(ourString, "72");
      if(ourString.find("div")!=std::string::npos) addInstruction(ourString, "73");
      if(ourString.find("cmp")!=std::string::npos) addInstruction(ourString, "74");
      if(ourString.find("shl")!=std::string::npos) shiftLeftInstruction(ourString, "90");
      if(ourString.find("shr")!=std::string::npos) shiftLeftInstruction(ourString, "91");
      if(ourString.find("pop")!=std::string::npos) popInstruction(ourString);
      if(ourString.find("push")!=std::string::npos) pushInstruction(ourString);
      if(ourString.find("xchg")!=std::string::npos) xchgInstruction(ourString);
      if(ourString.find("jmp")!=std::string::npos) jmpInstruction(ourString, "50");
      if(ourString.find("jeq")!=std::string::npos) jmpInstruction(ourString, "51");
      if(ourString.find("jne")!=std::string::npos) jmpInstruction(ourString, "52");
      if(ourString.find("jgt")!=std::string::npos) jmpInstruction(ourString, "53");
      if(ourString.find("call")!=std::string::npos) jmpInstruction(ourString, "30");
      if(ourString.find("ldr")!=std::string::npos) ldstInstruction(ourString, "A0");
      if(ourString.find("str")!=std::string::npos) ldstInstruction(ourString, "B0");
    }
    else if (isLabelOrSymbol(*iterator)) {
      std::string ourString = *iterator;
      symbolDefinition(ourString);
    }
    else { //doesn't match any regex
      throw MyException("neispravna naredba/instrukcija u asembleru " + *iterator);
    }
    iterator++;
  }
  symbolTable.createFinalRelocationsForAssembler();
  symbolTable.printSymbolTable(outfile);
  sectionTable.printSectionTable(outfile);
  if(undefinedSymbolsLeft())
  throw MyException("ostao nedefinisan simbol prilikom prolaza asemblera");
}

void Assembler::symbolDefinition(std::string ourString) {
  std::string symbolName = ourString.erase(ourString.size()-1, 1);
  Symbol* symbol = symbolTable.getSymbolByName(symbolName);

  if(symbol==NULL) { //zameniti ovo proverom symbolexists
    symbolTable.addToSymbolTable(Symbol(symbolName, currentSection->getSectionName(), 0));
    symbol = symbolTable.getSymbolByName(symbolName);
  }
  
    if(symbol->isDefined()) throw MyException("visestruka definicija simbola " + symbol->getSymbolName()); //multiple definition
    if(symbol->getSymbolTypeDef()==2) throw MyException("ne moze da se definise eksterni simbol " + symbol->getSymbolName());
    //extern symbol definition
    Section* section = currentSection;
    if(section->getSectionName()=="initial") throw MyException("Definicija simbola mora biti unutar sekcije");
    //cannot define symbol outside a section
    symbol->setSymbolValue(section->getLocationCounter());
    symbol->setSymbolSection(section->getSectionName());
    symbol->setDefined(true);
    std::string myHexNumber = decimalToHex(std::to_string(section->getLocationCounter())); //definition of a label is its section's location counter
    if(myHexNumber.size()%2==1) {
    myHexNumber = "0" + myHexNumber;
    }
    std::vector<std::string> hexLC = std::vector<std::string>();
    int i=0;
    while(i<myHexNumber.size()) {
    std::string myString(myHexNumber, i, 2);
    hexLC.push_back(myString);
    i=i+2;
    }
  symbol->backpatching(hexLC, currentSection->getSectionName());
}

bool Assembler::isInstruction(std::string ourString) {
  if(regex_match(ourString, noOpInstr)) return true;
  if(regex_match(ourString, oneOpRegInstr)) return true;
  if(regex_match(ourString, twoOpRegInstr)) return true;
  if(regex_match(ourString, oneOpJumpOrCall)) {
      std::vector<std::string> symbolList = std::vector<std::string>();
      std::stringstream sstream(ourString);
      while(sstream.good()) {
      std::string substr;
      getline(sstream, substr, ' '); //get first string delimited by comma
      std::remove_if(substr.begin(), substr.end(), isspace);
      symbolList.push_back(substr);
      }
    //check addressing types
      if(regex_match(symbolList.at(1), jmpAbsolute)) return true;
      if(regex_match(symbolList.at(1), jmpMemdir)) return true;      
      if(regex_match(symbolList.at(1), jmpPCRel)) return true;
      if(regex_match(symbolList.at(1), jmpRegdir)) return true;
      if(regex_match(symbolList.at(1), jmpRegind)) return true;
      if(regex_match(ourString.substr(4, ourString.size()), 
      jmpRegindDisplSpace) || regex_match(ourString.substr(5, ourString.size()), 
      jmpRegindDisplSpace) ||
      regex_match(ourString.substr(4, ourString.size()), 
      jmpRegindDisplNoSpace) || regex_match(ourString.substr(5, ourString.size()), 
      jmpRegindDisplNoSpace)
      ) return true;
    return false;    
  }
  if(regex_match(ourString, twoOpLdrStr)) return true;
  return false;
}

void Assembler::haltInstruction(std::string ourString) {
  currentSection->addToSection(ourString);
  currentSection->incLocationCounter();
}

void Assembler::notInstruction(std::string ourString) {
  std::string substring = ourString.erase(0, 4); //deletes 'not'
  std::remove_if(substring.begin(), substring.end(), isspace);
  int index = registerIndex(substring);
  std::string hexValue = decimalToHex(std::to_string(index));
  if(index==0) hexValue = "00";
  if(hexValue.size()==1) hexValue = hexValue + "0";
  addHexCodeToSection(currentSection, "80" + hexValue);
}

void Assembler::andInstruction(std::string ourString, std::string opcode){
  std::string substring = ourString.erase(0, 4); //deletes 'and'
      std::vector<std::string> symbolList = std::vector<std::string>();
      std::stringstream sstream(substring);
      while(sstream.good()) {
      std::string substr;
      getline(sstream, substr, ','); //get first string delimited by comma
      if(substr.at(0)==' ') substr.erase(0,1);
      symbolList.push_back(substr);
      }
  std::string registerOne = symbolList.at(0);
  std::string registerTwo = symbolList.at(1);
  std::string hexValueOne = decimalToHex(std::to_string(registerIndex(registerOne)));
  std::string hexValueTwo = decimalToHex(std::to_string(registerIndex(registerTwo)));
  if(registerIndex(registerOne)==0) hexValueOne="0";
  if(registerIndex(registerTwo)==0) hexValueTwo="0";
  std::string addingToSection = hexValueOne + hexValueTwo;
  addHexCodeToSection(currentSection, opcode + addingToSection);
}

void Assembler::addInstruction(std::string ourString, std::string opcode) {
  std::string substring = ourString.erase(0, 4); //deletes 'add'
      std::vector<std::string> symbolList = std::vector<std::string>();
      std::stringstream sstream(substring);
      while(sstream.good()) {
      std::string substr;
      getline(sstream, substr, ','); //get first string delimited by comma
      if(substr.at(0)==' ') substr.erase(0,1);
      symbolList.push_back(substr);
      }
  std::string registerOne = symbolList.at(0);
  std::string registerTwo = symbolList.at(1);
  std::string hexValueOne = decimalToHex(std::to_string(registerIndex(registerOne)));
  std::string hexValueTwo = decimalToHex(std::to_string(registerIndex(registerTwo)));
  if(registerIndex(registerOne)==0) hexValueOne="0";
  if(registerIndex(registerTwo)==0) hexValueTwo="0";
  std::string addingToSection = hexValueOne + hexValueTwo;
  addHexCodeToSection(currentSection, opcode + addingToSection);
}

void Assembler::testInstruction(std::string ourString){
  std::string substring = ourString.erase(0, 5); //deletes 'test'
      std::vector<std::string> symbolList = std::vector<std::string>();
      std::stringstream sstream(substring);
      while(sstream.good()) {
      std::string substr;
      getline(sstream, substr, ','); //get first string delimited by comma
      if(substr.at(0)==' ') substr.erase(0,1);
      symbolList.push_back(substr);
      }
  std::string registerOne = symbolList.at(0);
  std::string registerTwo = symbolList.at(1);
  std::string hexValueOne = decimalToHex(std::to_string(registerIndex(registerOne)));
  std::string hexValueTwo = decimalToHex(std::to_string(registerIndex(registerTwo)));
  if(registerIndex(registerOne)==0) hexValueOne="0";
  if(registerIndex(registerTwo)==0) hexValueTwo="0";
  std::string addingToSection = hexValueOne + hexValueTwo;
  addHexCodeToSection(currentSection, "84" + addingToSection);
}

void Assembler::orInstruction(std::string ourString) {
  std::string substring = ourString.erase(0, 3); //deletes 'or'
      std::vector<std::string> symbolList = std::vector<std::string>();
      std::stringstream sstream(substring);
      while(sstream.good()) {
      std::string substr;
      getline(sstream, substr, ','); //get first string delimited by comma
      if(substr.at(0)==' ') substr.erase(0,1);
      symbolList.push_back(substr);
      }
  std::string registerOne = symbolList.at(0);
  std::string registerTwo = symbolList.at(1);
  std::string hexValueOne = decimalToHex(std::to_string(registerIndex(registerOne)));
  std::string hexValueTwo = decimalToHex(std::to_string(registerIndex(registerTwo)));
  if(registerIndex(registerOne)==0) hexValueOne="0";
  if(registerIndex(registerTwo)==0) hexValueTwo="0";
  std::string addingToSection = hexValueOne + hexValueTwo;
  addHexCodeToSection(currentSection, "82" + addingToSection);
}

void Assembler::xchgInstruction(std::string ourString) {
  std::string substring = ourString.erase(0, 4); //deletes 'xchg'
      std::vector<std::string> symbolList = std::vector<std::string>();
      std::stringstream sstream(substring);
      while(sstream.good()) {
      std::string substr;
      getline(sstream, substr, ','); //get first string delimited by comma
      if(substr.at(0)==' ') substr.erase(0,1);
      symbolList.push_back(substr);
      }
  std::string registerOne = symbolList.at(0);
  std::string registerTwo = symbolList.at(1);
  std::string hexValueOne = decimalToHex(std::to_string(registerIndex(registerOne)));
  std::string hexValueTwo = decimalToHex(std::to_string(registerIndex(registerTwo)));
  if(registerIndex(registerOne)==0) hexValueOne="0";
  if(registerIndex(registerTwo)==0) hexValueTwo="0";
  std::string addingToSection = hexValueOne + hexValueTwo;
  addHexCodeToSection(currentSection, "60" + addingToSection);
}

void Assembler::popInstruction(std::string ourString) {
  std::string substring = ourString.erase(0, 4); //deletes 'pop'
  std::remove_if(substring.begin(), substring.end(), isspace);

  int index = registerIndex(substring);
  std::string hexValue = decimalToHex(std::to_string(index));
  if(index==0) hexValue = "0";
  if(hexValue.size()==1) hexValue = hexValue + "6";
  
  currentSection->addToSection("A0");
  currentSection->incLocationCounter();
  
  currentSection->addToSection(hexValue);
  currentSection->incLocationCounter(); 

  currentSection->addToSection("42");
  currentSection->incLocationCounter(); 
}

void Assembler::pushInstruction(std::string ourString) {
  std::string substring = ourString.erase(0, 5); //deletes 'push'
  std::remove_if(substring.begin(), substring.end(), isspace);

  int index = registerIndex(substring);
  std::string hexValue = decimalToHex(std::to_string(index));
  if(index==0) hexValue = "0";
  if(hexValue.size()==1) hexValue = hexValue + "6";
  
  currentSection->addToSection("B0");
  currentSection->incLocationCounter();
  
  currentSection->addToSection(hexValue);
  currentSection->incLocationCounter(); 

  currentSection->addToSection("12");
  currentSection->incLocationCounter(); 

}

void Assembler::intInstruction(std::string ourString) {
  std::string substring = ourString.erase(0, 4); //deletes 'int'
  std::remove_if(substring.begin(), substring.end(), isspace);
  std::string nibble = std::string();
  nibble = std::to_string(registerIndex(substring));

  std::string secondByte = nibble + "F";
  addHexCodeToSection(currentSection, "10" + secondByte);
}

void Assembler::shiftLeftInstruction(std::string ourString, std::string opcode) {
    std::string substring = ourString.erase(0, 3); //deletes 'shl' or 'shr'
      std::vector<std::string> symbolList = std::vector<std::string>();
      std::stringstream sstream(substring);
      while(sstream.good()) {
      std::string substr;
      getline(sstream, substr, ','); //get first string delimited by comma
      if(substr.at(0)==' ') substr.erase(0,1);
      symbolList.push_back(substr);
      }
  std::string registerOne = symbolList.at(0);
  std::string registerTwo = symbolList.at(1);
  std::string hexValueOne = decimalToHex(std::to_string(registerIndex(registerOne)));
  std::string hexValueTwo = decimalToHex(std::to_string(registerIndex(registerTwo)));
  if(registerIndex(registerOne)==0) hexValueOne="0";
  if(registerIndex(registerTwo)==0) hexValueTwo="0";
  std::string addingToSection = hexValueOne + hexValueTwo;
  addHexCodeToSection(currentSection, opcode + addingToSection);
}

void Assembler::jmpInstruction(std::string ourString, std::string opcode) {
  currentSection->addToSection(opcode); //jump instructions pass their opcode
  currentSection->incLocationCounter();
  std::string substring = ourString;
  if(ourString.find("call")!=std::string::npos) substring = ourString.erase(0, 5); //deletes 'call'
  else substring = ourString.erase(0, 4); //deletes 'jmp', 'jeq', 'jgt', 'jne'

  if(regex_match(substring, jmpRegindDisplSpace)) //regex for space
  substring.erase(std::remove_if(substring.begin(), substring.end(), ::isspace), substring.end());

  if(!regex_match(substring, jmpRegindDisplSpace))
    std::remove_if(substring.begin(), substring.end(), isspace);

  if(regex_match(substring, jmpRegdir)) { //form jmp reg
    substring = substring.erase(0,1);
    std::string nibble = decimalToHex(std::to_string(registerIndex(substring)));
    addHexCodeToSection(currentSection, "F" + nibble + "01");
    }
  else if(regex_match(substring, jmpRegind)) {//form jmp *[r0]
    substring = substring.erase(0,2); //deletes '*['
    substring = substring.erase(substring.size()-1, 1); //deletes ']'
    std::string nibble = decimalToHex(std::to_string(registerIndex(substring)));
    addHexCodeToSection(currentSection, "F" + nibble + "02");
  }
  else if(regex_match(substring, jmpMemdir)) {
    substring = substring.erase(0,1); //deletes '*'
    addHexCodeToSection(currentSection, "FF04");

    if(isDecimalNumber(substring)) {
      substring = decimalToHex(substring);
      addHexCodeToSection(currentSection, substring);
    }
    else if(isHexNumber(substring)) {
    substring = substring.erase(0,2);
    addHexCodeToSection(currentSection, substring);
    }
    else { //is a symbol
      if(!symbolTable.symbolExistsInTable(substring)) {
        Symbol symbol = Symbol(substring, currentSection->getSectionName(), 0);
        symbolTable.addToSymbolTable(symbol);
      }
      if(!symbolTable.getSymbolByName(substring)->isDefined()) {
        FCTEntry entry = FCTEntry(currentSection->getLocationCounter(), currentSection, 2, 1, false);
        symbolTable.getSymbolByName(substring)->addFCTEntry(entry);
        addHexCodeToSection(currentSection, "0000");
      }
      else {
        if(symbolTable.getSymbolByName(substring)->getSymbolTypeDef()==0) { //local symbol
        Relocation reloc = Relocation(currentSection->getSectionName(), substring, 
        currentSection->getLocationCounter(), 0, 0, symbolTable.getSymbolByName(substring)->getSymbolSection());
        currentSection->addToRelocationTable(reloc);
        std::string myHexNumber =
        decimalToHex(std::to_string(symbolTable.getSymbolByName(substring)->getSymbolValue()));
        addHexCodeToSection(currentSection, myHexNumber);
        }
        else { //global or extern symbol
        Relocation reloc = Relocation(currentSection->getSectionName(), substring,
         currentSection->getLocationCounter(), 0, 0,symbolTable.getSymbolByName(substring)->getSymbolSection());
        currentSection->addToRelocationTable(reloc);        
        addHexCodeToSection(currentSection, "0000");
        }
      }
    }
  }
  else if(regex_match(substring, jmpRegindDisplNoSpace)) {
    std::string substring2 = substring.substr(2,2);
    std::string nibble = decimalToHex(std::to_string(registerIndex(substring2)));
    addHexCodeToSection(currentSection, "F" + nibble + "03");

    std::string displ = substring.substr(substring.find("+")); //gets displacement
    
    displ = displ.substr(1, displ.size() - 2);
    std::remove_if(displ.begin(), displ.end(), isspace);
    if (isDecimalNumber(displ)){
      displ = decimalToHex(displ);
      addHexCodeToSection(currentSection, displ);
      }
    else if(isHexNumber(displ)) {
    displ = displ.erase(0,2);
      addHexCodeToSection(currentSection, displ);
    }
    else { //dispacement is a symbol
      if(!symbolTable.symbolExistsInTable(displ)) {
        
        Symbol symbol = Symbol(displ, currentSection->getSectionName(), 0);
        symbolTable.addToSymbolTable(symbol);
      }

      if(!symbolTable.getSymbolByName(displ)->isDefined()) {
        FCTEntry entry = FCTEntry(currentSection->getLocationCounter(), currentSection, 2, 0, false);
        symbolTable.getSymbolByName(displ)->addFCTEntry(entry);
        addHexCodeToSection(currentSection, "0000");
      }
      else {
      if(!symbolTable.symbolExistsInTable(displ)) {
        Symbol symbol = Symbol(displ, currentSection->getSectionName(), 0);
        symbolTable.addToSymbolTable(symbol);
      }
      if(!symbolTable.getSymbolByName(displ)->isDefined()) {
        FCTEntry entry = FCTEntry(currentSection->getLocationCounter(), currentSection, 2, 0, false);
          symbolTable.getSymbolByName(displ)->addFCTEntry(entry);
          addHexCodeToSection(currentSection, "0000");
      }
      else {
        if(symbolTable.getSymbolByName(displ)->getSymbolTypeDef()==0) { //local symbol
        Relocation reloc = Relocation(currentSection->getSectionName(), displ, 
        currentSection->getLocationCounter(), 0, 0, symbolTable.getSymbolByName(displ)->getSymbolSection());
        currentSection->addToRelocationTable(reloc);
        std::string myHexNumber =
        decimalToHex(std::to_string(symbolTable.getSymbolByName(displ)->getSymbolValue()));
        addHexCodeToSection(currentSection, myHexNumber);
        }
        else { //global or extern symbol
        Relocation reloc = Relocation(currentSection->getSectionName(), displ,
         currentSection->getLocationCounter(), 0, 0, symbolTable.getSymbolByName(displ)->getSymbolSection());
        currentSection->addToRelocationTable(reloc);        
        addHexCodeToSection(currentSection, "0000");
        }
      }
    }
    } 
  }
  else if (regex_match(substring, jmpAbsolute)) {
    addHexCodeToSection(currentSection, "FF00");
      if(isDecimalNumber(substring)) {
      substring = decimalToHex(substring);
      addHexCodeToSection(currentSection, substring);
    }
    else if(isHexNumber(substring)) {
    substring = substring.erase(0,2);
    addHexCodeToSection(currentSection, substring);
    }
    else {
      if(!symbolTable.symbolExistsInTable(substring)) {
        Symbol symbol = Symbol(substring, currentSection->getSectionName(), 0);
        symbolTable.addToSymbolTable(symbol);
      }
      if(!symbolTable.getSymbolByName(substring)->isDefined()) {
        FCTEntry entry = FCTEntry(currentSection->getLocationCounter(), currentSection, 2, 0, false);
        symbolTable.getSymbolByName(substring)->addFCTEntry(entry);
        addHexCodeToSection(currentSection, "0000");
        }
      else {
        if(symbolTable.getSymbolByName(substring)->getSymbolTypeDef()==0) { //local symbol
        Relocation reloc = Relocation(currentSection->getSectionName(), substring, 
        currentSection->getLocationCounter(), 0, 0, symbolTable.getSymbolByName(substring)->getSymbolSection());
        currentSection->addToRelocationTable(reloc);
        std::string myHexNumber =
        decimalToHex(std::to_string(symbolTable.getSymbolByName(substring)->getSymbolValue()));
        addHexCodeToSection(currentSection, myHexNumber);
        }
        else { //global or extern
        Relocation reloc = Relocation(currentSection->getSectionName(), substring,
         currentSection->getLocationCounter(), 0, 0, symbolTable.getSymbolByName(substring)->getSymbolSection());
        currentSection->addToRelocationTable(reloc);        
        addHexCodeToSection(currentSection, "0000");
        }
      }
    }
  }
  else if (regex_match(substring, jmpPCRel)) {
    std::string substring2 = substring.substr(1); //deletes '%'
    addHexCodeToSection(currentSection, "F705");
      if(!symbolTable.symbolExistsInTable(substring2)) {
        Symbol symbol = Symbol(substring2, currentSection->getSectionName(), 0);
        symbolTable.addToSymbolTable(symbol);
      }
      if(!symbolTable.getSymbolByName(substring2)->isDefined()) {
        FCTEntry entry = FCTEntry(currentSection->getLocationCounter(), currentSection, 2, 1, false);
          symbolTable.getSymbolByName(substring2)->addFCTEntry(entry);
          addHexCodeToSection(currentSection, "0000");
      }
      else {
        if(symbolTable.getSymbolByName(substring2)->getSymbolSection()==currentSection->getSectionName()) {
          int symbolAddress = symbolTable.getSymbolByName(substring2)->getSymbolValue();
          int offset = symbolAddress - currentSection->getLocationCounter() -2; //-2 is addend
          std::string hexCode = decimalToHex(std::to_string(offset));
          addHexCodeToSection(currentSection, hexCode);
        }
        else {
          Relocation reloc = Relocation(currentSection->getSectionName(), substring2, 
          currentSection->getLocationCounter(), 1, false, symbolTable.getSymbolByName(substring2)->getSymbolSection());
          currentSection->addToRelocationTable(reloc);
          addHexCodeToSection(currentSection, "0000");
        }
      }
  }
}

void Assembler::ldstInstruction(std::string ourString, std::string opcode) {
  currentSection->addToSection(opcode);
  currentSection->incLocationCounter();
  std::string substring = ourString.erase(0,4);
  std::string sourceReg = substring.substr(0, substring.find(","));
  substring = substring.substr(substring.find(",")+2);
  if(!regex_match(substring, ldrStrRegindDisplNoSpace)) //has spaces
  substring.erase(std::remove_if(substring.begin(), substring.end(), ::isspace), substring.end());

    if(regex_match(substring, ldrStrRegdir)) {
    std::string nibble = decimalToHex(std::to_string(registerIndex(substring)));
    std::string source = decimalToHex(std::to_string(registerIndex(sourceReg)));
    addHexCodeToSection(currentSection, source + nibble + "01");
  }

  else if(regex_match(substring, ldrStrRegind)) {
    substring = substring.erase(0,1);
    substring = substring.erase(substring.size()-1, 1);
    std::string nibble = decimalToHex(std::to_string(registerIndex(substring)));
    std::string source = decimalToHex(std::to_string(registerIndex(sourceReg)));
    addHexCodeToSection(currentSection, source + nibble + "02");
  }
  else if(regex_match(substring, ldrStrMemdir)) {
    std::string source = decimalToHex(std::to_string(registerIndex(sourceReg)));
    addHexCodeToSection(currentSection, source + "F" + "04");
    if(isDecimalNumber(substring)) {
      substring = decimalToHex(substring);
      addHexCodeToSection(currentSection, substring);
    }
    else if(isHexNumber(substring)) {
    substring = substring.erase(0,2);
    addHexCodeToSection(currentSection, substring);
    }
    else {
      if(!symbolTable.symbolExistsInTable(substring)) {
      Symbol symbol = Symbol(substring, currentSection->getSectionName(), 0);
      symbolTable.addToSymbolTable(symbol);
      }
      if(!symbolTable.getSymbolByName(substring)->isDefined()) {
        FCTEntry entry = FCTEntry(currentSection->getLocationCounter(), currentSection, 2, 0, false);
        symbolTable.getSymbolByName(substring)->addFCTEntry(entry);
        addHexCodeToSection(currentSection, "0000");
        }
      else {
        if(symbolTable.getSymbolByName(substring)->getSymbolTypeDef()==0) { //local symbol
        Relocation reloc = Relocation(currentSection->getSectionName(), substring, 
        currentSection->getLocationCounter(), 0, 0, symbolTable.getSymbolByName(substring)->getSymbolSection());
        currentSection->addToRelocationTable(reloc);
        std::string myHexNumber =
        decimalToHex(std::to_string(symbolTable.getSymbolByName(substring)->getSymbolValue()));
        addHexCodeToSection(currentSection, myHexNumber);
        }
        else { //global or extern
        Relocation reloc = Relocation(currentSection->getSectionName(), substring,
         currentSection->getLocationCounter(), 0, 0, symbolTable.getSymbolByName(substring)->getSymbolSection());
        currentSection->addToRelocationTable(reloc);        
        addHexCodeToSection(currentSection, "0000");
        }
      }
    }

  }
  else if(regex_match(substring, ldrStrAbsolute)) {
    std::string source = decimalToHex(std::to_string(registerIndex(sourceReg)));
    addHexCodeToSection(currentSection, source + "F" + "00");
    substring = substring.substr(1);
      if(isDecimalNumber(substring)) {
      substring = decimalToHex(substring);
      addHexCodeToSection(currentSection, substring);
    }
    else if(isHexNumber(substring)) {
    substring = substring.erase(0,2);
    addHexCodeToSection(currentSection, substring);
    }
    else {
          if(!symbolTable.symbolExistsInTable(substring)) {
        Symbol symbol = Symbol(substring, currentSection->getSectionName(), 0);
        symbolTable.addToSymbolTable(symbol);
      }
      if(!symbolTable.getSymbolByName(substring)->isDefined()) {
        FCTEntry entry = FCTEntry(currentSection->getLocationCounter(), currentSection, 2, 0, false);
        symbolTable.getSymbolByName(substring)->addFCTEntry(entry);
        addHexCodeToSection(currentSection, "0000");
      }
      else {
        if(symbolTable.getSymbolByName(substring)->getSymbolTypeDef()==0) { //local symbol
        Relocation reloc = Relocation(currentSection->getSectionName(), substring, 
        currentSection->getLocationCounter(), 0, 0, symbolTable.getSymbolByName(substring)->getSymbolSection());
        currentSection->addToRelocationTable(reloc);
        std::string myHexNumber =
        decimalToHex(std::to_string(symbolTable.getSymbolByName(substring)->getSymbolValue()));
        addHexCodeToSection(currentSection, myHexNumber);
        }
        else { //global or extern
        Relocation reloc = Relocation(currentSection->getSectionName(), substring,
         currentSection->getLocationCounter(), 0, 0, symbolTable.getSymbolByName(substring)->getSymbolSection());
        currentSection->addToRelocationTable(reloc);        
        addHexCodeToSection(currentSection, "0000");
        }
      }
    }
  }
  else if(regex_search(substring, ldrStrRegindDisplNoSpace)) {
    std::string substring2 = substring.substr(1,2);
    std::string nibble = decimalToHex(std::to_string(registerIndex(substring2)));
    std::string source = decimalToHex(std::to_string(registerIndex(sourceReg)));
    addHexCodeToSection(currentSection, source + nibble + "03");

    std::string displ = substring.substr(substring.find("+"));
    
    displ = displ.substr(1, displ.size() - 2);
    std::remove_if(displ.begin(), displ.end(), isspace);
    if (isDecimalNumber(displ)){
      displ = decimalToHex(displ);
      addHexCodeToSection(currentSection, displ);
      }
    else if(isHexNumber(displ)) {
    displ = displ.erase(0,2);
    addHexCodeToSection(currentSection, displ);
    }
    else {
      if(!symbolTable.symbolExistsInTable(displ)) {
        Symbol symbol = Symbol(displ, currentSection->getSectionName(), 0);
        symbolTable.addToSymbolTable(symbol);
      }
      if(!symbolTable.getSymbolByName(displ)->isDefined()) {
        FCTEntry entry = FCTEntry(currentSection->getLocationCounter(), currentSection, 2, 0, false);
          symbolTable.getSymbolByName(displ)->addFCTEntry(entry);
          addHexCodeToSection(currentSection, "0000");
      }
      else {
        if(symbolTable.getSymbolByName(displ)->getSymbolTypeDef()==0) { //local symbol
        Relocation reloc = Relocation(currentSection->getSectionName(), displ, 
        currentSection->getLocationCounter(), 0, 0, symbolTable.getSymbolByName(displ)->getSymbolSection());
        currentSection->addToRelocationTable(reloc);
        std::string myHexNumber =
        decimalToHex(std::to_string(symbolTable.getSymbolByName(displ)->getSymbolValue()));
        addHexCodeToSection(currentSection, myHexNumber);
        }
        else { //global or extern
        Relocation reloc = Relocation(currentSection->getSectionName(), displ,
         currentSection->getLocationCounter(), 0, 0, symbolTable.getSymbolByName(displ)->getSymbolSection());
        currentSection->addToRelocationTable(reloc);        
        addHexCodeToSection(currentSection, "0000");
        }
      }
    }
  }
  else if(regex_match(substring, ldrStrPCRel)) {
    std::string substring2 = substring.substr(1);
    std::string source = decimalToHex(std::to_string(registerIndex(sourceReg)));
    addHexCodeToSection(currentSection, source + "7" + "05");
      if(!symbolTable.symbolExistsInTable(substring2)) {
        Symbol symbol = Symbol(substring2, currentSection->getSectionName(), 0);
        symbolTable.addToSymbolTable(symbol);
      }
      if(!symbolTable.getSymbolByName(substring2)->isDefined()) {
        FCTEntry entry = FCTEntry(currentSection->getLocationCounter(), currentSection, 2, 1, false);
          symbolTable.getSymbolByName(substring2)->addFCTEntry(entry);
          addHexCodeToSection(currentSection, "0000");
      }
      else {
        if(symbolTable.getSymbolByName(substring2)->getSymbolSection()==currentSection->getSectionName()) {
          int symbolAddress = symbolTable.getSymbolByName(substring2)->getSymbolValue();
          int offset = symbolAddress - currentSection->getLocationCounter() -2; //-2 is addend
          std::string hexCode = decimalToHex(std::to_string(offset));
          addHexCodeToSection(currentSection, hexCode);
        }
        else {
          Relocation reloc = Relocation(currentSection->getSectionName(), substring2, 
          currentSection->getLocationCounter(), 1, false, symbolTable.getSymbolByName(substring2)->getSymbolSection());
          currentSection->addToRelocationTable(reloc);
          addHexCodeToSection(currentSection, "0000");
        }
      }
  }
}

bool Assembler::isLabelOrSymbol(std::string ourString) {
  if(ourString=="r0" || ourString=="r1" || ourString=="r2" || ourString=="r3"
  || ourString=="r4" || ourString=="r5" || ourString=="r6" || ourString=="r7"
  || ourString=="psw" || ourString=="pc" || ourString=="sp") return false; //exclude registers from symbol table
  if(regex_match(ourString, isLabelDeclaration)) return true;
  return false;
}

void Assembler::createSymbol(std::string symbolName, std::string sectionName, int binding) {
  Symbol newSymbol(symbolName, sectionName, binding);
  symbolTable.addToSymbolTable(newSymbol);
}

void Assembler::createSection(std::string string) {
  std::string sectionName = string.erase(0, 9); //deletes '.section'
  Section* newSection;
  if(sectionTable.sectionExists(sectionName)==false){
  newSection = new Section(sectionName);
  sectionTable.addToSectionTable(newSection);
  }
  else {
    throw MyException("Dvostruka definicija sekcije " + sectionName); //cannot define same section twice
  }
  Symbol newSymbol(sectionName, sectionName,0, true, 0); //add section to symbol table
  symbolTable.addToSymbolTable(newSymbol);
  Section* oldSection = currentSection;
  currentSection=newSection;
}

bool Assembler::isHexNumber(std::string ourString) {
  if(regex_match(ourString, isHexadecimal)) return true;
  return false;
}

bool Assembler::isDecimalNumber(std::string ourString) {
  if(ourString=="0") return true;
  if(regex_match(ourString, isDecimal)) return true;
  return false;
}

std::string Assembler::decimalToHex(std::string hexVal){
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

int Assembler::registerIndex(std::string ourString) {
  if(ourString=="r0") return 0;
  if(ourString=="r1") return 1;
  if(ourString=="r2") return 2;
  if(ourString=="r3") return 3;
  if(ourString=="r4") return 4;
  if(ourString=="r5") return 5;
  if(ourString=="r6" || ourString=="sp") return 6;
  if(ourString=="r7" || ourString=="pc") return 7;
  if(ourString=="psw") return 8;
  return -1;
}

bool Assembler::undefinedSymbolsLeft() {
  bool flag = false;
  for(int i=0; i<symbolTable.getSymbolTable().size(); i++) {
    if(sectionTable.getSectionByName(symbolTable.getSymbolTable().at(i).getSymbolName())!=NULL) continue;
    if(!symbolTable.getSymbolTable().at(i).isDefined() && 
    symbolTable.getSymbolTable().at(i).getSymbolTypeDef()!=2) {
      flag=true;
      }
    }
    return flag;
  }

void Assembler::addHexCodeToSection(Section* section, std::string substring) {
      if(substring.size()==1) {
        section->addToSection("00");
        section->incLocationCounter();
        section->addToSection("0" + substring);
        section->incLocationCounter();
      }
      else if(substring.size()==2) {
        section->addToSection("00");
        section->incLocationCounter();
        section->addToSection(substring);
        section->incLocationCounter();
      }
      else if(substring.size()==3) {
        substring = "0" + substring;
        section->addToSection(substring.substr(0,2));
        section->incLocationCounter();
        section->addToSection(substring.substr(2,3));
        section->incLocationCounter();      
        }
      else {
        section->addToSection(substring.substr(0,2));
        section->incLocationCounter();
        section->addToSection(substring.substr(2,3));
        section->incLocationCounter();      
      }
  }

void Assembler::addHexCodeLittleEndian(Section* section, std::string myHexNumber) {
      if(myHexNumber.size()==1) {
        myHexNumber = "0" + myHexNumber;
        section->addToSection(myHexNumber);
        section->addToSection("00");
      }
      else if (myHexNumber.size()==2)  {
        section->addToSection(myHexNumber);
        section->addToSection("00");
      }
      else if(myHexNumber.size()==3) {
        std::string firstHalf="0" + myHexNumber.substr(0,1);
        std::string secondHalf=myHexNumber.substr(1,2);
        section->addToSection(secondHalf);
        section->addToSection(firstHalf);
      }
      else {
      std::string firstHalf=myHexNumber.substr(0,2);
      std::string secondHalf=myHexNumber.substr(2,2);
      section->addToSection(secondHalf);
      section->addToSection(firstHalf);
      }
      section->incLocationCounter();
      section->incLocationCounter();
}