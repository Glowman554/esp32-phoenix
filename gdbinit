target remote :3333
set remote hardware-watchpoint-limit 2
monitor reset halt
flushregs
thb app_main
continue