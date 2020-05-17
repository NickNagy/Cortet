# Cortet
STM32 (Nucleo F767ZI) interactive audio and video project

MA_ILI9341.c/.h sourced from here: https://drive.google.com/file/d/1f4WZ3Bz8Tb-dCiqacXoX_CF3trXw5EcH/view, linked from this tutorial: https://www.youtube.com/watch?v=NUErX4dx2Tw.

FMC is not yet operational, I am in the process of deducing the proper ADDSET and DATAST values for my particular system, and revising the HAL drivers as needed. For now, commands and writes are driven by explicit user calls.
