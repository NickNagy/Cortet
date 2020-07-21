# Cortet
STM32 (Nucleo F767ZI) interactive audio processing device with an LCD display.

This project uses the CMSIS DSP library (requires the inclusion of **arm_math.h** as well as some source files).

The low-level configuration header files (**stm32f7xx_hal_conf.h**, **stm32f7xx_it.h**) and source files (**stm32f7xx_hal_msp.c**, **stm32f7xx_it.c**, **syscalls.c**, **system_stm32f7xx.c**) were originally generated with STM32CubeMX. I've since made slight modifications to the configuration and MSP files.

**ILI9341.c/.h** originally sourced from here: https://drive.google.com/file/d/1f4WZ3Bz8Tb-dCiqacXoX_CF3trXw5EcH/view, linked from this tutorial: https://www.youtube.com/watch?v=NUErX4dx2Tw. Some changes/additions to functions and address spaces.
