#include <AltSoftSerial.h>

#define PACKAGE_LENGTH 20
#define DEBUG

// AltSoftSerial always uses these pins:
//
// Board          Transmit  Receive   PWM Unusable
// -----          --------  -------   ------------
// Arduino Uno        9         8         10

// Arduino Uno Pin 9 <==> Bluetooth 4.0 UART CC2541 HM-10 RX
// Arduino Uno Pin 8 <==> Bluetooth 4.0 UART CC2541 HM-10 TX
// Arduino Uno Pin VCC <==> Bluetooth 4.0 UART CC2541 HM-10 VCC
// Arduino Uno Pin GND <==> Bluetooth 4.0 UART CC2541 HM-10 GND

AltSoftSerial hmSerial;
int en_pin = 10;
char buf[PACKAGE_LENGTH];
bool receive = false;
int ledState = LOW;
int test = 0;
int analogPin = 0;
float sensorVolt = 0.0f;
float sensorVoltPre = 0.0f;
float RS; 
float R0;
int R2 = 481;  // use multimeter to measure
float ratio;
float BAC;
float Vin = 4.0;

void setup() {
  // put your setup code here, to run once:
  hmSerial.begin(19200);
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);

  calibration();

}

void loop() {
  if (Serial.available()) {
    String data = Serial.readString();
    sendCommand(data);
  }
  
  readSerialHM();

  alcoholDataSend();

  delay(30);
}

void alcoholDataSend(){
  int sensorValue = 0;
  for(int i=0; i < 10; i++){
    sensorValue += analogRead(analogPin);
  }
  sensorVolt = ((float)sensorValue/10)/1024*Vin;
  sensorValue /= 10;

  RS = ((Vin * R2)/sensorVolt) - R2;
  ratio = RS/R0;// ratio = RS/R0

  String data = String(sensorValue);
  data += '\n';
  sendCommand(data);
}

void calibration(){
  int sensorValue = analogRead(A0);
  for(int i=0; i < 1000; i++){
    sensorVolt +=(float)sensorValue/1024*Vin;
  }
  sensorVolt /= 1000.0f;
  RS = ((Vin * R2)/sensorVolt) - R2;
  R0 = RS / 60;
  Serial.print("R0: ");
  Serial.println(R0);
}

void readSerialHM(){
  String reply;
  String strX;
  String strW;
  String strY;
  String strH;
  while (hmSerial.available()) {
    reply = hmSerial.readStringUntil('\n');
  }

#ifdef DEBUG
  if(reply.length() > 0){
    if(ledState==LOW){
      digitalWrite(LED_BUILTIN, HIGH);
      ledState = HIGH;
    }else{
      digitalWrite(LED_BUILTIN, LOW);
      ledState = LOW;
    }
    receive = true;
    Serial.println(reply);
  }
#endif
}

void sendCommand(String command){
  if (command.length() > PACKAGE_LENGTH){
    return;
  }
  // Serial.print("sendcommand: ");
  // Serial.println(command);
  // Serial.print("sendcommand length ");
  // Serial.println(command.length());
  command.toCharArray(buf,command.length());
  buf[command.length()]='\r';
  buf[command.length()+1]='\n';
  hmSerial.println(buf);
  receive = false;
  for(int i=0 ; i < 20; i++){
      buf[i] = '\0';
  }
}

