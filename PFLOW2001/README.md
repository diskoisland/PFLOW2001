# PFLOW2001 Arduino Library

Ross Edwards, Jan 2026
Angst+Pfister PFLOW2001 MEMS mass flow sensor family Arduino library. Based on https://sensorsandpower.angst-pfister.com/fileadmin/products/datasheets/263/PFLOW2001_1610-21914-0033-E-0622.pdf.,  https://sensorsandpower.angst-pfister.com/fileadmin/products/datasheets/263/PFLOW2001-AN-I2C_1610-21914-0032-E-0622.pdf, https://sensorsandpower.angst-pfister.com/fileadmin/products/datasheets/263/PFLOW2001-AN-AP-I2C_1610-21914-0067-E-0423.pdf

## Features

- Reads air flow data from Angst+Pfister PFLOW2001 over I2C
- returns flow data in standard cubic centimeters per minute (sccm, 20°C, 101.325kPa, dry clean air)
- Commands:
1. Read sensor air flow data.
2. Read sensor serial number.
3. Reset sensor zero flow offset.
4. Read sensor inline gas temperature.
5. Perform CRC-8 check.
6. Return sensor error strings (human-readable).

- Includes an I2C scanner example to search for the PFLOW2001 sensor.
- Includes an example of smoothed data "FilteredFlow" using an exponential moving average (EMA) filter.

## Installation

1. Download the ZIP file.
2. In Arduino IDE, use **Sketch > Include Library > Add .ZIP Library...**
3. Select the ZIP.

## Notes

- library is based on Angst+Pfister PFLOW2001 data sheeet - Tested using low power unidirectional PFLOW2001-
- The library defaults to the PFLOW2001 I2C address of `0x50`.
- If your platform expects the alternate 7-bit form, try `0x28`.
- The sensor requires a repeated-start read sequence, so the library uses `endTransmission(false)` before `requestFrom()`.
- Only reset the sensor zero flow offset when the sensor is under confirmed zero-flow conditions.



