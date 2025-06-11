#ifndef __SENSOR_H
#define __SENSOR_H
#include "at32f421.h"      
#include "at32f421_int.h"  //ϵͳ����ʱ�亯����ȡ
/**********************************�������************************************/






/******************************************************************************/
#define SENSOR_RUN_TIME   100
#define SENSOR_OUT_TIME   5000
#define SENSOR_WAIT_TIME  10

typedef enum 
{
	MEASURE_START,
	MEASURE_WAIT,
    MEASURE_FINISH
}SensorState_t;

void SensorProc(void);

#endif
