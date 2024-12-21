# inochip8
Chip8 Library for Arduino providing the core functions for building a chip8 emulator
## SAMPLE FRONTEND CODE (using ESP32):
```cpp



// ROMs can be found at https://github.com/kripod/chip8-roms
#include <Chip8.h>
#include <Keypad.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <SD.h>

#define TICKS_PER_FRAME 30
Chip8 chip8;


#define SCALE 2
const byte WINDOW_WIDTH = chip8.SCREEN_WIDTH * SCALE;
const byte WINDOW_HEIGHT  = chip8.SCREEN_HEIGHT * SCALE;
const uint16_t SCREEN_LENGTH = chip8.SCREEN_WIDTH * chip8.SCREEN_HEIGHT;

#define OLED_RESET -1
#define BEEPER 13

#define ROWS 4
#define COLS 4 

const char hexaKeys[ROWS][COLS] = {
  {'C', 'D', 'E', 'F'},
  {'3', '6', '9', 'B'},
  {'2', '5', '8', '0'},
  {'1', '4', '7', 'A'}
};

                   // R1, R2, R3, R4
byte rowPins[ROWS] = {33, 32, 15, 4}; 
                  // C1, C2, C3, C4
byte colPins[COLS] = {25, 26, 27, 14}; 
Keypad keypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire, OLED_RESET);

void setup() {
    delay(1500);
    Serial.begin(9600);
    Serial.println(F("Setup start..."));
    pinMode(BEEPER, OUTPUT);
    keypad.addEventListener(keypadEvent);
    delay(250); // wait for display to power up
    display.begin(0x3c, true);
    display.clearDisplay();
    if (!SD.begin(SS)) {
        show_text(10, 26, 1, "SD Card Init Error!");
        show_text(0, 34, 1, "NOTE: Restart device manually.");
        display.display();
        Serial.println("initialization failed. Things to check:");
        Serial.println("1. is a card inserted?");
        Serial.println("2. is your wiring correct?");
        Serial.println("3. did you change the chipSelect pin to match your shield or module?");
        Serial.println("Note: press reset button on the board and reopen this Serial Monitor after fixing your issue!");
        tone(BEEPER, 300); while(true);
    }

    game_select();
    Serial.println(F("Setup done."));
}

void loop() {
    char key = keypad.getKey();

    for (int i = 0; i < TICKS_PER_FRAME; i++) {
        chip8.tick();
    }
    chip8.tick_timers();

    if (chip8.beeping) {
        tone(BEEPER, 7000, 50);
    }
    draw_screen();
}

void game_select() {
    File dir = SD.open("/chip8/games");
    byte filecount = count_files(dir);
    byte game_index = 0;
    while (true) {
        File file = dir.openNextFile();
        game_index++;

        while (!is_rom(file))
            file = dir.openNextFile();

        display.clearDisplay();
        show_text(0, 0, 1, "Select Game:");
        show_text(98, 0, 1, String(game_index) + "/" + String(filecount));
        show_text(20, 48, 1, "Top(2) Select(5)");
        show_text(22, 56, 1, "Next(6) Skip(8)");

        // remove brackets
        String game_name = file.name();
        int8_t start = game_name.indexOf('[');
        int8_t end = game_name.lastIndexOf(']') - start + 2;
        if (start != -1 && end != -1)
            game_name.remove(start - 1, end);

        show_text(0, 16, 1, game_name);
        display.display();

        // wait keypress
        while (true) {
            char key = keypad.getKey();
            if (key == '2') {
                dir.rewindDirectory();
                game_index = 0;
                break;
            } else if (key == '5') {
                load_file(file);
                return;
            } else if (key == '6') {
                if (game_index == filecount) {
                    dir.rewindDirectory();
                    game_index = 0;
                }
                break;
            } else if (key == '8') {
                for (byte i = 0; i < 9; i++) {
                    if (game_index != filecount) {
                        file = dir.openNextFile();
                        game_index++;
                    } else {
                        dir.rewindDirectory();
                        game_index = 0;
                    }
                }
                break;
            }
        }

    }
}

void load_file(File rom) {
    // load rom to chip8 memory
    uint16_t bytes_read = 0;
    while (rom.available()) {
        byte data = rom.read();
        chip8.load_byte(data, bytes_read++);
    }
    rom.close();

    if (bytes_read == 0) {
        Serial.println(F("ROM not loaded! Restart device manually."));
        tone(BEEPER, 300); while(true);
    }

    Serial.println(F("ROM loaded."));
    Serial.print(bytes_read);
    Serial.println(F(" bytes read."));
}

void draw_screen() {
    bool *screen_buf = chip8.get_display();
    display.clearDisplay();
    for (unsigned int i = 0; i < SCREEN_LENGTH; i++) {
        if (screen_buf[i]) {
            unsigned int x = i % chip8.SCREEN_WIDTH;
            unsigned int y = i / chip8.SCREEN_WIDTH;
            // draw white filled square
            display.fillRect(x * SCALE, y * SCALE, SCALE, SCALE, SH110X_WHITE);
        }
    }
    display.display();
}

byte count_files(File dir) {
    if (!dir.isDirectory())
        return 0;

    byte count = 0;
    while (true) {
        File entry = dir.openNextFile();
        if (!entry) {
            dir.rewindDirectory();
            return count;
        }

        if (!entry.isDirectory() && is_rom(entry)) {
            count++;
        }
    }
}

// verify if the file ext is ".ch8"
bool is_rom(File file) {
    String name = file.name();
    int8_t dot_index = name.lastIndexOf('.');
    if (dot_index != -1) {
        return (name[dot_index + 1] == 'c') && (name[dot_index + 2] == 'h') && (name[dot_index + 3] == '8');
    } else
        return false;
}

void show_text(byte x, byte y, byte text_size, String text) {
    display.setTextColor(SH110X_WHITE);
    display.setCursor(x, y);
    display.setTextSize(text_size);
    display.print(text);
}

void keypadEvent(KeypadEvent key){
    KeyState key_state = keypad.getState();

    if (key) {
        byte key_code = get_key_code(key);

        if (key_state == PRESSED) {
            chip8.keypress(key_code, true);
        } else if (key_state == RELEASED) {
            chip8.keypress(key_code, false);
        }
    }
}

byte get_key_code(char key) {
    switch (key) {
        case '1':
            return 0x1;
        case '2':
            return 0x2;
        case '3':
            return 0x3;
        case '4':
            return 0x4;
        case '5':
            return 0x5;
        case '6':
            return 0x6;
        case '7':
            return 0x7;
        case '8':
            return 0x8;
        case '9':
            return 0x9;
        case '0':
            return 0x0;
        case 'A':
            return 0xA;
        case 'B':
            return 0xB;
        case 'C':
            return 0xC;
        case 'D':
            return 0xD;
        case 'E':
            return 0xE;
        case 'F':
            return 0xF;
    }
}
```
