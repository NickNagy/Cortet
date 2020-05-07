# Cortet
STM32 (Nucleo F767ZI) interactive audio and video project

MA_ILI9341.c/.h sourced from here: https://drive.google.com/file/d/1f4WZ3Bz8Tb-dCiqacXoX_CF3trXw5EcH/view, linked from this tutorial: https://www.youtube.com/watch?v=NUErX4dx2Tw. Only major change was to sendCommand() and sendData(), which were missing triggers for chip select, register select, and write signals.
