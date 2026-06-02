# Piano cromático digital con TIVA TM4C1294NCPDT

Este proyecto consiste en el diseño e implementación de un piano cromático digital de doce notas utilizando la tarjeta de desarrollo **TIVA TM4C1294NCPDT**. El sistema integra distintos periféricos digitales, analógicos y de comunicación para construir una interfaz musical funcional.

El piano permite ejecutar notas mediante botones físicos, generar señales sonoras con buzzers controlados por transistores, modificar el volumen y la octava mediante un joystick analógico, mostrar información en una pantalla OLED y recibir comandos externos desde un teléfono celular mediante Bluetooth.

## Características principales

* Piano cromático de 12 notas.
* Lectura de botones digitales con resistencias pull-up internas.
* Debounce por software para estabilizar las pulsaciones.
* Generación de audio por software mediante Timer0A.
* Señales rectangulares digitales para activar buzzers.
* Control de volumen y octava mediante joystick analógico.
* Lectura del joystick usando ADC0 con el secuenciador SS1.
* Pantalla OLED 128x32 conectada por I2C0.
* Comunicación Bluetooth mediante módulo HC-05 conectado a UART4.
* Reproducción de melodías programadas.
* Detección básica de acordes.
* Uso de SysTick como base de tiempo de 1 ms.
* Configuración del reloj del sistema a 120 MHz mediante PLL.

## Periféricos utilizados

El proyecto emplea los siguientes periféricos de la TIVA:

* **GPIO:** lectura de botones y control de buzzers.
* **Timer0A:** generación periódica de la señal de audio.
* **SysTick:** base de tiempo de 1 ms para tareas periódicas.
* **ADC0:** lectura de los ejes X/Y del joystick.
* **I2C0:** comunicación con la pantalla OLED.
* **UART4:** comunicación serial con el módulo Bluetooth HC-05.
* **PLL:** configuración del reloj del sistema a 120 MHz.

## Descripción general del funcionamiento

El sistema se organiza de forma modular. La función principal inicializa todos los periféricos y después ejecuta continuamente la actualización general del piano. En cada ciclo se leen las notas activas, se actualizan los valores de volumen y octava, se revisa si existe una melodía activa, se detectan acordes y se actualizan las salidas de audio, la pantalla OLED y la comunicación Bluetooth.

La generación de sonido no utiliza un DAC ni el módulo PWM dedicado. En su lugar, se emplea Timer0A para producir una interrupción periódica cada 25 us. En cada interrupción se actualiza el estado de los buzzers mediante contadores independientes por canal. Cada nota tiene un límite de conteo asociado, ajustado manualmente durante la etapa de afinación.

Para mejorar la precisión temporal, el reloj del sistema se configuró a 120 MHz mediante el PLL. Esto permitió trabajar con una base de tiempo más fina para Timer0A y facilitó el ajuste de las frecuencias de las notas.

## Control de volumen y octava

El joystick analógico permite modificar dos parámetros del piano:

* **Eje X:** cambia la octava.
* **Eje Y:** cambia el volumen.

El control se realiza por pasos. Para evitar cambios repetidos mientras el joystick permanece inclinado, cada eje debe regresar al centro antes de aceptar un nuevo ajuste.

## Pantalla OLED

La pantalla OLED muestra información del estado actual del piano:

* Acorde detectado.
* Notas activas.
* Volumen.
* Octava.
* Estado general del sistema.

La pantalla se comunica mediante I2C0, usando PB2 como SCL y PB3 como SDA. El módulo utilizado trabaja con dirección I2C `0x3C`.

## Comunicación Bluetooth

La comunicación Bluetooth se realiza con un módulo HC-05 conectado a UART4. Desde una aplicación de terminal serial Bluetooth se pueden enviar comandos para consultar el estado del piano, reproducir melodías o detenerlas.

### Comandos disponibles

| Comando | Acción                              |
| ------- | ----------------------------------- |
| `?`     | Muestra la ayuda de comandos.       |
| `S`     | Muestra el estado actual del piano. |
| `1`     | Reproduce la melodía 1.             |
| `2`     | Reproduce la melodía 2.             |
| `3`     | Reproduce la melodía 3.             |
| `X`     | Detiene la melodía activa.          |

## Estructura general del proyecto

| Archivo             | Función                                        |
| ------------------- | ---------------------------------------------- |
| `main.c`            | Punto de entrada del programa.                 |
| `Config.h`          | Parámetros generales del sistema.              |
| `Clock.c`           | Configuración del reloj del sistema a 120 MHz. |
| `GPIO.c`            | Configuración de botones y buzzers.            |
| `Buttons.c`         | Lectura de botones y debounce.                 |
| `Audio.c`           | Generación de señales sonoras.                 |
| `GPTM.c`            | Configuración de Timer0A.                      |
| `SysTick.c`         | Base de tiempo de 1 ms.                        |
| `ADC.c`             | Lectura y procesamiento del joystick.          |
| `I2C.c`             | Comunicación I2C con la OLED.                  |
| `OLED.c`            | Visualización de información en pantalla.      |
| `UART.c`            | Comunicación Bluetooth por UART4.              |
| `Notes.c / Notes.h` | Relación entre canales y notas musicales.      |
| `Chords.c`          | Detección básica de acordes.                   |
| `Melody.c`          | Reproducción de melodías programadas.          |
| `PianoApp.c`        | Integración general del sistema.               |

## Notas de implementación

Durante el desarrollo se realizaron varios ajustes importantes. El canal 0, correspondiente a la nota Si, se movió de PK0 a PQ0 debido a una falla física en PK0. También se corrigió la configuración de la pantalla OLED para trabajar como modelo de 128x32 pixeles y se ajustó el bus I2C para utilizar correctamente PB2/PB3 con función alterna I2C0.

La afinación de las notas se realizó modificando manualmente los límites de conteo de cada canal en el módulo de audio, usando como referencia un afinador de guitarra. Esto fue necesario porque los buzzers no se comportan como bocinas ideales y la señal generada es rectangular digital.

## Autor
Darío Rodríguez Pedroche

Proyecto desarrollado como práctica/proyecto final de la asignatura de Microprocesadores y Microcontroladores, Facultad de Ingeniería, UNAM.
