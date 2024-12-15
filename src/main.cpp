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

SoftwareSerial RS485Serial(RS485_RXD_PIN, RS485_TXD_PIN); 

DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2); 

bool motor3Enabled = false; 

void setup() {

    Serial.begin(115200);
    Serial.println("System test");

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

    pinMode(RS485_DE_PIN, OUTPUT);
    digitalWrite(RS485_DE_PIN, LOW); 
 RS485Serial.begin(9600);
   Serial.println("RS485 Initialized");
    // Konfiguracja UART1
  //  RS485Serial.begin(9600, SERIAL_8N1, RS485_RXD_PIN, RS485_TXD_PIN);

    Serial.println("PWM dla silników zainicjalizowane.");
}

void loop() {
      if (Serial.available()) {
        String command = Serial.readStringUntil('\n'); 
        command.trim(); 

        if (command == "START_FAN") {
            motor3Enabled = true; 
            Serial.println("Komenda: START_FAN - Wentylator 3 włączony.");
        } else if (command == "STOP_FAN") {
            motor3Enabled = false; 
            Serial.println("Komenda: STOP_FAN - Wentylator 3 wyłączony.");
        } else {
            Serial.println("Nieznana komenda: " + command);
        }
    }
   
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
        if (temperature >= 21 && temperature <= 24) {
            pwmValue = map(temperature, 21, 30, 0, 150);
        } else if (temperature >= 25 && temperature <= 30) {
            pwmValue = map(temperature, 25, 30, 238, 248);
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

        Serial.print("Temperatura: ");
        Serial.print(temperature);
        Serial.print(" *C, PWM: ");
        Serial.println(pwmValue);
    }

    digitalWrite(RS485_DE_PIN, HIGH);
    delay(10); 

    String message = "Temperatura: " + String(temperature, 2) + " C\n";
    RS485Serial.print(message);

digitalWrite(RS485_DE_PIN, LOW); 

    delay(1000); 
}
