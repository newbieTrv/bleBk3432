#ifndef _RF_TEST_H_
#define _RF_TEST_H_


#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "uart_rf.h"



#define SINGEWAVE_OPEN   "CMD+SINWAVE+OPEN"
#define SINGEWAVE_CLOSE  "CMD+SINWAVE+CLOSE"
#define SINGEWAVE_CONFIG "CMD+SINWAVE+CONF+"



void BT_delayUs(int num);


void rf_test_ready(void);

#endif
