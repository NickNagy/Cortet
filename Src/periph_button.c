#include "periph_button.h"

ButtonConfigStruct * buttons[NUM_FX_BUTTONS];

static void buttonInterruptConfig() {
    HAL_NVIC_SetPriority(FX_BUTTON1_EXTIRQ, FX_BUTTON_IT_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(FX_BUTTON1_EXTIRQ);
#if NUM_FX_BUTTONS > 1
    HAL_NVIC_SetPriority(FX_BUTTON2_EXTIRQ, FX_BUTTON_IT_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(FX_BUTTON2_EXTIRQ);
#endif
#if NUM_FX_BUTTONS > 2
    HAL_NVIC_SetPriority(FX_BUTTON3_EXTIRQ, FX_BUTTON_IT_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(FX_BUTTON3_EXTIRQ);

    /* menu buttons */
    HAL_NVIC_SetPriority(UP_BUTTON_EXTIRQ, MENU_BUTTON_IT_PRIORITY, 0);
    HAL_NVIC_SetPriority(DOWN_BUTTON_EXTIRQ, MENU_BUTTON_IT_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(UP_BUTTON_EXTIRQ);
    HAL_NVIC_EnableIRQ(DOWN_BUTTON_EXTIRQ);

#endif
}

void buttonGPIOInit() {
    GPIO_InitTypeDef GPIO_ButtonConfig = {0};

    GPIO_ButtonConfig.Mode = GPIO_MODE_IT_FALLING;
    GPIO_ButtonConfig.Pull = GPIO_NOPULL;
    GPIO_ButtonConfig.Speed = GPIO_SPEED_FREQ_HIGH;

    GPIO_ButtonConfig.Pin = FX_BUTTON1_PIN;
    HAL_GPIO_Init(FX_BUTTON1_PORT, &GPIO_ButtonConfig);

#if NUM_FX_BUTTONS > 1
    GPIO_ButtonConfig.Pin = FX_BUTTON2_PIN;
    HAL_GPIO_Init(FX_BUTTON2_PORT, &GPIO_ButtonConfig);
#endif
#if NUM_fX_BUTTONS > 2
    GPIO_ButtonConfig.Pin = FX_BUTTON3_PIN;
    HAL_GPIO_Init(FX_BUTTON3_PORT, &GPIO_ButtonConfig);
#endif

    /* menu buttons */
    GPIO_ButtonConfig.Speed = MENU_BUTTON_SPEED;
    GPIO_ButtonConfig.Pin = UP_BUTTON_PIN | DOWN_BUTTON_PIN;
    HAL_GPIO_Init(GPIOE, &GPIO_ButtonConfig);

    buttonInterruptConfig();
}

void buttonEffectConfig(uint8_t buttonNumber, uint8_t pressAndHoldType, void (*func_ptr)(AUDIO_BUFFER_PTR_T, uint16_t)) {
    buttons[buttonNumber]->effect_ptr = func_ptr;
    buttons[buttonNumber]->typeAndState = 1 << (pressAndHoldType);
}

/*extern void EXTI_IRQHandler(uint16_t GPIO_Pin) {
  if(__HAL_GPIO_EXTI_GET_IT(EXTI_LINE_2) != RESET)
  {
    buttons[GPIO_Pin]->isOnState ^= 1;
    __HAL_GPIO_EXTI_CLEAR_IT();
    HAL_GPIO_EXTI_Callback();
  }
}*/
