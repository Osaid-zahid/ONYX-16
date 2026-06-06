#pragma once
//use for only interpreter.h
#include <vector>
#include <string>
#include <cstdint>
#include <map>
//for motherboard 
#include <iostream>
using namespace std;
class mainboard;
class Processor;
enum AEC_OPERATION { //i have use snake case to provide them a sense of uniqueness plus its my opinion if you disagree kinldy change them to camal casting but in register.h change it again to camalcasting 
    AEC_ADD,
    AEC_SUB,
    AEC_MUL,
    AEC_DIV,
    AEC_AND,
    AEC_OR,
    AEC_XOR,
    AEC_NOT,
    AEC_INC,
    AEC_DEC,
    AEC_CMP
};
class TheSystemBuses {
private:
    unsigned short Address;// 16-bit so short is taken  it will Carries the target memory address for the pending function
    unsigned char Data;// 8-bit it will Carries the data being read from or written to memory 
    bool readEnable;     // READ ENABLE signal line
    bool writeEnable;    // WRITE ENABLE signal line
public://the main bugggg
    TheSystemBuses() :Address(0x0000), Data(0x00), readEnable(false), writeEnable(false) {

    }
    // Address Bus 
    void setAddress(unsigned short addr) {
        Address = addr;
    }
    unsigned short getAddress()const {
        return Address;
    }

    //  Data Bus 
    void  setData(unsigned char byte) {
        Data = byte;
    }
    unsigned char getData()const {
        return Data;
    }

    //  Control Bus
    void assertREAD() {
        writeEnable = false;   // only one at a time
        readEnable = true;
    }

    void assertWRITE() {
        readEnable = false;   // only one at a time
        writeEnable = true;
    }

    bool isReadEnabled() const {
        return readEnable;
    }
    bool isWriteEnabled() const {
        return writeEnable;
    }

    // Mainboard calls this after every transaction auto de-assert
    void releaseControl() {
        readEnable = false;
        writeEnable = false;
    }
};
class MemoryModule {
    //you know the reason of short 0x00-0x999
private:
    unsigned short validMin;
    unsigned short validMax;
    unsigned short faultOrigin;
    unsigned short capacity;
    unsigned char  cellWidth;//look the main thing if he write at 0x000 or beyon i will push seg error no return 
    unsigned char  powerOn;
    unsigned char* cells;

public:
    /* directly copy pasted from the assignement
    Property
    Value
    Total Capacity
    Valid Address Range
    Cell Width
    3,840 bytes
    0x0000 – 0x0EFF (inclusive)
    8 bits (byte-addressed)
    Power-On State
    All cells initialized to 0x00
    */
    MemoryModule() {
        validMin = 0x0000;//min and max value you know 0x000-0x0eff
        validMax = 0x0EFF;
        faultOrigin = 0x0F00;
        capacity = 3840;//as you can see about 
        cellWidth = 8;//as mention 8 width 
        powerOn = 0x00;

        cells = new unsigned char[capacity];
        for (unsigned short i = 0; i < capacity; i++)
            cells[i] = powerOn;
    }

    ~MemoryModule() {
        delete[] cells;
    }
    /*
    Out-of-Bounds Access: Any read or write targeting an address of 0x0F00 or higher constitutes a
    Segmentation Fault. On an invalid read, the Data Pathway floats to 0xFF. On an invalid write, the data is
    discarded. In both cases, a hardware fault diagnostic message is emitted.
    */

    unsigned char read(unsigned short address) const {
        if (address >= faultOrigin) {
            emitFault("READ", address);
            return 0xFF;//direct lines from assingment  On an invalid read, the Data Pathway floats to 0xFF.
        }
        return cells[address];
    }

    void loadRawBinary(unsigned short address, unsigned char data) {
        if (address >= faultOrigin) {
            emitFault("WRITE", address);
            return; //direct line from assignemnt : On an invalid write, the data is  discarded.
        }
        cells[address] = data;
    }

private:
    void emitFault(const char* operation, unsigned short address) const {
        //i did not included the std so keep an eye on it
        cout << "[ Segmentation Fault ] " << operation << " at 0x" << hex << address << " exceeds valid range (0x0000–0x0EFF)" << endl;
    }
};

// It is a monolithic sealed unit
class Register {
    /*it is not a
collection of loose components. It houses eleven individually sealed storage cells, each accessible only
through a defined interface. No circuit outside the Storage Bank, including the Processor itself, may
observe or modify the internal charge state of any cell by any means other than those signal lines. */


/*2.3 Access Constraint
All these registers must only be accessible to the outside using 2 ways:
● READ: Returns the registers current value
● WRITE: Writes the passed value to a register
There is no other way of editing or accessing the register bank. aka getters or setters
*/

private:
    //since the registed said from 0x0000 to 0xfff so it says unsigned from what i have learned since mention that they be initialized by 0x00000
    unsigned short registers[8];
    //here i use unsigned you know ffor 0x00 and short for 2 bytes and array of 8 means 16 byte of 8 registers which is what req
    //another optimal way is us uint16_t ie unsigned int 16 type here it exactly create an integer and read 16 bytes
    //baki same
    unsigned short programCounter; //the byte address of the next instruction to fetch
    unsigned short instructionRegister;// holds the currently executing 16-bit instruction word 
    unsigned char flags; //here  i used char  regarless of bool bool has 1 byte , but bool is not a container like it stores only true false form
    /*our FLAGS register is:

    ZF = 0x01
    NF = 0x02
    PF = 0x04
    HF = 0x80
    You are storing multiple independent bits inside one byte
    here if i use bool
    bool ZF;
    bool NF;
    bool PF;
    bool HF;
    Problems:

    Not a real FLAGS register anymore
    You lose bitmask behavior (0x01, 0x02 )
    You cannot preserve unused bits
    You break your spec (which explicitly says bits must be preserved)
    */
    const unsigned char ZF = 0x01;
    const unsigned char NF = 0x02;
    const unsigned char PF = 0x04;
    const unsigned char HF = 0x80;
    //here these are very use ful for us in writing code 
    void clearArithmeticFlags() {
        // preserve all other bits, only clear ZF/NF/PF
        flags &= ~(ZF | NF | PF); //normally in memory assuming the following case
        // pf be 000001
        // nf be 000010
        // zf be 000100
        //then its or be 
        //000111
        //hence its not be
        //111000
        //now when we and it we preserve the previous one while flip the other 
    }
public:
    Register() {
        //in the constructo we will intialize all that stuff with 0x0000 as mentioned " All the registers must be initialized to 0x0000 state at startup"
        for (int i = 0; i < 8; i++) { registers[i] = 0x0000; }//i tried your way so you can understand what to do 
        programCounter = 0x0000;
        instructionRegister = 0x0000;
        flags = 0x0000;


    }
    //ya mat kah dana === ya gpt dalata ha lol joke :P 
    // ok sir :)
    //=============read and write====================
    /*All these registers must only be accessible to the outside using 2 ways:
    ● READ: Returns the registers current value
    ● WRITE: Writes the passed value to a register
    There is no other way of editing or accessing the register bank. */

    unsigned short getRegister(int registerNumber) const {
        if (registerNumber < 0 || registerNumber >= 8)
            return 0x0000; // or handle error

        return this->registers[registerNumber];
    }
    void writeRegister(int registerNumber, unsigned short value) {
        if (registerNumber < 0 || registerNumber >= 8)
            return; // or handle error

        registers[registerNumber] = value;
    }

    unsigned short getPC() const { 
        return programCounter;
    }
    void setPC(unsigned short pc) {
        programCounter = pc;
    }

    unsigned short getIR() const {
        return instructionRegister; 
    }
    void setIR(unsigned short ir) {
        instructionRegister = ir;
    }

    unsigned char getFlags() const { 
        return flags;
    }
    unsigned char& getFlagsRef() {
        return flags; 
    }

    //=================[MAIN FLAGS]===============
    void set_Zero() {
        if (flags & HF) return; // halted it will be there irespective of half flag 
        clearArithmeticFlags();
        flags |= ZF;   // ZF
        //since it is mentioned that 
        //A comparison found both operands equal, or an 
      //arithmetic result was exactly 0x0000 
      //here in that case we need to se the zer of last bit to one as by the name suggest since that operation fond and 
      //taking or with hexa 0x01 means with 0000000001 ie 1 or which it 


    }

    void set_NegativeFlag() {
        if (flags & HF) return;
        clearArithmeticFlags();
        flags |= NF;
    }

    void set_positiveFlag() {
        if (flags & HF) return;
        clearArithmeticFlags();
        flags |= PF;
    }

    void set_haltFlag() {
        flags |= HF;
    }

    //very important thing as mentioned register is

    //everything 
    /*
     INITIAL STATE

    START

    FLAGS 00000000

    MEANING

    ZF 0
    NF 0
    PF 0
    HF 0


     STEP 1 RESULT IS ZERO

    WE CALL

    set Zero

    OPERATION

    CLEAR Z N P

    00000000 AND NOT 00000111 EQUAL 00000000

    SET ZF

    00000000 OR 00000001 EQUAL 00000001


    RESULT

    FLAGS 00000001

    ZF 1
    NF 0
    PF 0

    MUTUAL EXCLUSION IS SATISFIED


     STEP 2 NEXT OPERATION GIVES NEGATIVE RESULT

    WE CALL

    set NegativeFlag

    STEP 1 CLEAR Z N P

    00000001 AND NOT 00000111 EQUAL 00000000

    STEP 2 SET NF

    00000000 OR 00000010 EQUAL 00000010


    RESULT

    FLAGS 00000010

    ONLY NF IS SET
    ZF AND PF CLEARED

    MUTUAL EXCLUSION STILL SATISFIED


     STEP 3 HALT OCCURS

    NOW CPU OVERHEATS

    set haltFlag

    OPERATION

    FLAGS EQUAL FLAGS OR 10000000

    BEFORE

    00000010

    AFTER

    10000010


     FINAL STATE

    FLAGS 10000010

    MEANING

    HF 1
    NF 1
    ZF 0
    PF 0


     RULE CHECK


    RULE 1 MUTUAL EXCLUSION ZF NF PF

    ZF 0
    NF 1
    PF 0

    ONLY ONE IS ACTIVE

    RULE HOLDS


    RULE 2 HALT BEHAVIOR

    HF 1

    CPU STOPS EXECUTION
    NO FURTHER CHANGES ALLOWED
    PERMANENT STATE

    RULE HOLDS


     KEY PROOF INSIGHT

    EVEN AFTER HALT

    NF REMAINS SET
    ZF PF ARE IRRELEVANT
    CPU IGNORES EVERYTHING

    HF DOES NOT INTERFERE WITH Z N P RULE
    Z N P RULE DOES NOT INTERFERE WITH HF

    THEY OPERATE ON DIFFERENT BITS


     FINAL PROOF SUMMARY

    ZF NF PF MUTUAL EXCLUSION VALID ALWAYS
    HF TERMINAL BEHAVIOR INDEPENDENT
    BOTH TOGETHER NO CONFLICT


     ONE LINE PROOF

    YOU CAN ENFORCE Z N P EXCLUSIVITY ON BITS 0 TO 2 WHILE HF INDEPENDENTLY CONTROLS BIT 7 WITHOUT OVERLAP SO BOTH RULES COEXIST SAFELY IN ONE 8 BIT REGISTER
    */
    //hope you understand after knowing it delete the required comments these comments are written for your understanding   




    /*This is the circuit responsible for all the computation. It is a stateless block and it has no memory of past
    invocations.
    */

    /*
    WORKING
    3.1 Interface
    The ALU receives the following inputs upon invocation:
    1. An operator selector (OPCODE in the instruction)
    2. Operand A: a 16-bit value in the destination register.
    3. Operand B: a 16-bit value in the source register.
    4. A direct connection point to the FLAGS register.
    The ALU provides only one output:
    ● A 16-bit result value - It's written back to the destination register.
    The FLAGS registers update is never outputted. The ALU has a permanent connection to it and performs
    updates on it upon every execution. The invoking point of the ALU just checks the FLAGS register after
    execution to see if it's updated.
    */
    //SINCE ALU HAS NO INSTANCE IT IS NOT AN OBJECT THE VERY VERY IMPORTANT THING 

    //=========in the processor .h enum — Which operation the AEC should perform (ADD, SUB, MUL, etc.).  copied from assingment ab is m - ha to mana gpt ni kia :( ----delete krdio is comment ko 

    //here a and b are the operand one and two while op is the operator 
    unsigned short ALU(AEC_OPERATION op, unsigned short A, unsigned short B, unsigned char& flags) {

        unsigned short result = 0;
        switch (op) {

        case AEC_ADD:
            result = A + B;
            flags &= ~(ZF | NF | PF);
            if (result == 0) flags |= ZF;
            break;

        case AEC_SUB:
            result = A - B;
            flags &= ~(ZF | NF | PF);
            if (result == 0) flags |= ZF;
            break;

        case AEC_MUL:
            result = A * B;
            flags &= ~(ZF | NF | PF);
            if (result == 0) flags |= ZF;
            break;

        case AEC_DIV:
            if (B == 0) result = 0x0000;
            else        result = A / B;
            flags &= ~(ZF | NF | PF);
            if (result == 0) flags |= ZF;
            break;

        case AEC_AND:
            result = A & B;
            flags &= ~(ZF | NF | PF);
            if (result == 0) flags |= ZF;
            break;

        case AEC_OR:
            result = A | B;
            flags &= ~(ZF | NF | PF);
            if (result == 0) flags |= ZF;
            break;

        case AEC_XOR:
            result = A ^ B;
            flags &= ~(ZF | NF | PF);
            if (result == 0) flags |= ZF;
            break;

        case AEC_NOT:
            result = ~A;
            flags &= ~(ZF | NF | PF);
            if (result == 0) flags |= ZF;
            break;

        case AEC_INC:
            result = A + 1;
            flags &= ~(ZF | NF | PF);
            if (result == 0) flags |= ZF;
            break;

        case AEC_DEC:
            result = A - 1;
            flags &= ~(ZF | NF | PF);
            if (result == 0) flags |= ZF;
            break;

        case AEC_CMP:
            flags &= ~(ZF | NF | PF);
            if (A == B)
                flags |= ZF; // ZF: both operands equal
            else if (A < B)
                flags |= NF; // NF: left operand strictly less than right
            else
                flags |= PF; // PF: left operand strictly greater than right
            return 0;
        }

        return result;
    }

};
class PhosphorDisplay {
private:
    char** screen;
    int pxRow;
    int pxCol;
public:
    PhosphorDisplay() {
        pxRow = 0;
        pxCol = 0;
        screen = new char* [16];
        for (int i = 0; i < 16; i++) {
            screen[i] = new char[32];
            for (int j = 0; j < 32; j++) {
                screen[i][j] = ' ';
            }
        }
    }

    ~PhosphorDisplay() {
        for (int i = 0; i < 16; i++) {
            delete[] screen[i];
        }
        delete[] screen;
    }
    void writeChar(char c) {
        if (c == '\n') {
            pxRow++;
            pxCol = 0;
            return;

        }
        if (pxRow < 16) {
            screen[pxRow][pxCol] = c;
            pxCol++;
            if (pxCol >= 32) {
                pxCol = 0;
                pxRow++;
            }
        }
    }
    void render() {
        cout << "+--------------------------------+" << endl;
        for (int i = 0; i < 16; i++) {
            cout << "|";
            for (int j = 0; j < 32; j++) {
                cout << screen[i][j];
            }
            cout << "|" << endl;
        }


        cout << "+--------------------------------+" << endl;
    }
};

class GraphicsAdapter {
private:
    PhosphorDisplay* display;
    float currentPower;

public:
    GraphicsAdapter() {
        display = nullptr;
        currentPower = 2;
    }

    void connectDisplay(PhosphorDisplay* d) {
        display = d;
    }

    void CHAR_PAYLOAD(unsigned char byte) {
        currentPower = 15;
        if (display) {
            display->writeChar((char)byte);
        }
        currentPower = 2;
    }

    void INT_PAYLOAD(unsigned char byte) {
        currentPower = 15;
        if (display) {
            if (byte >= 100) {
                display->writeChar('0' + (byte / 100));
                display->writeChar('0' + ((byte / 10) % 10));
                display->writeChar('0' + (byte % 10));
            }
            else if (byte >= 10) {
                display->writeChar('0' + (byte / 10));
                display->writeChar('0' + (byte % 10));
            }
            else {
                display->writeChar('0' + byte);
            }
        }
        currentPower = 2;
    }

    float getCurrentPower() const {
        return currentPower;
    }
};
//============section 9.3 assignment ====================
class PowerSupplyUnit {
private:
    // "carries a maximum wattage capacity set at manufacturing time"
    float maxWattage;

public:
    // "maximum wattage capacity set at manufacturing time" = constructor parameter
    PowerSupplyUnit(float maxWatts) : maxWattage(maxWatts) {}

    //exact lines of assignment 
    // "on each clock cycle, total power draw is polled and passed to the PSU"
    // "if total draw exceeds PSU's rated capacity"
    // returns true if the PSU trips (overloaded)
    bool checkPower(float totalDraw) {
        return totalDraw > maxWattage;
    }
};


class Keyboard {
private:
    char* buffer;
    int head;
    int tail;
    int count;
    int capacity;

    void waitForInput() {
        cout << "[Hardware Interrupt] Input: ";
        char temp[200];
        cin.getline(temp, 200);

        int i = 0;
        while (temp[i] != '\0') {
            if (count < capacity) {
                buffer[tail] = temp[i];
                tail = (tail + 1) % capacity;
                count++;
            }
            i++;
        }
        if (count < capacity) {
            buffer[tail] = ' ';
            tail = (tail + 1) % capacity;
            count++;
        }
    }

public:
    Keyboard() {
        capacity = 200;
        buffer = new char[capacity];
        head = 0;
        tail = 0;
        count = 0;
    }

    ~Keyboard() {
        delete[] buffer;
    }

    bool PENDING_INPUT() {
        for (int i = 0; i < count; i++) {
            char c = buffer[(head + i) % capacity];
            if (c != ' ' && c != '\n' && c != '\t') {
                return true;
            }
        }
        return false;
    }

    unsigned char READ_CHAR() {
        if (count == 0) waitForInput();

        while (count > 0) {
            char c = buffer[head];
            head = (head + 1) % capacity;
            count--;
            if (c != ' ' && c != '\n' && c != '\t') {
                return (unsigned char)c;
            }
        }
        return 0x00;
    }

    unsigned char READ_INT() {
        if (count == 0) waitForInput();

        char c;
        while (count > 0) {
            c = buffer[head];
            if (c != ' ' && c != '\n' && c != '\t') break;
            head = (head + 1) % capacity;
            count--;
        }
        if (count == 0) waitForInput();
        int val = 0;
        while (count > 0) {
            c = buffer[head];
            if (c >= '0' && c <= '9') {
                val = val * 10 + (c - '0');
                head = (head + 1) % capacity;
                count--;
            }
            else {
                if (c == ' ' || c == '\n' || c == '\t') {
                    head = (head + 1) % capacity;
                    count--;
                }

                break;
            }
        }
        return (unsigned char)val;
    }
};
struct CompilationResult {
    std::vector<uint16_t> codeSegment;
    std::vector<uint16_t> dataSegment;
};

class Interpreter {
public:
    Interpreter() {}

    CompilationResult compile(const std::string& sourceCode);
    void loadProgramAndFlash(const std::string& filename, MemoryModule& ram);

private:
    std::map<std::string, uint16_t> labelMap;
    std::vector<uint16_t> parseLine(const std::string& line);
    uint8_t parseRegister(const std::string& regStr);

    std::vector<std::string> tokenize(const std::string& line);
};

class Processor {
private:
    TheSystemBuses* systemBus;
    mainboard* board;
    Register coreRegs;
    bool verboseMode;
    // --- 8.2 The Block Cache ---
    unsigned char cacheBlock[16];
    unsigned short cacheBaseAddress;
    bool cacheValid;

    // --- 8.3 The Instruction Decode Matrix ---
    struct DecodeSignal {
        bool IS_VALID;
        bool USES_AEC;
        AEC_OPERATION AEC_OP;
        bool IS_MEMORY_READ;
        bool IS_MEMORY_WRITE;
        bool IS_BRANCH;
        bool BRANCH_ON_ZERO;
        bool BRANCH_ON_NOT_ZERO;
        bool WRITES_TO_REGISTER;
        bool IS_4BIT_IMMEDIATE;
        bool IS_16BIT_IMMEDIATE;
    };

    DecodeSignal decodeMatrix[256];

    void buildDecodeMatrix() {
        for (int i = 0; i < 256; i++) {
            decodeMatrix[i] = { false, false, AEC_ADD, false, false, false, false, false, false, false, false };
        }

        decodeMatrix[0x00] = { true, false, AEC_ADD, false, false, false, false, false, false, false, false }; // NOP
        decodeMatrix[0x01] = { true, true, AEC_ADD, false, false, false, false, false, true, false, false };   // ADD
        decodeMatrix[0x02] = { true, true, AEC_SUB, false, false, false, false, false, true, false, false };   // SUB
        decodeMatrix[0x03] = { true, true, AEC_MUL, false, false, false, false, false, true, false, false };   // MUL
        decodeMatrix[0x04] = { true, true, AEC_DIV, false, false, false, false, false, true, false, false };   // DIV
        decodeMatrix[0x0A] = { true, true, AEC_CMP, false, false, false, false, false, false, false, false };  // CMP
        decodeMatrix[0x10] = { true, false, AEC_ADD, false, false, true, false, false, false, false, false };  // JMP
        decodeMatrix[0x11] = { true, false, AEC_ADD, false, false, true, true, false, false, false, false };   // JZ
        decodeMatrix[0x12] = { true, false, AEC_ADD, false, false, true, false, true, false, false, false };   // JNZ
        decodeMatrix[0x1A] = { true, false, AEC_ADD, false, false, false, false, false, true, true, false };   // LDR_IMM (4-bit)
        decodeMatrix[0x1B] = { true, false, AEC_ADD, false, false, false, false, false, true, false, true };   // LDR_IMM (16-bit)
        decodeMatrix[0x20] = { true, false, AEC_ADD, true, false, false, false, false, true, false, false };   // LDR
        decodeMatrix[0x21] = { true, false, AEC_ADD, false, true, false, false, false, false, false, false };  // STR
    }

public:
    Processor() {
        systemBus = nullptr;
        board = nullptr;
        cacheValid = false;
        cacheBaseAddress = 0;
        for (int i = 0; i < 16; i++) cacheBlock[i] = 0;
        verboseMode = false;
        buildDecodeMatrix();
    }
    //missing things 
    void setVerbose(bool v) {
        verboseMode = v;
    }
    unsigned short getRegVal(int n) {
        return coreRegs.getRegister(n);
    }
    unsigned short getIRVal() {
        return coreRegs.getIR();
    }
    unsigned char  getFlagsVal() {
        return coreRegs.getFlags(); 
    }
    unsigned short getPC() {
        return coreRegs.getPC();
    }

    bool isHalted() {
        return (coreRegs.getFlags() & 0x80) != 0;
    }

    void connectPins(TheSystemBuses* bus, mainboard* mb) {
        systemBus = bus;
        board = mb;
    }

    unsigned char processReadMemory(unsigned short address);
    void processWriteMemory(unsigned short address, unsigned char data);
    void tick();
};



//vvvvvvimp line 
/*
On an invalid write, the data is
discarded.
*/


class mainboard {
private:
    //Chapter 4  Composition (etched on, dies with board) as mentioned 
    /*The ONYX-16 mainboard contains 3 system buses. These buses are the conduits through which every
   memory transaction takes place. As these are copper pathways etched o
   nto the motherboard itself, if the
   motherboard is destroyed these buses are destroyed with it. */
   //composition part
    TheSystemBuses bus; // contains address, data and control in one object 
    //aggregation part

    Processor* processor;
    MemoryModule* memoryModule;
    GraphicsAdapter* graphicsAdapter;
    PowerSupplyUnit* psu;
    //reason of making psu in aggregation 
    /*
    The PSU does not own the mainboard. It holds a reference to it. The mainboard exists before the PSU is
connected and continues to exist in its current state after the PSU trips. Only the power flag changes.
*/
    Keyboard* keyboard;
    // ========= Chapter 6 :Address Decoder================ 
    void decodeAndRoute() {
        unsigned short address = bus.getAddress();

        if (address >= 0x0000 && address <= 0x0EFF) {
            routeToMemory();
        }
        else if (address >= 0x0F00 && address <= 0x0FEF) {
            routeToFaultZone();
        }
        else if (address >= 0x0FF0 && address <= 0x0FF3) {
            routeToMMIO();
        }
    }

    void routeToMemory() {
        unsigned short address = bus.getAddress();

        if (bus.isReadEnabled()) {
            unsigned char data = memoryModule->read(address);
            bus.setData(data);
        }
        else if (bus.isWriteEnabled()) {
            unsigned char data = bus.getData();
            memoryModule->loadRawBinary(address, data);
        }
    }

    void routeToFaultZone() {
        if (bus.isReadEnabled()) {
            cout << "[MOTHERBOARD FAULT] Invalid Read Address" << endl;
            bus.setData(0x00);
        }
        else if (bus.isWriteEnabled()) {
            cout << "[MOTHERBOARD FAULT] Invalid Write Address" << endl;
        }
    }

    void routeToMMIO() {
        unsigned short port = bus.getAddress();

        if (port == 0x0FF0) readKeyboardChar();
        else if (port == 0x0FF1) writeDisplayChar();
        else if (port == 0x0FF2) writeDisplayInt();
        else if (port == 0x0FF3) readKeyboardInt();
    }

    void readKeyboardChar() {
        if (!bus.isReadEnabled()) return;

        if (keyboard) {
            unsigned char val = keyboard->READ_CHAR();
            bus.setData(val);
        }
        else {
            bus.setData(0x00);
        }
    }

    void writeDisplayChar() {
        if (!bus.isWriteEnabled()) return;

        if (graphicsAdapter) {
            unsigned char payload = bus.getData();
            graphicsAdapter->CHAR_PAYLOAD(payload);
        }
    }

    void writeDisplayInt() {
        if (!bus.isWriteEnabled()) return;

        if (graphicsAdapter) {
            unsigned char payload = bus.getData();
            graphicsAdapter->INT_PAYLOAD(payload);
        }
    }

    void readKeyboardInt() {
        if (!bus.isReadEnabled()) return;

        if (keyboard) {
            unsigned char val = keyboard->READ_INT();
            bus.setData(val);
        }
        else {
            bus.setData(0x00);
        }
    }

public:
    // ==== Chapter 9.1 ==== Slot Management ===========
    void plugInProcessor(Processor* p) {
        processor = p;
        processor->connectPins(&bus, this);
    }

    void plugInMemory(MemoryModule* m) {
        memoryModule = m;
    }

    void plugInGraphics(GraphicsAdapter* g) {
        graphicsAdapter = g;
    }

    void plugInPSU(PowerSupplyUnit* p) {
        psu = p;
    }

    void plugInKeyboard(Keyboard* k) {
        keyboard = k;
    }

    // ========= Chapter 9.2 ======The Only Public Signal =======
    void pulseClock() {
        if (!systemPower) return;

        decodeAndRoute();

        // 8.5 thermodynamics
        systemTemperature += 0.05;

        if (graphicsAdapter && graphicsAdapter->getCurrentPower() > 2) {
            systemTemperature += 0.15;
        }

        if (systemTemperature >= 90) {
            cout << "[SYSTEM FAULT] Critical Thermal Event! Core temp: " << systemTemperature << "C. Emergency Shutdown triggered." << endl;
            killPower();
            return;
        }

        float totalDraw = 0;
        if (graphicsAdapter) totalDraw += graphicsAdapter->getCurrentPower();

        if (psu && psu->checkPower(totalDraw)) killPower();
    }

    // internal bus cycle used by the CPU cache prefetch 
    // routes the address decoder without charging thermal or PSU cost
    void internalBusCycle() {
        if (!systemPower) return;
        decodeAndRoute();
    }

    //chapter 9.3
    //=========sets the system power flag to false" and "all subsequent pulseClock() calls return immediately =======
    bool systemPower;
    float systemTemperature; // 8.5 Thermodynamics
    void killPower() {
        systemPower = false;
    }

    mainboard() : processor(nullptr), memoryModule(nullptr), graphicsAdapter(nullptr), psu(nullptr), keyboard(nullptr), systemPower(true), systemTemperature(25) {}
};


//======================[since it has no other instance so i included its there its an opinion if you want to change no issue]=============


/*
* following pattern followed
1. Forward declarations:   class mainboard;  class Processor; (declaration because they requied defination)
2. AEC_OPERATION enum
3. TheSystemBuses
4. MemoryModule
5. Register          (now AEC_OPERATION is already declared)
6. PhosphorDisplay
7. GraphicsAdapter
8. PowerSupplyUnit
9. Keyboard
10. CompilationResult struct
11. Interpreter class declaration
12. Processor        (mainboard* is a pointer -> forward decl is enough)
13. mainboard        (Processor* is a pointer -> forward decl is enough)

*/