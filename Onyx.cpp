#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <algorithm>

#include "Onyx.h"
using namespace std;


char RESET[] = "\033[0m";
char BOLD[] = "\033[1m";
char DIM[] = "\033[2m";
char GREEN[] = "\033[32m";
char BRIGHT_GREEN[] = "\033[92m";
char CYAN[] = "\033[36m";
char BRIGHT_CYAN[] = "\033[96m";
char WHITE[] = "\033[37m";
char BRIGHT_WHITE[] = "\033[97m";
char YELLOW[] = "\033[33m";
char RED[] = "\033[31m";


//ma dalo ga jo krna ha krlo === :)
//========================= Delay and Animation Utilities======================================

void busyDelay(long long int delayTime) {
    for (long long int i = 0; i < delayTime; i++); //for specific delay 
}

void printSlow(const char text[], long long charDelay = 4000000LL) {
    for (int i = 0; text[i] != '\0'; i++) {  // normal loop
        cout << text[i] << flush;
        busyDelay(charDelay);
    }
}

void clearScreen() {
    cout << "\033[2J\033[1;1H" << flush;
}//Clears the screen AND resets cursor to top left as in video 

void printSeparator(int width = 60) {
    cout << GREEN;
    for (int i = 0; i < width; i++) {
        cout << '-';
    }
    cout << RESET << endl;
}

 
//=====================Verbose Mode Global Flag=======================
 
bool verboseMode = false;

//====================Interpreter Method Implementations======================


vector<string> Interpreter::tokenize(const string& line) {
    vector<string> tokens;
    stringstream ss(line);
    string token;

    while (ss >> token) {
        if (!token.empty() && token.back() == ',') {
            token.pop_back();
        }
        tokens.push_back(token);
    }
    return tokens;
}

uint8_t Interpreter::parseRegister(const string& regStr) {
    string cleanReg = regStr;
    if (cleanReg.front() == '[') cleanReg.erase(0, 1);
    if (cleanReg.back() == ']')  cleanReg.pop_back();

    if (cleanReg.size() >= 2 && cleanReg[0] == 'R') {
        int regNum = cleanReg[1] - '0';
        if (regNum >= 0 && regNum < 8) return static_cast<uint8_t>(regNum);
    }
    return 0;
}

vector<uint16_t> Interpreter::parseLine(const string& line) {
    size_t commentPos = line.find("//");
    string cleanLine = (commentPos != string::npos) ? line.substr(0, commentPos) : line;

    vector<string> tokens = tokenize(cleanLine);
    if (tokens.empty()) return { 0x0000 }; // NOP

    string mnemonic = tokens[0];
    vector<uint16_t> payload;

    if (mnemonic == "AARAM" || mnemonic == "NOP") {
        payload.push_back(0x0000);
    }
    else if (mnemonic == "JAMA" || mnemonic == "ADD") {
        uint8_t dest = parseRegister(tokens[1]);
        uint8_t src = parseRegister(tokens[2]);
        payload.push_back((0x01 << 8) | ((dest & 0x0F) << 4) | (src & 0x0F));
    }
    else if (mnemonic == "TAFREEK" || mnemonic == "SUB") {
        uint8_t dest = parseRegister(tokens[1]);
        uint8_t src = parseRegister(tokens[2]);
        payload.push_back((0x02 << 8) | ((dest & 0x0F) << 4) | (src & 0x0F));
    }
    else if (mnemonic == "ZARAB" || mnemonic == "MUL") {
        uint8_t dest = parseRegister(tokens[1]);
        uint8_t src = parseRegister(tokens[2]);
        payload.push_back((0x03 << 8) | ((dest & 0x0F) << 4) | (src & 0x0F));
    }
    else if (mnemonic == "TAQSEEM" || mnemonic == "DIV") {
        uint8_t dest = parseRegister(tokens[1]);
        uint8_t src = parseRegister(tokens[2]);
        payload.push_back((0x04 << 8) | ((dest & 0x0F) << 4) | (src & 0x0F));
    }
    else if (mnemonic == "MUWAZANA" || mnemonic == "CMP") {
        uint8_t dest = parseRegister(tokens[1]);
        uint8_t src = parseRegister(tokens[2]);
        payload.push_back((0x0A << 8) | ((dest & 0x0F) << 4) | (src & 0x0F));
    }
    else if (mnemonic == "CHHALANG" || mnemonic == "JMP") {
        uint8_t src = parseRegister(tokens[1]);
        payload.push_back((0x10 << 8) | (src & 0x0F));
    }
    else if (mnemonic == "AGAR_SIFAR" || mnemonic == "JZ") {
        uint8_t src = parseRegister(tokens[1]);
        payload.push_back((0x11 << 8) | (src & 0x0F));
    }
    else if (mnemonic == "AGAR_MAUJOOD" || mnemonic == "JNZ") {
        uint8_t src = parseRegister(tokens[1]);
        payload.push_back((0x12 << 8) | (src & 0x0F));
    }
    else if (mnemonic == "BHARO" || mnemonic == "DAALO" || mnemonic == "LDR_IMM") {
        uint8_t dest = parseRegister(tokens[1]);
        uint16_t immFull = 0;

        bool isNumber = isdigit(tokens[2][0]) || (tokens[2].size() > 1 && tokens[2][1] == 'x');
        if (isNumber) {
            try {
                immFull = stoi(tokens[2], nullptr, 0);
            }
            catch (...) {
                cout << "[COMPILER ERROR] Invalid immediate format: " << tokens[2] << endl;
            }
        }
        else {
            if (labelMap.find(tokens[2]) != labelMap.end()) {
                immFull = labelMap[tokens[2]];
            }
            else {
                cout << "[COMPILER ERROR] Unresolved Label: " << tokens[2] << endl;
            }
        }

        // If it's a Label, it was tracked as 4 bytes in Pass 1, so we must force Format D
        if (!isNumber || immFull > 0x0F) {
            // Requires 2-word 16-bit payload (Format D)
            payload.push_back((0x1B << 8) | ((dest & 0x0F) << 4)); // Header word
            payload.push_back(immFull);                             // Payload word
        }
        else {
            // Fits in 4 bits (Format B)
            payload.push_back((0x1A << 8) | ((dest & 0x0F) << 4) | (immFull & 0x0F));
        }
    }
    else if (mnemonic == "PARHO" || mnemonic == "LDR") {
        uint8_t dest = parseRegister(tokens[1]);
        uint8_t src = parseRegister(tokens[2]);
        payload.push_back((0x20 << 8) | ((dest & 0x0F) << 4) | (src & 0x0F));
    }
    else if (mnemonic == "RAKHO" || mnemonic == "STR") {
        uint8_t srcVal = parseRegister(tokens[1]);
        uint8_t ptrReg = parseRegister(tokens[2]);
        payload.push_back((0x21 << 8) | ((srcVal & 0x0F) << 4) | (ptrReg & 0x0F));
    }
    else {
        cout << "[COMPILER ERROR] Unrecognized Mnemonic: " << mnemonic << endl;
    }

    return payload;
}

CompilationResult Interpreter::compile(const string& sourceCode) {
    CompilationResult result;
    stringstream ss(sourceCode);
    string line;

    // storing pairs of {type, content}
    // type 0 = code, type 1 = data
    vector<pair<int, string>> processLines;

    labelMap.clear();
    uint16_t currentCodePC = 0;
    uint16_t currentDataPC = 0x0800;
    bool inDataSection = false;

    // Pass 1: PC Tracking and Label Mapping
    while (getline(ss, line)) {
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        if (line.empty() || line.find("//") == 0) continue;

        size_t commentPos = line.find("//");
        string cleanLine = (commentPos != string::npos) ? line.substr(0, commentPos) : line;
        cleanLine.erase(cleanLine.find_last_not_of(" \t\r\n") + 1);

        if (cleanLine == ".MAWAAD" || cleanLine == ".DATA" ||
            cleanLine == ".mawaad" || cleanLine == ".data") {
            inDataSection = true;
            continue;
        }
        if (cleanLine == ".HIDAYAT" || cleanLine == ".CODE" ||
            cleanLine == ".hidayat" || cleanLine == ".code") {
            inDataSection = false;
            continue;
        }

        if (inDataSection) {
            size_t colonPos = cleanLine.find(':');
            if (colonPos != string::npos) {
                string labelName = cleanLine.substr(0, colonPos);
                labelName.erase(labelName.find_last_not_of(" \t") + 1);
                labelMap[labelName] = currentDataPC;

                string valueStr = cleanLine.substr(colonPos + 1);
                valueStr.erase(0, valueStr.find_first_not_of(" \t"));

                processLines.push_back({ 1, valueStr });

                if (!valueStr.empty() && valueStr.front() == '"') {
                    // String literal
                    int chars = 0;
                    for (size_t i = 1; i < valueStr.size() && valueStr[i] != '"'; ++i) {
                        if (valueStr[i] == '\\' && i + 1 < valueStr.size()) {
                            i++; // escape sequence
                        }
                        chars++;
                    }
                    int words = (chars + 1) / 2;
                    currentDataPC += words * 2;
                }
                else {
                    // Integer array or single int
                    int commas = count(valueStr.begin(), valueStr.end(), ',');
                    int words = commas + 1;
                    currentDataPC += words * 2;
                }
            }
        }
        else {
            if (cleanLine.back() == ':') {
                string labelName = cleanLine.substr(0, cleanLine.size() - 1);
                labelMap[labelName] = currentCodePC;
                continue; // Label takes 0 bytes
            }

            processLines.push_back({ 0, cleanLine });

            vector<string> tokens = tokenize(cleanLine);
            if (tokens.empty()) continue;

            // Calculate instruction byte size
            string mnemonic = tokens[0];
            if (mnemonic == "BHARO" || mnemonic == "DAALO" || mnemonic == "LDR_IMM") {
                if (tokens.size() > 2) {
                    bool isNumber = isdigit(tokens[2][0]) || (tokens[2].size() > 1 && tokens[2][1] == 'x');
                    if (isNumber) {
                        try {
                            uint16_t immFull = stoi(tokens[2], nullptr, 0);
                            if (immFull <= 0x0F) currentCodePC += 2;
                            else                 currentCodePC += 4; // Format D
                        }
                        catch (...) {
                            currentCodePC += 4; // Fallback
                        }
                    }
                    else {
                        currentCodePC += 4; // Labels unconditionally 2 words
                    }
                }
                else {
                    currentCodePC += 2;
                }
            }
            else {
                currentCodePC += 2; // Fixed width defaults
            }
        }
    }

    // Pass 2: Instruction Generation
    for (const auto& item : processLines) {
        if (item.first == 1) { // DATA
            string valueStr = item.second;
            if (!valueStr.empty() && valueStr.front() == '"') {
                vector<uint8_t> bytes;
                for (size_t i = 1; i < valueStr.size() && valueStr[i] != '"'; ++i) {
                    if (valueStr[i] == '\\' && i + 1 < valueStr.size()) {
                        i++;
                        if (valueStr[i] == '0') bytes.push_back(0);
                        else if (valueStr[i] == 'n') bytes.push_back('\n');
                        else                         bytes.push_back(valueStr[i]);
                    }
                    else {
                        bytes.push_back(valueStr[i]);
                    }
                }
                if (bytes.size() % 2 != 0) bytes.push_back(0); // Word alignment
                for (size_t i = 0; i < bytes.size(); i += 2) {
                    result.dataSegment.push_back((bytes[i] << 8) | bytes[i + 1]);
                }
            }
            else {
                stringstream vss(valueStr);
                string token;
                while (getline(vss, token, ',')) {
                    token.erase(0, token.find_first_not_of(" \t"));
                    token.erase(token.find_last_not_of(" \t") + 1);
                    try {
                        uint16_t val = stoi(token, nullptr, 0);
                        result.dataSegment.push_back(val);
                    }
                    catch (...) {
                        result.dataSegment.push_back(0);
                    }
                }
            }
        }
        else { // CODE
            vector<uint16_t> words = parseLine(item.second);
            for (uint16_t word : words) {
                result.codeSegment.push_back(word);
            }
        }
    }

    return result;
}

void Interpreter::loadProgramAndFlash(const string& filename, MemoryModule& ram) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "[BIOS FAULT] Could not locate OS payload: " << filename << endl;
        return;
    }

    stringstream buffer;
    buffer << file.rdbuf();
    string targetOS = buffer.str();

    CompilationResult compiledOS = compile(targetOS);
    cout << "[BIOS] Compiled OS script into " << compiledOS.codeSegment.size()
        << " native 16-bit instruction words." << endl;

    cout << "[BOOTLOADER] Flashing Code Execution block to RAM Address 0x0000..." << endl;
    uint16_t codeAddress = 0x0000; //look ibtassam bhai also use uinst16_t i told you it convert int to 2 byte but i used short 
    for (uint16_t word : compiledOS.codeSegment) {
        ram.loadRawBinary(codeAddress, (word >> 8) & 0xFF);     // High byte (opcode)
        ram.loadRawBinary(codeAddress + 1, word & 0xFF);     // Low byte (operands)
        codeAddress += 2;
    }

    cout << "[BOOTLOADER] Flashing Data Variables (.MAWAAD) to RAM Address 0x0800..." << endl;
    uint16_t dataAddress = 0x0800;
    for (uint16_t word : compiledOS.dataSegment) {
        ram.loadRawBinary(dataAddress, (word >> 8) & 0xFF);
        ram.loadRawBinary(dataAddress + 1, word & 0xFF);
        dataAddress += 2;
    }

    cout << "[BOOTLOADER] Flash Complete." << endl;
    cout << "------------------------------------------------------------" << endl;
}


//Processor Method Implementations

//Read cache memory (8.2)
unsigned char Processor::processReadMemory(unsigned short address) {
    if (!systemBus || !board) 
        return 0xFF; //system fault

    if (address >= 0x0F00) {
        systemBus->setAddress(address);
        systemBus->assertREAD();
        board->pulseClock();
        unsigned char val = systemBus->getData();
        systemBus->releaseControl();
        return val;
    }

    //check cache
    if (cacheValid && address >= cacheBaseAddress && address < cacheBaseAddress + 16) {
        return cacheBlock[address - cacheBaseAddress];
    }

    unsigned short alignedBase = address & 0xFFF0;
    for (int i = 0; i < 16; i++) {
        systemBus->setAddress(alignedBase + i);
        systemBus->assertREAD();
        board->internalBusCycle();
        cacheBlock[i] = systemBus->getData();
        systemBus->releaseControl();
    }
    cacheBaseAddress = alignedBase;
    cacheValid = true;

    return cacheBlock[address - cacheBaseAddress];
}

//Write cache memory (8.2)
void Processor::processWriteMemory(unsigned short address, unsigned char data) {
    if (!systemBus || !board) return; //system fault

    //do bus write
    systemBus->setAddress(address);
    systemBus->setData(data);
    systemBus->assertWRITE();
    board->pulseClock();
    systemBus->releaseControl();

    //check cache invalidation
    if (cacheValid && address >= cacheBaseAddress && address < cacheBaseAddress + 16) {
        cacheValid = false;
    }
}

//fetch decode execute loop (8.4)
void Processor::tick() {
    if (!systemBus || !board) return;

    //phase 1 fetch
    unsigned short pc = coreRegs.getPC();

    unsigned char highByte = processReadMemory(pc);
    unsigned char lowByte = processReadMemory(pc + 1);
    unsigned short instruction = (highByte << 8) | lowByte;

    coreRegs.setIR(instruction);
    coreRegs.setPC(pc + 2);

    //phase 2 decode
    unsigned char opcode = highByte;
    DecodeSignal sig = decodeMatrix[opcode];

    if (!sig.IS_VALID) {
        cout << RED << "[FATAL CPU FAULT] Invalid Opcode: 0x" << hex << (int)opcode << RESET << endl;
        return;
    }

    //halt instruction
    if (opcode == 0x00) {
        cout << GREEN << "[PROCESSOR] End-Of-File (0x0000) reached at PC 0x"
            << hex << coreRegs.getPC() << ". Halting." << RESET << endl;
        coreRegs.set_haltFlag();
        return;
    }

    unsigned char destReg = (lowByte >> 4) & 0x0F;
    unsigned char srcReg = lowByte & 0x0F;

    //phase 3 execute
    unsigned short finalResult = 0;
    unsigned short srcValue = coreRegs.getRegister(srcReg);
    unsigned short destValue = coreRegs.getRegister(destReg);

    if (sig.IS_16BIT_IMMEDIATE) {
        unsigned short newPc = coreRegs.getPC();
        unsigned char immHigh = processReadMemory(newPc);
        unsigned char immLow = processReadMemory(newPc + 1);
        srcValue = (immHigh << 8) | immLow;
        coreRegs.setPC(newPc + 2);

        if (verboseMode) {
            cout << CYAN << "  -> Decode: Format D (2-Word). Fetched Immediate: 0x" << hex << srcValue << RESET << endl;
        }
    }
    else if (sig.IS_4BIT_IMMEDIATE) {
        srcValue = srcReg;
    }

    if (sig.USES_AEC) {
        finalResult = coreRegs.ALU(sig.AEC_OP, destValue, srcValue, coreRegs.getFlagsRef());
    }
    else {
        finalResult = srcValue;
    }

    if (sig.IS_MEMORY_READ) {
        //put in low byte
        unsigned char val = processReadMemory(srcValue);
        finalResult = (unsigned short)val;

        if (verboseMode) {
            cout << CYAN << "  -> Exec: Read 0x" << hex << (int)val
                << " from Mem Address 0x" << hex << srcValue << RESET << endl;
        }
    }
    else if (sig.IS_MEMORY_WRITE) {
        processWriteMemory(srcValue, destValue & 0xFF);
    }

    if (sig.WRITES_TO_REGISTER) {
        coreRegs.writeRegister(destReg, finalResult);

        if (verboseMode) {
            cout << CYAN << "  -> Writeback: R" << dec << (int)destReg << " = " << dec << finalResult << " (0x" << hex << finalResult << ")" << RESET << endl;
        }
    }

    if (sig.IS_BRANCH) {
        bool shouldBranch = false;
        unsigned char flags = coreRegs.getFlags();
        if (sig.BRANCH_ON_ZERO && (flags & 0x01)) shouldBranch = true;
        else if (sig.BRANCH_ON_NOT_ZERO && !(flags & 0x01)) shouldBranch = true;
        else if (!sig.BRANCH_ON_ZERO && !sig.BRANCH_ON_NOT_ZERO) shouldBranch = true;

        if (shouldBranch) {
            coreRegs.setPC(srcValue);
        }
    }
}


// Main Entry Point
void mystrCopy(char* destination, const char* source) {
    int i = 0;

    while (source[i] != '\0') {
        destination[i] = source[i];
        i++;
    }

    destination[i] = '\0';
}

int main() {
      //UI
  
    clearScreen();
    busyDelay(300000000LL);
    cout << "\n\n\n\n";
    cout << "  " << DIM << WHITE;
    printSlow("L a z a r u s   M a c h i n a e", 7000000LL);
    cout << RESET << endl;
    busyDelay(700000000LL);
    clearScreen();
    cout << "\n\n";
    cout << "  " << BOLD << BRIGHT_CYAN;
    printSlow("O N Y X  -  1 6", 6000000LL);
    cout << RESET << "\n";
    cout << "  " << WHITE;
    printSlow("16-Bit Virtual Turing Architecture", 3000000LL);
    cout << RESET << "\n";
    busyDelay(600000000LL);
    clearScreen();
    cout << "\n";
    cout << GREEN;
    printSlow("---- BOOTING SILICON PROTOCOL ----", 3000000LL);
    cout << RESET << "\n";
    cout << WHITE;
    printSlow( "[BIOS] Initializing Custom OS Interpreter...", 2000000LL);
    cout << RESET << "\n";
    busyDelay(400000000LL);
    cout << "\n";
    cout << GREEN << "=====================================" << RESET << endl;
    cout << GREEN << "        O N Y X   B O O T   M E N U" << RESET << endl;
    cout << GREEN << "=====================================" << RESET << endl;
    cout << "\n";
    cout << BRIGHT_CYAN << "  [1] Turing Complete Roman Urdu Calculator" << RESET << endl;
    cout << BRIGHT_CYAN << "  [2] 'HELLO WORLD' Urdu Printer" << RESET << endl;
    cout << BRIGHT_CYAN << "  [3] Hardware Authentication Firewall" << RESET << endl;
    cout << "\n";
    cout << WHITE << "Selection (1-3): " << RESET;
    int choice;
    cin >> choice;

    char filename[20];
   

    switch (choice) {
    case 1:
        mystrCopy(filename, "calculator.txt");
        break;
    case 2:
        mystrCopy(filename, "hello.txt");
        break;
    case 3:
        mystrCopy(filename, "auth.txt");
        break;
    default:
        cout << "[BIOS FAULT] Invalid selection. Halting." << endl;
        return 1;
    }

    // Verbose debug prompt
    cout << WHITE << "Enable Cycle-by-Cycle Verbose Debug Logging? (Y/N): " << RESET;
    char debugChoice;
    cin >> debugChoice;
    verboseMode = (debugChoice == 'Y' || debugChoice == 'y');

    
    // Hardware Initialization
   
    mainboard mb;
    Processor cpu;
    MemoryModule ram;
    Keyboard kb;
    PhosphorDisplay monitor;
    GraphicsAdapter gpu;
    PowerSupplyUnit psu(500);

    gpu.connectDisplay(&monitor);

    mb.plugInPSU(&psu);
    mb.plugInMemory(&ram);
    mb.plugInGraphics(&gpu);
    mb.plugInKeyboard(&kb);
    mb.plugInProcessor(&cpu);

    cpu.setVerbose(verboseMode);

  
    // Flash Program and Run
    
    Interpreter compiler;
    compiler.loadProgramAndFlash(filename, ram);

    int cycleCount = 0;
    while (mb.systemPower && cycleCount < 10000) {
        cpu.tick();
        cycleCount++;
        if (cpu.isHalted()) break;
    }

   
    // Processor State Dump
   
    cout << "\n";
    printSeparator(60);
    cout << "\n";
    cout << GREEN << BOLD << "==== PROCESSOR STATE DUMP ====" << RESET << endl;
    cout << GREEN << "Temperature: " << fixed << setprecision(1)<< mb.systemTemperature << " C (Max: 90 C)" << RESET << endl;
    cout << "\n";
    cout << GREEN << "--- INTERNAL REGISTERS (16-bit) ---" << RESET << endl;
    for (int i = 0; i < 8; i++) {
        cout << GREEN << "R" << dec << i << ": 0x"<< hex << setw(4) << setfill('0') << cpu.getRegVal(i) << RESET << endl;
    }
    cout << GREEN << "PC  : 0x" << hex << setw(4) << setfill('0') << cpu.getPC() << RESET << endl;
    cout << GREEN << "IR  : 0x" << hex << setw(4) << setfill('0') << cpu.getIRVal() << RESET << endl;
    cout << GREEN << "FLAGS: 0x" << hex << setw(2) << setfill('0') << (int)cpu.getFlagsVal() << RESET << endl;
    cout << GREEN << "==========================" << RESET << endl;

    
    // Phosphor Display Render
   
    cout << "\n";
    cout << GREEN << "+--------------------------------+" << RESET << endl;
    cout << GREEN << "| PHOSPHOR CRT DISPLAY RENDER   |" << RESET << endl;
    monitor.render();

    return 0;
 }