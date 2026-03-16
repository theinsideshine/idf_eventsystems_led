
# IDF EventSystems LED

Este proyecto forma parte de una serie didáctica que muestra cómo migrar firmware
desde una arquitectura estilo Arduino hacia una arquitectura nativa de ESP‑IDF
utilizando FreeRTOS.

El punto de partida es el proyecto basado en Arduino:

https://github.com/theinsideshine/esp32S3-led-arduino

El objetivo es migrar esa arquitectura paso a paso hacia un diseño de firmware
embebido profesional.

El foco de este repositorio no es solo la funcionalidad, sino también explicar
la transición arquitectónica desde un modelo secuencial hacia un modelo concurrente
basado en RTOS.

------------------------------------------------------------

## Objetivo del Proyecto

La mayoría de los proyectos en Arduino utilizan un modelo de ejecución secuencial:

loop()
{
    web
    control
    estados
}

Todo ocurre dentro de un único flujo de ejecución.

Los sistemas embebidos modernos, en cambio, se basan en arquitecturas concurrentes
utilizando:

- FreeRTOS
- event loops
- tareas independientes
- comunicación entre tareas
- configuración persistente

Este repositorio muestra cómo evolucionar hacia ese modelo paso a paso.

------------------------------------------------------------

## Estrategia del Repositorio

El desarrollo está organizado en fases incrementales.

Cada fase introduce un concepto arquitectónico nuevo y queda almacenada en el
historial del repositorio utilizando ramas (branches).

Ejemplo de estructura:

main      -> última fase estable
phase1    -> abstracción de hardware
phase2    -> configuración + NVS + WiFi + Web
phase3    -> separación RTOS (WEB / CORE)
phase4    -> arquitectura completamente orientada a eventos

Esto permite que los estudiantes inspeccionen cómo evoluciona la arquitectura.

------------------------------------------------------------

## Hardware

Placa

ESP32-S3-DevKitC-1

LED RGB integrado

GPIO 48 (WS2812B)

------------------------------------------------------------

## Requisitos de Software

ESP-IDF v5.x

Guía de instalación:

https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/

------------------------------------------------------------

## Compilar y Flashear

Clonar el repositorio

git clone https://github.com/theinsideshine/idf_eventsystems_led.git

Entrar al directorio del proyecto

cd idf_eventsystems_led

Compilar y flashear

idf.py build flash monitor

------------------------------------------------------------

## Arquitectura del Software

El firmware está organizado en módulos independientes.

main
 ├── led_control
 ├── timer_control
 ├── config_manager
 ├── wifi_manager
 ├── web_server
 └── app_runtime

Cada módulo tiene una responsabilidad clara.

------------------------------------------------------------

# Fase 1 - Abstracción de Hardware

Objetivo

Separar el acceso al hardware de la lógica de la aplicación.

Módulos introducidos

led_control

Encapsula el control del LED RGB direccionable utilizando el periférico RMT.

Responsabilidades

- inicialización del LED
- control de color
- abstracción del driver

timer_control

Implementa temporizadores no bloqueantes basados en los ticks de FreeRTOS.

Evita el uso de delay() y permite que la CPU continúe ejecutando otras tareas.

------------------------------------------------------------

# Fase 2 - Configuración Persistente y Conectividad

Esta fase introduce tres componentes clave:

- configuración persistente
- conectividad WiFi
- interfaz web

config_manager

Gestiona la configuración persistente utilizando NVS.

Arquitectura utilizada:

NVS -> RAM (copia de trabajo)

Al arrancar el sistema:

config_load()

carga la configuración desde NVS hacia la RAM.

Los setters modifican únicamente la copia en RAM.

La persistencia ocurre solo cuando se llama a:

config_save()

Ejemplo de parámetros persistentes:

led_blink_time
led_blink_quantity
led_color
wifi_ssid
wifi_pass
log_level

------------------------------------------------------------

wifi_manager

Inicializa el stack de WiFi y gestiona la conexión en modo estación.

Utiliza el event loop de ESP-IDF para reaccionar a eventos como:

WIFI_EVENT_STA_START
WIFI_EVENT_STA_DISCONNECTED
IP_EVENT_STA_GOT_IP

------------------------------------------------------------

web_server

Proporciona una interfaz HTTP para:

- ver el estado del sistema
- cambiar la configuración
- guardar parámetros

Flujo típico:

Browser
   |
web_server
   |
config_set()
   |
config_save()

------------------------------------------------------------

app_runtime

Contiene el estado dinámico del sistema en ejecución.

Ejemplos:

running
state
remaining

Estos valores:

- existen solo en RAM
- no se almacenan en NVS
- se reinician al reiniciar el dispositivo

Esto separa claramente:

CONFIGURACIÓN (persistente)
ESTADO DE EJECUCIÓN (volátil)

------------------------------------------------------------

# Arquitectura Actual (Fase 2)

Browser
   |
WEB SERVER
   |
config_manager
   |
app_runtime
   |
LED control

El sistema ya incluye:

- configuración persistente
- conectividad WiFi
- servidor web
- temporización no bloqueante

Pero la ejecución todavía está centralizada.

------------------------------------------------------------

# Fase 3 - Separación de Tareas (WEB + CORE)

Esta fase introduce una arquitectura RTOS real.

Se crean dos tareas independientes:

WEB TASK
CORE TASK

La comunicación ocurre mediante una cola de FreeRTOS.

Arquitectura objetivo:

Browser
   |
WEB SERVER TASK
   |
QUEUE
   |
CORE TASK
   |
LED / TEST

El servidor web ya no controla el hardware directamente.
Solo envía comandos.

Ejemplo de comandos:

APP_CMD_START
APP_CMD_STOP

------------------------------------------------------------

# Fase 4 - Sistema Completamente Orientado a Eventos

El paso final convierte el firmware en una arquitectura completamente orientada a eventos.

En lugar de comandos directos entre módulos, el sistema reacciona a eventos.

Ejemplo de eventos:

EVT_WEB_SAVE
EVT_TEST_START
EVT_TEST_STOP
EVT_LED_ON
EVT_LED_OFF
EVT_WIFI_CONNECTED
EVT_WIFI_DISCONNECTED
EVT_TEST_FINISHED

Arquitectura objetivo:

Browser
   |
WEB SERVER
   |
EVENT BUS
   |
EVENT HANDLERS
   |
SYSTEM MODULES

Ventajas:

- fuerte desacoplamiento
- mayor escalabilidad
- mejor observabilidad del sistema
- arquitectura de firmware modular

------------------------------------------------------------

# Objetivo Didáctico Final

Mostrar la evolución completa:

Arduino loop
    |
abstracción de hardware
    |
configuración persistente
    |
WiFi + interfaz web
    |
separación de tareas con RTOS
    |
arquitectura orientada a eventos

El objetivo es ayudar a los desarrolladores a entender cómo se estructura
un firmware embebido profesional utilizando ESP-IDF y FreeRTOS.

Autor: theinsideshine
Licencia: MIT
