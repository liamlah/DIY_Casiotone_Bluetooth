// Originally based on ESP32_Host_MIDI by Saulo Veríssimo 
// https://github.com/sauloverissimo/ESP32_Host_MIDI 
// Modified by Liam Jones, 2025

#include "BLEConnection.h"

BLEConnection::BLEConnection()
    : pServer(nullptr), pCharacteristic(nullptr), pBleCallback(nullptr), midiCallback(nullptr),rxHead(0),rxTail(0),rxMux(portMUX_INITIALIZER_UNLOCKED)
{
}

BLEConnection::~BLEConnection() {
    delete pBleCallback;
    pBleCallback = nullptr;
}

void BLEConnection::begin(const std::string& deviceName) {
    BLEDevice::init(String(deviceName.c_str()));
    pServer = BLEDevice::createServer();
    class ServerCallbacks : public BLEServerCallbacks {
    void onDisconnect(BLEServer* pServer) override {
        BLEDevice::startAdvertising();
        }
    };
    pServer->setCallbacks(new ServerCallbacks());

    BLEService* pService = pServer->createService(BLE_MIDI_SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
        BLE_MIDI_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE_NR |
        BLECharacteristic::PROPERTY_NOTIFY    
    ); // Added notify

    // This descriptor is required for notify to work
    BLEDescriptor *pCCCD = new BLEDescriptor(BLEUUID((uint16_t)0x2902));
    pCharacteristic->addDescriptor(pCCCD);


    // Create a write callback that extracts the first 4 bytes and forwards them.
    class BLECallback : public BLECharacteristicCallbacks {
    public:
        BLEConnection* bleCon;
        BLECallback(BLEConnection* con) : bleCon(con) {}
        void onWrite(BLECharacteristic* characteristic) override {
            // getData()/getLength() preserve zero bytes — never use c_str() on MIDI data
            const uint8_t* data = characteristic->getData();
            size_t len = characteristic->getLength();

            // Enqueue only — never parse or touch USB from the BT task
            if (len >= 3 && len <= sizeof(BleRxPacket::data)) {
                portENTER_CRITICAL(&bleCon->rxMux);
                int next = (bleCon->rxHead + 1) % RX_QUEUE_SIZE;
                if (next != bleCon->rxTail) {
                    memcpy(bleCon->rxQueue[bleCon->rxHead].data, data, len);
                    bleCon->rxQueue[bleCon->rxHead].length = len;
                    bleCon->rxHead = next;
                }
                portEXIT_CRITICAL(&bleCon->rxMux);
            }
        }
    };

    // Store the pointer for cleanup in the destructor
    delete pBleCallback;
    pBleCallback = new BLECallback(this);
    pCharacteristic->setCallbacks(pBleCallback);
    pService->start();

    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(BLE_MIDI_SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    BLEDevice::startAdvertising();
}

    // New send MIDI function
bool BLEConnection::sendMidi(const uint8_t* data, size_t length) {
    if (!pCharacteristic || !isConnected()) return false;
    pCharacteristic->setValue(const_cast<uint8_t*>(data), length);
    pCharacteristic->notify();
    return true;
}

void BLEConnection::task() {
    while (true) {
        BleRxPacket pkt;
        portENTER_CRITICAL(&rxMux);
        if (rxTail == rxHead) {
            portEXIT_CRITICAL(&rxMux);
            break;
        }
        pkt = rxQueue[rxTail];
        rxTail = (rxTail + 1) % RX_QUEUE_SIZE;
        portEXIT_CRITICAL(&rxMux);

        if (midiCallback) midiCallback(pkt.data, pkt.length);
        onMidiDataReceived(pkt.data, pkt.length);
    }
}

bool BLEConnection::isConnected() const {
    if(pServer)
        return (pServer->getConnectedCount() > 0);
    return false;
}

void BLEConnection::setMidiMessageCallback(MIDIMessageCallback cb) {
    midiCallback = cb;
}

void BLEConnection::onMidiDataReceived(const uint8_t* data, size_t length) {
    // Default implementation: no-op.
    (void)data;
    (void)length;
}
