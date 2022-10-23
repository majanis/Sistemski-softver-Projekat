#ifndef EMULATOR_HPP_
#define EMULATOR_HPP_

#include <fstream>
#include <sstream>
#include "../inc/Exception.hpp"
#include <vector>
#include <iostream>
#include <regex>

class Emulator{

std::string inputFilename;
std::ifstream in;
std::vector<std::string> hexContent = std::vector<std::string>();

unsigned char memoryContent[65536]; //emulated memory
unsigned short registers[9] = {0}; //PC is reg7, SP is reg6

int memorySize = 65280; //this is available memory, up to FF00

std::regex blankLine = std::regex("^\\s*$");

short regIR; //our PC is pointing to the next instruction in pipeline, this tracks the current instruction

public:

Emulator(std::string filename);
void startEmulator();
void loadContentIntoEmulator();
bool isMemoryOverloaded(int size);
short hexToDec(std::string hex);
std::string decimalToHex(std::string dec);
std::string formatHexOutput(std::string hex);
void handleData();
void writeBytesToMemory(unsigned short valueToWrite, unsigned short address /*, bool isLittleEndian*/);
short getOperandFromAddressModeJmp(unsigned char addressMode, unsigned char updateData);
short getOperandFromAddressModeLoad(unsigned char addressMode, unsigned char updateData);
void executeStoreInstruction(int regIndex, unsigned char addressMode, unsigned char updateData);
bool zeroFlagActivated();
bool greaterThanFlagsActivated();
void updateZeroFlag(int destReg, int sourceReg);
void updateOverflowFlag(int destReg, int sourceReg);
void updateNegativeFlag(int destReg, int sourceReg);
void updateCarryFlag(int destReg, int sourceReg);
void printEmulationData();
std::string decToBin(short dec);
void interruptError();
void enableInterrupts(); //postavlja I na 1
void initializePCToAddr(int startInMem); //krece od adrese na koju ukazuje prvi ulaz u IVT
};

#endif