#ifndef LINKER_HPP
#define LINKER_HPP

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include "../inc/Exception.hpp"
#include "../inc/Symbol.hpp"
#include "../inc/Section.hpp"
#include "../inc/SectionTable.hpp"
#include "../inc/SymbolTable.hpp"
#include "../inc/linkerSection.hpp"
#include <iostream>
#include <regex>

class Linker{
std::vector<std::string> inputFiles;
std::string outputFile;
std::ifstream in;
std::ofstream out;
std::vector<std::string> linesVector;

std::vector<std::string> hexContent; //content we send to emulator

std::regex blankLine = std::regex("^\\s*$");
std::string sectionTableBeginning = "******SECTION TABLE******";
std::string symbolTableBeginning = "******SYMBOL TABLE******";
std::string relocationTableBeginning = "******RELOCATION TABLE FOR ";

SymbolTable globalSymbolTable;

SectionTable sectionTable;
Section* currentSection;
LinkerSection* currentLinkerSection;
LinkerSectionTable linkerSectionTable;

std::string currentFile;

int globalLocationCounter;
int fileLocalLocationCounter = 0;

public:

Linker(std::vector<std::string> inputFiles, std::string outputFiles);
void fillSymbolTable(int index);
void fillSectionTable(int index, std::string currentFile);
void loadLinesIntoVectorByFile(std::string inputFile);
void startLinker();
void multipleDefinitionOfGlobalSymbol(std::string symbolName);
void noDefinitionOfExternSymbol();
void addToHexCode(std::vector<std::string> hexCodeArray);
void updateSectionStartAddresses(Section* currentSection, int increment);
void fillRelocationTable(int i, Section* section, std::string currentFile);
void emptyVector();
void hexCommandPrint();
std::string decimalToHex(std::string simb);
std::string formatHexOutput(std::string input);
void updateOffsetsForLinkerSections();
void updateOffsetsForSymbolTable();
void solveRelocations();
void patchAbs(std::vector<std::string> hexCode, int startAddress, int isLittleEndian);
void patchRel(int symbolValue, int startAddress, int isLittleEndian);
int hexToDec(std::string code);
};

#endif