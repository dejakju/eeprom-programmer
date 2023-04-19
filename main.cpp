/*
 * Name:    Goran Todorovic (dejakju@gmail.com)
 * Date:    2022-05-07
 * Update:  2022-09-04
 * Desc.:   An Arduino Nano Programmer for the AT28C16 EEPROM
 */

#include <Arduino.h>

const int SHIFT_DATA = 2;
const int SHIFT_LATCH = 3;
const int SHIFT_CLK = 4;

const int D0 = 5;
const int D1 = 6;
const int D2 = 7;
const int D3 = 8;
const int D4 = 9;
const int D5 = 10;
const int D6 = 11;
const int D7 = 12;

const int CE = 14;
const int OE = 15;
const int WE = 16;

/*
 * Helper functions to toggle the Chip enable (CE), Output enable (OE) and Write enable (WE) pins
 * @param state - either LOW (0x0) or HIGH (0x01)
 * @remarks improves the readability alot!
 */
void set_CE(byte state) { digitalWrite(CE, state); }
void set_OE(byte state) { digitalWrite(OE, state); }
void set_WE(byte state) { digitalWrite(WE, state); }

/*
 * Set all data pins as input (for read operations)
 * @remark could have done this in a loop, but this is quicker
 */
void set_BusInput()
{
    pinMode(D0, INPUT);
    pinMode(D1, INPUT);
    pinMode(D2, INPUT);
    pinMode(D3, INPUT);
    pinMode(D4, INPUT);
    pinMode(D5, INPUT);
    pinMode(D6, INPUT);
    pinMode(D7, INPUT);
}

/*
 * Set all data pins as output (for write operations)
 * @remark could have done this in a loop, but this is quicker
 */
void set_BusOutput()
{
    pinMode(D0, OUTPUT);
    pinMode(D1, OUTPUT);
    pinMode(D2, OUTPUT);
    pinMode(D3, OUTPUT);
    pinMode(D4, OUTPUT);
    pinMode(D5, OUTPUT);
    pinMode(D6, OUTPUT);
    pinMode(D7, OUTPUT);
}

/*
 * Latch (set) an address through both shift-registers (SN74HC595) to the address pins of the EEPROM
 * @param address - a 16 bit unsigned integer
 */
void set_Address(int address)
{
    digitalWrite(SHIFT_LATCH, LOW);
    shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (address >> 8));
    shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, address);
    digitalWrite(SHIFT_LATCH, HIGH);
}

/*
 * Read a single byte at the given address
 * @param address - a 16 bit unsigned integer
 * @returns a byte value
 */
byte read_EEPROM(int address)
{
    // Disable everything (CE, OE & WE are active-LOW !)
    set_CE(HIGH);
    set_OE(HIGH);
    set_WE(HIGH);

    // Set the data pins as input, since we are reading
    set_BusInput();

    // Set the address of the EEPROM
    set_Address(address);

    // Enable the chip (CE) and output mode (OE)
    set_CE(LOW);
    set_OE(LOW);

    // Combine all data pins (D0 through D7) to a single byte
    return ((digitalRead(D7) << 7) +
            (digitalRead(D6) << 6) +
            (digitalRead(D5) << 5) +
            (digitalRead(D4) << 4) +
            (digitalRead(D3) << 3) +
            (digitalRead(D2) << 2) +
            (digitalRead(D1) << 1) +
            digitalRead(D0));

    // Disable the CE and OE pins again
    set_OE(HIGH);
    set_CE(HIGH);
}

/*
 * Write a single byte at the given address
 * @param address - a 16 bit unsigned integer
 * @param data - the byte value to write
 * @remark timing is crucial, especially the internal write cycle of the EEPROM itself
 */
void write_EEPROM(int address, byte data)
{
    // Disable everything (CE, OE & WE are active-LOW !)
    set_CE(HIGH);
    set_OE(HIGH);
    set_WE(HIGH);

    // Set the data pins as output, since we are writing
    set_BusOutput();

    // Set the address of the EEPROM
    set_Address(address);

    // Split the data byte into single bits (HIGH = 1 or LOW = 0) for the data pins (D0 through D7)
    digitalWrite(D7, (data >> 7) & 1);
    digitalWrite(D6, (data >> 6) & 1);
    digitalWrite(D5, (data >> 5) & 1);
    digitalWrite(D4, (data >> 4) & 1);
    digitalWrite(D3, (data >> 3) & 1);
    digitalWrite(D2, (data >> 2) & 1);
    digitalWrite(D1, (data >> 1) & 1);
    digitalWrite(D0, (data) & 1);

    // Initiate the write impulse and wait for the internal write-cycle to finish
    set_CE(LOW);
    set_WE(LOW);
    delayMicroseconds(10);

    // Disable the WE and CE pins again and wait for another 10 ms before
    set_WE(HIGH);
    set_CE(HIGH);

    delay(10);
}

/*
 * Print the whole content of the EEPROM to the serial monitor
 * @remark after each block of 256 bytes the header is printed
 */
void read_Contents()
{
    for (int base = 0; base < 0x800; base += 16)
    {
        byte data[16];
        for (int offset = 0; offset < 16; offset++)
        {
            data[offset] = read_EEPROM(base + offset);
        }
        
        char outBuffer[92];
        
        sprintf(outBuffer, "%08X   %02X %02X %02X %02X %02X %02X %02X %02X  %02X %02X %02X %02X %02X %02X %02X %02X   %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
        base, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7],
                data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15],
                ((data[0] >= 0x20 && data[0] <= 0x7F) ? (char)data[0] : '.'),
                ((data[1] >= 0x20 && data[1] <= 0x7F) ? (char)data[1] : '.'),
                ((data[2] >= 0x20 && data[2] <= 0x7F) ? (char)data[2] : '.'),
                ((data[3] >= 0x20 && data[3] <= 0x7F) ? (char)data[3] : '.'),
                ((data[4] >= 0x20 && data[4] <= 0x7F) ? (char)data[4] : '.'),
                ((data[5] >= 0x20 && data[5] <= 0x7F) ? (char)data[5] : '.'),
                ((data[6] >= 0x20 && data[6] <= 0x7F) ? (char)data[6] : '.'),
                ((data[7] >= 0x20 && data[7] <= 0x7F) ? (char)data[7] : '.'),
                ((data[8] >= 0x20 && data[8] <= 0x7F) ? (char)data[8] : '.'),
                ((data[9] >= 0x20 && data[9] <= 0x7F) ? (char)data[9] : '.'),
                ((data[10] >= 0x20 && data[10] <= 0x7F) ? (char)data[10] : '.'),
                ((data[11] >= 0x20 && data[11] <= 0x7F) ? (char)data[11] : '.'),
                ((data[12] >= 0x20 && data[12] <= 0x7F) ? (char)data[12] : '.'),
                ((data[13] >= 0x20 && data[13] <= 0x7F) ? (char)data[13] : '.'),
                ((data[14] >= 0x20 && data[14] <= 0x7F) ? (char)data[14] : '.'),
                ((data[15] >= 0x20 && data[15] <= 0x7F) ? (char)data[15] : '.'));

        if (base % 256 == 0)
        {
          Serial.println(" OFFSET     0  1  2  3  4  5  6  7   8  9  A  B  C  D  E  F         ASCII     ");
        }

        Serial.println(outBuffer);
    }
}

/*
 * Erase the entire EEPROM
 */
void erase_EEPROM()
{
    Serial.print("Erasing EEPROM");
    unsigned long starttime = millis();
    byte data = 0xFF;
    for (int address = 0x00; address < 0x0800; address++)
    {
        write_EEPROM(address, data);

        if (address % 32 == 0)
        {
          Serial.print(".");
        }
    }
    unsigned long endtime = millis();
    Serial.print(" done. (time elapsed = ");
    Serial.print((endtime - starttime), DEC);
    Serial.println("ms)");
}

/*
 * A dummy write test function
 */
void write_Test()
{
    byte data = 0x00;
    Serial.print("Start WRITING");
    unsigned long starttime = millis();
    // write just to the first 256 bytes...
    for (unsigned int address = 0x00; address < 0x0100; address++)
    {
        write_EEPROM(address, data++);

        if (address % 32 == 0)
        {
          Serial.print(".");
        }
    }
    unsigned long endtime = millis();
    Serial.print(" done. (time elapsed = ");
    Serial.print((endtime - starttime), DEC);
    Serial.println("ms)");
}

void setup()
{
    pinMode(SHIFT_DATA, OUTPUT);
    pinMode(SHIFT_LATCH, OUTPUT);
    pinMode(SHIFT_CLK, OUTPUT);

    digitalWrite(CE, HIGH);
    pinMode(CE, OUTPUT);
    digitalWrite(OE, HIGH);
    pinMode(OE, OUTPUT);
    digitalWrite(WE, HIGH);
    pinMode(WE, OUTPUT);

    Serial.begin(57600);
    delay(1000);


    // Erase the entire EEPROM
    erase_EEPROM();

    // Write some test data
    write_Test();

    write_EEPROM(0x7D2, 0x11);
    write_EEPROM(0x7DD, 0x11);
    write_EEPROM(0x7E1, 0x11);
    write_EEPROM(0x7EE, 0x11);

    // Print the contents of the EEPROM to the serial monitor
    read_Contents();

}

void loop()
{
}
