#include "Chip8.h"
#include "Arduino.h"

typedef unsigned short u16;
typedef unsigned char byte;

Chip8::Chip8() {
    pc = START_ADDR; // program counter 
    i_reg = 0;
    sp = 0; // stack pointer
    dt = 0; // delay timer
    st = 0; // sound timer

    // Copy fontset to ram
    for (int i = 0; i < FONTSET_SIZE; i++) {
        ram[i] = FONTSET[i];
    }
}

void Chip8::push(u16 item) {
    stack[sp] = item;
    sp++;
}

u16 Chip8::pop() {
    sp--;
    return stack[sp];
}

bool* Chip8::get_display() {
    return screen;
}

void Chip8::keypress(unsigned int idx, bool pressed) {
    keys[idx] = pressed;
}

void Chip8::load(byte *data, unsigned int size) {
    unsigned int start = START_ADDR;
    unsigned int end = START_ADDR + size;
    
    for (unsigned int i = start; i < end; i++) {
        ram[i] = data[i];
    }
} 

void Chip8::reset() {
    pc = START_ADDR;
    i_reg = 0;
    sp = 0;
    dt = 0;
    st = 0;

    // ram
    for (int i = 0; i < RAM_SIZE; i++) {
        // Avoids overidding the existing fontset data
        if (i >= FONTSET_SIZE) {
            ram[i] = 0;
        }
    }
    //screen
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        screen[i] = false;
    }
    // v registers
    for (int i = 0; i < NUM_REGS; i++) {
        v_reg[i] = 0;
    }
    // stack
    for (int i = 0; i < STACK_SIZE; i++) {
        stack[i] = 0;
    }
    // keys
    for (int i = 0; i < NUM_KEYS; i++) {
        keys[i] = false;
    }
}

void Chip8::tick() {
    u16 op = fetch();
    execute(op);
}

u16 Chip8::fetch() {
    u16 high = ram[pc];
    u16 low = ram[pc + 1];

    pc += 2;
    return (high << 8) | low;
}

void Chip8::tick_timers() {
    // delay timer
    if (dt > 0) {
        dt--;
    }

    // sound timer
    if (st > 0) {
        beeping = true;

        st--;
    } else {
        beeping = false;
    }
}

void Chip8::execute(u16 op) {
    u16 digit1 = (op & 0xF000) >> 12;
    u16 digit2 = (op & 0x0F00) >> 8;
    u16 digit3 = (op & 0x00F0) >> 4;
    u16 digit4 = op & 0x000F;

    // 0nnn
    // 00E0
    // 00EE
    // 1nnn
    // 2nnn
    // 3xkk
    // 4xkk
    // 5xy0
    // 6xkk
    // 7xkk
    // 8xy0
    // 8xy1
    // 8xy2
    // 8xy3
    // 8xy4
    // 8xy5
    // 8xy6
    // 8xy7
    // 8xyE
    // 9xy0
    // Annn
    // Bnnn
    // Cxkk
    // Dxyn
    // Ex9E
    // ExA1
    // Fx07
    // Fx0A
    // Fx15
    // Fx18
    // Fx1E
    // Fx29
    // Fx33
    // Fx55
    // Fx65
}
