#!/bin/sh

@echo off
pacmd "set-default-source alsa_input.usb-C-Media_Electronics_Inc._USB_PnP_Sound_Device-00.analog-mono"
pacmd "set-default-sink alsa_output.usb-C-Media_Electronics_Inc._USB_PnP_Sound_Device-00.analog-stereo>"
amixer set Mic 100%
amixer set PCM 100%

BASEDIR=$(dirname "$0")
${BASEDIR}/code/Server/Donna $*
