#include "psm.h"

void psm_init(psm_state_t *state)
{
    if (state == 0) {
        return;
    }

    state->value = 0U;
    state->accumulator = 0U;
    state->skip = false;
}

uint16_t psm_map_adc_to_logic(uint16_t adc_raw)
{
    uint32_t numerator;

    if (adc_raw > PSM_ADC_MAX) {
        adc_raw = PSM_ADC_MAX;
    }

    if (adc_raw <= PSM_ADC_INPUT_MIN) {
        return PSM_WORKING_MIN;
    }

    if (adc_raw >= PSM_ADC_INPUT_MAX) {
        return PSM_WORKING_MAX;
    }

    numerator = (uint32_t)(adc_raw - PSM_ADC_INPUT_MIN) * PSM_WORKING_MAX;
    numerator += (uint32_t)(PSM_ADC_INPUT_MAX - PSM_ADC_INPUT_MIN) / 2U;

    return (uint16_t)(numerator / (uint32_t)(PSM_ADC_INPUT_MAX - PSM_ADC_INPUT_MIN));
}

uint16_t psm_map_adc_to_working(uint16_t adc_raw)
{
    uint16_t mapped = psm_map_adc_to_logic(adc_raw);

    if (mapped > PSM_WORKING_MAX) {
        return PSM_WORKING_MAX;
    }

    return mapped;
}

void psm_set_value(psm_state_t *state, uint16_t mapped_value)
{
    if (state == 0) {
        return;
    }

    if (mapped_value > PSM_WORKING_MAX) {
        mapped_value = PSM_WORKING_MAX;
    }

    state->value = mapped_value;
}

bool psm_calculate_skip(psm_state_t *state)
{
    if (state == 0) {
        return true;
    }

    state->accumulator = (uint16_t)(state->accumulator + state->value);

    if (state->accumulator >= PSM_RANGE) {
        state->accumulator = (uint16_t)(state->accumulator - PSM_RANGE);
        state->skip = false;
    } else {
        state->skip = true;
    }

    if (state->accumulator > PSM_RANGE) {
        state->accumulator = 0U;
        state->skip = false;
    }

    return state->skip;
}
