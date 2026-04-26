#ifndef PSM_H
#define PSM_H

#include <stdbool.h>
#include <stdint.h>

#define PSM_ADC_MAX               4095U
#define PSM_RANGE                 127U
#define PSM_VALUE_FACTOR          16U
#define PSM_WORKING_MIN           0U
#define PSM_WORKING_MAX           PSM_RANGE

typedef struct {
	uint16_t value;
	uint16_t accumulator;
	bool skip;
} psm_state_t;

void psm_init(psm_state_t *state);
uint16_t psm_map_adc_to_logic(uint16_t adc_raw);
uint16_t psm_map_adc_to_working(uint16_t adc_raw);
void psm_set_value(psm_state_t *state, uint16_t mapped_value);
bool psm_calculate_skip(psm_state_t *state);

#endif
