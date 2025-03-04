# Práctica 2-MarcGrau: Interrupciones

## Tipos de Interrupciones

Existen tres tipos de eventos que pueden desencadenar una interrupción: eventos de hardware, eventos programados o temporizadores, y llamadas por software. Sin embargo, en Arduino, solo se admiten las interrupciones de hardware y de temporizadores.

## Objetivo de la Práctica

El objetivo de esta práctica es comprender el funcionamiento de las interrupciones mediante un ejercicio práctico. Controlaremos dos LEDs de manera periódica y una entrada. Cuando ocurra un evento en la entrada, cambiará la frecuencia de parpadeo de uno de los LEDs.

En resumen, las interrupciones ofrecen una manera eficiente de organizar un programa. En lugar de verificar continuamente si ha ocurrido un evento, podemos definir una función que se ejecute automáticamente cuando se produzca la interrupción, lo que simplifica el código y hace que el programa sea más elegante y eficiente.

## Práctica A: Interrupciones por GPIO

### Interrupciones en ESP32

En el ESP32, podemos configurar una función de rutina de servicio de interrupción que se activará cuando un pin GPIO cambie su estado de señal. Con la placa ESP32, todos los pines GPIO pueden ser configurados para funcionar como entradas de solicitud de interrupción.

Para adjuntar una interrupción a un pin GPIO en el IDE de Arduino, utilizamos la función `attachInterrupt()`. La sintaxis recomendada es la siguiente:

```cpp
attachInterrupt(GPIOPin, ISR, Mode);
```

Esta función toma tres parámetros:

- **GPIOPin**: Define el pin GPIO como una entrada de interrupción, lo que indica al ESP32 qué pin debe monitorear.
- **ISR**: Es el nombre de la función que se ejecutará cada vez que se produzca la interrupción.
- **Mode**: Define cuándo se activará la interrupción. Hay cinco constantes predefinidas como valores válidos:
  - `LOW`: La interrupción se activa cuando el pin está en estado LOW.
  - `HIGH`: La interrupción se activa cuando el pin está en estado HIGH.
  - `CHANGE`: La interrupción se activa cuando el pin cambia de estado, de HIGH a LOW o de LOW a HIGH.
  - `FALLING`: La interrupción se activa cuando el pin cambia de estado de HIGH a LOW.
  - `RISING`: La interrupción se activa cuando el pin cambia de estado de LOW a HIGH.

Opcionalmente, cuando ya no necesitemos que el ESP32 monitoree un pin, podemos desvincular la interrupción utilizando la función `detachInterrupt()`. La sintaxis es la siguiente:

```cpp
detachInterrupt(GPIOPin);
```

La rutina de servicio de interrupción es la función que se ejecuta cuando ocurre el evento de interrupción. Debe ser breve en cuanto a su tiempo de ejecución. Su sintaxis es la siguiente:

```cpp
void IRAM_ATTR ISR() {
  // Declaraciones;
}
```

El identificador `IRAM_ATTR` es recomendado por Espressif para colocar este fragmento de código en la memoria RAM interna en lugar de la flash. Esto asegura una ejecución más rápida y un servicio de interrupción más eficiente.

### Código A

```cpp
#include <Arduino.h>

int LED_BUILTIN = 23;
int DELAY = 500;
int count = 0;

void IRAM_ATTR isr() {
  Serial.println("Interrupted");
  count = 0;
}

void setup() {
  pinMode(LED_BUILTIN, INPUT_PULLUP);
  Serial.begin(115200);
  attachInterrupt(LED_BUILTIN, isr, FALLING);
}

void loop() {
  for (count; count < 1000; count++) {
    Serial.println(count);
    delay(DELAY);
  }
}
```

---

## Práctica B: Interrupciones por Temporizador

En este programa seguimos usando interrupciones, pero hemos cambiado el tipo de interrupción. Ahora utilizamos interrupciones de tipo temporizador o "timer" en lugar de GPIO o "pin de interrupción".

El funcionamiento del programa es muy sencillo: cada microsegundo (este tiempo es modificable) se produce una interrupción automática. La acción que realizamos cada vez que se produce una interrupción, es decir, cada microsegundo, es sumar +1 a una variable llamada `interruptCounter`.

Es importante destacar que esta es la acción que se ejecuta automáticamente cuando se produce una interrupción. Si recordáis el ejercicio anterior, diferenciábamos entre las acciones que se ejecutaban cuando se producía una interrupción, lo que había dentro de `ISR`, y las consecuencias que estas producían, las cuales se podían ver dentro del `loop()` del programa.

En este ejercicio ocurre lo mismo, solo que la función `ISR` ahora se llama `onTimer`. Cuando se produce una interrupción, llamamos a `onTimer` y esta suma +1 a la variable mencionada anteriormente, `interruptCounter`.

Fijémonos en el `void loop()`, hay un `if` que se ejecutará cuando `interruptCounter` sea mayor que 0 y esto solo ocurrirá cuando se produzca una interrupción (básicamente lo mismo que el ejercicio anterior).

¿Qué se ejecuta dentro de este `if`? Pues sumaremos +1 a otra variable llamada `totalInterruptCounter` y restaremos -1 a la variable `interruptCounter` para que se pueda volver a ejecutar este `if` (como hacíamos en el ejercicio anterior, pero en lugar de usar una variable booleana, utilizamos una variable entera). También enviaremos un mensaje por el puerto serie que nos informará de cuántas interrupciones han ocurrido; esto lo hará gracias a `totalInterruptCounter`:

```cpp
Serial.print("An interrupt has occurred. Total number: ");
Serial.println(totalInterruptCounter);
```

Podemos modificar el tiempo en el que ocurre una interrupción en el `1000000` de:

```cpp
timerAlarmWrite(timer, 1000000, true);
```

### Código B

```cpp
#include <Arduino.h>

volatile int interruptCounter;
int totalInterruptCounter;
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  interruptCounter++;
  portEXIT_CRITICAL_ISR(&timerMux);
}

void setup() {
  Serial.begin(115200);
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000000, true);
  timerAlarmEnable(timer);
}

void loop() {
  if (interruptCounter > 0) {
    portENTER_CRITICAL(&timerMux);
    interruptCounter--;
    portEXIT_CRITICAL(&timerMux);
    totalInterruptCounter++;

    Serial.print("An interrupt has occurred. Total number: ");
    Serial.println(totalInterruptCounter);
  }
}
```
