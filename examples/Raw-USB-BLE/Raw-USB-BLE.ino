// Originally based on ESP32_Host_MIDI by Saulo Veríssimo 
// https://github.com/sauloverissimo/ESP32_Host_MIDI 
// Modified by Liam Jones, 2025




#include <Arduino.h>
#include "USBConnection.h"
#include "BLEConnection.h"

#include <FastLED.h>

#define LED_PIN 48
#define NUM_LEDS 1
CRGB leds[NUM_LEDS];
extern BLEConnection bleMidi;

// --- USB: Subclass with override ---

class MyRawUSB : public USBConnection {
public:
    void onMidiDataReceived(const uint8_t* data, size_t length) override {
        // data[0] = Cable Number / Code Index Number (USB-MIDI header)
        // data[1] = MIDI status byte (e.g., 0x90 = NoteOn ch1)
        // data[2] = MIDI data byte 1 (e.g., note number)
        // data[3] = MIDI data byte 2 (e.g., velocity)
        Serial.printf("[USB] CIN:%02X  Status:%02X  Data1:%02X  Data2:%02X  BLE:%s\n",
                  data[0], data[1], data[2], data[3],
                  bleMidi.isConnected() ? "CONNECTED" : "NOT CONNECTED");

        // Forward over BLE if connected
        if (bleMidi.isConnected()) {
            // BLE MIDI packet format: [header, timestamp, status, data1, data2]
            uint8_t blePkt[5];
            blePkt[0] = 0x80;  // BLE MIDI header byte
            blePkt[1] = 0x80;  // BLE MIDI timestamp byte
            blePkt[2] = data[1]; // MIDI status
            blePkt[3] = data[2]; // MIDI data 1
            blePkt[4] = data[3]; // MIDI data 2
            bleMidi.sendMidi(blePkt, 5);
        }
    }

    void onDeviceConnected() override {
        Serial.println("[USB] MIDI device connected!");
        Serial.println("[USB] Last error: " + getLastError());
    }

    void onDeviceDisconnected() override {
        Serial.println("[USB] MIDI device disconnected!");
    }
};

MyRawUSB usbMidi;

// --- BLE: Callback function ---

BLEConnection bleMidi;

void onBleRawData(const uint8_t* data, size_t length) {
    Serial.printf("[BLE] Raw %d bytes:", length);
    for (size_t i = 0; i < length; i++) {
        Serial.printf(" %02X", data[i]);
    }
    Serial.println();
}

// --- Setup & Loop ---

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("=== Raw MIDI Example ===");
    Serial.println("No MIDIHandler — direct access to raw bytes.");
    Serial.println();

    // Initialize USB Host (runs on core 0)
    if (usbMidi.begin()) {
        Serial.println("USB Host initialized. Waiting for MIDI device...");
    } else {
        Serial.println("USB Host init failed: " + usbMidi.getLastError());
    }

    // Initialize BLE MIDI server
    bleMidi.setMidiMessageCallback(onBleRawData);
    bleMidi.begin("WU-BT10 MIDI");
    FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(15);
    Serial.println("BLE MIDI server started. Advertising as 'WU-BT10'.");
    Serial.println();
    
}

void loop() {
    usbMidi.task();
    bleMidi.task();
    delayMicroseconds(10);
    // Blue flashing when BLE not connected
    static uint32_t lastLedMs = 0;
    static bool ledState = false;

    if (!usbMidi.isConnected()) {
        // Red when no USB MIDI device
        leds[0] = CRGB::Red;
        FastLED.show();
    } else if (!bleMidi.isConnected()) {
        // Blue flashing when USB connected but BLE not connected
        if (millis() - lastLedMs > 500) {
            lastLedMs = millis();
            ledState = !ledState;
            leds[0] = ledState ? CRGB::Blue : CRGB::Black;
            FastLED.show();
        }
    } else {
        // Solid green when both connected
        FastLED.setBrightness(1);
        leds[0] = CRGB::Green;
        FastLED.show();
    }
}