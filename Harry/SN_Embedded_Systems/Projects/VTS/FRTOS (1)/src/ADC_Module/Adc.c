
/************************************************************************************************************************************/
/*																																	*/
/*  File Name:   Adc.c																								                */
/*																																	*/
/*  Description:  contains Ignition recognition	Functionalities.																    */
/*																																	*/
/*  -----------------------------------------------------------------------------------------------------------------------------   */
/*  |  Author              |    Date      | Rev  |									Comments								|		*/
/*  -----------------------------------------------------------------------------------------------------------------------------	*/
/*  | Naveen G S		   |  01-12-2021  | 1.0  |			Created													        |	    */
/*  | Sanjay S N           |              | 1.0  |          Added Adc module                                                |       */
/*  | Harish N  		   |  30-06-2022  | 1.0  |			Added SMS,Services and AIS-140 module							|		*/
/*	| Harish N 		       |  26-09-2021  | 1.0	 |          Added I2C module												|		*/
/*------------------------------------------------------------------------------------------------------------------------------	*/
/*																																	*/
/*  Copyright (c) 2021, SNES																										*/
/*  All rights reserved.																											*/
/*																																    */
/************************************************************************************************************************************/

#include "ADC_Module/Adc.h"
#include "asf.h"
#include <stdio.h>
extern volatile bool IsBatterySafe;

//Analog to digital conversion and averaging of 5 samples
static void adc_end_conversion(void)
{
	
		g_adc_samples[g_adc_sample_count] = afec_channel_get_value(AFEC0, AFEC_CHANNEL_0);
		g_adc_value = ((g_adc_samples[0] + g_adc_samples[1]+g_adc_samples[2]+g_adc_samples[3]+g_adc_samples[4]) / 5.0 );
		g_adc_sample_count++;
		if( g_adc_sample_count > 4 )
		{
			g_adc_sample_count = 0;
			g_is_adc_conversion_done = true;
		}
		else
		afec_start_software_conversion(AFEC0);
}

//Initializes ignition recognition process
void IntialiseIgnitionRecognition(void)
{
	struct afec_config afec_cfg;
	afec_enable(AFEC0);
	afec_get_config_defaults(&afec_cfg);
	afec_init(AFEC0, &afec_cfg);
	afec_channel_set_analog_offset(AFEC0, AFEC_CHANNEL_0, 0x200);
	afec_set_callback(AFEC0, AFEC_INTERRUPT_EOC_0, adc_end_conversion, 1);
	afec_channel_enable(AFEC0, AFEC_CHANNEL_0);
	afec_start_software_conversion(AFEC0);
}

void RecogniseIgnition()
{
	uint32_t ul_vol;
	
	if(g_is_adc_conversion_done == true)
	{
		ul_vol = g_adc_value * VOLT_REF / MAX_DIGITAL;
		//printf("IgnitionON ADC Value is: %d miliVolts\r", (int)ul_vol);
		g_is_adc_conversion_done = false;
		if( ul_vol > IGNITION_ON_MIN_MILIVOLT )
		{
			g_IsIgnitionON = true;
			IsBatterySafe = false;
		}
		else if( ul_vol <= IGNITION_OFF_MAX_MILIVOLT )
		{
			g_IsIgnitionON = false;
			IsBatterySafe = false;
		}
		else
		{
			IsBatterySafe = true;
		}
		afec_start_software_conversion(AFEC0);	
	}
	else
	{
		afec_start_software_conversion(AFEC0);
	}
}   