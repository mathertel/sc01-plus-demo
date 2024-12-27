# SC01 plus example using ESP-IDF 5.3.1 ff.

The ESP-IDF has got some major changes with version 5.x regarding I80 based lcd communication.

This is example code that works with the SC01-plus board

* ST7796 via i80 demo on SC01 PLUS...
* using ESP-IDF v5.3.1 (new i80 interface)
* and esp_lcd_st7796 component from espressif

## How to use example

* Checkout
* Build
* Flash
* See 

Select the instructions depending on Espressif chip installed on your development board:

- [ESP32 Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/stable/get-started/index.html)
- [ESP32-S3 Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/index.html)


## Example folder contents

The project is based on the hello world example and contains one source file in C language
[sc01_plus_main.c](main/sc01_plus_main.c) in the [main folder](main).

ESP-IDF projects are built using CMake.  The project build configuration is contained in `CMakeLists.txt` files that
provide set of directives and instructions describing the project's source files and targets (executable, library, or
both).

For more information on structure and contents of ESP-IDF projects, please refer to Section
[Build System](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html) of the ESP-IDF
Programming Guide.

