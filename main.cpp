#include <stdio.h>
#include <stdlib.h>
#include <SFML/Graphics.hpp>

using namespace std;

const int SCALE = 10;

const unsigned char fontset [80] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0,		// 0
	0x20, 0x60, 0x20, 0x20, 0x70,		// 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0,		// 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0,		// 3
	0x90, 0x90, 0xF0, 0x10, 0x10,		// 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0,		// 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0,		// 6
	0xF0, 0x10, 0x20, 0x40, 0x40,		// 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0,		// 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0,		// 9
	0xF0, 0x90, 0xF0, 0x90, 0x90,		// A
	0xE0, 0x90, 0xE0, 0x90, 0xE0,		// B
	0xF0, 0x80, 0x80, 0x80, 0xF0,		// C
	0xE0, 0x90, 0x90, 0x90, 0xE0,		// D
	0xF0, 0x80, 0xF0, 0x80, 0xF0,		// E
	0xF0, 0x80, 0xF0, 0x80, 0x80		// F
};

unsigned short pc;
unsigned short index;
unsigned char registers [0xF];

unsigned char memory [4096];
unsigned short stack [16];
unsigned short sp;

bool display [64*32];
bool keypad [16];

unsigned char delay_timer;
unsigned char sound_timer;

bool drawFlag;

sf::RenderWindow window(sf::VideoMode(64*SCALE, 32*SCALE), "CHIP-8");

unsigned char memoryRead ( unsigned short address) {
    return memory[address];
}

void memoryWrite ( unsigned short address, unsigned char data ) {
    memory[address] = data;
}

void getInput () {
    keypad[0] = sf::Keyboard::isKeyPressed(sf::Keyboard::Num0);
    keypad[1] = sf::Keyboard::isKeyPressed(sf::Keyboard::Num1);
    keypad[2] = sf::Keyboard::isKeyPressed(sf::Keyboard::Num2);
    keypad[3] = sf::Keyboard::isKeyPressed(sf::Keyboard::Num3);
    keypad[4] = sf::Keyboard::isKeyPressed(sf::Keyboard::Num4);
    keypad[5] = sf::Keyboard::isKeyPressed(sf::Keyboard::Num5);
    keypad[6] = sf::Keyboard::isKeyPressed(sf::Keyboard::Num6);
    keypad[7] = sf::Keyboard::isKeyPressed(sf::Keyboard::Num7);
    keypad[8] = sf::Keyboard::isKeyPressed(sf::Keyboard::Num8);
    keypad[9] = sf::Keyboard::isKeyPressed(sf::Keyboard::Num9);
    keypad[10] = sf::Keyboard::isKeyPressed(sf::Keyboard::A);
    keypad[11] = sf::Keyboard::isKeyPressed(sf::Keyboard::B);
    keypad[12] = sf::Keyboard::isKeyPressed(sf::Keyboard::C);
    keypad[13] = sf::Keyboard::isKeyPressed(sf::Keyboard::D);
    keypad[14] = sf::Keyboard::isKeyPressed(sf::Keyboard::E);
    keypad[15] = sf::Keyboard::isKeyPressed(sf::Keyboard::F);
}

int waitInput () {
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                exit(EXIT_SUCCESS);
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num0)) return 0x0;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) return 0x1;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) return 0x2;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) return 0x3;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num4)) return 0x4;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num5)) return 0x5;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num6)) return 0x6;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num7)) return 0x7;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num8)) return 0x8;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num9)) return 0x9;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) return 0xA;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::B)) return 0xB;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::C)) return 0xC;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) return 0xD;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) return 0xE;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::F)) return 0xF;
    }

    exit(EXIT_SUCCESS);
}

void beep () {
    printf("BEEP!\n");
}

void clearDisplay () {
    for(int i = 0; i < 64*32; ++i)
        display[i] = 0;
    drawFlag = true;
}

void drawGraphics () {
    window.clear();

    sf::RectangleShape rectangle(sf::Vector2f(SCALE, SCALE));
    rectangle.setFillColor(sf::Color::White);
    
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            //printf("%d ", display[x + (y*64)]);
            if (display[x + (y*64)]) {
                rectangle.setPosition(sf::Vector2f(x*SCALE, y*SCALE));
                window.draw(rectangle);
            }
        }
        //printf("\n");
    }

    window.display();
    drawFlag = false;
}

void drawSprite (unsigned char x, unsigned char y, unsigned short pointer, unsigned char height) {
    printf("Draw sprite at %02x, %02x from %03x\n", x, y, pointer);
    registers[0xF] = false;
    for (int j = 0; j < height; j++) {
        unsigned char row = memory[pointer + j];
        for (int i = 0; i < 8; i++) {
            if (display[(x+i) + ((y+j)*64)] ^ ((row >> (7-i)) & 1)) registers[0xF] = 1;
            display[(x+i) + ((y+j)*64)] ^= (row >> (7-i)) & 1;
        }
    }

    drawFlag = true;
}

void initialise () {
    pc = 0x200;     // Reset PC
    index = 0;      // Reset index
    sp = 0;         // Reset stack pointer

    // Clear stack
    for(int i = 0; i < 16; ++i)
        stack[i] = 0;

    // Clear registers
    for(int i = 0; i < 16; ++i)
        registers[i] = 0;

    // Clear memory
    for(int i = 0; i < 4096; ++i)
        memory[i] = 0;

    // Clear display
    clearDisplay();
}

void loadFontset () {
    for(int i = 0; i < 80; ++i)
        memory[i] = fontset[i];
}

void loadProgram ( const char *filename ) {
    FILE *fp;
    fp = fopen(filename, "r");

    if(fp == NULL) {
      perror("Error opening file");
      exit(EXIT_FAILURE);
    } else {
        int i = 0;
        while (!feof(fp)) {
            printf("%03x\n", i);
            memory[i + 0x200] = fgetc(fp);
            i++;
        }
    }
    
    fclose(fp);
}

void emulate () {
    unsigned short opcode = (memoryRead(pc) << 8) | memoryRead(pc + 1);
    printf("%04x: %04x\tREG: %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, \tINDEX: %03x\n", pc, opcode, registers[0], registers[1], registers[2], registers[3], registers[4], registers[5], registers[6], registers[7], registers[8], registers[9], registers[10], registers[11], registers[12], registers[13], registers[14], registers[15], index);

    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode & 0x00FF) {
                // Clear display
                case 0x00E0:
                    clearDisplay();
                    pc += 2;
                    break;

                // Return
                case 0x00EE:
                    sp -= 1;
                    pc = stack[sp];
                    pc += 2;
                    break;

                default:
                    printf ("Unknown opcode: 0x%04x\n", opcode);
                    exit(EXIT_FAILURE);
                    pc += 2;
                    break;
            }
            break;

        // Jump
        // PC = NNN
        case 0x1000:
            pc = opcode & 0x0FFF;
            break;

        // Call
        // Push PC
        // PC = NNN
        case 0x2000:
            stack[sp] = pc;
            sp += 1;
            pc = opcode & 0x0FFF;
            break;

        // Skip if VX == NN
        case 0x3000:
            if (registers[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;

        // Skip if VX != NN
        case 0x4000:
            if (registers[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;

        // SKip if VX == VY
        case 0x5000:
            if (registers[(opcode & 0x0F00) >> 8] == registers[(opcode & 0x00F0) >> 4]) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;

        // VX = NN
        case 0x6000:
            registers[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
            pc += 2;
            break;

        // VX += NN
        case 0x7000:
            registers[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
            pc += 2;
            break;

        case 0x8000:
            switch (opcode & 0x000F) {
                // VX = VY
                case 0x0:
                    registers[(opcode & 0x0F00) >> 8] = registers[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                // VX = VX | VY
                case 0x1:
                    registers[(opcode & 0x0F00) >> 8] |= registers[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                // VX = VX & VY
                case 0x2:
                    registers[(opcode & 0x0F00) >> 8] &= registers[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                // VX = VX ^ VY
                case 0x3:
                    registers[(opcode & 0x0F00) >> 8] ^= registers[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                // VX += VY
                case 0x4:
                    registers[0xF] = (registers[(opcode & 0x0F00) >> 8] + registers[(opcode & 0x00F0) >> 4]) > 0xFF;
                    registers[(opcode & 0x0F00) >> 8] += registers[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                // VX -= VY
                case 0x5:
                    registers[0xF] = !(registers[(opcode & 0x00F0) >> 4] > registers[(opcode & 0x0F00) >> 8]);
                    registers[(opcode & 0x0F00) >> 8] -= registers[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                // VX = VY >> 1
                case 0x6:
                    registers[0xF] = registers[(opcode & 0x0F00) >> 8] & 0x01;
                    registers[(opcode & 0x0F00) >> 8] = registers[(opcode & 0x0F00) >> 8] >> 1;
                    pc += 2;
                    break;

                // VX = VY - VX
                case 0x7:
                    registers[0xF] = !(registers[(opcode & 0x0F00) >> 8] > registers[(opcode & 0x00F0) >> 4]);
                    registers[(opcode & 0x0F00) >> 8] = registers[(opcode & 0x0F00) >> 8] - registers[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                // VX = VY << 1
                case 0xE:
                    registers[0xF] = (registers[(opcode & 0x00F0) >> 4] >> 7) & 0x01;
                    registers[(opcode & 0x0F00) >> 8] = registers[(opcode & 0x0F00) >> 8] << 1;
                    pc += 2;
                    break;
                
                default:
                    printf ("Unknown opcode: 0x%04x\n", opcode);
                    exit(EXIT_FAILURE);
                    pc += 2;
                    break;
            }
            break;

        // SKip if VX != VY
        case 0x9000:
            if (registers[(opcode & 0x0F00) >> 8] != registers[(opcode & 0x00F0) >> 4]) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;
        
        // I = NNN
        case 0xA000:
            index = opcode & 0x0FFF;
            pc += 2;
            break;

        // PC = V0 + NNN
        case 0xB000:
            pc = registers[0] + (opcode & 0x0FFF);
            break;

        // VX = RAND & NN
        case 0xC000:
            registers[(opcode & 0x0F00) >> 8] = rand() & (opcode & 0x00FF);
            pc += 2;
            break;

        // Draw sprite
        case 0xD000:
            drawSprite(registers[(opcode & 0x0F00) >> 8], registers[(opcode & 0x00F0) >> 4], index, opcode & 0x000F);
            pc += 2;
            break;

        case 0xE000:
            switch (opcode & 0x00FF) {
                // Skip if key stored in VX pressed
                case 0x009E:
                    printf("Check for key %02x", registers[(opcode & 0x0F00) >> 8]);
                    if (keypad[registers[(opcode & 0x0F00) >> 8]]) {
                        pc += 4;
                    } else {
                        pc += 2;
                    }
                    break;

                // Skip if key stored in VX not pressed
                case 0x00A1:
                    printf("Check for key %02x", registers[(opcode & 0x0F00) >> 8]);
                    if (!keypad[registers[(opcode & 0x0F00) >> 8]]) {
                        pc += 4;
                    } else {
                        pc += 2;
                    }
                    break;

                default:
                    printf ("Unknown opcode: 0x%04x\n", opcode);
                    //exit(EXIT_FAILURE);
                    pc += 2;
                    break;
            }
            break;

        case 0xF000:
            switch (opcode & 0x00FF) {
                // Get delay timer
                // VX = delay_timer
                case 0x0007:
                    registers[(opcode & 0x0F00) >> 8] = delay_timer;
                    pc += 2;
                    break;

                // Wait for keypress
                // VX = key
                case 0x000A:
                    registers[(opcode & 0x0F00) >> 8] = waitInput();
                    pc += 2;
                    break;

                // Set delay timer
                // delay_timer = VX
                case 0x0015:
                    delay_timer = registers[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;

                // Set sound timer
                // sound_timer = VX
                case 0x0018:
                    sound_timer = registers[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;

                // I += VX
                case 0x001E:
                    registers[0xF] = (index + registers[(opcode & 0x0F00) >> 8]) > 0x0FFF;
                    index += registers[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;

                // I = character[VX]
                case 0x0029:
                    index = registers[(opcode & 0x0F00) >> 8] * 5;
                    pc += 2;
                    break;

                // BCD
                case 0x0033:
                    memory[index]     = registers[(opcode & 0x0F00) >> 8] / 100;
                    memory[index + 1] = (registers[(opcode & 0x0F00) >> 8] / 10) % 10;
                    memory[index + 2] = (registers[(opcode & 0x0F00) >> 8] % 100) % 10;
                    pc += 2;
                    break;

                // Register dump
                // Store registers V0 through VX to memory[I]
                case 0x0055:
                    for(int i = 0; i <= ((opcode & 0x0F00) >> 8); i++)
                        memory[index + i] = registers[i];
                    index += ((opcode & 0x0F00) >> 8) + 1;
                    pc += 2;
                    break;

                // Register load
                // Load registers V0 through VX from memory[I]
                case 0x0065:
                    for(int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
                        registers[i] = memory[index + i];
                    index += ((opcode & 0x0F00) >> 8) + 1;
                    pc += 2;
                    break;

                default:
                    printf ("Unknown opcode: 0x%04x\n", opcode);
                    exit(EXIT_FAILURE);
                    pc += 2;
                    break;
            }
            break;

        default:
            printf ("Unknown opcode: 0x%04x\n", opcode);
            exit(EXIT_FAILURE);
            pc += 2;
            break;
    }

    // Update timers
    if(delay_timer > 0) {
        delay_timer--;
    }
    if(sound_timer > 0)
    {
        if(sound_timer == 1) beep();
        sound_timer--;
    }  
}

int main () {
    initialise();
    loadFontset();
    loadProgram("ROMs/pong.ch8");

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                exit(EXIT_SUCCESS);
            }
        }
        
        getInput();
        emulate();

        if (drawFlag) drawGraphics();
    }

    return EXIT_SUCCESS;
}