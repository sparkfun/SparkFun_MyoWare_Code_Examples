# MyoWare Example analogRead SINGLE
# By: Ho Yun "Bobby" Chan @ SparkFun Electronics
# Date: 3/24/2022
#
# This code was adapted from the Raspberry Pi MicroPython example for the RP2040
# and ported over from Pete Lewis' Arduino example.
#
# This example streams the data from a single MyoWare sensor attached to ADC A0.
# Graphical representation is available using the Thonny IDE v3.3.13 Plotter (View > Plotter).
# Make sure to rename this file as main.py before running the code on your RP2040!
#
# *Only run on a laptop using its battery. Do not plug in laptop charger/dock/monitor.
#
# *Do not touch your laptop trackpad or keyboard while the MyoWare sensor is powered.
#
# Hardware:
# SparkFun RP2040 Pro Micro
# USB from RP2040 Pro Micro to Computer
# Output from sensor (ENV, RECT, RAW) connected to your pin A0
# 
# ==================HARDWARE HOOKUP==================
# MyoWare Muscle Sensor    <=>    RP2040
# ===================================================
#           VIN            <=>      3.3V
#           GND            <=>      GND
#    ENV, RECT, or RAW     <=>       A0
# ===================================================
# We recommend connecting the ENV pin to easily view.
#
# For more information, check out the reference language on the MicroPython functions.
#    https://docs.micropython.org/en/latest/rp2/quickref.html#adc-analog-to-digital-conversion
#    https://docs.micropython.org/en/v1.15/library/utime.html#utime.sleep_us
#
# Distributed as-is; no warranty is given
# -----------------------------------------------------------------------------

import machine
import utime   #similar to time module in Python but pared down for MicroPython

#Define analog pin for RP2040. You can reference the ADC channel or the GPIO number:
#    ADC0 = GP26
#    ADC1 = GP27
#    ADC2 = GP28
sensor_myoware = machine.ADC(0) #initialize channel 0, 12-bit ADC (default)

while True:

    #0-65535 across voltage range 0.0v - 3.3v
    reading = sensor_myoware.read_u16() #Read sensor data

    print(reading)     #output reading to serial terminal or Thonny Plotter
    utime.sleep_ms(50) #small delay so the serial terminal is not flooded with data