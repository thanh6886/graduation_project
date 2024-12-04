#define RX1 14
#define TX1 15


HardwareSerial mySerial(1);
void setup() {
    mySerial.begin(9600, SERIAL_8N1, RX1, TX1); 
    Serial.begin(9600);
    Serial.println("setup done");
}

void loop() {
    mySerial.print("hello");
     mySerial.print("\n");
    Serial.println(".");
    delay(1000);
}
