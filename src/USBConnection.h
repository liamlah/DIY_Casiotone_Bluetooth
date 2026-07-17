// Originally based on ESP32_Host_MIDI by Saulo Veríssimo 
// https://github.com/sauloverissimo/ESP32_Host_MIDI 
// Modified by Liam Jones, 2025

#ifndef USB_CONNECTION_H
#define USB_CONNECTION_H

#include <Arduino.h>
#include <usb/usb_host.h>
#include <freertos/portmacro.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Structure to store a raw USB packet.
// Although transfers can be up to 64 bytes, only the first 4 are relevant per USB-MIDI event.
struct RawUsbMessage {
    uint8_t data[64];
    size_t length;
};

class USBConnection {
public:
    USBConnection();

    // Initializes the USB Host, registers the client, and starts the USB task on core 0.
    bool begin();

    // Drains the ring buffer and forwards MIDI data to onMidiDataReceived(). Call from loop().
    void task();

    // Returns whether the USB connection is ready.
    bool isConnected() const { return isReady; }
    // Send MIDI data out to the keyboard over USB
    bool sendMidi(const uint8_t* data, size_t length);
    // Queue a MIDI message for outbound USB transmission
    bool enqueueMidiOut(const uint8_t* data, size_t length);
    void processOutQueue();
    // Returns the last error message (empty if none).
    const String& getLastError() const { return lastError; }

    // Virtual callback to forward raw MIDI data (4 bytes: CIN + 3 MIDI bytes).
    // Upper layer should override this method to process the data.
    virtual void onMidiDataReceived(const uint8_t* data, size_t length);

    // Connection callbacks (empty by default).
    virtual void onDeviceConnected();
    virtual void onDeviceDisconnected();

    // Queue access methods (for debugging or external analysis)
    int getQueueSize() const;
    const RawUsbMessage& getQueueMessage(int index) const;

protected:
    bool isReady;
    uint8_t interval;         // Polling interval (ms)
    unsigned long lastCheck;

    usb_host_client_handle_t clientHandle;
    usb_device_handle_t deviceHandle;
    uint32_t eventFlags;
    usb_transfer_t* midiTransfer;
    usb_transfer_t* midiOutTransfer;


    // Ring buffer for raw USB packets.
    // Protected by spinlock for thread-safe access on dual-core ESP32.
    static const int QUEUE_SIZE = 128;
    RawUsbMessage usbQueue[QUEUE_SIZE];
    volatile int queueHead;
    volatile int queueTail;
    portMUX_TYPE queueMux;
    // Outbound queue
    RawUsbMessage usbOutQueue[QUEUE_SIZE];
    volatile int outQueueHead;
    volatile int outQueueTail;
    portMUX_TYPE outQueueMux;
    volatile bool outTransferInFlight;
    volatile bool transferInFlight;

    volatile bool enumRetryPending;
    uint32_t enumRetryTime;

    // Connection control data
    bool firstMidiReceived;
    bool isMidiDeviceConfirmed;
    String deviceName;
    String lastError;

    // Helper functions to manage the queue.
    bool enqueueMidiMessage(const uint8_t* data, size_t length);
    bool dequeueMidiMessage(RawUsbMessage &msg);
    void processQueue();

    // Dedicated FreeRTOS task for USB event handling (core 0)
    TaskHandle_t usbTaskHandle;
    static void _usbTask(void* arg);

    // Internal USB Host callbacks.
    static void _clientEventCallback(const usb_host_client_event_msg_t *eventMsg, void *arg);
    static void _onReceive(usb_transfer_t *transfer);
    static void _onSend(usb_transfer_t *transfer);
    void _processConfig(const usb_config_desc_t *config_desc);
};

#endif // USB_CONNECTION_H
