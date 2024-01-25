esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 115200 read_flash 0x110000 0x100000 spiffs_test.bin
/home/kirill/esp/mkspiffs/mkspiffs -u data_test spiffs_test.bin