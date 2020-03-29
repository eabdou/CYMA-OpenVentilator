/**************************************************** 
 * sketch = CYMA-OpenVentilator
 *
 * CYMA DIGITAL
 *
 * Controlador = Arduino Mega 2560
 * 
 * 
 ****************************************************/

/*  
 History
  000 - Started 26/03/2020

 Known issues:
  
 Future changes:
  
 ToDo:
   1) Validate pressure reading
  
 Authors:
 Eduardo Abdou    eduardo@abdou.com.br

 */

#include <LiquidCrystal.h>

//lcd Initialization
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

int lcd_key     = 0;
int adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

/*
  STATE MACHINE
  States
    1   - WAITING
    12  - START
    2   - RUNNING
    21  - STOP
*/

//  Pins
#define MOTOR_PIN   22
#define SWITCH_PIN  24 
#define PRESS_PIN   A8

//current state
int gState = 1;

//switch delay time
int gDelaySwitch = 1000;
int gDelayPressDisplay = 400;
int gDisplayMode = 0;

//panel flags
bool btnPressed = false;

//pressure var
float gPressure = 0;

int gBpm = 0;
int gBpmSet = 0;

float gDelayActivation = 0;
float gLastActivation = 0;
float gLastPressureDisplay = 0;

void setup() {
  
  //pins
  pinMode(MOTOR_PIN, OUTPUT);
  
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  //pinMode(PRESS_PIN, INPUT);

  //start lcd
  lcd.begin(16, 2);              // start the library
  
//  lcd.setCursor(0,0);
//  lcd.print("CYMA DIGITAL"); // print a simple message
//  lcd.setCursor(0,1);
//  lcd.print("Open Ventilator"); // print a simple message

  //temp
  gState = 1;
  gBpm = 12;
  gBpmSet = 12;

  Serial.begin(9600);
  delay(500);
}

void loop() {

  //screen mode
  monitorScreen();

  //read buttons
  readButtons();

  readBpm();

  readPressure();
  
  stateMachine();

  checkAlarms();  
}

void stateMachine() {

  switch (gState)
  {
    case 1:
      waitingMode();
      break;

    case 12:
      startMode();
      break;

    case 2:
      runningMode();
      break;

    case 21:
      stopMode();
      break;
  }
}

void waitingMode() {
  motorOff();

  if (millis() >= gLastActivation + gDelayActivation){
    //deu o tempo
    gLastActivation = millis();
    gState = 12;

    Serial.println("ENTERING START STATE");
  }
}

void startMode() {
  motorOn();

  if (digitalRead(SWITCH_PIN) == HIGH) {
    gState = 2;

    Serial.println("ENTERING RUNNING STATE");
  }
}

void runningMode() {
  motorOn();

  if (digitalRead(SWITCH_PIN) == LOW) {
    gState = 21;

    Serial.println("ENTERING STOP STATE");
  }
}

void stopMode() {
  motorOff();
  gState = 1;

  Serial.println("ENTERING WAITING STATE");
}

void motorOn(){
  digitalWrite(MOTOR_PIN, LOW);
}

void motorOff(){
  digitalWrite(MOTOR_PIN, HIGH  );
}

void readBpm(){

  if (Serial.available() > 0){
    int b = Serial.parseInt(); 
    if (b > 0) gBpm = b;
    Serial.println(gBpm);
  }
  
  //get bpm Input  
  gDelayActivation = 60 / gBpm * 1000; 
}

void readPressure(){

  int rawADC = analogRead(PRESS_PIN);
  float volt = (float) rawADC * 5.0 / 1024.0;
  float kpa = (volt - 0.2) / 4.5 * 700.0;   // minus offset, divide by voltage range, multiply with pressure range.

  gPressure = kpa;// * 10.2; //kpa -> mbar
  Serial.println(gPressure);
}

void checkAlarms(){

  if (millis() > gLastActivation + (gDelayActivation) + 500)
    Serial.println("ALARM !");
  
}

void monitorScreen(){
   
  lcd.setCursor(0,0);
  lcd.print("Bpm"); 
  
  if (gBpm != gBpmSet)
    lcd.print("*"); 
  else
    lcd.print(" ");
 
  lcd.setCursor(0,1);
  lcd.print(gBpmSet); 
  lcd.print("  "); 


  lcd.setCursor(5,0);
  lcd.print("Press"); 

  if (millis() >= gLastPressureDisplay + gDelayPressDisplay ) {
    gLastPressureDisplay = millis();
    lcd.setCursor(5,1);
    lcd.print(gPressure); 
    lcd.print("   "); 
  }

  lcd.setCursor(11,0);
  lcd.print("State");

  lcd.setCursor(11,1);
  lcd.print(gState); 
  lcd.print("   ");
}

int readButtons(){

  lcd.setCursor(0,1);            // move to the begining of the second line
  lcd_key = read_LCD_buttons();  // read the buttons

  switch (lcd_key)               // depending on which button was pushed, we perform an action
  {
    case btnRIGHT:
    {
      //Serial.println("RIGHT ");
      break;
    }
    case btnLEFT:
    {
      //Serial.println("LEFT   ");
      break;
    }
    case btnUP:
    {
      if (btnPressed == false){
        btnPressed = true;
        //Serial.println("UP    ");
        gBpmSet += 1;
      }
      break;
    }
    case btnDOWN:
    {
      if (btnPressed == false){
        btnPressed = true;
        //Serial.println("DOWN    ");
        gBpmSet -= 1;
      }
      break;
    }
    case btnSELECT:
    {
      if (btnPressed == false){
        btnPressed = true;
        //Serial.println("SELECT    ");

        gBpm = gBpmSet;

      }
      break;
    }
    case btnNONE:
    {
      btnPressed = false;
      //Serial.println("NONE  ");
      break;
    }
  }


}

int read_LCD_buttons()
{
  adc_key_in = analogRead(0);     
  
  if (adc_key_in > 1000) return btnNONE;

  if (adc_key_in < 50)   return btnRIGHT;
  if (adc_key_in < 250)  return btnUP;
  if (adc_key_in < 450)  return btnDOWN;
  if (adc_key_in < 650)  return btnLEFT;
  if (adc_key_in < 850)  return btnSELECT;

  return btnNONE;  
}
