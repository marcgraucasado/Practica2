#include <Arduino.h>

#define DELAY 500

struct Button {
  const uint8_t PIN;
  volatile uint32_t numberKeyPresses;
  volatile bool pressed;
};

volatile Button button1 = {38, 0, false};

void IRAM_ATTR isr() {  // Usa IRAM_ATTR solo si estás en ESP32
  button1.numberKeyPresses += 1;
  button1.pressed = true;
}

void setup() {
  Serial.begin(9600);  // dejar 9600 para mostrar los datos correctos por pantalla
  pinMode(button1.PIN, INPUT_PULLUP);
  attachInterrupt(button1.PIN, isr, FALLING);
}

void loop() {
  if (button1.pressed) {
    Serial.printf("Button 1 has been pressed %u times\n", button1.numberKeyPresses);
    button1.pressed = false;
  }

  // Detach Interrupt after 1 Minute
  static uint32_t lastMillis = 0;
  static bool detached = false;

  if (!detached && millis() - lastMillis > 60000) {
    lastMillis = millis();
    detachInterrupt(button1.PIN);
    Serial.println("Interrupt Detached!");
    detached = true;
  }
}