#ifndef ASSEMBLER_HPP_
#define ASSEMBLER_HPP_

#include <stdlib.h>
#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include "../inc/Symbol.hpp"
#include "../inc/Section.hpp"
#include "../inc/SymbolTable.hpp"
#include "../inc/SectionTable.hpp"
#include "../inc/Exception.hpp"
#include <regex>
#include <sstream>

class Assembler {
  private:
  std::ifstream infile;
  std::ofstream outfile;
  std::vector<std::string> linesFromFile = std::vector<std::string>();
  SymbolTable symbolTable;
  SectionTable sectionTable;
  Section* currentSection;

  public:

  std::string symbol=std::string("[a-zA-Z][a-zA-Z0-9_]*");
  std::string rangeOfRegisters = std::string("[0-7]"); 
  std::string decimalLiteral = std::string("-?[0-9]+"); //integers can be pozitive or negative
  std::string hexLiteral = std::string("0x[0-9A-F]+");
  std::string space = std::string(" ");
  std::string symbolOrLiteral = symbol + "|" + decimalLiteral + "|" + hexLiteral;
  std::string spaceOrNone = "[ \t\r\f]*";
  std::string dot = "\\.*";
  //std::string registers;

  std::regex global_directive=std::regex("^\\.global (" + spaceOrNone + symbol +
   "(," + spaceOrNone + symbol + ")*)$");
  std::regex extern_directive=std::regex("^\\.extern (" + spaceOrNone + symbol + 
  "(," + spaceOrNone + symbol + ")*)$");
  std::regex section_directive=std::regex("^\\.section (" + dot + symbol + ")$"); //can start with dot
  std::regex word_directive=std::regex("^\\.word ((" + symbolOrLiteral + ")(,(" + symbolOrLiteral + "))*)$");
  std::regex skip_directive=std::regex("^\\.skip (" + decimalLiteral + "|" + hexLiteral + ")$");
  
  std::regex noOpInstr=std::regex("^(halt|iret|ret)$");
  std::regex oneOpRegInstr=std::regex("^(push|pop|int|not) (r[0-7]|psw)$");
  std::regex twoOpRegInstr=
  std::regex("^(xchg|add|sub|mul|div|cmp|and|or|xor|test|shl|shr) (r[0-7]|psw)" + spaceOrNone + 
  "," + spaceOrNone + "(r[0-7]|psw)$");

  std::regex oneOpJumpOrCall=std::regex("^(call|jmp|jeq|jne|jgt) (.*)$");
  std::regex twoOpLdrStr=
  std::regex("^(ldr|str) (r[0-7]|psw),(.*)$");

  std::regex jmpAbsolute=std::regex("^(" + symbolOrLiteral + ")$");
  std::regex jmpMemdir=
  std::regex("^\\*(" + symbolOrLiteral + ")$");
  std::regex jmpPCRel=
  std::regex("^%(" + symbol + ")$");
  std::regex jmpRegdir=
  std::regex("^\\*(r[0-7]|psw)$");
  std::regex jmpRegind=std::regex("^\\*\\[(r[0-7]|psw)\\]$");
  std::regex jmpRegindDisplSpace=
  std::regex("^\\*\\[(r[0-7]|psw) \\+ (" + symbolOrLiteral + ")\\]$");
  std::regex jmpRegindDisplNoSpace=
  std::regex("^\\*\\[(r[0-7]|psw)\\+(" + symbolOrLiteral + ")\\]$");

  std::regex isSymbol = std::regex(symbol);
  std::regex isLabelDeclaration = std::regex(symbol + ":");
  std::regex isDecimal = std::regex(decimalLiteral);
  std::regex isHexadecimal = std::regex(hexLiteral);

  std::regex ldrStrAbsolute=
  std::regex("^\\$(" + symbolOrLiteral + ")$");
  std::regex ldrStrMemdir=std::regex("^(" + symbolOrLiteral + ")$");
  std::regex ldrStrPCRel=std::regex("^%(" + symbol + ")$");
  std::regex ldrStrRegdir=std::regex("^(r[0-7]|psw)$");
  std::regex ldrStrRegind=std::regex("^\\[(r[0-7]|psw)\\]$");
  std::regex ldrStrRegindDisplSpace=
  std::regex("^\\[(r[0-7]|psw) \\+ (" + symbolOrLiteral + ")\\]$");

  std::regex ldrStrRegindDisplNoSpace=
  std::regex("^\\[(r[0-7]|psw)\\+(" + symbolOrLiteral + ")\\]$");

  std::regex blankLine = std::regex("^\\s*$");

  Assembler(std::string inputFilename, std::string outputFileName);
  void analyzeAssemblyCode();
  ~Assembler();
  void loadLinesIntoVector();
  bool isDirective(std::string string);
  void createSymbol(std::string SymbolName, std::string sectionName, int bindingType);
  void createSection(std::string sectionName);
  void wordDirective(Section* section, std::string ourString);
  void skipDirective(Section* section, std::string ourString);
  void globalDirective(std::string ourString);
  void externDirective(std::string ourString);
  bool isHexNumber(std::string ourString);
  bool isDecimalNumber(std::string ourString);
  std::string decimalToHex(std::string ourString);
  bool isInstruction(std::string ourString);
  bool isLabelOrSymbol(std::string ourString);
  void symbolDefinition(std::string ourString);
  void haltInstruction(std::string outString);
  void intInstruction(std::string ourString);
  void notInstruction(std::string ourString);
  void andInstruction(std::string ourString, std::string opcode);
  void orInstruction(std::string ourString);
  void testInstruction(std::string ourString);
  void shiftLeftInstruction(std::string ourString, std::string opcode);
  void xchgInstruction(std::string ourString);
  void addInstruction(std::string ourString, std::string opcode);
  void pushInstruction(std::string ourString);
  void popInstruction(std::string ourString);
  void jmpInstruction(std::string ourString, std::string opcode);
  int registerIndex(std::string ourString);
  void ldstInstruction(std::string ourString, std::string opcode);
  bool undefinedSymbolsLeft();
  void addHexCodeToSection(Section* section, std::string substring);
  void addHexCodeLittleEndian(Section* section, std::string substring);
};

#endif