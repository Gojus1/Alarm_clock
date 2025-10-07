#include <LiquidCrystal.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const int BUZZER_PIN = 8;
const int STOP_BUTTON_PIN = 7;
const int SET_BUTTON_PIN = 6;

#define EEPROM_MAGIC 0xAA
#define EEPROM_VERSION 1
struct PersistentData {
  uint8_t magic;
  uint8_t version;
  uint8_t alarmHour;
  uint8_t alarmMinute;
  bool alarmEnabled;
};

int hours = 11, minutes = 11, seconds = 55;
int alarmHour = 11;
int alarmMinute = 12;
bool alarmOn = true;
bool alarmTriggered = false;

unsigned long prevMillis = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastButtonCheck = 0;

enum AppState {
  STATE_NORMAL,
  STATE_SET_ALARM_HOUR,
  STATE_SET_ALARM_MINUTE,
  STATE_ALARM_TRIGGERED
};
AppState currentState = STATE_NORMAL;

bool lastStopButtonState = HIGH;
bool lastSetButtonState = HIGH;
unsigned long lastDebounceTime = 0;

bool playing = false;
int currentNote = 0;
unsigned long noteStartMillis = 0;

#define NOTE_B0 31
#define NOTE_C1 33
#define NOTE_CS1 35
#define NOTE_D1 37
#define NOTE_DS1 39
#define NOTE_E1 41
#define NOTE_F1 44
#define NOTE_FS1 46
#define NOTE_G1 49
#define NOTE_GS1 52
#define NOTE_A1 55
#define NOTE_AS1 58
#define NOTE_B1 62
#define NOTE_C2 65
#define NOTE_CS2 69
#define NOTE_D2 73
#define NOTE_DS2 78
#define NOTE_E2 82
#define NOTE_F2 87
#define NOTE_FS2 93
#define NOTE_G2 98
#define NOTE_GS2 104
#define NOTE_A2 110
#define NOTE_AS2 117
#define NOTE_B2 123
#define NOTE_C3 131
#define NOTE_CS3 139
#define NOTE_D3 147
#define NOTE_DS3 156
#define NOTE_E3 165
#define NOTE_F3 175
#define NOTE_FS3 185
#define NOTE_G3 196
#define NOTE_GS3 208
#define NOTE_A3 220
#define NOTE_AS3 233
#define NOTE_B3 247
#define NOTE_C4 262
#define NOTE_CS4 277
#define NOTE_D4 294
#define NOTE_DS4 311
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_FS4 370
#define NOTE_G4 392
#define NOTE_GS4 415
#define NOTE_A4 440
#define NOTE_AS4 466
#define NOTE_B4 494
#define NOTE_C5 523
#define NOTE_CS5 554
#define NOTE_D5 587
#define NOTE_DS5 622
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_FS5 740
#define NOTE_G5 784
#define NOTE_GS5 831
#define NOTE_A5 880
#define NOTE_AS5 932
#define NOTE_B5 988
#define NOTE_C6 1047
#define NOTE_CS6 1109
#define NOTE_D6 1175
#define NOTE_DS6 1245
#define NOTE_E6 1319
#define NOTE_F6 1397
#define NOTE_FS6 1480
#define NOTE_G6 1568
#define NOTE_GS6 1661
#define NOTE_A6 1760
#define NOTE_AS6 1865
#define NOTE_B6 1976
#define NOTE_C7 2093
#define NOTE_CS7 2217
#define NOTE_D7 2349
#define NOTE_DS7 2489
#define NOTE_E7 2637
#define NOTE_F7 2794
#define NOTE_FS7 2960
#define NOTE_G7 3136
#define NOTE_GS7 3322
#define NOTE_A7 3520
#define NOTE_AS7 3729
#define NOTE_B7 3951
#define NOTE_C8 4186
#define NOTE_CS8 4435
#define NOTE_D8 4699
#define NOTE_DS8 4978
#define REST 0

int tempo = 140;

int melody[] = {
  NOTE_FS5,8, NOTE_FS5,8, NOTE_D5,8, NOTE_B4,8, REST,8, NOTE_B4,8, REST,8, NOTE_E5,8,
  REST,8, NOTE_E5,8, REST,8, NOTE_E5,8, NOTE_GS5,8, NOTE_GS5,8, NOTE_A5,8, NOTE_B5,8,
  NOTE_A5,8, NOTE_A5,8, NOTE_A5,8, NOTE_E5,8, REST,8, NOTE_D5,8, REST,8, NOTE_FS5,8,
  REST,8, NOTE_FS5,8, REST,8, NOTE_FS5,8, NOTE_E5,8, NOTE_E5,8, NOTE_FS5,8, NOTE_E5,8,
};

int notes = sizeof(melody) / sizeof(melody[0]) / 2;

int wholenote = (60000 * 4) / tempo;

void saveAlarmSettings() {
  PersistentData data;
  data.magic = EEPROM_MAGIC;
  data.version = EEPROM_VERSION;
  data.alarmHour = alarmHour;
  data.alarmMinute = alarmMinute;
  data.alarmEnabled = alarmOn;
  
  EEPROM.put(0, data);
}

bool loadAlarmSettings() {
  PersistentData data;
  EEPROM.get(0, data);
  
  if (data.magic == EEPROM_MAGIC && data.version == EEPROM_VERSION) {
    alarmHour = data.alarmHour;
    alarmMinute = data.alarmMinute;
    alarmOn = data.alarmEnabled;
    return true;
  }
  return false;
}

void handleButtons() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastButtonCheck < 50) return;
  lastButtonCheck = currentMillis;

  bool currentStopButton = digitalRead(STOP_BUTTON_PIN);
  bool currentSetButton = digitalRead(SET_BUTTON_PIN);

  if (lastStopButtonState == HIGH && currentStopButton == LOW) {
    switch(currentState) {
      case STATE_NORMAL:
        if (playing) {
          playing = false;
          alarmTriggered = false;
          noTone(BUZZER_PIN);
        }
        break;
      case STATE_SET_ALARM_HOUR:
        currentState = STATE_SET_ALARM_MINUTE;
        break;
      case STATE_SET_ALARM_MINUTE:
        currentState = STATE_NORMAL;
        break;
      case STATE_ALARM_TRIGGERED:
        playing = false;
        alarmTriggered = false;
        currentState = STATE_NORMAL;
        noTone(BUZZER_PIN);
        break;
    }
  }

  if (lastSetButtonState == HIGH && currentSetButton == LOW) {
    switch(currentState) {
      case STATE_NORMAL:
        currentState = STATE_SET_ALARM_HOUR;
        break;
      case STATE_SET_ALARM_HOUR:
        alarmHour = (alarmHour + 1) % 24;
        saveAlarmSettings();
        break;
      case STATE_SET_ALARM_MINUTE:
        alarmMinute = (alarmMinute + 1) % 60;
        saveAlarmSettings();
        break;
    }
  }

  lastStopButtonState = currentStopButton;
  lastSetButtonState = currentSetButton;
}

void updateDisplays() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastDisplayUpdate >= 500) {
    lastDisplayUpdate = currentMillis;
    
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    
    switch(currentState) {
      case STATE_NORMAL:
        display.println("October 6");
        break;
      case STATE_SET_ALARM_HOUR:
        display.println("Set Hour:");
        display.setCursor(0, 16);
        display.print("  ");
        display.print(alarmHour);
        break;
      case STATE_SET_ALARM_MINUTE:
        display.println("Set Minute:");
        display.setCursor(0, 16);
        display.print("  ");
        display.print(alarmMinute);
        break;
      case STATE_ALARM_TRIGGERED:
        display.println("Good Morning Princess");
        break;
    }
    display.display();
  }

  static unsigned long lastLCDUpdate = 0;
  if (currentMillis - lastLCDUpdate >= 250) {
    lastLCDUpdate = currentMillis;
    
    lcd.setCursor(0, 0);
    lcd.print("Time: ");
    lcd.print(hours < 10 ? "0" : ""); lcd.print(hours);
    lcd.print(":");
    lcd.print(minutes < 10 ? "0" : ""); lcd.print(minutes);
    lcd.print(":");
    lcd.print(seconds < 10 ? "0" : ""); lcd.print(seconds);

    lcd.setCursor(0, 1);
    lcd.print("Alarm: ");
    if (alarmOn) {
      lcd.print(alarmHour < 10 ? "0" : ""); lcd.print(alarmHour);
      lcd.print(":");
      lcd.print(alarmMinute < 10 ? "0" : ""); lcd.print(alarmMinute);
    } else {
      lcd.print("OFF  ");
    }
  }
 }

void checkAlarm() {
  if (alarmOn && !playing && hours == alarmHour && minutes == alarmMinute && seconds == 0) {
    playing = true;
    alarmTriggered = true;
    currentState = STATE_ALARM_TRIGGERED;
    currentNote = 0;
    noteStartMillis = millis();
  }
}

void playAlarmSong() {
  if (!playing) return;
  
  unsigned long currentMillis = millis();
  int divider = melody[currentNote * 2 + 1];
  int noteDuration = (divider > 0) ? (wholenote / divider) : (wholenote / abs(divider)) * 1.5;

  if (currentMillis - noteStartMillis >= noteDuration) {
    noTone(BUZZER_PIN);
    currentNote++;
    noteStartMillis = currentMillis;

    if (currentNote >= notes) {
      currentNote = 0;
    }
  } else if (currentMillis - noteStartMillis < noteDuration * 0.9) {
    int note = melody[currentNote * 2];
    if (note != REST) tone(BUZZER_PIN, note);
  }
}

void setup() {
  Serial.begin(9600);
  
  // Initialize displays
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED initialization failed");
  }
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Alarm Clock");
  display.display();
  
  lcd.begin(16, 2);
  lcd.print("HELLO");

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SET_BUTTON_PIN, INPUT_PULLUP);

  if (!loadAlarmSettings()) {
    alarmHour = 11;
    alarmMinute = 12;
    alarmOn = true;
    saveAlarmSettings();
  }
  
  delay(1000);
  lcd.clear();
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - prevMillis >= 1000) {
    prevMillis = currentMillis;
    seconds++;
    if (seconds == 60) { seconds = 0; minutes++; }
    if (minutes == 60) { minutes = 0; hours++; }
    if (hours == 24) { hours = 0; }
    
    checkAlarm();
  }

  handleButtons();

  updateDisplays();

  if (alarmTriggered) {
    playAlarmSong();
  }
}
