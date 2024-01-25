/home/kirill/esp/mkspiffs/mkspiffs -c ./data/ -b 4096 -p 256 -s 0x100000 spiffs.bin

esptool.py --chip esp32 --port /dev/ttyUSB0 erase_flash
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 115200 write_flash -z 0x110000 spiffs.bin