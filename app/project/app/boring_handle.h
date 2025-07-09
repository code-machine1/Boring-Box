#ifndef _BORING_HANDLE_H
#define _BORING_HANDLE_H

#include "at32f421.h"
#include "stdbool.h"

typedef enum
{
    OPEN_LID= 0,
    CLOSE_LID,

}lid_state_e;

typedef enum
{
    PUSH_ROD= 0,
    PULL_ROD,

}push_rod_state_e;

typedef enum
{
   LID_MOTOR = 0,
   ROD_MOTOR,

}motor_select_e;

typedef struct
{
	uint8_t lid_open_speed;
	uint8_t lid_close_speed;

	uint8_t rod_push_speed;
	uint8_t rod_pull_speed;
	
	motor_select_e motor;

}motor_set_t;

typedef enum
{
    
}
emotional_state_e;

typedef struct box
{
	emotional_state_e emotional;
	
	lid_state_e lid_state;
    push_rod_state_e push_rod_state;
	motor_set_t motor_set;
	
	void (* open_lid)(struct box * p_box); 
    void (* close_lid)(struct box * p_box);
	
	void (* push_lid)(struct box * p_box); 
    void (* pull_lid)(struct box * p_box);	
	
	void (*box_init)(struct box* p_box);
	

}box,*p_box ;


extern box boring_box;
#endif