# Intel 8086 Emulator

This is a homework solution for HW of the Performance-Aware Programming course, Part 1, by Casey Muratory. [Link to course](https://www.computerenhance.com/p/table-of-contents).

---
## Overview
This project is a partial emulator of the Intel 8086 CPU. All the code is written by me, and its aim is to learn how a CPU works.
It is not intended to showcase best practices in coding but could be a great help for those who attempt to follow the course. 
Also, this repo is meant to showcase what I study in my free time and serves as an opportunity to engage with others who are interested in such topics.
---
## Supported Instructions

The emulator supports decoding and executing the following assembly commands:

| Opcode | Instruction       |
|--------|-------------------|
| `MOV`  | Move              |
| `ADD`  | Add               |
| `SUB`  | Subtract          |
| `CMP`  | Compare           |
| `JMP`  | Jump              |
| `JNZ`  | Jump if not zero  |
| `JNE`  | Jump if Not Equal |

---
## Assembly Code Examples
All examples of assembly code are located in the `asm` folder.

---
## Requirements
You need a NASM compiler to generate binary instructions that can be fed to this emulator.
No external dependencies.
CMake is needed

---
## Program Output
The program produces two main outputs:

1. **output.asm**: This file is generated to verify that the decoding process has proceeded as expected.
2. **memory.data**: This file is created when the `-dump` argument is provided. It contains the saved memory of the CPU, with a size of 1024 * 1024 bytes.
---

Feel free to reach out if you have any questions or need further assistance!