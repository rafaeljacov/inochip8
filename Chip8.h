#pragma once
#include "Arduino.h"

typedef unsigned short u16;
typedef unsigned char u8;

class Chip8 {
public:
    Chip8();
    static constexpr u16 SCREEN_WIDTH = 64;
    static constexpr u16 SCREEN_HEIGHT = 32;
    bool beeping;
    void push(u16 item);
    u16 pop();
    bool *get_display();
    void keypress(unsigned int idx, bool pressed);
    void load(u8 *data, unsigned int size);
    void reset();
    void tick();
    u16 fetch();
    void tick_timers();
    void execute(u16 op);
private:
    static constexpr short RAM_SIZE = 4096;
    static constexpr short NUM_REGS = 16;
    static constexpr short STACK_SIZE = 16;
    static constexpr short NUM_KEYS = 16;
    static constexpr u16 START_ADDR = 0x200;

    static constexpr short FONTSET_SIZE = 80;
    static constexpr u8 FONTSET[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80, // F
    };

    u16 pc; // program counter
    u8 ram[RAM_SIZE] = {};
    bool screen[SCREEN_WIDTH * SCREEN_HEIGHT] = {};
    u8 v_reg[NUM_REGS] = {};
    u16 i_reg;
    u16 sp; // stack pointer
    u16 stack[STACK_SIZE] = {};
    bool keys[NUM_KEYS] = {};
    u8 dt; // delay timer
    u8 st; // sound timer
};                   
