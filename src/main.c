#include "ch32v00x.h"

#include <stdbool.h>
#include <stdint.h>

#include "psm.h"

#define ADC_GPIO_PORT        GPIOC
#define ADC_GPIO_PIN         GPIO_Pin_4
#define ADC_CHANNEL_PC4      ADC_Channel_2

#define OUT_GPIO_PORT        GPIOC
#define OUT_GPIO_PIN         GPIO_Pin_2

#define EVENT_GPIO_PORT      GPIOC
#define EVENT_GPIO_PIN       GPIO_Pin_1

static volatile uint16_t g_adc_raw = 0U;
static volatile uint16_t g_adc_working_value = PSM_WORKING_MIN;
static psm_state_t g_psm_state;

static void init_clock(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOC | RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);
}

static void init_gpio(void)
{
    GPIO_InitTypeDef gpio_init = {0};

    gpio_init.GPIO_Pin = OUT_GPIO_PIN;
    gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(OUT_GPIO_PORT, &gpio_init);
    GPIO_WriteBit(OUT_GPIO_PORT, OUT_GPIO_PIN, Bit_RESET);

    gpio_init.GPIO_Pin = EVENT_GPIO_PIN;
    gpio_init.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(EVENT_GPIO_PORT, &gpio_init);

    gpio_init.GPIO_Pin = ADC_GPIO_PIN;
    gpio_init.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(ADC_GPIO_PORT, &gpio_init);
}

static void init_exti(void)
{
    EXTI_InitTypeDef exti_init = {0};

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource1);

    exti_init.EXTI_Line = EXTI_Line1;
    exti_init.EXTI_Mode = EXTI_Mode_Interrupt;
    exti_init.EXTI_Trigger = EXTI_Trigger_Falling;
    exti_init.EXTI_LineCmd = ENABLE;
    EXTI_Init(&exti_init);

    NVIC_EnableIRQ(EXTI7_0_IRQn);
}

static void init_adc(void)
{
    ADC_InitTypeDef adc_init = {0};

    ADC_DeInit(ADC1);

    adc_init.ADC_Mode = ADC_Mode_Independent;
    adc_init.ADC_ScanConvMode = DISABLE;
    adc_init.ADC_ContinuousConvMode = DISABLE;
    adc_init.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    adc_init.ADC_DataAlign = ADC_DataAlign_Right;
    adc_init.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &adc_init);

    ADC_RegularChannelConfig(ADC1, ADC_CHANNEL_PC4, 1, ADC_SampleTime_241Cycles);
    ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);

    NVIC_EnableIRQ(ADC_IRQn);

    ADC_Cmd(ADC1, ENABLE);

    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1) == SET) {
    }

    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1) == SET) {
    }
}

static void on_event_falling_edge(void)
{
    bool skip;

    psm_set_value(&g_psm_state, g_adc_working_value);
    skip = psm_calculate_skip(&g_psm_state);

    GPIO_WriteBit(OUT_GPIO_PORT, OUT_GPIO_PIN, skip ? Bit_RESET : Bit_SET);

    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

static void on_adc_read_complete(void)
{
    g_adc_raw = ADC_GetConversionValue(ADC1);
    g_adc_working_value = psm_map_adc_to_working(g_adc_raw);
}

void EXTI7_0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line1) != RESET) {
        EXTI_ClearITPendingBit(EXTI_Line1);
        on_event_falling_edge();
    }
}

void ADC1_IRQHandler(void)
{
    if (ADC_GetITStatus(ADC1, ADC_IT_EOC) != RESET) {
        ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
        on_adc_read_complete();
    }
}

int main(void)
{
    psm_init(&g_psm_state);

    init_clock();
    init_gpio();
    init_exti();
    init_adc();

    for (;;) {
        __WFI();
    }
}
