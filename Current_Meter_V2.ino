#include <Wire.h>
//#include <SPI.h>
#include <tinyFAT.h>

#include <Adafruit_INA219.h>
#include "SSD1306AsciiWire.h"

#define LOG_FILE "output.csv"
#define INTERVAL 200

Adafruit_INA219 ina219;
SSD1306AsciiWire oled;

//File outputFile;

double energy = 0.0;
double energy_ = 0.0;
double voltage = 0.0;
double current = 0.0;
double power = 0.0;

bool print_always = true;

void startScreen();
void showValue(String unit, double * old_value, double new_value, short line, bool print_always);
byte res;

void setup() {
  Serial.begin(115200);
  Serial.println(F("VOLTAGE,CURRENT,POWER"));
  ina219.begin();
  startScreen();
  res=file.initFAT(); 
  if (!res) {
    oled.setCursor(34,2);
    oled.print(F("No SD"));
    oled.setCursor(10,4);
    oled.print(F("detected!"));
    delay(1000);
    oled.clear();
  }
}

void loop() {
  int c = millis()%INTERVAL;
  if(c == 0){
   
    double shuntvoltage = ina219.getShuntVoltage_mV();
    double busvoltage = ina219.getBusVoltage_V();
    const int t = 3600*(1000/INTERVAL);
    showValue("W",&power,ina219.getPower_mW()/1000, 0, print_always);
    showValue("V",&voltage,busvoltage + (shuntvoltage / 1000), 2, print_always);
    showValue("A",&current,ina219.getCurrent_mA()/1000, 4, print_always);
    oled.set1X();
    showValue("Wh",&energy,(energy + power/t), 6, print_always);
    showValue("Ah",&energy_,(energy_ + current/t), 7, print_always);
    oled.set2X();
    
    print_always = false;
    Serial.println((String)voltage+","+(String)current+","+(String)power);
    bool first_open = file.exists(LOG_FILE);
    res = file.openFile(LOG_FILE, FILEMODE_TEXT_WRITE);
    if (res == NO_ERROR ) {
      if(!first_open)
        file.writeLn("TIME,VOLTAGE,CURRENT,POWER,ENERGY");
      //res.seek(EOF);
      //(String)millis()+","+(String)voltage+","+(String)current+","+(String)power+","+(String)energy
      file.writeLn("---------------------------------------");
      file.closeFile();
    }
  }
}


void startScreen(){
  Wire.begin();
  Wire.setClock(400000L);
  oled.begin(&Adafruit128x64, 0x3C);
  oled.setFont(Adafruit5x7);
  oled.set2X();
  oled.clear();
  delay(50);
}

bool comparFloats(double v1, double v2){
  return (roundf(v1 * 1000)) == (roundf(v2 * 1000));
}

void showValue(String unit, double * old_value, double new_value, short line, bool print_always){
  if(!comparFloats((*old_value), new_value) || print_always){
    oled.setCursor(0,line);
    oled.print(new_value,3);
    oled.clearToEOL();
    oled.print(unit);
  }
  *old_value = new_value;
}
