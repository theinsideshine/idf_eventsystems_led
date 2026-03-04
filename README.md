IDF EventSystems LED 🚀
Este proyecto forma parte de la arquitectura base para el desarrollo de firmware profesional utilizando ESP-IDF nativo para el ESP32-S3. El objetivo es migrar un modelo de ejecución secuencial (tipo Arduino) hacia un Sistema Basado en Eventos aprovechando las capacidades de FreeRTOS.

📌 Descripción
El firmware implementa una capa de abstracción de hardware para el control de LEDs RGB direccionables y la gestión de tiempos no bloqueantes. Está diseñado para ser la base de un sistema más complejo que integrará conectividad Wi-Fi, almacenamiento persistente (NVS) y un servidor web asíncrono.

🏗️ Arquitectura de Software
El proyecto se divide en módulos independientes para garantizar la escalabilidad:

led_control: Abstracción del periférico RMT para controlar el LED direccionable (WS2812B) del ESP32-S3.

timer_control: Implementación de temporizadores no bloqueantes basados en los Ticks de FreeRTOS, evitando el uso de delays que detengan el procesador.

app_main: Hilo principal que ejecuta la lógica de control y gestiona el ciclo de vida del sistema.

🛠️ Requisitos de Hardware
Placa: ESP32-S3-DevKitC-1 (o compatible).

LED: RGB integrado en el GPIO 48.

🚀 Cómo empezar
Clonar el repositorio:

Bash
git clone https://github.com/theinsideshine/idf_eventsystems_led.git
Configurar el entorno: Asegurarse de tener instalado el ESP-IDF v5.x.

Compilar y flashear:
Utilizar la extensión de VS Code o la terminal:

Bash
idf.py build flash monitor
📅 Hoja de Ruta (Roadmap)
[x] Abstracción de Hardware (LED y Timer).

[ ] Implementación de Almacenamiento Persistente (NVS).

[ ] Integración de Stack Wi-Fi y Event Loop.

[ ] Servidor Web para configuración dinámica.

Autor: [theinsideshine]

Licencia: MIT