@echo off

mkdir tools
curl https://raw.githubusercontent.com/espressif/esp-idf/4c98bee8a4417d92a387ef2416be14ff1fdfa92a/components/spiffs/spiffsgen.py -o tools/spiffsgen.py

python tools/spiffsgen.py 0x100000 data fs.bin 
python -m esptool --chip esp32 --baud 115200 write_flash -z 0x00210000 fs.bin
