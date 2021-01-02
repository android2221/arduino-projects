/*
 * IRremote: IRreceiveDemo - demonstrates receiving IR codes with IRrecv
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * Initially coded 2009 Ken Shirriff http://www.righto.com/
 */

#include <IRremote.h>
#include <map>
#include "Freenove_WS2812_Lib_for_ESP32.h"


int IR_RECEIVE_PIN = 23;
int FLICKS_TO_TURN_OFF = 5;

IRrecv IrReceiver(IR_RECEIVE_PIN);
int counter;
std::map<int, int> colorMap;
int flickCount = 0;

// On the Zero and others we switch explicitly to SerialUSB
#if defined(ARDUINO_ARCH_SAMD)
#define Serial SerialUSB
#endif


#define LEDS_COUNT  8  // The number of led
#define LEDS_PIN    21  // define the pin connected to the Freenove 8 led strip
#define CHANNEL     0  // RMT module channel


Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(LEDS_COUNT, LEDS_PIN, CHANNEL, TYPE_GRB);

int m_color[5][3] = { {255, 0, 0}, {0, 255, 0}, {0, 0, 255}, {255, 255, 255}, {0, 0, 0} };
int delayval = 100;

void setup() {
    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_USB) || defined(SERIAL_PORT_USBVIRTUAL)
    delay(2000); // To be able to connect Serial monitor after reset and before first printout
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__));

    // In case the interrupt driver crashes on setup, give a clue
    // to the user what's going on.
    Serial.println("Enabling IRin");
    IrReceiver.enableIRIn();  // Start the receiver

    Serial.print(F("Ready to receive IR signals at pin "));
    Serial.println(IR_RECEIVE_PIN);
    strip.begin();
}

std::map<int, int> buildColorData(int ledAmount){
  std::map<int, int> return_map;

  for (int i = 0; i < ledAmount; i++){
    //strip.setLedColorData(i, strip.Wheel(rand() % 254 + 1));
    return_map.insert(std::pair<int,int>(i, rand() % 254 + 1));
  }

  return return_map;
}

void smoothTransitionColors(String state, std::map<int, int> colorsMap){
  if (state == "off"){
    Serial.println("turning off");
    for(int i = 100; i >= 0; i--){
      // setting brightness
      strip.setBrightness(i);
      for (std::map<int, int>::iterator it = colorsMap.begin(); it != colorsMap.end(); ++it)
      {
        // Setting LEDS
        strip.setLedColorData(it->first, strip.Wheel(it->second));
      }
      Serial.println(i);
      strip.show();
      delay(2);
    }
  } else{
    Serial.println("turning on");
    for(int i = 0; i <= 100; i++){
        Serial.println(i);
        strip.setBrightness(i);
        for (std::map<int, int>::iterator it = colorsMap.begin(); it != colorsMap.end(); ++it)
        {
          // Setting LEDS
           strip.setLedColorData(it->first, strip.Wheel(it->second));
        }
        strip.show();
        delay(2);
    }
  }

}


void loop() {
    if (IrReceiver.decode()) {
        IrReceiver.printResultShort(&Serial);
        IrReceiver.printIRResultRawFormatted(&Serial); 
        Serial.println();
        Serial.println("Wand id:");
        Serial.println(IrReceiver.results.value);

        int wandId = IrReceiver.results.value;

        if (wandId == 449 || wandId == 458){
          if (flickCount == FLICKS_TO_TURN_OFF){
            smoothTransitionColors("off", colorMap);
            flickCount = 0;
            delay(5000);
            IrReceiver.resume();
            std::map<int, int> newMap;
            colorMap = newMap;
            return;
          }
          
          // Drew's Wand is 449
          if (colorMap.empty()){
            colorMap = buildColorData(LEDS_COUNT);
            smoothTransitionColors("on", colorMap);

          }
          else{
            smoothTransitionColors("off", colorMap);
            colorMap = buildColorData(LEDS_COUNT);
            smoothTransitionColors("on", colorMap);
          }

        }

        flickCount++;
        
        IrReceiver.resume(); // Receive the next value
    }

    delay(100);
}
