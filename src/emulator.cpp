#include "../inc/emulator.hpp"

Emulator::Emulator(std::string filename) {
  this->inputFilename=filename;
  try {
  for(int i=0; i<65536; i++) memoryContent[i] = 0;
  registers[9] = {0}; 
  registers[8] = 24576; //masks terminal and timer, they arent implemented here
  in.open(inputFilename, std::ios::in);
  if(in.fail()) throw MyException("GRESKA: Ne moze da otvori fajl " + inputFilename);
  }  
  catch(MyException& ex) {
  std::cout<<ex.cause()<<std::endl; //catches file exceptions
  }
}

void Emulator::startEmulator() {
  loadContentIntoEmulator();
  handleData();
  printEmulationData();
}

void Emulator::loadContentIntoEmulator() {
  std::string line;
  while(getline(in, line)) {
    if(regex_match(line, blankLine)) continue;
    std::string lineAddress = line.substr(0,4);
    int addressDec = hexToDec(lineAddress);
    line = line.substr(6); //removes address of first byte from loaded string
    std::vector<std::string> bytes = std::vector<std::string>();
    std::stringstream sstream(line);
      while(sstream.good()) {
      int numberOfBytesInLine = 0;
      std::string substr;
      getline(sstream, substr, ' ');
      if(substr.empty()) break;
      if(substr.size()==0) break;
      while(std::isspace(substr.at(0))) substr = substr.erase(0,1);
      while(std::isspace(substr.at(substr.size()-1))) substr.erase(substr.size()-1, 1);
      hexContent.push_back(substr);
      numberOfBytesInLine++;
      try{
      if(isMemoryOverloaded(addressDec+numberOfBytesInLine)) throw MyException("Prekoracenje memorije!");
      }
      catch(MyException& ex) {
        std::cout<<ex.cause()<<std::endl;
      }
      }
  }
  for(int i=0; i<hexContent.size(); i++) {
    memoryContent[i] = static_cast<unsigned char>(hexToDec(hexContent.at(i)));
  }
  in.close();
  in.clear();
  }

bool Emulator::isMemoryOverloaded(int size) {
    if(size>memorySize) return true;
    return false;
  }

short Emulator::hexToDec(std::string ourString) {
if(ourString=="0") return 0;
short s = 0;
s=std::stoi(ourString, NULL, 16);
if(s>32767) { //signed decimal number!
  s = ~s + 1;
  s = -s;
}
return s;
}


std::string Emulator::decimalToHex(std::string hexVal){
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

std::string Emulator::formatHexOutput(std::string input) {
  std::string output="";
  if(input.size()==1) output = "000" + input;
  else if(input.size()==2) output = "00" + input;
  else if(input.size()==3) output="0" + input;
  else output=""+input;
  return output;
}

void Emulator::handleData() {
  bool reachedHalt = false;
  int regIndex;
  int destReg, srcReg;
  unsigned short ivtEntry;
  unsigned char lowerByte;
  unsigned char higherByte;
  unsigned char addressMode;
  unsigned char update;
  short operand; //operand is signed 
  std::string nextByte="";
  initializePCToAddr(0);
  while(true) {
  if(reachedHalt) break;
  regIR = registers[7]; //current instruction beginning
  switch(memoryContent[registers[7]++]) { //opcode
    case 0: { //halt, 1 byte length
    registers[7]--;
    reachedHalt = true;
    break;
    }
    case 16: { //int, 2 byte length
    std::string hexForm = hexContent.at(registers[7]);
    registers[7]++;
    regIndex = hexToDec(hexForm.substr(0,1));
    if(regIndex<0 || regIndex>8) { //error
      registers[7] = regIR;
      interruptError();
      continue;
    }
    ivtEntry = registers[regIndex % 8]*2;
    lowerByte = memoryContent[ivtEntry];
    higherByte = memoryContent[ivtEntry+1];
    registers[6]-= 2; //decrement stack pointer
    writeBytesToMemory(registers[7], registers[6]); //push pc
    registers[6]-= 2;
    writeBytesToMemory(registers[8], registers[6]); //push psw
    registers[7] = higherByte<<8;
    registers[7]+=lowerByte;
    continue; //avoid PC increment
    //break;
    }
    case 32: { //iret, 2 bytes
    //pop psw
    lowerByte = memoryContent[registers[6]];
    higherByte = memoryContent[registers[6]+1]; //littleEndian format
    registers[6]+=2;
    registers[8] = higherByte<<8;
    registers[8] += lowerByte;
    //pop pc
    lowerByte = memoryContent[registers[6]];
    higherByte = memoryContent[registers[6]+1];
    registers[6]+=2;
    registers[7] = higherByte<<8;
    registers[7] += lowerByte;
    continue; //avoid PC increment
    //break;
    }
    case 48: { //call, 3 or 5 bytes
    std::string hexForm = decimalToHex(std::to_string(memoryContent[registers[7]]));
    regIndex = hexToDec(hexForm.substr(1,1));
    registers[7]++;
    std::string hexUpdtAddr = hexContent.at(registers[7]);
    update = hexToDec(hexUpdtAddr.substr(0,1));
    addressMode = hexToDec(hexUpdtAddr.substr(1,1));
    operand = getOperandFromAddressModeJmp(addressMode, update);
    //push pc
    registers[6]-=2;
    registers[7]++;
    writeBytesToMemory(registers[7], registers[6]);
    registers[7]=operand; //pc<=operand
    continue; //avoid PC increment
    //break;
    }
    case 64: { //ret, 1 byte
    //pop pc
    lowerByte = memoryContent[registers[6]];
    higherByte = memoryContent[registers[6]+1];
    registers[6]+=2;
    registers[7] = higherByte<<8;
    registers[7] += lowerByte;
    continue; //avoid PC increment
    //break;
    }
    case 80: { //jmp, 3 or 5 bytes
    //pc<=operand
    srcReg = hexToDec(hexContent.at(registers[7]).substr(1,1)); //removes 'F'
    registers[7]++;
    addressMode = hexToDec(hexContent.at(registers[7]).substr(1,1));
    update = hexToDec(hexContent.at(registers[7]).substr(0,1));
    operand = getOperandFromAddressModeJmp(addressMode, update);
    registers[7] = operand;
    continue; //avoid PC increment
    //break;
    }
    case 81: { //jeq, 3 or 5 bytes
    srcReg = hexToDec(hexContent.at(registers[7]).substr(1,1)); //removes 'F'
    registers[7]++;
    addressMode = hexToDec(hexContent.at(registers[7]).substr(1,1));
    update = hexToDec(hexContent.at(registers[7]).substr(0,1));
    operand = getOperandFromAddressModeJmp(addressMode, update);
    if(zeroFlagActivated()) { //only execute the jump if the flag is activated otherwise just read the instruction
    registers[7] = operand;
    continue; //avoid PC increment
    }
    break;
    }
    case 82: { //jne, 3 or 5 bytes
    srcReg = hexToDec(hexContent.at(registers[7]).substr(1,1)); //removes 'F'
    registers[7]++;
    addressMode = hexToDec(hexContent.at(registers[7]).substr(1,1));
    update = hexToDec(hexContent.at(registers[7]).substr(0,1));
    operand = getOperandFromAddressModeJmp(addressMode, update);
    if(!zeroFlagActivated()) { //only execute the jump if the flag is disabled otherwise just read the instruction
    registers[7] = operand;
    continue; //avoid PC increment
    }
    break;
    }
    case 83: { //jgt, 3 or 5 bytes
    srcReg = hexToDec(hexContent.at(registers[7]).substr(1,1)); //removes 'F'
    registers[7]++;
    addressMode = hexToDec(hexContent.at(registers[7]).substr(1,1));
    update = hexToDec(hexContent.at(registers[7]).substr(0,1));
    operand = getOperandFromAddressModeJmp(addressMode, update);
    if(greaterThanFlagsActivated()) { //only execute the jump if the flags are activated otherwise just read the instruction
    registers[7] = operand;
    continue; //avoid PC increment
    }
    break;
    }
    case 176: { //push or str, 3 or 5 bytes
    nextByte = hexContent.at(registers[7]);
    destReg = hexToDec(nextByte.substr(0,1));
    srcReg = hexToDec(nextByte.substr(1,1));
    if(destReg<0 || destReg>8) { //error
      registers[7] = regIR;
      interruptError();
      continue; 
    }
    registers[7]++;
    nextByte = hexContent.at(registers[7]);
    addressMode = hexToDec(nextByte.substr(1,1));
    update = hexToDec(nextByte.substr(0,1));
    executeStoreInstruction(destReg, addressMode, update);
    break;
    }
    case 160: { //pop or ldr, 3 or 5 bytes
    nextByte = hexContent.at(registers[7]);
    destReg = hexToDec(nextByte.substr(0,1));
    srcReg = hexToDec(nextByte.substr(1,1));
    if(destReg<0 || destReg>8) { //error
      registers[7] = regIR;
      interruptError();
      continue;
    }
    operand = registers[destReg];
    registers[7]++;
    nextByte = hexContent.at(registers[7]);
    addressMode = hexToDec(nextByte.substr(1,1));
    update = hexToDec(nextByte.substr(0,1));
    operand = getOperandFromAddressModeLoad(addressMode, update);
    registers[destReg] = operand;
    break;
    }
    case 96: { //xchg, 2 bytes
    nextByte = hexContent.at(registers[7]);
    destReg = hexToDec(nextByte.substr(0,1));
    srcReg = hexToDec(nextByte.substr(1,1));
    if(destReg<0 || destReg>8 || srcReg<0 || srcReg>8) { //error
      registers[7] = regIR;
      interruptError();
      continue;
    }
    short valueFromFirst = registers[destReg];
    registers[destReg] = registers[srcReg];
    registers[srcReg] = valueFromFirst;
    break;
    }
    case 112: {
    nextByte = hexContent.at(registers[7]);
    destReg = hexToDec(nextByte.substr(0,1));
    srcReg = hexToDec(nextByte.substr(1,1));
    if(destReg<0 || destReg>8 || srcReg<0 || srcReg>8) { //error
      registers[7] = regIR;
      interruptError();
      continue;
    }
    registers[destReg] = registers[destReg] + registers[srcReg];
    break;
    }
    case 113: { //sub, 2 bytes
    nextByte = hexContent.at(registers[7]);
    destReg = hexToDec(nextByte.substr(0,1));
    srcReg = hexToDec(nextByte.substr(1,1));
    if(destReg<0 || destReg>8 || srcReg<0 || srcReg>8) { //error
      registers[7] = regIR;
      interruptError();
      continue;
    }
    registers[destReg] = registers[destReg] - registers[srcReg];
    break;
    }
    case 114: { //mul, 2 bytes
    nextByte = hexContent.at(registers[7]);
    destReg = hexToDec(nextByte.substr(0,1));
    srcReg = hexToDec(nextByte.substr(1,1));
    if(destReg<0 || destReg>8 || srcReg<0 || srcReg>8) { //error
      registers[7] = regIR;
      interruptError();
      continue;
    }
    registers[destReg] = registers[destReg] * registers[srcReg];
    break;
    }
    case 115: { //div, 2 bytes
    nextByte = hexContent.at(registers[7]);
    destReg = hexToDec(nextByte.substr(0,1));
    srcReg = hexToDec(nextByte.substr(1,1));
    if(destReg<0 || destReg>8 || srcReg<0 || srcReg>8) { //error
      registers[7] = regIR;
      interruptError();
      continue;
    }
    if(registers[srcReg]==0) { //division by zero
      registers[7] = regIR;
      interruptError();
      continue;
    }
    registers[destReg] = registers[destReg] / registers[srcReg];
    break;
    }
    case 116: { //cmp, 2 bytes
    nextByte = hexContent.at(registers[7]);
    destReg = hexToDec(nextByte.substr(0,1));
    srcReg = hexToDec(nextByte.substr(1,1)); 
    if(destReg<0 || destReg>8 || srcReg<0 || srcReg>8) { //error
      registers[7] = regIR;
      interruptError();
      continue;
    }  
    updateZeroFlag(destReg, srcReg);
    updateOverflowFlag(destReg, srcReg);
    updateCarryFlag(destReg, srcReg);
    updateNegativeFlag(destReg, srcReg);
    break;
    }
    case 128: { //not, 2 bytes
    nextByte = hexContent.at(registers[7]);
    destReg = hexToDec(nextByte.substr(0,1));
    if(destReg<0 || destReg>8) { //error
      registers[7] = regIR;
      interruptError();
      continue;
    }
    registers[destReg] = ~registers[destReg];     
    break;
    }
    case 129: { //and, 2 bytes
    nextByte = hexContent.at(registers[7]);
    destReg = hexToDec(nextByte.substr(0,1));
    srcReg = hexToDec(nextByte.substr(1,1));
    if(destReg<0 || destReg>8 || srcReg<0 || srcReg>8) { //error
      registers[7] = regIR;
      interruptError();
      continue;
    }
    registers[destReg] &= registers[srcReg];   
    break;
    }
    case 130: { //or, 2 bytes
    nextByte = hexContent.at(registers[7]-1);
    destReg = hexToDec(nextByte.substr(0,1));
    srcReg = hexToDec(nextByte.substr(1,1));
    if(destReg<0 || destReg>8 || srcReg<0 || srcReg>8) { //error
      registers[7] = regIR;
      interruptError();
      continue;
    } 
    registers[destReg] |= registers[srcReg];
    break;
    }
    case 131: { //xor, 2 bytes
    nextByte = hexContent.at(registers[7]);
    destReg = hexToDec(nextByte.substr(0,1));
    srcReg = hexToDec(nextByte.substr(1,1));
    if(destReg<0 || destReg>8 || srcReg<0 || srcReg>8) { //error
      registers[7] = regIR;
      interruptError();
      continue;
    } 
    registers[destReg] ^= registers[srcReg];
    break;
    }
    case 132: { //test, 2 bytes
    nextByte = hexContent.at(registers[7]);
    destReg = hexToDec(nextByte.substr(0,1));
    srcReg = hexToDec(nextByte.substr(1,1));
    if(destReg<0 || destReg>8 || srcReg<0 || srcReg>8) { //error
      registers[7] = regIR;
      interruptError();
      continue;
    }
    updateZeroFlag(destReg,srcReg);
    updateNegativeFlag(destReg,srcReg);
    break;
    }
    case 144: { //shl, 2 bytes
    nextByte = hexContent.at(registers[7]-1);
    destReg = hexToDec(nextByte.substr(0,1));
    srcReg = hexToDec(nextByte.substr(1,1));
    if(destReg<0 || destReg>8 || srcReg<0 || srcReg>8) { //error
      registers[7] = regIR;
      interruptError();
      continue;
    }
    registers[destReg] = registers[destReg]<<registers[srcReg];
    updateZeroFlag(destReg,srcReg);
    updateCarryFlag(destReg,srcReg);
    updateNegativeFlag(destReg,srcReg);
    break;
    }
    case 145: { //shr, 2 bytes
    nextByte = hexContent.at(registers[7]);
    destReg = hexToDec(nextByte.substr(0,1));
    srcReg = hexToDec(nextByte.substr(1,1));
    if(destReg<0 || destReg>8 || srcReg<0 || srcReg>8) { //error
      registers[7] = regIR;
      interruptError();
      continue;
    }
    registers[destReg] = registers[destReg]>>registers[srcReg]; 
    updateZeroFlag(destReg,srcReg);
    updateCarryFlag(destReg,srcReg);
    updateNegativeFlag(destReg,srcReg);    
    break;
    }
    default:{ //error in opcode
      registers[7] = regIR;
      interruptError();
      continue;
      break;
      }
  }
  registers[7]++;
  }
}

void Emulator::writeBytesToMemory(unsigned short valueToWrite, unsigned short address){
  unsigned char upperByte = valueToWrite>>8;
  upperByte = upperByte & 0xFF;
  unsigned char lowerByte = valueToWrite & 0xFF;
    memoryContent[address] = lowerByte; //writes in little endian format
    address++;
    memoryContent[address] = upperByte;
}

short Emulator::getOperandFromAddressModeLoad(unsigned char addressMode, unsigned char updateData) {
  bool solved = false;
  unsigned short operand, addr;
  unsigned char firstByte, secondByte;
  unsigned char firstByteOfPom, secondByteOfPom;
  unsigned char regIndex;
  short sourceVal;
  switch(addressMode) {
    case 0: {//immed
    registers[7]++;
      solved=true;
      secondByte = memoryContent[registers[7]++];
      firstByte = memoryContent[registers[7]];
      operand = secondByte<<8; //big endian
      operand += firstByte;
    break;
    }
    case 1: {//regdir
      solved=true;
      operand = registers[hexToDec(hexContent.at(registers[7]-1).substr(1,1))]; //register index is in previous byte
    }
    break;
    case 2: {
      if(updateData!=4 && updateData!=0) { //update bits error
      registers[7] = regIR;
      interruptError();
      }
      else {
        if(updateData==4) { //pop instruction
          registers[6]+=2;
        }
      solved=true;
      addr=registers[hexToDec(hexContent.at(registers[7]-1).substr(1,1))]; //in case of pop the index is 6
      firstByte = memoryContent[addr++];
      secondByte = memoryContent[addr];
      operand = secondByte<<8;
      operand += firstByte;
      }
    break;
    }
    case 3: {//regind with displacement
      addr=registers[hexToDec(hexContent.at(registers[7]-1).substr(1,1))]; //reg index is in previous byte
      registers[7]++;
      unsigned char firstByteOfPom = hexToDec(hexContent.at(registers[7]++));
      unsigned char secondByteOfPom = hexToDec(hexContent.at(registers[7]));
      unsigned short pom = firstByteOfPom<<8;
      pom+=secondByteOfPom; //displacement
      addr = addr + (pom);
      firstByte = memoryContent[addr++];
      secondByte = memoryContent[addr];
      operand = secondByte<<8;
      operand +=firstByte;
      solved=true;
    break;
    }
    case 4: {//memdir
      solved=true;
      registers[7]++;
      addr = memoryContent[registers[7]++]<<8; //big endian
      addr += memoryContent[registers[7]];
      firstByteOfPom = memoryContent[addr++];
      secondByteOfPom = memoryContent[addr];
      operand = secondByteOfPom<<8; //little endian
      operand += firstByteOfPom;
    break;
    }
    case 5: {//regdir with displacement/pcrel
      solved=true;
      regIndex = hexToDec(hexContent.at(registers[7]-1).substr(1,1));
      registers[7]++;
      firstByteOfPom = hexToDec(hexContent.at(registers[7]++));
      secondByteOfPom = hexToDec(hexContent.at(registers[7]));
      operand = firstByteOfPom<<8;
      operand += secondByteOfPom;
      short operand2 = (short)operand; //displacement has to be signed!
      addr = registers[regIndex]+1; //pc
      addr += operand2;
      operand = memoryContent[addr++];
      operand += (memoryContent[addr]<<8); //little endian
    break;
    }
    default:{ //undefined address mode
      registers[7] = regIR;
      interruptError();
    break;
    }
  }
  return operand;
}

void Emulator::executeStoreInstruction(int regIndexDest, unsigned char addressMode, unsigned char updateData) {
 bool solved = false;
  unsigned short operand, addr;
  unsigned char firstByte, secondByte;
  unsigned char firstByteOfPom, secondByteOfPom;
  unsigned char regIndex;
  short sourceVal;
  switch(addressMode){
    case 0: {//immed is not allowed
      registers[7] = regIR;
      interruptError();
    break;
    }
    case 1: {//regdir
      solved=true;
      operand = registers[hexToDec(hexContent.at(registers[7]-1).substr(1,1))]; //index is in previous byte
      registers[operand] = registers[regIndexDest];
    }
    break;
    case 2: {//regind
      if(updateData!=1 && updateData!=0) { //update bits error
      registers[7] = regIR;
      interruptError();
      }
      else {
        if(updateData==1) { //push instruction
          registers[6]-=2;
        }

      addr=registers[hexToDec(hexContent.at(registers[7]-1).substr(1,1))]; //source contains memory address
      operand = registers[regIndexDest];
      writeBytesToMemory(operand, addr);
      solved=true;
      }
    break;
    }
    case 3: {//regind with displacement
      addr=registers[hexToDec(hexContent.at(registers[7]-1).substr(1,1))]; //source contains memory address
      registers[7]++;
      unsigned char firstByteOfPom = hexToDec(hexContent.at(registers[7]++));
      unsigned char secondByteOfPom = hexToDec(hexContent.at(registers[7]));
      unsigned short pom = firstByteOfPom<<8;
      pom+=secondByteOfPom;
      addr = addr + (pom);

      registers[regIndexDest] = operand;
      writeBytesToMemory(operand, addr);
      solved=true;
    break;
    }
    case 4: {//memdir
      solved=true;
      registers[7]++;
      addr = memoryContent[registers[7]++]<<8; //big endian
      addr += memoryContent[registers[7]];
      if(addr>memorySize) { //memory overloaded
      registers[7] = regIR;
      interruptError();
      }
      else {
      operand = registers[regIndexDest];
      writeBytesToMemory(operand, addr);
      }
    break;
    }
    case 5: {//regdir with displacement/pcrel
      solved=true;
      regIndex = hexToDec(hexContent.at(registers[7]-1).substr(1,1));
      registers[7]++;
      firstByteOfPom = hexToDec(hexContent.at(registers[7]++));
      secondByteOfPom = hexToDec(hexContent.at(registers[7]));
      operand = firstByteOfPom<<8;
      operand += secondByteOfPom;
      addr = registers[regIndex]+1;
      addr += operand;
      operand = memoryContent[addr++];
      operand += (memoryContent[addr]<<8); //little endian
    break;
    }
    default:{ //error in addressing type
      registers[7] = regIR;
      interruptError();
    break;
    }
  }
}

short Emulator::getOperandFromAddressModeJmp(unsigned char addressMode, unsigned char updateData) {
  bool solved = false;
  unsigned short operand, addr;
  unsigned char firstByte;
  unsigned char secondByte;
  unsigned char firstByteOfPom, secondByteOfPom;
  char regIndex;
  switch(addressMode){
    case 0: {//immed
    registers[7]++;
      solved=true;
      secondByte = memoryContent[registers[7]++];
      firstByte = memoryContent[registers[7]];
      operand = secondByte<<8; //big endian
      operand += firstByte;
    break;
    }
    case 1: {//regdir
      solved=true;
      operand = registers[hexToDec(hexContent.at(registers[7]-1).substr(1,1))]; //reg index is in previous byte
    break;
    }
    case 2: {//regind
      solved=true;
      addr=registers[hexToDec(hexContent.at(registers[7]-1).substr(1,1))]; 
      firstByte = memoryContent[addr++]; //little endian memory access
      secondByte = memoryContent[addr];
      operand = secondByte<<8;
      operand += firstByte;
    break;
    }
    case 3: {//regind with displacement
      addr=registers[hexToDec(hexContent.at(registers[7]-1).substr(1,1))]; 
      registers[7]++;
      unsigned char firstByteOfPom = hexToDec(hexContent.at(registers[7]++));
      unsigned char secondByteOfPom = hexToDec(hexContent.at(registers[7]));
      unsigned short pom = firstByteOfPom<<8;
      pom+=secondByteOfPom;
      addr = addr + (pom);
      firstByte = memoryContent[addr++]; //little endian memory access
      secondByte = memoryContent[addr];
      operand = secondByte<<8;
      operand += firstByte;
      solved=true;
    break;
    }
    case 4: {//mem
      solved=true;
      registers[7]++;
      addr = memoryContent[registers[7]++]<<8; //big endian
      addr += memoryContent[registers[7]];
      firstByteOfPom = memoryContent[addr++];
      secondByteOfPom = memoryContent[addr];
      operand = secondByteOfPom<<8; //little endian
      operand += firstByteOfPom;
    break;
    }
    case 5: {//pcrel
      solved=true;
      regIndex = hexToDec(hexContent.at(registers[7]-1).substr(1,1)); //register
      registers[7]++;
      firstByteOfPom = hexToDec(hexContent.at(registers[7]++)); //first byte of displacement
      secondByteOfPom = hexToDec(hexContent.at(registers[7])); //second byte of displacement
      addr=registers[regIndex]+1; //pc of next instruction
      operand = firstByteOfPom<<8; //big endian
      operand += secondByteOfPom;
      operand += addr;
    break;
    }
    default:{ //addressing error
      registers[7] = regIR;
      interruptError();
    break;
    }
  }
  return operand;
}

bool Emulator::zeroFlagActivated() {
  if(registers[8]&1) return true;
  return false;
}

bool Emulator::greaterThanFlagsActivated() {
  if(registers[8]&1) return false;
  if(registers[8]&8 ^ registers[8]&2) return false; //negative and overflow bits
  return true;
}

void Emulator::updateOverflowFlag(int destReg, int sourceReg) {
  //has to treat registers as signed short type
  short registerDestReg = (short)registers[destReg];
  short registerSourceReg = (short)registers[sourceReg];
if((registerDestReg>0 && registerSourceReg>0 && registerDestReg+registerSourceReg<0) ||
 (registerDestReg<0 && registerSourceReg<0 && registerDestReg+registerSourceReg<0))
 {
registers[8] = registers[8] | (1<<1);
}
else {
registers[8] = registers[8] & ~(1<<1);
}  
}

void Emulator::updateZeroFlag(int destReg, int sourceReg) {
  short registerDestReg = (short)registers[destReg];
  short registerSourceReg = (short)registers[sourceReg];
if(registerDestReg-registerSourceReg==0) {
  registers[8] = registers[8] | 1; //activated
}
else {
  registers[8] = registers[8] & ~1; //cleared
}
}

void Emulator::updateNegativeFlag(int destReg, int sourceReg) {
    short registerDestReg = (short)registers[destReg];
    short registerSourceReg = (short)registers[sourceReg];
if ((registerDestReg - registerSourceReg) < 0){
  registers[8] = registers[8] | (1<<3); //activated
}
else {
  registers[8] = registers[8] & ~(1<<3); //cleared
}
}

void Emulator::updateCarryFlag(int destReg, int sourceReg) {
  short registerDestReg = (short)registers[destReg];
  short registerSourceReg = (short)registers[sourceReg];
if(registerDestReg<registerSourceReg) { //set carry flag
  registers[8] = registers[8] | (1<<2); //activated
}
else {
  registers[8] = registers[8] & ~(1<<2); //cleared
}  
}

std::string Emulator::decToBin(short dec) { //helper function for emulator output
  std::string binary = std::bitset<16>(dec).to_string();
  return binary;
}

void Emulator::printEmulationData() {
std::cout<<"  ------------------------------------------------ "<<std::endl;
std::cout<<"Emulated processor executed halt instruction"<<std::endl;
std::cout<<"Emulated processor state: psw=0b"<<decToBin(registers[8])<<std::endl;
std::cout<<"r0=0x"<<formatHexOutput(decimalToHex(std::to_string(registers[0])))<<
" r1=0x"<<formatHexOutput(decimalToHex(std::to_string(registers[1])))<<
" r2=0x"<<formatHexOutput(decimalToHex(std::to_string(registers[2])))<<
" r3=0x"<<formatHexOutput(decimalToHex(std::to_string(registers[3])))<<std::endl;
std::cout<<"r4=0x"<<formatHexOutput(decimalToHex(std::to_string(registers[4])))<<
" r5=0x"<<formatHexOutput(decimalToHex(std::to_string(registers[5])))<<
" r6=0x"<<formatHexOutput(decimalToHex(std::to_string(registers[6])))<<
" r7=0x"<<formatHexOutput(decimalToHex(std::to_string(registers[7])))<<std::endl;
}

void Emulator::interruptError() { //for errors
//push psw
registers[6]-=2;
writeBytesToMemory(registers[8], registers[6]);
//push pc
registers[6]-=2;
writeBytesToMemory(registers[7], registers[6]);
initializePCToAddr(2); //entry number 1 starts at address 2, because the entry is 2 bytes
enableInterrupts();
}

void Emulator::enableInterrupts() {
  registers[8] = registers[8] | (1<<15);
}

void Emulator::initializePCToAddr(int startInMem) { //used to direct PC to address in memory
  unsigned short firstByte;
  unsigned short secondByte;
  firstByte = memoryContent[startInMem];
  secondByte = memoryContent[startInMem+1];
  registers[7] = secondByte<<8;
  registers[7] += firstByte;
}