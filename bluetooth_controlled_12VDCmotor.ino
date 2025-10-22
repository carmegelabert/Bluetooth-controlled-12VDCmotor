#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "esp_system.h"
#include "BTS7960.h"



BTS7960 motor(2, 1, 7, 6);

BLEServer *pServer;
BLECharacteristic *pCharacteristic;

const int SLOW = 50;
const int MEDIUM = 100;
const int FAST = 150;  // deixar marge seguretat
int lastSpeed = 0;

// Durades (en ms)
const unsigned long slowTime = 45*1000;// 1UL * 60UL * 1000UL;  //1min
const unsigned long mediumTime = 45*1000; //1UL * 60UL * 1000UL;
const unsigned long fastTime = 45*1000; //1UL * 60UL * 1000UL;
const unsigned long stopTime = 15*1000; //20UL * 1000UL;  //20s stop

// Estat general
enum State { AUTO,
             MANUAL };
State currentState = AUTO;

// AUTO mode sub-states
enum AutoStep {
  AUTO_SLOW_FWD,
  AUTO_MEDIUM_FWD,
  AUTO_FAST_FWD,
  STOP1,
  AUTO_SLOW_BACK,
  AUTO_MEDIUM_BACK,
  AUTO_FAST_BACK,
  STOP2
};

// default first mode: slow forward
AutoStep autoStep = AUTO_SLOW_FWD;
unsigned long stepStartMillis = 0;
unsigned long stepDuration = slowTime;



void SPEED(int speed) {

  if (speed == 0) {
    if(lastSpeed > 0){
      for (int aux = lastSpeed - 1; aux > 0; aux--) {
        motor.setSpeed(aux);
        delay(100);  // ajustar? en quant de temps volem decrementar velocitat motor?
      }
    }else if(lastSpeed < 0){
      for (int aux = lastSpeed + 1; aux < 0; aux++) {
        motor.setSpeed(aux);
        delay(100);
      }      
    }
  
  } else if (speed < lastSpeed) {
    for (int aux = lastSpeed - 1; aux >= speed; aux --) {
      motor.setSpeed(aux);
      delay(100);  // ajustar? en quant de temps volem decrementar velocitat motor?
    }

  } else if (speed > lastSpeed) {
    for (int aux = lastSpeed + 1; aux <= speed; aux ++) {
      motor.setSpeed(aux);
      delay(100);
    }
  }

  ////Serial.print("previous motor speed: ");
  ////Serial.println(lastSpeed);
  ////Serial.print("current motor speed: ");
  ////Serial.println(speed);
  lastSpeed = speed;
}


void nextAutoStep() {
  switch (autoStep) {
    case AUTO_SLOW_FWD: startAutoStep(AUTO_MEDIUM_FWD); break;
    case AUTO_MEDIUM_FWD: startAutoStep(AUTO_FAST_FWD); break;
    case AUTO_FAST_FWD: startAutoStep(STOP1); break;
    case STOP1: startAutoStep(AUTO_SLOW_BACK); break;
    case AUTO_SLOW_BACK: startAutoStep(AUTO_MEDIUM_BACK); break;
    case AUTO_MEDIUM_BACK: startAutoStep(AUTO_FAST_BACK); break;
    case AUTO_FAST_BACK: startAutoStep(STOP2); break;
    case STOP2: startAutoStep(AUTO_SLOW_FWD); break;  // reinici cicle
  }
}


void startAutoStep(AutoStep step) {
  autoStep = step;
  stepStartMillis = millis();

  switch (step) {
    case AUTO_SLOW_FWD:
      SPEED(SLOW);
      stepDuration = slowTime;
      //Serial.println("AUTO → Lent endavant");
      break;

    case AUTO_MEDIUM_FWD:
      SPEED(MEDIUM);
      stepDuration = mediumTime;
      //Serial.println("AUTO → Normal endavant");
      break;

    case AUTO_FAST_FWD:
      SPEED(FAST);
      stepDuration = fastTime;
      //Serial.println("AUTO → Ràpid endavant");
      break;

    case STOP1:
      SPEED(0);
      stepDuration = stopTime;
      //Serial.println("AUTO → STOP1");
      break;

    case AUTO_SLOW_BACK:
      SPEED(-SLOW);
      stepDuration = slowTime;
      //Serial.println("AUTO → Lent enrere");
      break;

    case AUTO_MEDIUM_BACK:
      SPEED(-MEDIUM);
      stepDuration = mediumTime;
      //Serial.println("AUTO → Normal enrere");
      break;

    case AUTO_FAST_BACK:
      SPEED(-FAST);
      stepDuration = fastTime;
      //Serial.println("AUTO → Ràpid enrere");
      break;

    case STOP2:
      SPEED(0);
      stepDuration = stopTime;
      //Serial.println("AUTO → STOP2");
      break;
  }
}


// Callback quan es connecta/desconnecta un client
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    //Serial.println("Client connectat!");
  }

  void onDisconnect(BLEServer *pServer) {
    //Serial.println("Client desconnectat!");
    // reiniciar AUTO mode si no estava iniciat!
    if (currentState != AUTO) {
      currentState = AUTO;
      startAutoStep(AUTO_SLOW_FWD);
    }
    // tornar a mostrar dispositiu com disponible
    BLEDevice::startAdvertising();
  }
};


// Callback quan arriben dades BLE
class MyCallbacks : public BLECharacteristicCallbacks {
  String lastMsg = "";

  void onWrite(BLECharacteristic *pCharacteristic) {
    String rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0) {
      String msg = String(rxValue.c_str());

      if (msg != lastMsg) {
        lastMsg = msg;
        //Serial.print("Missatge rebut via BLE: ");
        //Serial.println(msg);

        ///////////////////////// tractament missatges
        if (msg == "SF") {
          if (currentState == AUTO) {
            //Serial.println("MODE MANUAL ACTIVAT");
            currentState = MANUAL;
          }
          SPEED(SLOW);
        } else if (msg == "MF") {
          if (currentState == AUTO) {
            //Serial.println("MODE MANUAL ACTIVAT");
            currentState = MANUAL;
          }
          SPEED(MEDIUM);
        } else if (msg == "FF") {
          if (currentState == AUTO) {
            //Serial.println("MODE MANUAL ACTIVAT");
            currentState = MANUAL;
          }
          SPEED(FAST);
        } else if (msg == "STOP") {
          if (currentState == AUTO) {
            //Serial.println("MODE MANUAL ACTIVAT");
            currentState = MANUAL;
          }
          SPEED(0);
        } else if (msg == "SB") {
          if (currentState == AUTO) {
            //Serial.println("MODE MANUAL ACTIVAT");
            currentState = MANUAL;
          }
          SPEED(-SLOW);
        } else if (msg == "MB") {
          if (currentState == AUTO) {
            //Serial.println("MODE MANUAL ACTIVAT");
            currentState = MANUAL;
          }
          SPEED(-MEDIUM);
        } else if (msg == "FB") {
          if (currentState == AUTO) {
            //Serial.println("MODE MANUAL ACTIVAT");
            currentState = MANUAL;
          }
          SPEED(-FAST);
        } else if (msg == "FORCE_RESTART") {
          //Serial.println("restarting the esp32 because of command sent");
          delay(1000);
          ESP.restart();
        } else {
          // reiniciar AUTO mode si no estava iniciat!
          if (currentState != AUTO) {
            //Serial.println("REINICIANT AUTO MODE");
            currentState = AUTO;
            startAutoStep(AUTO_SLOW_FWD);
          }
        }
      }
    }
  }
};



///////////////////////////////////////////////
void setup() {
  //Serial.begin(115200);

  /************** BLE INIT **************/
  //Serial.println("Inicialitzant ESP32-C3 BLE...");

  // Inicialitza BLE
  BLEDevice::init("AVAF_dancing_tapestries");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Crea un servei
  BLEService *pService = pServer->createService("12345678-1234-1234-1234-1234567890ab");

  // Crea característica (escriptura des del mòbil)
  pCharacteristic = pService->createCharacteristic("abcd1234-5678-90ab-cdef-1234567890ab", BLECharacteristic::PROPERTY_WRITE);

  // nom descriptor
  /*
  BLEDescriptor *pDescr = new BLEDescriptor((uint16_t)0x2901); 
  pDescr->setValue("Enviar Comanda");   // aquest text sortirà a nRF Connect
  pCharacteristic->addDescriptor(pDescr);
  */

  pCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();

  // Comença a anunciar-se
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID("12345678-1234-1234-1234-1234567890ab");
  BLEDevice::startAdvertising();

  //Serial.println("Esperant connexió BLE...");


  /************** MOTOR INIT **************/
  motor.enable();
  currentState = AUTO;  // redundant? per si un cas interferència bluetooth a nes setup...
  startAutoStep(AUTO_SLOW_FWD);
}


void loop() {

  if (currentState == AUTO) {
    unsigned long now = millis();
    if (now - stepStartMillis >= stepDuration) {
      nextAutoStep();
    }
  }
  delay(100);  // ajustar?
}
