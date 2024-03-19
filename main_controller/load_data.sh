/home/kirill/esp/mkspiffs/mkspiffs -c ./data/ -s 0x200000 spiffs.bin

esptool.py --chip esp32 --port /dev/ttyUSB0 erase_flash
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 115200 write_flash -z 0x190000 spiffs.bin