/*
    Task_mech.c

    Task mechanism
*/

#include <LPC214x.H>
#include <stdio.h>
#include "Type.h"

#include "Task_mech.h"
#include "Task_list.h"

#define ISR_CYCLE 1000 //ISR @ every ISR_CYCLE useconds
void T0isr (void)    __irq;

typedef struct _task_struct
{
    uint32 period;
    void (*name)(void);
    uint8 enable;
    volatile uint32 counter;
} task_struct;

/*
    Tasks config
*/
//====================================================
#define NR_TASKS 2

task_struct task[NR_TASKS] = 
{
    {100, hall_timeout, 0, 0},
    {500, my_task1, 0, 0}
};

//====================================================

int32 task_config(uint8 nr, uint8 state)
{
    if (nr < NR_TASKS)
    {
        task[nr].enable = state;
        if (task[nr].enable == 0) task[nr].counter = 0;
        return 0;
    }

    return -1;
}

int32 task_enable(uint8 nr)
{
    if (nr < NR_TASKS)
    {
        task[nr].enable = 1;
        return 0;
    }

    return -1;
}

int32 task_disable(uint8 nr)
{
    if (nr < NR_TASKS)
    {
        task[nr].enable = 0;
        task[nr].counter = 0;
        return 0;
    }

    return -1;
}

int32 task_set_period(uint8 nr, uint32 period)
{
    if (nr < NR_TASKS)
    {
        task[nr].period = period;
        return 0;
    }

    return -1;
}

void task_init(void)
{
    T0PR  = 0x0000000E;                  //Load prescaler (14 + 1). PCLK / 15 = 1 MHz

    T0TCR = 0x00000002;                  //Reset counter and prescaler
    T0MCR = 0x00000003;                  //On match reset the counter and generate an interrupt
    T0MR0 = ISR_CYCLE;                   //Set the cycle time
    
    T0TCR = 0x00000001;                  //Enable timer

    VICVectAddr2 = (uint32) T0isr;     //Set the timer ISR vector address
    VICVectCntl2 = 0x00000020 | 4;       //Set channel for TIMER0
    VICIntEnable |= 0x00000010;          //Enable the interrupt TIMER1
}

void T0isr (void)	__irq
{
    uint8 i;

    for (i = 0; i < NR_TASKS; i++)
        if (task[i].enable)
        {
            task[i].counter++;
            if (task[i].counter >= task[i].period)
            {
                task[i].counter = 0;
                (*task[i].name)();
            }
        }

    T0IR |= 0x00000001;            //Clear match 0 interrupt
    VICVectAddr = 0x00000000;      //Dummy write to signal end of interrupt
}
