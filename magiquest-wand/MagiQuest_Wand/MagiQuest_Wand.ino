/*
 * IRremote: IRreceiveDemo - demonstrates receiving IR codes with IRrecv
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * Initially coded 2009 Ken Shirriff http://www.righto.com/
 */

#include <IRremote.h>
#include <map>
#include "Freenove_WS2812_Lib_for_ESP32.h"


int IR_RECEIVE_PIN = 23;
IRrecv IrReceiver(IR_RECEIVE_PIN);
int counter;

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
    for(int i = 10; i >= 0; i--){
      strip.setBrightness(i * 10);
      strip.setLedColorData(1, strip.Wheel(rand() % 254 + 1));
      Serial.println(i);
      strip.show();
      delay(100);
    }
  } else{
    Serial.println("turning on");
    for(int i = 0; i <= 10; i++){
        Serial.println(i);
        strip.setBrightness(i * 10);
        strip.show();
        delay(100);
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

        if (wandId == 449){
          // Drew's Wand
          std::map<int, int> colorMap = buildColorData(LEDS_COUNT);
          smoothTransitionColors("off", colorMap);
          smoothTransitionColors("on", colorMap);
          //strip.show();   // Send color data to LED, and display.
        }

        if (wandId == 458){
          // Brays Wand
          strip.setLedColorData(1, m_color[1][0], m_color[1][1], m_color[1][2]);// Set color data.
          strip.show();   // Send color data to LED, and display.
        }
        
        IrReceiver.resume(); // Receive the next value
    }

    delay(100);
}
