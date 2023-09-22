/*
 * Adc.h
 *
 * Created: 26-09-2022 3.31.25 PM
 *  Author: HarishN
 */ 

#include <stdio.h>
#include "asf.h" 

#ifndef ADC_H_
#define ADC_H_
/** Reference voltage for AFEC,in mv. */
#define VOLT_REF        (3300)
#define IGNITION_ON_MIN_MILIVOLT (3250)
#define IGNITION_OFF_MAX_MILIVOLT (500)
/** The maximal digital value */
#define MAX_DIGITAL     (4095UL)

volatile Bool g_is_adc_conversion_done;
volatile Bool g_IsIgnitionON;
volatile uint32_t g_adc_value;
volatile uint32_t g_adc_samples[5];
volatile uint8_t g_adc_sample_count;

static void adc_end_conversion(void);
void IntialiseIgnitionRecognition(void);
void RecogniseIgnition(void);


#endif /* ADC_H_ */



