#define SERIAL_SPEED  19200

//define pins for the red, blue and green LEDs
#define RED_LED 7
#define BLUE_LED 6
#define GREEN_LED 9

#include <LiquidCrystal.h>
#include <sSense-CCS811.h>

/*
  The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
 */

//overall brightness value for the LEDs
int brightness = 0;
//individual brightness values for the red, green, and blue LEDs
int gBright = 0;
int rBright = 0;
int bBright = 0;

int danger_value = 0;
int CO2_danger_value = 0;
int tVOC_danger_value = 0;

CCS811 ssenseCCS811;

LiquidCrystal lcd(12, 11, 5, 4, 3, 2); 

void setup() {
  // AirQuality - set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  Serial.begin(SERIAL_SPEED);
  delay(5000);
  Serial.println("s-Sense CCS811 I2C sensor.");
  if(!ssenseCCS811.begin(uint8_t(I2C_CCS811_ADDRESS), uint8_t(CCS811_WAKE_PIN), driveMode_1sec))
    Serial.println("Initialization failed.");

  // LED - setup pins to output
  pinMode( GREEN_LED, OUTPUT);
  pinMode( RED_LED, OUTPUT);
  pinMode( BLUE_LED, OUTPUT);

  // the blue LED stays off
  analogWrite(BLUE_LED, brightness);
}

int CO2;
int tVOC;

void loop() {
  ssenseCCS811.setEnvironmentalData((float)(21.102), (float)(57.73));

  if (ssenseCCS811.checkDataAndUpdate()) {
    CO2 = ssenseCCS811.getCO2();
    tVOC = ssenseCCS811.gettVOC();
    
    lcd.setCursor(0, 0);
    if (CO2 < 410) {
      lcd.print(String ("CO2:LOW PPM ")); 
    } else {
      lcd.print(String ("CO2:")+ String (CO2)+String(" PPM "));
    }
    lcd.setCursor(13,0);
    lcd.print("MIN");
    
    lcd.setCursor(0, 1);
    lcd.print(String ("TVOC:")+ String (tVOC)+String(" PPB "));
    lcd.setCursor(12,1);
    lcd.print(String (millis()/1000/60));

    // LED logic
    //                       CO2            tVOC (ppb)
    // red 255, 0 0       40000+ ppm        2000-5000
    //                    5000-40000        600-2000
    // yellow 255 255 0   2000-5000 ppm     200-600
    //                    400-2000          60-200
    // green 0 255 0      400 ppm           0-60
    //
    // calculate danger value
    // whichever tVOC or CO2 level is more dangerous, that's what
    //  the danger value will be
    // These equations are approximations of the given danger levels
    //  as seen above
    
    // CO2 danger level
    if (CO2 < 5000) {
      CO2_danger_value = 29.773 * log(CO2) - 178.23;
      }
    else {
      CO2_danger_value = 0.000417 * CO2 + 71.4;
      }
     
     // tVOC danger level
     if (tVOC < 60) {
      tVOC_danger_value = 0.4168 * tVOC;
      }
    else {
      tVOC_danger_value = 21.511 * log(tVOC) - 63.287;
      }   

    // determine Danger Value
    if (CO2_danger_value > tVOC_danger_value) {
      danger_value = CO2_danger_value;
      } else {
      danger_value = tVOC_danger_value;
      }
    Serial.println("danger_value: " + String(danger_value));
    // calculate red
    if (danger_value < 50) {
      rBright = 5.08 * danger_value;
      } else {
      rBright = 255;
      }

    // calculate green
    if (danger_value <= 50) {
      gBright = 255;
      } else {
      gBright = -5.1103 * danger_value + 510;
      }
    
    //rBright = 255;
    //gBright = 255;
    Serial.println("rBright: " + String(rBright));
    Serial.println("gBright: " + String(gBright));
    Serial.println("bBright: " + String(bBright));
    Serial.println("");
  
    analogWrite(RED_LED, rBright);
    analogWrite(GREEN_LED, gBright);
    
  } else {
    lcd.print("ERROR1");
  }
  delay(2000);  
}
