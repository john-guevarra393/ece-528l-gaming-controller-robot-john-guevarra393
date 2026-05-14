#include <Bluepad32.h>

// ============================================================
// ESP32 Gamepad to MSP432 UART Transmitter
//
// Gamepad -> ESP32 Bluetooth -> UART -> MSP432
//
// UART wiring:
// ESP32 GPIO17 / TX2  -> MSP432 P9.6 / UCA3RXD
// ESP32 GPIO16 / RX2  -> MSP432 P9.7 / UCA3TXD
// ESP32 GND           -> MSP432 GND
// ============================================================

// -----------------------------
// ESP32 pins
// -----------------------------

const int builtInLed = 2;

// ESP32 UART2 pins
const int ESP32_RX2_PIN = 16;
const int ESP32_TX2_PIN = 17;

// -----------------------------
// Bluepad32 controllers
// -----------------------------

ControllerPtr myControllers[BP32_MAX_GAMEPADS];

// Store previous command so ESP32 does not spam UART
char lastCommand = 'S';

// -----------------------------
// Send command to MSP432
// -----------------------------

void sendCommandToMSP432(char command) {
    if (command != lastCommand) {
        Serial2.write(command);

        Serial.print("Sent to MSP432: ");
        Serial.println(command);

        lastCommand = command;
    }
}

// -----------------------------
// Bluepad32 connected callback
// -----------------------------

void onConnectedController(ControllerPtr ctl) {
    bool foundEmptySlot = false;

    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == nullptr) {
            Serial.printf("CALLBACK: Controller connected, index=%d\n", i);

            ControllerProperties properties = ctl->getProperties();

            Serial.printf(
                "Controller model: %s, VID=0x%04x, PID=0x%04x\n",
                ctl->getModelName().c_str(),
                properties.vendor_id,
                properties.product_id
            );

            myControllers[i] = ctl;
            foundEmptySlot = true;
            break;
        }
    }

    if (!foundEmptySlot) {
        Serial.println("CALLBACK: Controller connected, but no empty slot was found");
    }
}

// -----------------------------
// Bluepad32 disconnected callback
// -----------------------------

void onDisconnectedController(ControllerPtr ctl) {
    bool foundController = false;

    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == ctl) {
            Serial.printf("CALLBACK: Controller disconnected from index=%d\n", i);
            myControllers[i] = nullptr;
            foundController = true;
            break;
        }
    }

    sendCommandToMSP432('S');
    digitalWrite(builtInLed, LOW);

    if (!foundController) {
        Serial.println("CALLBACK: Controller disconnected, but was not found");
    }
}

// -----------------------------
// Debug print
// -----------------------------

void dumpGamepad(ControllerPtr ctl) {
    Serial.printf(
        "idx=%d, dpad: 0x%02x, buttons: 0x%04x, axis L: %4d, %4d, axis R: %4d, %4d, brake: %4d, throttle: %4d, misc: 0x%02x\n",
        ctl->index(),
        ctl->dpad(),
        ctl->buttons(),
        ctl->axisX(),
        ctl->axisY(),
        ctl->axisRX(),
        ctl->axisRY(),
        ctl->brake(),
        ctl->throttle(),
        ctl->miscButtons()
    );
}

// -----------------------------
// Process gamepad input
// -----------------------------

void processGamepad(ControllerPtr ctl) {
    int dpad = ctl->dpad();
    int buttons = ctl->buttons();

    /*
      Commands sent to MSP432:

      F = forward
      B = backward
      L = rotate left
      R = rotate right
      S = stop

      8BitDo SN30 pro D-pad values:

      Up    = 0x01
      Down  = 0x02
      Right = 0x04
      Left  = 0x08

      8BitDo SN30 pro button values:

      A     = 0x0002
      B     = 0x0001
      X     = 0x0008
      Y     = 0x0004
    */

    if (buttons == 0x0002) {
        sendCommandToMSP432('F');
        digitalWrite(builtInLed, HIGH);
        Serial.println("Forward");
    }
    else if (buttons == 0x0001) {
        sendCommandToMSP432('B');
        digitalWrite(builtInLed, HIGH);
        Serial.println("Backward");
    }
    else if (dpad == 0x08) {
        sendCommandToMSP432('L');
        digitalWrite(builtInLed, HIGH);
        Serial.println("Left");
    }
    else if (dpad == 0x04) {
        sendCommandToMSP432('R');
        digitalWrite(builtInLed, HIGH);
        Serial.println("Right");
    }
    else {
        sendCommandToMSP432('S');
        digitalWrite(builtInLed, LOW);
    }

    dumpGamepad(ctl);
}

// -----------------------------
// Process all controllers
// -----------------------------

void processControllers() {
    for (auto myController : myControllers) {
        if (myController && myController->isConnected() && myController->hasData()) {
            if (myController->isGamepad()) {
                processGamepad(myController);
            }
            else {
                Serial.println("Unsupported controller");
            }
        }
    }
}

// -----------------------------
// Arduino setup
// -----------------------------

void setup() {
    Serial.begin(115200);

    pinMode(builtInLed, OUTPUT);
    digitalWrite(builtInLed, LOW);

    // UART to MSP432
    // 9600 baud matches the MSP432 BLE_UART_Init setup from the lab.
    Serial2.begin(9600, SERIAL_8N1, ESP32_RX2_PIN, ESP32_TX2_PIN);

    Serial.printf("Firmware: %s\n", BP32.firmwareVersion());

    const uint8_t* addr = BP32.localBdAddress();

    Serial.printf(
        "BD Addr: %02X:%02X:%02X:%02X:%02X:%02X\n",
        addr[0],
        addr[1],
        addr[2],
        addr[3],
        addr[4],
        addr[5]
    );

    BP32.setup(&onConnectedController, &onDisconnectedController);

    /*
      Keep this while testing/pairing.
      After pairing works, you can comment it out so the controller reconnects faster.
    */
    BP32.forgetBluetoothKeys();

    BP32.enableVirtualDevice(false);

    Serial.println("ESP32 gamepad UART transmitter ready.");
}

// -----------------------------
// Arduino loop
// -----------------------------

void loop() {
    bool dataUpdated = BP32.update();

    if (dataUpdated) {
        processControllers();
    }

    delay(10);
}