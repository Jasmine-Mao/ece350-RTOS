#include "stm32f4xx_it.h"
#include "common.h"
#include "main.h"

U32* MSP_INIT_VAL;
//= *(U32**)0x0;
// ^gets set when kernel init runs
TCB* current_task;
int initialized = 0;
int first_run = 1;

int states[MAX_TASKS] = {0};      // size of the array is defined by max tasks in main.c; 0 if not taken, 1 if taken
// state 0 -> dormant
// state 1 -> available
// state 2 -> active
// state 3 ->

TCB task_queue[MAX_TASKS];


int stack_starting_address(int thread_number){
  int our_address_value;
  if(thread_number == 1){
    return (int)MSP_INIT_VAL - MAIN_STACK_SIZE;
  }
  else{
    int retrieved_address_value = stack_starting_address(thread_number--);
    our_address_value = retrieved_address_value - THREAD_STACK_SIZE;
  }
  return our_address_value;
}

void assign_TID(TCB* task){
  // iterate through the atomic array
  for(int i = 1; i < 16; i++){
    if(states[i] == 0){
      task->tid = i;
      states[i] = 1;
      return;
    }
  }
}

int is_empty(){
	for (int i = 0; i < 16; i++){
		if (states[i]) return 0;
	}
	return 1;
}

TCB * find_TCB(task_t tid_input){
	for (int i = 1; i < MAX_TASKS; i++) {
		if (task_queue[i].tid == tid_input)
			return &task_queue[i];
	}
	printf("uh oh!");
	return NULL;
}

void scheduler(){
  if(!is_empty()){
    current_task = find_TCB(current_task->next);         // pop off a task from the task queue and set that as the currently running task
    __set_PSP(current_task->stack_high);
  }
}

void osKernelInit(void){
  //initialize variables
  MSP_INIT_VAL = *(U32**)0x0;
  current_task = NULL;

  initialized = 1;
  osKernelStart();
}

int osKernelStart(void){
  if (first_run && initialized){
    first_run = 0;
    SVC_Handler();
  }
  else return RTX_ERR;
}

int osTaskInfo(task_t TID, TCB* task_copy){
  if (TID < 0 || TID >= MAX_TASKS) { //check if the TID is valid
        return RTX_ERR;
	  for (int i = 1; i < MAX_TASKS; i++){
		  if(task_queue[i].tid == TID){ //if the TID matches, copy the task info
		        task_copy = &task_queue[i];
		        return RTX_OK;
		      }
	  }
  return RTX_ERR;
  }
}

task_t getTID (void){
  if (!first_run && initialized){
    return current_task->tid;
  }
  else return RTX_ERR;
}



int osTaskExit(void){

}

//
//int osCreateTask(TCB* task){
//  //register with RTX
//  assign_TID(task);
//  task->stack_high = stack_starting_address(task->TID);
//  int stackptr = task->stack_high;
//  *(--stackptr) = 1<<24; //This is xPSR, setting the chip to Thumb mode
//  *(--stackptr) = (uint32_t)print_continuously; //the function name
//  for (int i = 0; i < 14; i++) *(--stackptr) = 0xA; //An arbitrary number, repeat this 14 times in total
//
//}
void osYield(void){


}