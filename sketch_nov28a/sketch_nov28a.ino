#define RXD2 16
#define TXD2 17


void setup(){
   Serial.begin(115200);
   Serial2.begin(4800, SERIAL_8N1, RXD2, TXD2);
   Serial.println("setup done");
}
void loop(){
     if(Serial2.available() > 0) {
        String receivedStauts = Serial2.readStringUntil('\n');
         if(receivedStauts == "A"){
          receivedStauts = "";
          Serial.println("true");
         }
       }
}
