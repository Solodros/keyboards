# MCU name
MCU = atmega32u4

# Bootloader selection
BOOTLOADER = atmel-dfu


OS_DETECTION_DEBUG_ENABLE = yes
OPT_DEFS += -DOS_DETECTION_DEBUG_ENABLE
SEND_STRING_ENABLE = yes
