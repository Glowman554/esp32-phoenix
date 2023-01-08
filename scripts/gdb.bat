@echo off

start idf.py openocd
xtensa-esp32-elf-gdb -x gdbinit build\phoenix.elf 