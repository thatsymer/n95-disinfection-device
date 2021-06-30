#include <math.h>
#include <Wire.h>                   //wire library
#include <LiquidCrystal_I2C.h>      // i2c library
LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display
#include <ezButton.h>               // start button library

ezButton button(7);                 // Start button pin
const int buzzer = 6;               //buzzer to arduino pin 6
int finish=0;
const int UVRELAY_PIN = 11;          // UV RELAY PIN
const int HEATRELAY_PIN = 10;        // HEAT RELAY PIN
int LED_START = 2;
int LED_STOP = 8;
int LED_UV = 9;
int LED_HEAT = 4;
unsigned long count;
unsigned long currentMillis = millis();
unsigned long timeLeft,timeNow;
unsigned long timeStart;
unsigned long previousMillis = 0;   
long countdown_minute;
long countdown_sec;
long minute = 59, second = 59;//second
long countdown_time = (minute * 60) + second;
const int B = 4275;               // B value of the thermistor
const int R0 = 100000;            // R0 = 100k
const int pinTempSensor = A1;     // Temperature Sensor connected to A1
const int pinUVSensor= A0;        // UV Sensor connected to A0
int sensorValue;                  // UV sensor value
int analogValue;                  // UV analog value
float temperature;                // Temp value 
float R;                          //TempSensor  sensor value
int a;                            //TempSensor  analog value

#if defined(ARDUINO_ARCH_AVR)
#define debug  Serial
#elif defined(ARDUINO_ARCH_SAMD) ||  defined(ARDUINO_ARCH_SAM)
#define debug  SerialUSB
#else
#define debug  Serial
#endif
 
void setup()
{
    lcd.init();          // Initiate the LCD module
    lcd.backlight();     // Turn on the backlight
    Serial.begin(9600);  // Serial Monitor
    pinMode(UVRELAY_PIN, OUTPUT); // UV LAMP OUTPUT
    pinMode(HEATRELAY_PIN, OUTPUT);// HEAT LAMP OUTPUT
    pinMode(LED_START, OUTPUT);
    pinMode(LED_STOP, OUTPUT);
    pinMode(LED_UV, OUTPUT);
    pinMode(LED_HEAT, OUTPUT);
    button.setDebounceTime(50); // set debounce time to 50 milliseconds
    button.setCountMode(COUNT_FALLING);
    pinMode(buzzer, OUTPUT); // Set buzzer - pin 6 as an output
}

void UVsensor()
{
    analogValue=analogRead(pinUVSensor);
    if(analogValue<10)
    {
      sensorValue= 0;
      
    }
    else
    {
      sensorValue= 0.3*analogValue-1;    // convert to UV INDEX via datasheet
    }
}

void tempSensor()
{
    a = analogRead(pinTempSensor);
 
    R = 1023.0/a-1.0;
    R = R0*R;
 
    temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15;   // convert to temperature via datasheet
}

void timer()
  {
    if (finish<=0){
  unsigned long currentMillis = millis();
  currentMillis - previousMillis;
    Serial.println("Time Now:");
    Serial.println(currentMillis);
    long countdowntime_seconds = countdown_time - ((currentMillis-timeStart) / 1000);
    if (countdowntime_seconds >= 0) {
    countdown_minute = ((countdowntime_seconds / 60)%30);
    countdown_sec = countdowntime_seconds % 60;
    lcd.print("TIMER= ");
    lcd.setCursor(7 , 2);
    
    if (countdown_minute < 10) {
      lcd.print("0");
    }
    lcd.print(countdown_minute);
    lcd.print(":");
    if (countdown_sec < 10) {
      lcd.print("0");
    }
      lcd.print(countdown_sec);
      delay(200);
    }

  }


}

void start()
{
      while (!button.isPressed()) // start button is pressed
      button.loop();
      unsigned long count = button.getCount();
      count++;
       timeStart = millis();

}


void loop()
{      
          unsigned long currentMillis = millis();
          currentMillis - previousMillis;
          Serial.println("==============================");
          Serial.print("time started:");
          Serial.println(timeStart);
          Serial.print("temperature = ");
          Serial.println(temperature);
          Serial.print("UV index = ");
          Serial.println(sensorValue);
          Serial.print("Button Counter=");
          Serial.println(count);
          Serial.println("==============================");
          
//*************ON START SEQUENCE**********************//
          
          UVsensor();                                 //call UV function
          tempSensor();                               //call UV function
          button.loop();                            
          count = button.getCount();
     if (count == 0)                                  // if start button is not pressed execute ff lines
        {
          digitalWrite(LED_STOP, HIGH);               // DISPLAY LED's
          digitalWrite(LED_START, LOW);
          lcd.setCursor(1,1);                         // Starting
          lcd.print("PRESS GREEN BUTTON");            // Message
          lcd.setCursor(6,2);                         
          lcd.print("TO START ");
          start();   
          digitalWrite(LED_STOP,LOW);
          delay(200);
          digitalWrite(UVRELAY_PIN, HIGH);   // Turns off UV Lamp (HIGH=ON, LOW=OFF)
           digitalWrite(HEATRELAY_PIN, HIGH);       // Turns off Heat Lamp(HIGH=OFF, LOW=ON)

          } 
          
//*************END OF ON START SEQUENCE****************//

if(count>0 ){                                          // execute if start button is pressed
    if (temperature < 50 && finish== 0) {              // 55 celsius is needed in order to start the timer
      lcd.clear();
      lcd.setCursor(4,0);
      lcd.print("Initialiazing");
      lcd.setCursor(0, 1);
      lcd.print("Temp in C: ");
      lcd.print(temperature);
      lcd.setCursor(0,2);
      lcd.print("UV index: ");
      lcd.print(sensorValue);
      digitalWrite(UVRELAY_PIN, LOW);                // Turns on UV & Heat Lamp, display LED's
      digitalWrite(HEATRELAY_PIN, LOW);
      digitalWrite(LED_HEAT, HIGH);
      digitalWrite(LED_UV, HIGH);
      digitalWrite(LED_START, HIGH);
      delay(1000);
      }
      
    if (temperature > 50 && finish == 0)             // if the ideal temperature is reached the timer will start
    { 
      if(temperature >75 && finish ==0){              // turns off the heat lamp if it reaches 80 celsius.
        digitalWrite(LED_HEAT, LOW);
        digitalWrite(HEATRELAY_PIN, HIGH);
        }
        else
        {
          digitalWrite(LED_HEAT, HIGH);
          digitalWrite(HEATRELAY_PIN, HIGH);
          }
          
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp in C: ");
    lcd.print(temperature);
    lcd.setCursor(0,1);
    lcd.print("UV index: 11");
    lcd.print(sensorValue);
    lcd.setCursor(0,2);
    digitalWrite(UVRELAY_PIN, LOW);
    digitalWrite(LED_UV, HIGH);
    digitalWrite(LED_START, HIGH);
    timer();
  
      if(countdown_sec <1 && countdown_minute<1){     //ALARM RINGS AFTER TIMER RUNS OUT
      finish++;
      lcd.clear();
      lcd.setCursor(5 , 1);
      lcd.print("FINISHED");
      lcd.setCursor(3 , 2);
      lcd.print("DISINFECTING");
      tone( 6, 2000, 500);
      delay(2000);
      tone( 6, 2000, 500);
      delay(2000);
      digitalWrite(UVRELAY_PIN, HIGH);
      digitalWrite(HEATRELAY_PIN, HIGH);
      digitalWrite(LED_HEAT, LOW);
      digitalWrite(LED_UV, LOW);
      digitalWrite(LED_START, LOW); 
      digitalWrite(LED_STOP, HIGH);                              
    }

}
}}
