/**
 *
 * @file: steppermotor.h
 * @description: stepper motor  API and data structure
 * @author: infinite.ft
 * @version: 0.0.1
 * @create_at: 2017/05/21
 * @update_at: 2017/05/21
 *
 */


#ifndef __STEPPERMOTOR_H
#define __STEPPERMOTOR_H


#ifdef __cplusplus
extern "C"
{
#endif


#include "stm32f10x.h"


typedef enum
{
	CLOCKWISE = 0,
	ANTI_CLOCKWISE = 1,
}MotorDirection_t;

struct STEPPER_MOTOR_HANDLE_T;
typedef struct STEPPER_MOTOR_HANDLE_T
{
	GPIO_TypeDef * pxPort;
	uint8_t ucPinIndex[4];
	// step counter
	int32_t lStepCnt;
	// aim step
	int32_t lAimStep;
	// step encoding table offset
	uint32_t ulStepTblOff;
	// step period
	uint32_t ulStepPrd;
	// step period counter
	uint32_t ulStepPrdCnt;
	// start rotation switch
	uint32_t ulStartRotation;
	// next motor handle pointer
	struct STEPPER_MOTOR_HANDLE_T * pxNext;
	// rotation direction
	MotorDirection_t eDirection;

}StepperMotorHandle_t;


int32_t steppermotor_Initiate(StepperMotorHandle_t * pxHandle, GPIO_TypeDef * pxPort, uint32_t ulPinIndexMask);
void steppermotor_Start(StepperMotorHandle_t * pxHandle);
void steppermotor_Stop(StepperMotorHandle_t * pxHandle);
void steppermotor_SetRotationDir(StepperMotorHandle_t * pxHandle, MotorDirection_t eDirection);
void steppermotor_ReverseRotationDir(StepperMotorHandle_t * pxHandle);
void steppermotor_RotateNStep(StepperMotorHandle_t * pxHandle, int32_t lStepNum);
void steppermotor_RotateNCircle(StepperMotorHandle_t * pxHandle, int32_t lCircleNum);


#ifdef __cplusplus
}
#endif


#endif
