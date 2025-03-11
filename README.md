# PR2-MarcGrau

## PRÁCTICA 2: TIPOS DE INTERRUPCIONES

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
## Observaciones:

###  **`void IRAM`**
- **`IRAM_ATTR`** se usa para indicar que una función debe ser almacenada en la memoria **IRAM** del ESP32.
- La **IRAM** (Internal RAM) es una memoria rápida y eficiente que se utiliza para funciones que deben ejecutarse rápidamente, como las funciones de **interrupción**.
- Cuando se configura una función como `IRAM_ATTR`, se garantiza que esta funcione con la menor latencia posible, especialmente cuando se trata de interrupciones generadas por eventos de alta velocidad (por ejemplo, un cambio de estado en un botón).
- En el contexto de interrupciones, este tipo de memoria es fundamental para evitar retrasos en el procesamiento de eventos rápidos y críticos.
- El identificador `IRAM_ATTR` es recomendado por Espressif para colocar este fragmento de código en la memoria RAM interna en lugar de la flash. Esto asegura una ejecución más rápida y un servicio de interrupción más eficiente.

## Observaciones sobre el Contador de Pulsaciones:

El **contador** cuenta de más porque también registra los **rebotes** que ocurren en los flancos del botón. Los rebotes son señales erráticas generadas cuando el botón cambia de estado, lo que puede hacer que una sola pulsación se registre varias veces.

Para evitar este problema, es necesario **reducir el "tiempo de reacción"** utilizando un filtro que ignore los cambios rápidos que ocurren en los rebotes. Una forma común de hacerlo es **filtrar los rebotes** durante unos 300 ms, asegurando que solo se registre una pulsación después de que el botón se haya estabilizado.

En nuestro caso, utilizamos un **`bounce_delay`** de 300 ms para garantizar que el contador solo registre **una pulsación por vez**, eliminando los rebotes y asegurando que el botón se cuente correctamente.


### Código A

A continuación, se presenta el código utilizado para la implementación de la interrupción en el ESP32:

```cpp
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
## Observaciones:

### 2B. **Reloj e `int timer`**

- **`int timer`** es una variable de tipo entero que se usa para mantener un conteo de unidades de tiempo o para realizar una acción repetitiva después de ciertos intervalos.
- El concepto de **reloj** en este contexto se refiere a la capacidad de medir y gestionar el paso del tiempo sin bloquear la ejecución del programa.
- **`millis()`** es una función que devuelve el número de milisegundos transcurridos desde que el programa comenzó a ejecutarse, lo que permite gestionar los temporizadores sin necesidad de bloquear el código (como lo haría un `delay()`).
- Al comparar el tiempo actual con un valor anterior (`previousMillis`), se puede ejecutar un bloque de código cada vez que transcurre un intervalo específico, creando así un "reloj" que activa acciones a intervalos regulares.
