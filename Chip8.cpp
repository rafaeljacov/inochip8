#include "Chip8.h"
#include "Arduino.h"

// typedef unsigned short u16;
// typedef unsigned char byte;

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

void Chip8::load(u8 *data, unsigned int size) {
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

    // 00E0
    if (digit1 == 0 && digit2 == 0 &&
        digit3 == 0xE && digit4 == 0)
    {
        for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
            screen[i] = false;
        }
    }
    // 00EE
    else if (digit1 == 0 && digit2 == 0 &&
        digit3 == 0xE && digit4 == 0xE) 
    {
        pc = pop();
    }
    // 1nnn
    else if (digit1 == 1) {
        u16 nnn = op & 0xFFF;
        pc = nnn;
    }
    // 2nnn
    else if (digit1 == 2) {
        u16 nnn = op & 0xFFF;
        push(pc);
        pc = nnn;
    }
    // 3xkk
    else if (digit1 == 3) {
        u8 vx = v_reg[digit2];
        u8 nn = op & 0xFF;

        if (vx == nn) 
            pc += 2;
    }
    // 4xkk
    else if (digit1 == 4) {
        u8 vx = v_reg[digit2];
        u8 nn = op & 0xFF;

        if (vx != nn) 
            pc += 2;
    }
    // 5xy0
    else if (digit1 == 5 && digit4 == 0) {
        u8 vx = v_reg[digit2];
        u8 vy = v_reg[digit3];

        if (vx == vy)
            pc += 2;
    }
    // 6xkk
    else if (digit1 == 6) {
        u8 kk = op & 0xFF;
        v_reg[digit2] = kk;
    }
    // 7xkk
    else if (digit1 == 7) {
        u8 *vx = &v_reg[digit2];
        u8 kk = op & 0xFF;

        *vx += kk;
    }
    // 8xy0
    else if (digit1 == 8 && digit4 == 0) {
        u8 *vx = &v_reg[digit2];
        u8 vy = v_reg[digit3];
        *vx = vy;
    }
    // 8xy1
    else if (digit1 == 8 && digit4 == 1) {
        u8 *vx = &v_reg[digit2];
        u8 vy = v_reg[digit3];
        *vx |= vy;
    }
    // 8xy2
    else if (digit1 == 8 && digit4 == 2) {
        u8 *vx = &v_reg[digit2];
        u8 vy = v_reg[digit3];
        *vx &= vy;
    }
    // 8xy3
    else if (digit1 == 8 && digit4 == 3) {
        u8 *vx = &v_reg[digit2];
        u8 vy = v_reg[digit3];
        *vx ^= vy;
    }
    // 8xy4
    else if (digit1 == 8 && digit4 == 4) {
        u8 *vx = &v_reg[digit2];
        u8 vy = v_reg[digit3];
        
        u8 sum = *vx + vy;
        bool carry = (sum < *vx) || (sum < vy);

        *vx = sum;
        v_reg[0xF] = carry ? 1 : 0;
    }
    // 8xy5
    else if (digit1 == 8 && digit4 == 5) {
        u8 *vx = &v_reg[digit2];
        u8 vy = v_reg[digit3];

        *vx -= vy;
        v_reg[0xF] = (*vx > vy) ? 1 : 0;
    }
    // 8xy6
    else if (digit1 == 8 && digit4 == 6) {
        u8 lsb = v_reg[digit2] & 1;
        v_reg[0xF] = lsb;
        v_reg[digit2] >>= 1;
    }
    // 8xy7
    else if (digit1 == 8 && digit4 == 7) {
        u8 *vx = &v_reg[digit2];
        u8 vy = v_reg[digit3];

        *vx = vy - *vx;
        v_reg[0xF] = (vy > *vx) ? 1 : 0;
    }
    // 8xyE
    else if (digit1 == 8 && digit4 == 0xE) {
        u8 msb = (v_reg[digit2] >> 7) & 1;
        v_reg[0xF] = msb;
        v_reg[digit2] <<= 1;
    }
    // 9xy0
    else if (digit1 == 9 && digit4 == 0) {
        if (v_reg[digit2] == v_reg[digit3])
            pc += 2;
    }
    // Annn
    else if (digit1 == 0xA) {
        u16 nnn = op & 0xFFF;
        i_reg = nnn;
    }
    // Bnnn
    else if (digit1 == 0xB) {
        u16 nnn = op & 0xFFF;
        pc = v_reg[0] + nnn;
    }
    // Cxkk
    else if (digit1 == 0xC) {
        u8 kk = op & 0xFF;
        u8 rng = random(256);
        v_reg[digit2] = rng & kk;
    }
    // Dxyn
    else if (digit1 == 0xD) {
        bool has_erased = false;

        u16 x_coord = v_reg[digit2];
        u16 y_coord = v_reg[digit3];

        for (int y_line = 0; y_line < digit4; y_line++) {
            u16 addr = i_reg + y_line;
            u8 pixels = ram[addr];

            unsigned int y = (y_coord + y_line) % SCREEN_HEIGHT;
            for (int x_col = 0; x_col < 8; x_col++) {
                if ((pixels & (0x80 >> x_col)) != 0) {
                    unsigned int x = (x_coord + x_col) % SCREEN_WIDTH;
                    unsigned int idx = y * SCREEN_WIDTH + x;

                    has_erased |= screen[idx];
                    screen[idx] ^= true;
                }
            }
        }

        v_reg[0xF] = has_erased ? 1 : 0;
    }
    // Ex9E
    else if (digit1 == 0xE && digit3 == 9 && digit4 == 0xE) {
        u8 vx = v_reg[digit2];
        if (keys[vx]) {
            pc += 2;
        }
    }
    // ExA1
    else if (digit1 == 0xE && digit3 == 0xA && digit4 == 1) {
        u8 vx = v_reg[digit2];
        if (!keys[vx]) {
            pc += 2;
        }
    }
    // Fx07
    else if (digit1 == 0xF && digit3 == 0 && digit4 == 7) {
        v_reg[digit2] = dt;
    }
    // Fx0A
    else if (digit1 == 0xF && digit3 == 0 && digit4 == 0xA) {
        bool pressed = false;
        for (int k = 0; k < NUM_KEYS; k++) {
            if (keys[k]) {
                v_reg[digit2] = k;
                pressed = true;
                break;
            }
        }

        if (!pressed) {
            pc -= 2;
        }
    }
    // Fx15
    else if (digit1 == 0xF && digit3 == 1 && digit4 == 5) {
        dt = v_reg[digit2];
    }
    // Fx18
    else if (digit1 == 0xF && digit3 == 1 && digit4 == 8) {
        st = v_reg[digit2];
    }
    // Fx1E
    else if (digit1 == 0xF && digit3 == 1 && digit4 == 0xE) {
        i_reg += v_reg[digit2];
    }
    // Fx29
    else if (digit1 == 0xF && digit3 == 2 && digit4 == 9) {
        i_reg = v_reg[digit2] * 5;
    }
    // Fx33
    else if (digit1 == 0xF && digit3 == 3 && digit4 == 3) {
        u8 vx = v_reg[digit2];
        u8 hundreds = (vx / 100);
        u8 tens = ((vx / 10) % 10);
        u8 ones = (vx % 10);

        u8 bcd[] = {hundreds, tens, ones};
        for (int i = 0; i < 3; i++) {
            ram[i_reg + i] = bcd[i];
        }
    }
    // Fx55
    else if (digit1 == 0xF && digit3 == 5 && digit4 == 5) {
        for (int i = 0; i <= digit2; i++) {
            ram[i_reg + i] = v_reg[i];
        }
    }
    // Fx65
    else if (digit1 == 0xF && digit3 == 6 && digit4 == 5) {
        for (int i = 0; i <= digit2; i++) {
            v_reg[i] = ram[i_reg + i];
        }
    }
}