#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

#define I2C_SDA 26
#define I2C_SCL 25
#define DHT_PIN 13
#define DHT_TYPE DHT11  
#define MOTOR1_IN1 12  
#define MOTOR1_IN2 14  
#define MOTOR2_IN1 33  
#define MOTOR2_IN2 32 
#define MOTOR3_IN1 4  
#define MOTOR3_IN2 5  
#define RS485_TXD_PIN 17  
#define RS485_RXD_PIN 16  
#define RS485_DE_PIN 22    

// Używamy RS485 w trybie półdupleksowym
SoftwareSerial rs485(RS485_RXD_PIN, RS485_TXD_PIN); // RX, TX dla komunikacji

DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2); 

bool motor3Enabled = false; 

volatile bool updateTemperature = false;  
hw_timer_t *timer = NULL;                 

// Funkcja obsługi przerwania timera
void IRAM_ATTR onTimer() {
    updateTemperature = true;  
}

void printRS485(String message) {
    digitalWrite(RS485_DE_PIN, HIGH);  // Ustawiamy RS485 w tryb nadawania
    delay(10);  

    rs485.println(message);  

    delay(10);  
    digitalWrite(RS485_DE_PIN, LOW);  // Ustawiamy RS485 w tryb odbioru
}

void printRS485(float value) {
    digitalWrite(RS485_DE_PIN, HIGH);  // Ustawiamy RS485 w tryb nadawania
    delay(10);

    rs485.println(value);  

    delay(10);
    digitalWrite(RS485_DE_PIN, LOW);  
}

void setup() {
    pinMode(RS485_DE_PIN, OUTPUT);
    digitalWrite(RS485_DE_PIN, LOW); 

    rs485.begin(9600); 

    Serial.begin(9600); 

    Wire.begin(I2C_SDA, I2C_SCL);
    Serial.println("I2C test");

    dht.begin();
    Serial.println("Czujnik test");

    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Inicjalizacja...");
    delay(1000);
    lcd.clear();

    ledcSetup(0, 5000, 8); 
    ledcAttachPin(MOTOR1_IN1, 0);
    ledcAttachPin(MOTOR1_IN2, 1);

    ledcSetup(2, 5000, 8);
    ledcAttachPin(MOTOR2_IN1, 2);
    ledcAttachPin(MOTOR2_IN2, 3);

    ledcSetup(4, 5000, 8); 
    ledcAttachPin(MOTOR3_IN1, 4);
    ledcAttachPin(MOTOR3_IN2, 5);

    Serial.println("RS485 Initialized");
    Serial.println("PWM dla silników zainicjalizowane.");
    delay(10);


    timer = timerBegin(0, 80, true);  
    timerAttachInterrupt(timer, &onTimer, true);  
    timerAlarmWrite(timer, 1000000, true);        
    timerAlarmEnable(timer);                     
}

void loop() {

    if (rs485.available()) {
        String command = rs485.readStringUntil('\n'); 
        command.trim(); 

        // Obsługa komend RS485
        if (command == "START_FAN") {
            motor3Enabled = true; 
            printRS485("Komenda: START_FAN - Wentylator 3 włączony.");
        } else if (command == "STOP_FAN") {
            motor3Enabled = false;  
            printRS485("Komenda: STOP_FAN - Wentylator 3 wyłączony.");
        } else {
            printRS485("Nieznana komenda: " + command);
        }
    }

    if (updateTemperature) {
        updateTemperature = false;  

    // Odczyt temperatury z czujnika DHT11
    float temperature = dht.readTemperature();
    if (isnan(temperature)) {
        Serial.println("Błąd odczytu z czujnika DHT");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Blad odczytu");
    } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Temp: ");
        lcd.print(temperature);
        lcd.print(" C");

        int pwmValue = 0;
        if (temperature >= 20 && temperature <= 30) {
            pwmValue = map(temperature, 20, 30, 238, 255);
        } else if (temperature >= 31) {
            pwmValue = 255; 
        } else {
            pwmValue = 0; 
        }

        
        ledcWrite(0, pwmValue);
        ledcWrite(1, 0);
        ledcWrite(2, pwmValue);
        ledcWrite(3, 0);

        if (motor3Enabled) {
            ledcWrite(4, pwmValue); 
            ledcWrite(5, 0);   
        } else {
            ledcWrite(4, 0);
            ledcWrite(5, 0);
        }

        
        printRS485("Temperatura: ");
        printRS485(temperature);
        printRS485(" *C, PWM: ");
        printRS485(pwmValue);
    }
    }
}
