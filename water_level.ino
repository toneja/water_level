#include <bluefruit.h>

// BLUETOOTH
BLEUart bleuart;
const char bleName[12] = "WATER LEVEL";

// WATER LEVEL
#define ZERO 25
bool waterLevelZero = false;
unsigned long zeroStart = 0;

void setup() {
  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, HIGH);
  Serial.begin(115200);
  delay(2000);
  // while (!Serial) { ; }
  Serial.println("Starting water level sensor...");
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  for (uint8_t i = 0; i < 8; i++) {
    digitalToggle(LED_GREEN);
    digitalToggle(LED_BLUE);
    delay(250);
  }
  pinMode(WB_A1, INPUT);
  analogReference(AR_INTERNAL_3_0);
  analogReadResolution(12);
  ble_init();
}

void loop() {
  int waterLevel = analogRead(WB_A1);
  if (waterLevel <= ZERO && !waterLevelZero) {
    waterLevelZero = true;
    zeroStart = millis();
  } else if (waterLevel > ZERO && waterLevelZero) {
    waterLevelZero = false;
  }
  if (waterLevelZero) {
    unsigned long elapsed = (millis() - zeroStart) / 1000;
    unsigned int hours = elapsed / 3600;
    unsigned int minutes = (elapsed % 3600) / 60;
    unsigned int seconds = elapsed % 60;
    Serial.printf("Water level: 0 | Time at zero: %02u:%02u:%02u\n", hours, minutes, seconds);
    if (Bluefruit.connected() && bleuart.notifyEnabled()) {
      bleuart.printf("%02u:%02u:%02u", hours, minutes, seconds);
    }
  } else {
    Serial.println(waterLevel);
    if (Bluefruit.connected() && bleuart.notifyEnabled()) {
      bleuart.printf("%d", waterLevel);
    }
  }
  delay(1000);
}

void ble_init(void) {
  Bluefruit.autoConnLed(false);
  Bluefruit.begin(1, 0);
  Bluefruit.setTxPower(8);
  Bluefruit.setName(bleName);
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);
  bleuart.begin();
  startAdv();
}

void startAdv(void) {
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(bleuart);
  Bluefruit.ScanResponse.addName();
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);
  Bluefruit.Advertising.setFastTimeout(30);
  Bluefruit.Advertising.start(0);
}

void connect_callback(uint16_t conn_handle) {
  BLEConnection* connection = Bluefruit.Connection(conn_handle);
  char central_name[32];
  memset(central_name, 0, sizeof(central_name));
  connection->getPeerName(central_name, sizeof(central_name));
  Serial.printf("Connected to %s\n", central_name);
}

void disconnect_callback(uint16_t conn_handle, uint8_t reason) {
  (void)conn_handle;
  (void)reason;
  Serial.printf("Disconnected, reason = %#X\n", reason);
}
