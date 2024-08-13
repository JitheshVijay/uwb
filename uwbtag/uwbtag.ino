#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <DW1000.h>
#include <DW1000Ranging.h>

#define TAG_ADDR "7D:00:22:EA:82:60:3B:9B"

#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23

#define UWB_RST 27 // reset pin
#define UWB_IRQ 34 // irq pin
#define UWB_SS 21 // spi select pin



#define I2C_SDA 4
#define I2C_SCL 5

Adafruit_SSD1306 display(128, 64, &Wire, -1);

void setup() {
  Serial.begin(115200);

  // Initialize I2C for OLED display
  Wire.begin(I2C_SDA, I2C_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay(); 


  // Show logo
  displayLogo();

  // Initialize SPI for UWB
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

  // Initialize UWB
  DW1000Ranging.initCommunication(UWB_RST, UWB_SS, UWB_IRQ); // Reset, CS, IRQ pin
  DW1000Ranging.attachNewRange(newRange);
  DW1000Ranging.attachNewDevice(newDevice);
  DW1000Ranging.attachInactiveDevice(inactiveDevice);

  // Set UWB parameters for longer range
  DW1000.setAntennaDelay(16436); // Example value, you may need to calibrate this for your setup
  DW1000.enableMode(DW1000.MODE_LONGDATA_RANGE_LOWPOWER); // Use low power mode for long range
  DW1000.setChannel(DW1000.CHANNEL_5); // Set the channel (e.g., Channel 5)
  DW1000.setPulseFrequency(DW1000.TX_PULSE_FREQ_64MHZ); // Set the pulse frequency to higher for better range
  DW1000.setDataRate(DW1000.TRX_RATE_110KBPS); // Use low data rate for longer range
  DW1000.setPreambleLength(DW1000.TX_PREAMBLE_LEN_2048); // Increase preamble length for better range

  // Attempt to maximize transmit power
  //DW1000.writeTransmitPower(0x1F1F1F1F); // Max transmit power setting

  DW1000Ranging.startAsTag(TAG_ADDR, DW1000.MODE_LONGDATA_RANGE_LOWPOWER);
}

void loop() {
  DW1000Ranging.loop();
  static long lastUpdateTime = 0;
  if (millis() - lastUpdateTime > 1000) {
    displayUWBInfo();
    lastUpdateTime = millis();
  }
}



void newRange() {
    Serial.print("From: ");
    Serial.print(DW1000Ranging.getDistantDevice()->getShortAddress(), HEX);
    Serial.print("\t Range: ");
    Serial.print(DW1000Ranging.getDistantDevice()->getRange());
    Serial.print(" m");
    Serial.print("\t RX power: ");
    Serial.print(DW1000Ranging.getDistantDevice()->getRXPower());
    Serial.println(" dBm");
}

void newDevice(DW1000Device *device) {
    Serial.print("Ranging init; 1 device added! -> ");
    Serial.print("Short: ");
    Serial.println(device->getShortAddress(), HEX);
}

void inactiveDevice(DW1000Device *device) {
    Serial.print("Delete inactive device: ");
    Serial.println(device->getShortAddress(), HEX);
}

void displayLogo() {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(F("LVLALPHA"));
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println(F("DW1000 DEMO"));
    display.display();
    delay(2000);
}

void displayUWBInfo() {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    DW1000Device *device = DW1000Ranging.getDistantDevice();

    if (device != NULL) {
        display.setTextSize(2);
        display.setCursor(0, 0);
        display.print("Range: ");
        display.print(device->getRange());
        display.println(" m");

        display.setTextSize(1);
        display.setCursor(0, 20);
        display.print("RX Power: ");
        display.print(device->getRXPower());
        display.println(" dBm");
    } else {
        display.setTextSize(2);
        display.setCursor(0, 0);
        display.println("No Device");
    }

    display.display();
}
