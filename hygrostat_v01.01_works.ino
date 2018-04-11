// Include libraries
#include <Wire.h>                                                           // I2C library
#include <EEPROM.h>                                                         // EEPROM library
#include <Adafruit_SSD1306.h>                                               // OLED library
#include <SimpleDHT.h>                                                      // DHT library

// Display defines
#define OLED_I2C_ADDRESS 0x3C                                               // I2C address of OLED display
#define OLED_RESET 4

// Hardware pin defines
#define DHT_PIN 2                                                           // Pin of DHT11 sensor
#define MINUS_BUTTON 12                                                     // Pin of minus button
#define PLUS_BUTTON 13                                                      // Pin of plus button
#define RELAY_PIN 6

// EEPROM addresses
#define EEPROM_SOLL 0x00                                                    // Soll value of humidity is stored here

// Timing variables
#define CYCLE_TIME 10                                                       // in milliseconds
#define DISPLAY_ON_TIME 2                                                   // in seconds

// Software constants
#define HALF_HYST 5                                                         // Half of the hysteresis. Switching happens from soll value plus, respectively minus, this value

// Declaration & definitions
bool buttonFlag[2] = {0,0};                                                 // Used for edge detection of button presses
bool displayPower = 1;                                                      // Stores current state of display
bool relayPower = 0;                                                        // Stores current state of relay
byte soll;                                                                  // Stores the soll value of the humidity in percent
byte temperature;                                                           // Stores the temperature information read from the sensor                                                    
byte humidity;                                                              // Stores the humidity information read from the sensor
byte timeoutCounter = 0;                                                    // Used for waiting out the display backlight

// Object classification
Adafruit_SSD1306 display(OLED_RESET);                                       // Create display object
SimpleDHT11 dht11;                                                          // Create sensor object

// Function declarations
void WriteDisplay();                                                        // Writes the current sensor data onto the display
void ClearDisplay();                                                        // Writes temperature onto display and later clears it

// Setup/Init
void setup(){
  // Set pin modes
  pinMode(MINUS_BUTTON, INPUT);
  pinMode(PLUS_BUTTON, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  
  // Preset relay
  digitalWrite(RELAY_PIN, relayPower);
  
  // Get soll value out of non volatile storage (EEPROM)
  soll = EEPROM.read(EEPROM_SOLL);
  
  // Initialize display
  display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  
  // Write title screen to display
  display.clearDisplay();
  display.println("Hygrostat");
  display.println("  v1.01");
  display.display();

  // Wait a while
  delay(1500);

  // Start into hygrostat settings with first measurement
  dht11.read(DHT_PIN, &temperature, &humidity, NULL);
  WriteDisplay();
}

// Main loop
void loop(){
  // Get sensor values
  dht11.read(DHT_PIN, &temperature, &humidity, NULL);
  
  // Get button presses
  if(digitalRead(MINUS_BUTTON) && !buttonFlag[0]){
    if(soll > 5){
      soll -= 5;
      EEPROM.write(EEPROM_SOLL, soll);
      displayPower = 1;
      WriteDisplay();
      timeoutCounter = 0;
    }
  }
  buttonFlag[0] = digitalRead(MINUS_BUTTON);
  if(digitalRead(PLUS_BUTTON) && !buttonFlag[1]){
    if(soll < 95){
      soll += 5;
      EEPROM.write(EEPROM_SOLL, soll);
      displayPower = 1;
      WriteDisplay();
      timeoutCounter = 0;
    }
  }
  buttonFlag[1] = digitalRead(PLUS_BUTTON);

  // Timeout counter for display power
  if (++timeoutCounter == (DISPLAY_ON_TIME * 100)) {
        timeoutCounter = 0;
        if(displayPower){
          ClearDisplay();
        }
        displayPower = 0;
  }

  // Switching the relay according to the soll value, ist value and hysteresis rules
  if(humidity < (soll - HALF_HYST)){
    relayPower = 1;
    digitalWrite(RELAY_PIN, relayPower);
  }
  if(humidity > (soll + HALF_HYST)){
    relayPower = 0;
    digitalWrite(RELAY_PIN, relayPower);
  }

  // Wait preset period
  delay(CYCLE_TIME);
}

void WriteDisplay(){
  // Clear the buffer.
  display.clearDisplay();

  // Write humidity
  display.setCursor(0,0);
  display.print("Ist:  "); display.print(humidity); display.println(" %");
  display.print("Soll: "); display.print(soll); display.println(" %");
  display.display();
}

void ClearDisplay(){
    // Clear the buffer.
  display.clearDisplay();

  // Write temperature
  display.setCursor(0,0);
  display.println("Temperatur");
  display.print("   "); display.print(temperature); display.println(" C");
  display.display();
  delay(3000);
  
  // Clear the buffer.
  display.clearDisplay();
  display.display();
}
