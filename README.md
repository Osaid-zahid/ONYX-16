# 🖥️ ONYX-16 — Lazarus Machinae

> *"The physical hardware is extinct. There is no machine left on Earth that can execute these programs. We have the software — but the body it lived in is dead."*

A full software emulation of the **ONYX-16**, a fictional 16-bit computer architecture, built from scratch in C++ with no standard containers. Every hardware component — the processor, memory, mainboard, graphics adapter, keyboard, and display — is faithfully reconstructed from hardware docs, communicating exclusively through a simulated system bus.

---

## 📐 Architecture Overview

The ONYX-16 is a **16-bit word machine with byte-level addressing**. Instructions are 16 bits wide and require two consecutive bus transactions to fetch. No component ever speaks directly to another — all traffic goes through the **Mainboard's `pulseClock()`**.

| Class | Hardware Role |
|---|---|
| `mainboard` | Central authority — owns the 3 system buses, routes every transaction |
| `Processor` | Fetch-Decode-Execute loop, 16-byte block cache, thermodynamic model |
| `MemoryModule` | 3,840 bytes (0x0000–0x0EFF), byte-addressed flat RAM |
| `GraphicsAdapter` | Receives MMIO write payloads, forwards to display |
| `PhosphorDisplay` | 32×16 CRT buffer, rendered once at program end |
| `Keyboard` | FIFO circular input buffer, blocks on empty read |
| `PowerSupplyUnit` | Polls total draw each cycle; kills system if over capacity |
| `TheSystemBuses` | Address (16-bit) + Data (8-bit) + Control (READ/WRITE) pathways |
| `Register` | Sealed register bank: R0–R7, PC, IR, FLAGS (8-bit) |
| `Interpreter` | Two-pass assembler + program loader / flash utility |

---

## 🧠 Processor Internals

- **8 × 16-bit general-purpose registers** (R0–R7)
- **Program Counter (PC)**, **Instruction Register (IR)**, **FLAGS** with ZF/NF/PF/HF bits
- **Block Cache**: 16-byte direct-mapped prefetch buffer reduces bus traffic
- **ALU**: Stateless — ADD, SUB, MUL, DIV, AND, OR, XOR, NOT, INC, DEC, CMP
- **Instruction Decode Matrix**: 256-entry lookup table pre-burned at construction, eliminates branching in the execute path
- **Thermodynamics**: Temperature rises 0.05°C/tick; thermal shutdown at 90°C permanently sets the Halt Flag

---

## 🗺️ Memory Map

| Range | Region |
|---|---|
| `0x0000–0x07FF` | Code Segment |
| `0x0800–0x0EFF` | Data Segment (`.MAWAAD` variables) |
| `0x0F00–0x0FEF` | Fault Zone (Segmentation Fault) |
| `0x0FF0` | Keyboard — Read Character (MMIO) |
| `0x0FF1` | Display — Write Character (MMIO) |
| `0x0FF2` | Display — Write Integer (MMIO) |
| `0x0FF3` | Keyboard — Read Integer (MMIO) |

---

## 📜 Instruction Set

Supports both the **Roman Urdu dialect** and the **English transliteration** — both compile to identical machine code.

| Opcode | Urdu | English | Operation |
|---|---|---|---|
| `0x00` | `AARAM` | `NOP` | No operation / end-of-program |
| `0x01` | `JAMA` | `ADD` | R[dest] += R[src] |
| `0x02` | `TAFREEK` | `SUB` | R[dest] -= R[src] |
| `0x03` | `ZARAB` | `MUL` | R[dest] *= R[src] |
| `0x04` | `TAQSEEM` | `DIV` | R[dest] /= R[src] (0 if div-by-zero) |
| `0x0A` | `MUWAZANA` | `CMP` | Compare, update ZF/NF/PF |
| `0x10` | `CHHALANG` | `JMP` | PC = R[src] (unconditional) |
| `0x11` | `AGAR_SIFAR` | `JZ` | Jump if ZF=1 |
| `0x12` | `AGAR_MAUJOOD` | `JNZ` | Jump if ZF=0 |
| `0x1A` | `BHARO` | `LDR_IMM` | Load 4-bit immediate (Format B) |
| `0x1B` | `BHARO` | `LDR_IMM` | Load 16-bit immediate (Format D) |
| `0x20` | `PARHO` | `LDR` | R[dest] = Memory[R[src]] |
| `0x21` | `RAKHO` | `STR` | Memory[R[src]] = R[dest] |

**Three encoding formats:**
- **Format A** — Standard two-register (1 word)
- **Format B** — 4-bit immediate packed in low nibble (1 word, opcode `0x1A`)
- **Format D** — 16-bit immediate as a second word (2 words, opcode `0x1B`); used automatically by the assembler for values > 15 and all labels

---

## 🖥️ Sample Programs

Three programs are included, selectable from the boot menu:

### `hello.txt` — Urdu Hello World
Stores a string in the `.MAWAAD` data segment at `0x0800` and prints it character-by-character through the MMIO display port using a null-terminated loop.

### `calculator.txt` — 4-Operation Calculator
Reads two integers and an operator character from the keyboard via MMIO, evaluates the expression deterministically using `CMP` + conditional jumps, and prints the result. Fully Turing-complete arithmetic dispatch.

### `auth.txt` — Hardware Authentication Firewall
Stores a secret PIN (`786`) in ROM. Reads keyboard input character-by-character and compares against the stored value. Prints `ACCESS GRANTED` or `ACCESS DENIED` to the phosphor display.

---

## 🚀 Build & Run

```bash
g++ -std=c++17 -o onyx16 Q3_Main.cpp
./onyx16
```

At startup, the ONYX boot menu lets you select a program and optionally enable **cycle-by-cycle verbose debug logging** (shows every register writeback, memory access, and immediate decode).

---

## 📁 File Structure

```
Q3_Main.cpp          # Bootloader, Interpreter impl, Processor tick loop, main()
Q3_Submission.h      # All hardware class definitions (buses, RAM, CPU, GPU, display, keyboard, PSU)
hello.txt            # Sample program: Urdu string printer
calculator.txt       # Sample program: 4-op calculator
auth.txt             # Sample program: PIN authentication firewall
```

---

## ⚙️ Design Notes

- **No STL containers in hardware classes** — `MemoryModule`, `PhosphorDisplay`, `Keyboard`, and `Register` all use raw `new`/`delete` arrays. (`Interpreter` uses `vector` and `map` for the assembler only, as a build tool.)
- **All inter-component communication goes through `pulseClock()`** — the Processor never touches the MemoryModule directly.
- **Thermodynamic model** — the mainboard tracks `systemTemperature`; overclocked or graphics-heavy cycles heat faster.
- **Assembler is two-pass** — Pass 1 calculates exact byte addresses for all labels (including Format B vs D expansion); Pass 2 resolves all label references to 16-bit addresses.

---

## 📚 Context

Built for **CS-1004 Object-Oriented Programming**, Spring 2026 — FAST NUCES Islamabad.  
Assignment 3, Question 3: *Lazarus Machinae*.
