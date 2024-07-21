#include <HardwareSerial.h>

// UART parameters
#define RXD2 25
#define TXD2 27

// Command to read temperature data
const uint8_t requestTemperatureCommand[] = {0x3A, 0x10, 0x03, 0x00, 0x04, 0x01, 0x00, 0x00, 0x82, 0x62}; 

// Function to calculate CRC16
unsigned short modbus_CRC16(unsigned char *ptr, unsigned char len) {
    unsigned short wcrc = 0xFFFF;
    for (int i = 0; i < len; i++) {
        wcrc ^= *ptr++;
        for (int j = 0; j < 8; j++) {
            if (wcrc & 0x0001) {
                wcrc = wcrc >> 1 ^ 0xA001;
            } else {
                wcrc >>= 1;
            }
        }
    }
    return (wcrc << 8) | (wcrc >> 8);
}

void setup() {
    // Initialize Serial Monitor
    Serial.begin(115200);
    
    // Initialize Serial Communication with sensor
    Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

    // Wait for the serial port to be available
    while (!Serial2) {
        delay(10);
    }

    Serial.println("Serial Communication started");
}

void loop() {
    // Send request command to read temperature data
    Serial2.write(requestTemperatureCommand, sizeof(requestTemperatureCommand));
    
    // Wait for a response
    delay(100);

    // Read response
    uint8_t response[32]; // Increased buffer size to handle unexpected lengths
    int len = Serial2.readBytes(response, 32);

    // Ini Raw Bang
    Serial.print("Response length: ");
    Serial.println(len);
    Serial.print("Raw response: ");
    for (int i = 0; i < len; i++) {
        Serial.print(response[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    // Checking 12 Baris Ngab
    if (len >= 9) { // Adjust to minimum expected length
        // Check CRC
        unsigned short crc = modbus_CRC16(response, len - 2);
        if ((response[len - 2] == (crc >> 8)) && (response[len - 1] == (crc & 0xFF))) {
            // Extract (Multifitamin) the temperature value (2 bytes, big-endian)
            uint16_t tempRaw = (response[6] << 8) | response[7];
            
            // Convert to temperature in degrees Celsius
            float temperature = tempRaw / 100.0;

            // Print the temperature value ABANGGGGG
            Serial.print("Temperature (°C): ");
            Serial.println(temperature);
        } else {
            Serial.println("CRC check failed");
        }
    } else {
        Serial.println("Incorrect response length");
    }

    // Wait before sending the next request
    delay(1000);
}

