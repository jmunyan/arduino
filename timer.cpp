#include <LiquidCrystal_I2C.h>

// LCD setup (address 0x27, 20x4 display)
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Button pins
const int BUTTON_FIVE_MIN = 2;
const int BUTTON_MIN = 3;
const int BUTTON_HOUR = 4;
const int BUTTON_RESET = 5;
const int BUTTON_START = 6;
const int DOOR_OPEN = 7;
const int BUTTON_SETTINGS = 9;

// Relay pin for buzzer
const int RELAY_BUZZER = 8;

// Timer variables
unsigned long hours = 0;
unsigned long minutes = 0;
unsigned long seconds = 0;
unsigned long lastMillis = 0;
bool timerRunning = false;
bool buzzerActive = false;
unsigned long shutoffTime = 5; // Buzzer shutoff time in seconds
bool settingsMode = false;

/**
 * Initializes LCD, button pins, relay pin, and displays initial timer state.
 */
void setup() {
    // Initialize LCD
    lcd.init();
    lcd.backlight();
    lcd.print("Timer Ready");

    // Setup button pins as input with pull-up resistors
    pinMode(BUTTON_FIVE_MIN, INPUT_PULLUP);
    pinMode(BUTTON_MIN, INPUT_PULLUP);
    pinMode(BUTTON_HOUR, INPUT_PULLUP);
    pinMode(BUTTON_RESET, INPUT_PULLUP);
    pinMode(BUTTON_START, INPUT_PULLUP);
    pinMode(DOOR_OPEN, INPUT_PULLUP);
    pinMode(BUTTON_SETTINGS, INPUT_PULLUP);

    // Setup relay pin for buzzer
    pinMode(RELAY_BUZZER, OUTPUT);
    digitalWrite(RELAY_BUZZER, LOW);  // Buzzer off initially

    lastMillis = millis();
    displayTime();
}

/**
 * Main loop - continuously checks buttons and updates timer display.
 */
void loop() {
    checkButtons();

    if (timerRunning) {
        updateTimer();
    }

    displayTime();
    delay(50);
}

/**
 * Handles all button input and adjusts timer or settings accordingly.
 * Includes debouncing for buttons.
 */
void checkButtons() {
    // Add 5 minutes
    if (digitalRead(BUTTON_FIVE_MIN) == LOW) {
        minutes += 5;
        if (minutes >= 60) {
            minutes -= 60;
            hours += 1;
        }
        delay(50);  // Simple debouncing
    }

    // Add 1 minute (only if not in settings mode)
    if (digitalRead(BUTTON_MIN) == LOW && !settingsMode) {
        minutes++;
        if (minutes >= 60) {
            minutes = 0;
            hours += 1;
        }
        delay(50);  // Simple debouncing
    }

    // Add 1 hour
    if (digitalRead(BUTTON_HOUR) == LOW) {
        hours++;
        while (digitalRead(BUTTON_HOUR) == LOW);  // Wait for release
        delay(20);
    }

    // Reset timer
    if (digitalRead(BUTTON_RESET) == LOW) {
        delay(20);
        if (digitalRead(BUTTON_RESET) == LOW) {
            timerRunning = false;
            hours = minutes = seconds = 0;
            while (digitalRead(BUTTON_RESET) == LOW);  // Wait for release
            delay(20);
        }
    }

    // Start/Stop timer
    if (digitalRead(BUTTON_START) == LOW) {
        delay(20);
        if (digitalRead(BUTTON_START) == LOW) {
            if (settingsMode) {
                settingsMode = false;
            }
            timerRunning = !timerRunning;
            lastMillis = millis();
            while (digitalRead(BUTTON_START) == LOW);  // Wait for release
            delay(20);
        }
    }

    // Door open sensor - turn off buzzer after shutoff time
    if (digitalRead(DOOR_OPEN) == LOW) {
        delay(shutoffTime * 1000);  // Wait for the shutoff time
        if (digitalRead(DOOR_OPEN) == LOW) {
            buzzerActive = false;
            digitalWrite(RELAY_BUZZER, LOW);
            delay(20);
        }
    }

    // Enter settings mode
    if (digitalRead(BUTTON_SETTINGS) == LOW) {
        settingsMode = true;
        lcd.clear();
        while (digitalRead(BUTTON_SETTINGS) == LOW) {
            settings();
            // Adjust shutoff time with minute button
            if (digitalRead(BUTTON_MIN) == LOW) {
                shutoffTime++;
                if (shutoffTime >= 15) shutoffTime = 1; // Wrap around to 1 second after 15 seconds
                while (digitalRead(BUTTON_MIN) == LOW);
                delay(20);
            }
        }
        settingsMode = false;
        lcd.clear();
    }
}

/**
 * Decrements the timer by one second each time a full second has elapsed.
 * If timer reaches zero, stops the timer and activates the buzzer.
 */
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
            // Timer finished
            timerRunning = false;
            buzzerActive = true;
            digitalWrite(RELAY_BUZZER, HIGH);  // Turn on buzzer relay
        }
        lastMillis = currentMillis;
    }
}

/**
 * Updates the LCD display with current timer status or buzzer message.
 */
void displayTime() {
    lcd.setCursor(0, 0);
    lcd.print("Timer: ");

    // Format time as HH:MM:SS with leading zeros
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
        lcd.setCursor(0, 2);
        lcd.print("BAKING DONE");
    } else {
        lcd.print(timerRunning ? "RUNNING  " : "STOPPED  ");
        lcd.setCursor(0, 2);
        lcd.print("                    ");  // Clear line
    }
}

/**
 * Displays settings screen for adjusting buzzer shutoff time.
 */
void settings() {
    lcd.setCursor(0, 0);
    lcd.print("BUZZER SHUTOFF GAP");
    lcd.setCursor(0, 1);
    lcd.print(shutoffTime);
    lcd.print(" SECONDS ");
    lcd.setCursor(0, 2);
    lcd.print("ADJUST W/ MIN BTN");
}