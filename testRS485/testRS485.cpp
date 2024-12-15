#include <SoftwareSerial.h>

#define RS485_TXD_PIN 17 
#define RS485_RXD_PIN 16  
#define RS485_DE_PIN 22   

SoftwareSerial RS485Serial(RS485_RXD_PIN, RS485_TXD_PIN);

void setup() {
    Serial.begin(115200);
    Serial.println("RS485 Test Start");
    pinMode(RS485_DE_PIN, OUTPUT);
    digitalWrite(RS485_DE_PIN, LOW); 
    RS485Serial.begin(9600);
    Serial.println("RS485 Initialized");
}

void loop() {
    
    Serial.println("Wysyłanie wiadomości przez RS485...");
    digitalWrite(RS485_DE_PIN, HIGH);
    delay(50); 
    RS485Serial.println("Testowa wiadomość przez RS485"); // Odbior w puttim
    delay(50); 
    digitalWrite(RS485_DE_PIN, LOW);
    delay(1000); 

}
