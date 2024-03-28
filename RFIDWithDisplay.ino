#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <MFRC522.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


#if defined(ESP32)
#define SS_PIN 5
#define RST_PIN 22
#elif defined(ESP8266)
#define SS_PIN D8
#define RST_PIN D0
#endif

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

byte nuidPICC[4];


String DatoHex;
const String UserReg_1 = "43CF5330";
const String UserReg_2 = "E38B5214";


void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();
  Serial.println();
  Serial.print(F("Reader :"));
  rfid.PCD_DumpVersionToSerial();
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  DatoHex = printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
  Serial.println();
  Serial.println();
  Serial.println("Ingresa tu credencial");

  //Configuracion display
  SPI.begin();
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("Ingresa tu credencial");
  display.display();
}

void loop() {

  if (!rfid.PICC_IsNewCardPresent()) { return; }

  if (!rfid.PICC_ReadCardSerial()) { return; }

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && piccType != MFRC522::PICC_TYPE_MIFARE_1K && piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println("Su Tarjeta no es del tipo MIFARE Classic.");
    return;
  }

  if (rfid.uid.uidByte[0] != nuidPICC[0] || rfid.uid.uidByte[1] != nuidPICC[1] || rfid.uid.uidByte[2] != nuidPICC[2] || rfid.uid.uidByte[3] != nuidPICC[3]) {
    Serial.println("Se ha detectado una nueva tarjeta.");


    for (byte i = 0; i < 4; i++) { nuidPICC[i] = rfid.uid.uidByte[i]; }

    DatoHex = printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.print("Codigo Tarjeta: ");
    Serial.println(DatoHex);

    if (UserReg_1 == DatoHex) {
      Serial.println("ALUMNO 1 - PUEDE INGRESAR");
    } else if (UserReg_2 == DatoHex) {
      Serial.println("ALUMNO 2 - PUEDE INGRESAR");
    } else {
      Serial.println("NO ES UN ALUMNO - PROHIBIDO EL INGRESO");
    }
    Serial.println();
  } else {
    Serial.println("El ALUMNO YA A INGRESADO");
  }

  rfid.PICC_HaltA();

  rfid.PCD_StopCrypto1();
}


String printHex(byte *buffer, byte bufferSize) {
  String DatoHexAux = "";
  for (byte i = 0; i < bufferSize; i++) {
    if (buffer[i] < 0x10) {
      DatoHexAux = DatoHexAux + "0";
      DatoHexAux = DatoHexAux + String(buffer[i], HEX);
    } else {
      DatoHexAux = DatoHexAux + String(buffer[i], HEX);
    }
  }

  for (int i = 0; i < DatoHexAux.length(); i++) { DatoHexAux[i] = toupper(DatoHexAux[i]); }
  return DatoHexAux;
}
