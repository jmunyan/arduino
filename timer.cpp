#include <LiquidCrystal_I2C.h>

// LCD setup (address 0x27, 20x2 display)
LiquidCrystal_I2C lcd(0x27, 20, 2);

// Button pins
const int BUTTON_FIVE_MIN = 2;
const int BUTTON_MIN = 3;
const int BUTTON_HOUR = 4;
const int BUTTON_RESET = 5;
const int BUTTON_START = 6;
const int DOOR_OPEN = 7;

// Relay pin for buzzer
const int RELAY_BUZZER = 8;

// Timer variables
unsigned long hours = 0;
unsigned long minutes = 0;
unsigned long seconds = 0;
unsigned long lastMillis = 0;
bool timerRunning = false;
bool buzzerActive = false;

void setup() {
    // Initialize LCD
    lcd.init();
    lcd.backlight();
    lcd.print("Timer Ready");
    
    // Setup button pins
    pinMode(BUTTON_FIVE_MIN, INPUT_PULLUP);
    pinMode(BUTTON_MIN, INPUT_PULLUP);
    pinMode(BUTTON_HOUR, INPUT_PULLUP);
    pinMode(BUTTON_RESET, INPUT_PULLUP);
    pinMode(BUTTON_START, INPUT_PULLUP);
    pinMode(DOOR_OPEN, INPUT_PULLUP);
    
    // Setup relay pin for buzzer
    pinMode(RELAY_BUZZER, OUTPUT);
    digitalWrite(RELAY_BUZZER, LOW);  // Buzzer off initially
    
    lastMillis = millis();
    displayTime();
}

void loop() {
    checkButtons();
    
    if (timerRunning) {
        updateTimer();
    }
    
    displayTime();
    delay(50);
}

void checkButtons() {
    if (digitalRead(BUTTON_FIVE_MIN) == LOW) {
      minutes += 5;
      if (minutes >= 60) {
        minutes = 0;
        hours += 1;
      }
      delay(50);
    }
    
    if (digitalRead(BUTTON_MIN) == LOW) {
      minutes++;
      if (minutes >= 60) {
        minutes = 0;
        hours += 1;
      }
      delay(50);
    }
    
    if (digitalRead(BUTTON_HOUR) == LOW) {
      hours++;
      while (digitalRead(BUTTON_HOUR) == LOW);
      delay(20);
    }
    
    if (digitalRead(BUTTON_RESET) == LOW) {
        delay(20);
        if (digitalRead(BUTTON_RESET) == LOW) {
            timerRunning = false;
            hours = minutes = seconds = 0;
            while (digitalRead(BUTTON_RESET) == LOW);
            delay(20);
        }
    }
    
    if (digitalRead(BUTTON_START) == LOW) {
        delay(20);
        if (digitalRead(BUTTON_START) == LOW) {
            timerRunning = !timerRunning;
            lastMillis = millis();
            while (digitalRead(BUTTON_START) == LOW);
            delay(20);
        }
    }
    
    if (digitalRead(DOOR_OPEN) == LOW) {
        delay(5000);
        if (digitalRead(DOOR_OPEN) == LOW) {
            buzzerActive = false;
            digitalWrite(RELAY_BUZZER, LOW);
            delay(20);
        }
    }
}

void updateTimer() {
    unsigned long currentMillis = millis();
    unsigned long elapsed = currentMillis - lastMillis;
    
    if (elapsed >= 1000) {
        if (seconds > 0) {
            seconds--;
        } else if (minutes > 0) {
            minutes--;
            seconds = 59;
        } else if (hours > 0) {
            hours--;
            minutes = 59;
            seconds = 59;
        } else {
            timerRunning = false;
            buzzerActive = true;
            digitalWrite(RELAY_BUZZER, HIGH);  // Turn on buzzer relay
        }
        lastMillis = currentMillis;
    }
}

void displayTime() {
    lcd.setCursor(0, 0);
    lcd.print("Timer: ");
    
    if (hours < 10) lcd.print("0");
    lcd.print(hours);
    lcd.print(":");
    if (minutes < 10) lcd.print("0");
    lcd.print(minutes);
    lcd.print(":");
    if (seconds < 10) lcd.print("0");
    lcd.print(seconds);
    
    lcd.setCursor(0, 1);
    if (buzzerActive) {
        lcd.print("PULL RACK");
        lcd.setCursor(0,2);
        lcd.print("BAKING DONE");
    } else {
        lcd.print(timerRunning ? "RUNNING  " : "STOPPED  ");
        lcd.setCursor(0, 2);
        lcd.print("                    ");
    }
}