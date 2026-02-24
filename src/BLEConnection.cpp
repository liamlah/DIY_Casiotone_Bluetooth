#include "BLEConnection.h"

BLEConnection::BLEConnection()
    : pServer(nullptr), pCharacteristic(nullptr), pBleCallback(nullptr), midiCallback(nullptr)
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
            std::string rxValue = std::string(characteristic->getValue().c_str());
            // If at least 4 bytes are available, extract the first 4.
            if(rxValue.size() >= 4) {
                const uint8_t* data = reinterpret_cast<const uint8_t*>(rxValue.data());
                // Invoke the user-defined callback (if set).
                if(bleCon->midiCallback) {
                    bleCon->midiCallback(data, 4);
                }
                // Also invoke the virtual callback so subclasses can override.
                bleCon->onMidiDataReceived(data, 4);
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
    // BLE generally does not require periodic processing.
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
