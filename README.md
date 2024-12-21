# Chip8 Library Documentation

## Overview
`inochip8` is a library implementation of the CHIP-8 virtual machine written in C++ for Arduino. CHIP-8 is an interpreted programming language primarily used to emulate simple games from the 1970s and 1980s. This library provides the necessary components to run CHIP-8 programs, including memory, CPU emulation, input handling, and display management.

---

## Class: `Chip8`

### Public Members

#### Constants
- **`SCREEN_WIDTH`** *(constexpr unsigned short)*: The width of the CHIP-8 display (64 pixels).
- **`SCREEN_HEIGHT`** *(constexpr unsigned short)*: The height of the CHIP-8 display (32 pixels).

#### Member Variables
- **`beeping`** *(bool)*: Indicates whether the sound timer has triggered a beep.

#### Constructor
- **`Chip8()`**: Initializes the CHIP-8 virtual machine by resetting its state and loading the fontset into memory.

#### Methods

1. **`bool* get_display()`**
   - Returns a pointer to the `screen` buffer representing the current display state.

2. **`void keypress(byte idx, bool pressed)`**
   - Updates the state of a specific key.
   - **Parameters**:
     - `idx`: The index of the key 0-15 or 0x0 - 0xF (hexadecimal).
     - `pressed`: `true` if the key is pressed; `false` otherwise.

3. **`void load_byte(byte data, size_t index)`**
   - Loads a single byte of data into memory at the specified index relative to the program start address.
   - **Parameters**:
     - `data`: The byte to load.
     - `index`: The offset from the program start address.

4. **`void reset()`**
   - Resets the CHIP-8 virtual machine to its initial state.

5. **`void tick()`**
   - Executes one instruction cycle by fetching, decoding, and executing the next instruction.

6. **`void tick_timers()`**
   - Updates the delay and sound timers. Decrements each timer if it is greater than zero. Sets `beeping` to `true` if the sound timer is active.

---

### Private Members

#### Constants
- **`RAM_SIZE`** *(constexpr short)*: The total size of CHIP-8 memory (4 KB).
- **`NUM_REGS`** *(constexpr short)*: The number of 8-bit general-purpose registers (16).
- **`STACK_SIZE`** *(constexpr short)*: The size of the stack (16 levels).
- **`NUM_KEYS`** *(constexpr short)*: The number of keys in the CHIP-8 keypad (16).
- **`START_ADDR`** *(constexpr unsigned short)*: The memory address where programs start (0x200).
- **`FONTSET_SIZE`** *(const short)*: The size of the built-in fontset (80 bytes).

#### Variables
- **`FONTSET`** *(const byte array)*: The built-in fontset for rendering hexadecimal digits (0-F).
- **`pc`** *(unsigned short)*: The program counter.
- **`ram`** *(byte array)*: The CHIP-8 memory.
- **`screen`** *(bool array)*: The display buffer (2D array flattened to 1D).
- **`v_reg`** *(byte array)*: The 16 general-purpose registers.
- **`i_reg`** *(unsigned short)*: The index register.
- **`sp`** *(unsigned short)*: The stack pointer.
- **`stack`** *(unsigned short array)*: The call stack.
- **`keys`** *(bool array)*: The state of the keypad.
- **`dt`** *(byte)*: The delay timer.
- **`st`** *(byte)*: The sound timer.

#### Methods

1. **`void push(unsigned short item)`**
   - Pushes an item onto the stack.

2. **`unsigned short pop()`**
   - Pops an item from the stack.

3. **`unsigned short fetch()`**
   - Fetches the next instruction from memory and increments the program counter.

4. **`void execute(unsigned short op)`**
   - Decodes and executes the given instruction.

---

## CHIP-8 Instruction Set
The `Chip8` class implements a subset of the CHIP-8 instruction set. Notable instructions include:

- **Display Control**
  - `00E0`: Clears the screen.
  - `DXYN`: Draws a sprite at coordinates `(VX, VY)` with a height of `N` pixels.

- **Flow Control**
  - `00EE`: Returns from a subroutine.
  - `1NNN`: Jumps to address `NNN`.
  - `2NNN`: Calls a subroutine at `NNN`.

- **Conditional Execution**
  - `3XKK`: Skips the next instruction if `VX == KK`.
  - `4XKK`: Skips the next instruction if `VX != KK`.
  - `5XY0`: Skips the next instruction if `VX == VY`.
  - `9XY0`: Skips the next instruction if `VX != VY`.

- **Math Operations**
  - `8XY0`: Sets `VX = VY`.
  - `8XY4`: Adds `VY` to `VX` with carry.
  - `8XY5`: Subtracts `VY` from `VX` with borrow.
  - `8XY6`: Right shifts `VX` by 1.
  - `8XYE`: Left shifts `VX` by 1.

- **Timers**
  - `FX07`: Sets `VX` to the value of the delay timer.
  - `FX15`: Sets the delay timer to `VX`.
  - `FX18`: Sets the sound timer to `VX`.

- **Memory**
  - `ANNN`: Sets `I` to address `NNN`.
  - `FX1E`: Adds `VX` to `I`.
  - `FX29`: Sets `I` to the sprite location for the digit stored in `VX`.
  - `FX33`: Stores the BCD representation of `VX` in memory.
  - `FX55`: Stores registers `V0` through `VX` in memory starting at address `I`.
  - `FX65`: Fills registers `V0` through `VX` with values from memory starting at address `I`.

- **Randomization**
  - `CXKK`: Sets `VX` to a random number ANDed with `KK`.

---

## Usage Example
***Refer to examples folder for a complete real world implementation*** 
```cpp
#include "Chip8.h"

#define TICKS_PER_FRAME 30

Chip8 chip8;

void setup() {
    // Load program data into memory
    chip8.load_byte(0xA2, 0);  // Example opcode
    chip8.load_byte(0xF0, 1);
    // NOTE: you will most likely use this by reading from a chip8 rom (.ch8)
    //       and load each byte using a loop
}

void loop() {
    // Implement key events handling

    // Run program instructions
    for (byte i = 0; i < TICKS_PER_FRAME; i++) {
        chip8.tick();
    }
    chip8.tick_timers();

    if (chip8.beeping) {
        // Trigger sound
    }

    bool* display = chip8.get_display();
    // Render display buffer to screen

}
```

---

## Notes
- Ensure a valid CHIP-8 program is loaded starting at address `0x200`.
- This library assumes a consistent 60 Hz update rate for the timers.
- The display buffer is a linearized array of size `SCREEN_WIDTH * SCREEN_HEIGHT`.

---

## Limitations
- Designed for Arduino platforms with sufficient memory and performance (This library alone can take about 8kb of SRAM, tried it on Arduino Mega and regretted it).
- Does not include advanced debugging tools or error handling for invalid instructions.

