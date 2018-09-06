
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"

LiquidCrystal_I2C lcd(0x27);


// how many milliseconds between grabbing data and logging it. 1000 ms is once a second
#define LOG_INTERVAL  1000 // mills between entries (reduce to take more/faster data)

// how many milliseconds before writing the logged data permanently to disk
// set it to the LOG_INTERVAL to write each time (safest)
// set it to 10*LOG_INTERVAL to write all data every 10 datareads, you could lose up to 
// the last 10 reads if power is lost but it uses less power and is much faster!
#define SYNC_INTERVAL 1000// mills between calls to flush() - to write data to the card
uint32_t syncTime = 0; // time of last sync()

#define ECHO_TO_SERIAL   1 // echo data to serial port
#define WAIT_TO_START    0 // Wait for serial input in setup
     

RTC_PCF8523 RTC; // define the Real Time Clock object

// for the data logging shield, we use digital pin 10 for the SD cs line
const int chipSelect = 10;

// the logging file
File logfile;

void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);
  
  // red LED indicates error


  while(1);
}

void setup(void)
{
  
  Serial.begin(9600);
  Serial.println();

  lcd.begin(16, 2); 
  lcd.setCursor(0,0);
  lcd.print("Welcome to APTC!");

  
#if WAIT_TO_START
  Serial.println("Type any character to start");
  while (!Serial.available());
#endif //WAIT_TO_START

  // initialize the SD card
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    error("Card failed, or not present");
  }
  Serial.println("card initialized.");
  
  // create a new file
   char filename[] = "LOGGER00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE); 
      break;  // leave the loop!
    }
  }
  
  
  if (! logfile) {
    error("couldnt create file");
  }
  
  Serial.print("Logging to: ");
  Serial.println(filename);

  // connect to RTC
  Wire.begin();  
  if (!RTC.begin()) {
    logfile.println("RTC failed");
#if ECHO_TO_SERIAL
    Serial.println("RTC failed");
#endif  //ECHO_TO_SERIAL
 
  
  }
  

  logfile.println("millis, stamp, datetime, CH4VOLTAGE_01, CH4VOLTAGE_02, CO2VOLTAGE_01, CO2VOLTAGE_02, CH4PPM_01, CH4PPM_02, CO2PPM_01, CO2PPM_02, CH4PPM_AVG, CO2PPM_AVG");
  #if ECHO_TO_SERIAL
  Serial.println("millis, stamp, datetime, CH4VOLTAGE_01, CH4VOLTAGE_02, CO2VOLTAGE_01, CO2VOLTAGE_02, CH4PPM_01, CH4PPM_02, CO2PPM_01, CO2PPM_02, CH4PPM_AVG, CO2PPM_AVG");
  #endif
 
  // If you want to set the aref to something other than 5v

}

void loop(void)
{
  DateTime now;
 
  // delay for the amount of time we want between readings
  delay((LOG_INTERVAL -1) - (millis() % LOG_INTERVAL));
  
  // log milliseconds since starting
  uint32_t m = millis();
  logfile.print(m);           // milliseconds since start
  logfile.print(", ");    
#if ECHO_TO_SERIAL
  Serial.print(m);         // milliseconds since start
  Serial.print(", ");  
#endif

  // fetch the time
  now = RTC.now();
  // log time
  logfile.print(now.unixtime()); // seconds since 1/1/1970
  logfile.print(", ");
  logfile.print('"');
  logfile.print(now.year(), DEC);
  logfile.print("/");
  logfile.print(now.month(), DEC);
  logfile.print("/");
  logfile.print(now.day(), DEC);
  logfile.print(" ");
  logfile.print(now.hour(), DEC);
  logfile.print(":");
  logfile.print(now.minute(), DEC);
  logfile.print(":");
  logfile.print(now.second(), DEC);
  logfile.print('"');
#if ECHO_TO_SERIAL
  Serial.print(now.unixtime()); // seconds since 1/1/1970
  Serial.print(", ");
  Serial.print('"');
  Serial.print(now.year(), DEC);
  Serial.print("/");
  Serial.print(now.month(), DEC);
  Serial.print("/");
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(":");
  Serial.print(now.minute(), DEC);
  Serial.print(":");
  Serial.print(now.second(), DEC);
  Serial.print('"');
#endif //ECHO_TO_SERIAL

  float CH4PPM_01, CH4PPM_02, CO2PPM_01, CO2PPM_02;
  
  int CH4READING_01 = analogRead(A0); 
  float CH4VOLTAGE_01 = CH4READING_01 * (5.0/1023.0);  
  CalculateCH4_01(CH4VOLTAGE_01);
  
  int CH4READING_02 = analogRead(A1);  
  float CH4VOLTAGE_02 = CH4READING_02 * (5.0/1023.0);
  CalculateCH4_02( CH4VOLTAGE_02);
  
  int CO2READING_01 = analogRead(A2);  
  float CO2VOLTAGE_01 = CO2READING_01 * (5.0/1023.0);
  CalculateCO2_01(CO2VOLTAGE_01);
   
  int CO2READING_02 = analogRead(A3);  
  float CO2VOLTAGE_02 = CO2READING_02 * (5.0/1023.0);
  CalculateCO2_02(CO2VOLTAGE_02);

  float CH4PPM_AVG = (CH4PPM_01 + CH4PPM_02)/2;
  float CO2PPM_AVG = (CO2PPM_01 + CO2PPM_02)/2;
  
  logfile.print(CH4VOLTAGE_01);
  logfile.print(", ");
  logfile.print(CH4VOLTAGE_02);
  logfile.print(", ");
  logfile.print(CO2VOLTAGE_01);
  logfile.print(", ");
  logfile.print(CO2VOLTAGE_02);
  logfile.print(", "); 
  logfile.print(CH4PPM_01);
  logfile.print(", ");
  logfile.print(CH4PPM_02);
  logfile.print(", ");
  logfile.print(CO2PPM_01);
  logfile.print(", ");
  logfile.print(CO2PPM_02);
  logfile.print(", ");
  logfile.print(CH4PPM_AVG);
  logfile.print(", ");
  logfile.print(CO2PPM_AVG);
  logfile.println();

  
  int Counter = 1;
  int ButtonPin = 8;
  pinMode(ButtonPin, INPUT);
  int ButtonInput = digitalRead(ButtonPin);
  
  if(ButtonInput = HIGH){
  
    Counter++;
  
   }

  if(Counter > 3){

    Counter = 1;
  
  }
  
  
  switch(Counter){

  case 1:
  lcd.setCursor(0, 0);
  lcd.print("AVGCH4:");
  lcd.setCursor(0, 9);
  lcd.print("AVGCO2:");
  lcd.setCursor(1, 5);
  lcd.print("ppm");
  lcd.setCursor(1, 13);
  lcd.print("ppm");
  lcd.setCursor(1, 0);
  lcd.print(CH4PPM_AVG);
  lcd.setCursor(1, 9);
  lcd.print(CO2PPM_AVG);
  break;

  case 2:
  lcd.setCursor(0, 0);
  lcd.print("CH4PPM_01:");
  lcd.setCursor(0, 9);
  lcd.print("CO2PPM_01:");
  lcd.setCursor(1, 5);
  lcd.print("ppm");
  lcd.setCursor(1, 13);
  lcd.print("ppm");
  lcd.setCursor(1, 0);
  lcd.print(CH4PPM_01);
  lcd.setCursor(1, 9);
  lcd.print(CO2PPM_01);
  break;
  
  case 3:
  lcd.setCursor(0, 0);
  lcd.print("CH4PPM_02:");
  lcd.setCursor(0, 9);
  lcd.print("CO2PPM_02:");
  lcd.setCursor(1, 5);
  lcd.print("ppm");
  lcd.setCursor(1, 13);
  lcd.print("ppm");
  lcd.setCursor(1, 0);
  lcd.print(CH4PPM_02);
  lcd.setCursor(1, 9);
  lcd.print(CO2PPM_02);
  break;
    
  }
 
  
 #if ECHO_TO_SERIAL 
  Serial.print(CH4VOLTAGE_01); 
  Serial.print(", "); 
  Serial.print(CH4VOLTAGE_02);
  Serial.print(", ");
  Serial.print(CO2VOLTAGE_01);
  Serial.print(", ");
  Serial.print(CO2VOLTAGE_02);
  Serial.print(", ");
  Serial.print(CH4PPM_01); 
  Serial.print(", "); 
  Serial.print(CH4PPM_02);
  Serial.print(", ");
  Serial.print(CH4PPM_AVG);
  Serial.print(", ");
  Serial.print(CO2PPM_01);
  Serial.print(", ");
  Serial.print(CO2PPM_02);
  Serial.print(", ");
  Serial.print(CO2PPM_AVG);
  Serial.println();
#endif //ECHO_TO_SERIAL

  // Log the estimated 'VCC' voltage by measuring the internal 1.1v ref

  logfile.println();

  // Now we write data to disk! Don't sync too often - requires 2048 bytes of I/O to SD card
  // which uses a bunch of power and takes time
  if ((millis() - syncTime) < SYNC_INTERVAL) return;
  syncTime = millis();
  logfile.flush();
  
}

 float CalculateCH4_01(float CH4VOLTAGE_01)
 {
     //define variables
     float lnPPM_CH401, CH4PPM_01, ratio_CH401, Rs_CH401;
     float Ro_CH4 = 98800, R1_CH4 = 7700, Vref = 5.0;
     
     //Calculate resistance
     Rs_CH401 = (R1_CH4 * CH4VOLTAGE_01) / (Vref - CH4VOLTAGE_01);
    
     //Calculate resistance ratio
     ratio_CH401 = Rs_CH401/Ro_CH4;
    
     //Use log statistics to convert resistance reading
     lnPPM_CH401 = (log(ratio_CH401)-1.92)/-0.602;
    
     //Convert from log value to PPM reading
     CH4PPM_01 = exp(lnPPM_CH401); 

     return(CH4PPM_01);
  }
  
 float CalculateCH4_02(float CH4VOLTAGE_02)
   {
     
     float lnPPM_CH402, CH4PPM_02, ratio_CH402, Rs_CH402;
     float Ro_CH4 = 98800, R1_CH4 = 7700, Vref = 5.0;
     
     Rs_CH402= (R1_CH4 * CH4VOLTAGE_02) / (Vref - CH4VOLTAGE_02);
     
     ratio_CH402 = Rs_CH402/Ro_CH4;
     
     lnPPM_CH402 = (log(lnPPM_CH402)-1.92)/-0.602;
     
     CH4PPM_02 = exp(lnPPM_CH402);  

     return(CH4PPM_02);
  }

  float CalculateCO2_01(float CO2VOLTAGE_01)
  {
     
     float lnPPM_CO201, CO2PPM_01, ratio_CO201, Rs_CO201;
     float Ro_CO2 = 98800, R1_CO2 = 7700, Vref = 5.0;

     Rs_CO201 = (R1_CO2 * CO2VOLTAGE_01) / (Vref - CO2VOLTAGE_01);
     
     ratio_CO201 = Rs_CO201/Ro_CO2;
     
     lnPPM_CO201 = (log(ratio_CO201)-1.64)/-0.346;
     
     CO2PPM_01 = exp(lnPPM_CO201);

     return(CO2PPM_01);
  }

float CalculateCO2_02(float CO2VOLTAGE_02)
 {
     
     float lnPPM_CO202, CO2PPM_02, ratio_CO202, Rs_CO202;
     float Ro_CO2 = 98800, R1_CO2 = 7700, Vref = 5.0;

     Rs_CO202 = (R1_CO2 * CO2VOLTAGE_02) / (Vref - CO2VOLTAGE_02);
     
     ratio_CO202 = Rs_CO202/Ro_CO2;
     
     lnPPM_CO202 = (log(ratio_CO202)-1.64)/-0.346;
     
     CO2PPM_02 = exp(lnPPM_CO202);

     return(CO2PPM_02);
  }
