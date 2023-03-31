#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Ultrasonic.h>

int status=0;
RTC_DS3231 rtc;

void setup() {
  Serial.begin(9600);
  rtc.begin();
  pinMode(4, INPUT);
  pinMode(13, OUTPUT);
  status=1;
  show_msg(0);
  delay(500);
  update_time();
  latest_time();
}

void loop() {
  touch_sensor();
  water_detect();
  humidity();
}

void message(int message_code) {
  //This function can get all message, include error message.
  if(message_code == -1){
    status=0;
  }else if (message_code == 1) {
    show_msg(1);
  }else if (message_code == 2) {
    show_msg(2);
  }else if (message_code == 3) {
    latest_time();
    show_msg(3);
  }
}

int water_detect_status_pointer=0;
bool water_detect_status[5]={};
void water_detect() {
  //to detect water
  int distance;
  Ultrasonic ultrasonic(2, 3);
  distance = ultrasonic.read(); 
  if(distance >= 8){
    water_detect_status[water_detect_status_pointer%5]=1;
  }else {
    water_detect_status[water_detect_status_pointer%5]=0;
  }
  if(water_detect_status[0] && water_detect_status[1] && water_detect_status[2] && water_detect_status[3] && water_detect_status[4]) {
    message(1);
  }else {
    message(2);
  }
  if(water_detect_status_pointer == 100000) water_detect_status_pointer=0;
  water_detect_status_pointer++;
  //pin Trig->2 Echo->3
}

void touch_sensor() {
  //to detect touch event
  if(digitalRead(4) == HIGH){
    message(3);
  }
  // pin SIG->4
}

void give_water() {
  //5V to motor
  analogWrite(13 ,1023);
  delay(500);
  analogWrite(13 ,0);
  delay(2000);
}
int humidity_status_pointer=0;
bool humidity_status[5]={};
void humidity() {
  //to detect humidity of plant
  int hum=analogRead(A0);
  hum-=260;
  if(hum >= 260) {
    humidity_status[humidity_status_pointer%5]=1;
  }else {
    humidity_status[humidity_status_pointer%5]=0;
  }
  if(humidity_status[0] && humidity_status[1] && humidity_status[2] && humidity_status[3] && humidity_status[4]) {
    give_water();
    update_time();
  }
  if (humidity_status_pointer == 100000) humidity_status_pointer=0;
  humidity_status_pointer++;
  // pin A->A1
}

int uy,um,ud,uh,umin;
void update_time() {
  //get the watered time.
  DateTime now = rtc.now();
  uy=now.year();
  um=now.month();
  ud=now.day();
  uh=now.hour();
  umin=now.minute();
  // pin SDA->A4 SCL->A5
}

int ly,lm,ld,lh,lmin;
void latest_time() {
  //get the latest time.
  DateTime now = rtc.now();
  ly=now.year();
  lm=now.month();
  ld=now.day();
  lh=now.hour();
  lmin=now.minute();
}

int previous_message_code = -1;
void show_msg(int message_code) {
  //control LCD to show messages.
  LiquidCrystal_I2C lcd(0x27, 16, 2);
  lcd.backlight();
  if(message_code == 0) {
    //make sure arduino is ready.
    lcd.init();
    if(status) {
      lcd.setCursor(4, 0);
      lcd.print("success!");
    }else {
      lcd.setCursor(0, 0);
      lcd.print("!!unknow error!!");
    }
  }else if(message_code == 1) {
    //water_detect's message -> no water
    lcd.init();
    lcd.setCursor(3, 0);
    lcd.print("no water!!");
  }else if(message_code == 2 && previous_message_code == 1) {
    //water_detect's message -> enough water
    lcd.init();
  }else if(message_code == 3) {
    //touch_sensor's message -> touching
    lcd.init();
    lcd.setCursor(0, 0);
    lcd.print("Watered ");
    lcd.setCursor(9, 0);
    lcd.print((lmin-umin+(lh-uh)*60+(ld-ud)*1440));
    lcd.setCursor(12, 0);
    lcd.print("mins");
    lcd.setCursor(0, 1);
    lcd.print("ago.");
  }
  previous_message_code = message_code;
  // pin SDA->A4  SCL->A5
}
