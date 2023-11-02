/**
 ******************************************************************************
 * @file    CortexM3/BitBand/stm32f10x_it.c
 * @author  MCD Application Team
 * @version V3.1.2
 * @date    09/28/2009
 *  Main Interrupt Service Routines.
 *          This file provides template for all exceptions handler and peripherals
 *          interrupt service routine.
 ******************************************************************************
 * @copy
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
 */

/******************************************************************************/
#include "newlib.h"
#include "vsn.h"
#include "stm32f10x_it.h"
#include "vsntime.h"
#include "vsnusart.h"
#include "vsnpm.h"
#include "vsnsd.h"
#include "vsnsetup.h"
#include "vsnledind.h"
#include "vsnresetbutton.h"
#include "uart1.h"

extern void at86rf2xx_isr(void);
extern void at86rf215_isr(void);
extern void contiki_rtimer_isr(void);
extern void clock_interrupt_handler(void);

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/
/**
 * This function handles NMI exception.
 * @param  None
 * @return None
 */
void NMI_Handler(void)
{
	/* This interrupt is generated when HSE clock fails, nothing can be printed here because the
	 * SysClock has changed */
	if (RCC_GetITStatus(RCC_IT_CSS) != RESET) {
		/* At this stage: HSE, PLL are disabled (but no change on PLL config) and HSI
	       is selected as system clock source */
		/* Enable HSE */
		RCC_HSEConfig(RCC_HSE_ON);
		/* Enable HSE Ready interrupt */
		RCC_ITConfig(RCC_IT_HSERDY, ENABLE);
        /* Enable PLL Ready interrupt */
		RCC_ITConfig(RCC_IT_PLLRDY, ENABLE);
		/* Clear Clock Security System interrupt pending bit */
		RCC_ClearITPendingBit(RCC_IT_CSS);
		/* Once HSE clock recover, the HSERDY interrupt is generated and in the RCC ISR
		 routine the system clock will be reconfigured to its previous state (before
		 HSE clock failure) */
		/* TODO if HSE fails completely we have to reinitialize clock dependent drivers or restart the system */
	}
}
/******************************************************************************/
unsigned int faultStack[51];
unsigned int *stackPointer;
/**
 * This function handles Hard Fault exception.
 * Useful documents for debugging Hard Faults: 	Application Note 209: Using Cortex-M3 and Cortex-M4 Fault Exceptions (apnt209.pdf)
 * 												Cortex-M3 Devices Generic User Guide (DUI0552A_cortex_m3_dgug.pdf)
 * When Hard Fault occurs some system registers are pushed to stack. The Hard Fault Handler
 * saves the stack pointer and creates a new fault stack in case the stack pointer or
 * the stack is corrupted. The registers that are pushed to stack are printed to debug
 * port along with the fault status registers
 * @param  None
 * @return None
 *
 * @TODO Implement the HardFault_Handler for the process stack (PSP), check actual fault stack size needed, maybe reset MCU after hard fault
 */
void HardFault_Handler(void)
{
	asm (	"mrs	%[origStackPtr], msp\n\t"		/* Get the stack pointer */
			"msr	msp, %[newStackPtr]\n\t"     	/* Set the new stack MSP to fault_stack */
			"push	{r7, lr}\n\t"					/* Start the ISR on the new stack */
			"add 	r7, sp, #0\n\t"
			:[origStackPtr] "=r" (stackPointer)
			:[newStackPtr] "r" (&(faultStack[51]))
		);
	/* Disable all interrupts */
	__disable_irq();
	/* Disable both DMAs */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, DISABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, DISABLE);

	debug_str("\r\nHARD FAULT EXCEPTION");
	debug_str("\r\nR0 =  ");
	debug_hex(*(stackPointer + STACKED_R0_OFFSET), 8);
	debug_str("\r\nR1 = ");
	debug_hex(*(stackPointer + STACKED_R1_OFFSET), 8);
	debug_str("\r\nR2 = ");
	debug_hex(*(stackPointer + STACKED_R2_OFFSET), 8);
	debug_str("\r\nR3 = ");
	debug_hex(*(stackPointer + STACKED_R3_OFFSET), 8);
	debug_str("\r\nR12 = ");
	debug_hex(*(stackPointer + STACKED_R12_OFFSET), 8);
	debug_str("\r\nLR = ");
	debug_hex(*(stackPointer + STACKED_LR_OFFSET), 8);
	debug_str("\r\nPC = ");
	debug_hex(*(stackPointer + STACKED_PC_OFFSET), 8);
	debug_str("\r\nPSR = ");
	debug_hex(*(stackPointer + STACKED_PSR_OFFSET), 8);


	debug_str("\r\nBFAR = ");
	debug_hex((*((volatile unsigned long *)(0xE000ED38))),8);
	debug_str("\r\nCFSR = ");
	debug_hex((*((volatile unsigned long *)(0xE000ED28))),8);
	debug_str("\r\nHFSR = ");
	debug_hex((*((volatile unsigned long *)(0xE000ED2C))),8);
	debug_str("\r\nDFSR = ");
	debug_hex((*((volatile unsigned long *)(0xE000ED30))),8);
	debug_str("\r\nAFSR = ");
	debug_hex((*((volatile unsigned long *)(0xE000ED3C))),8);

	/* Go to infinite loop when Hard Fault exception occurs */
	while (1)
	{
		NVIC_SystemReset();
	}
}
/******************************************************************************/
/**
 * This function handles Memory Manage exception.
 * @param  None
 * @return None
 */
void MemManage_Handler(void)
{
	/* Disable all interrupts */
	__disable_irq();
	/* Disable both DMAs */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, DISABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, DISABLE);

	debug_str("\r\nMEMORY MANAGE EXCEPTION");
	/* Go to infinite loop when Memory Manage exception occurs */
	while (1)
	{
	}
}
/******************************************************************************/
/**
 * This function handles Bus Fault exception.
 * @param  None
 * @return None
 */
void BusFault_Handler(void)
{
	/* Disable all interrupts */
	__disable_irq();
	/* Disable both DMAs */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, DISABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, DISABLE);
	debug_str("\r\nBUS FAULT EXCEPTION");
	/* Go to infinite loop when Bus Fault exception occurs */
	while (1)
	{
	}
}
/******************************************************************************/
/**
 * This function handles Usage Fault exception.
 * @param  None
 * @return None
 */
void UsageFault_Handler(void)
{
	/* Disable all interrupts */
	__disable_irq();
	/* Disable both DMAs */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, DISABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, DISABLE);
	debug_str("\r\nUSGAGE FAULT EXCEPTION");
	/* Go to infinite loop when Usage Fault exception occurs */
	while (1)
	{
	}
}
/******************************************************************************/
/**
 * This function handles SVCall exception.
 * @param  None
 * @return None
 */
void SVC_Handler(void)
{
}
/******************************************************************************/
/**
 * This function handles Debug Monitor exception.
 * @param  None
 * @return None
 */
void DebugMon_Handler(void)
{
}
/******************************************************************************/
/**
 * This function handles PendSV_Handler exception.
 * @param  None
 * @return None
 */
void PendSV_Handler(void)
{
}
/******************************************************************************/
/**
 * This function handles SysTick interrupt.
 * @param  None
 * @return None
 */
void SysTick_Handler(void)
{
	vsnLEDInd_toggle();
	vsnTime_uptimeIsr();
    clock_interrupt_handler(); // rename to contiki_clock_isr
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/******************************************************************************/
/**
 * This function handles RTC Second interrupt.
 * @param  None
 * @return None
 */
void RTC_IRQHandler(void)
{
	// RTC Second IRQ handler
	if (RTC_GetITStatus(RTC_IT_SEC) != RESET) {
		vsnSetup_measureHsiFreq();
		vsnTime_rtcSecondIsr();
		// Clear the RTC Second interrupt flag
		RTC_WaitForLastTask();
		RTC_ClearITPendingBit(RTC_IT_SEC);
	}
}
/******************************************************************************/
/**
 * This function handles USART1 interrupt.
 * @param  None
 * @return None
 */
void USART1_IRQHandler(void)
{
	vsnUSART_usart1Isr();

	//contiki_uart1_isr(); // it just informs process
}
/******************************************************************************/
/**
 * This function handles SPI1 interrupt Handler.
 * @param  None
 * @return None
 */
void SPI1_IRQHandler(void)
{
#ifdef SPI1_DRIVER_MODE_INTERRUPT
	vsnSPI1_processSpiIrq();
#endif
#ifdef SPI1_NEW_DRIVER_MODE_INTERRUPT
	vsnSPILowx_processIrq(SPI1);
#endif
}
/******************************************************************************/
/**
 * This function handles DMA1 Channel2 Handler.
 * @param  None	DMA1_Channel2
 * @return None  vsnfram_processdmarxirq();
 */
void DMA1_Channel2_IRQHandler(void)
{
#ifdef SPI1_DRIVER_MODE_DMA
	vsnSPI1_processDmaRxIrq();
#endif

#ifdef SPI1_NEW_DRIVER_MODE_DMA
	vsnSPILowx_dmaRxIrq(SPI1);
#endif
}
/******************************************************************************/
/**
 * This function handles DMA1 Channel3 Handler.
 * @param  None	DMA1_Channel3
 * @return None
 */
void DMA1_Channel3_IRQHandler(void)
{
#ifdef SPI1_DRIVER_MODE_DMA
	vsnSPI1_processDmaTxIrq();
#endif

#ifdef SPI1_NEW_DRIVER_MODE_DMA
	vsnSPILowx_dmaTxIrq(SPI1);
#endif
}
/******************************************************************************/
/**
 * This function handles DMA1 Channel4 Handler.
 * @param  None	DMA1_Channel4
 * @return None
 */
void DMA1_Channel4_IRQHandler(void)
{
#ifdef USART1_DMA_MODE
	vsnUSART_dmaTxUsart1Isr();
#endif
}
/******************************************************************************/
/**
 * This function handles DMA1 Channel5 Handler.
 * @param  None	DMA1_Channel5
 * @return None
 */
void DMA1_Channel5_IRQHandler(void)
{
#ifdef USART1_DMA_MODE
	vsnUSART_dmaRxUsart1Isr();
#endif
}
/******************************************************************************/
/**
 * This function handles External interrupt line 2.
 * @param  None
 * @return None
 */
// void EXTI2_IRQHandler(void) 
// {
// }
/******************************************************************************/
/** TODO:
 * Interrupt handler sources are board specific - we could move 
 * this file to boards supported by VESNA, each board
 * containing its file with EXTI IRQ Handler configuration
 */
void EXTI3_IRQHandler(void) {
	#if (BOARD_SNE_ISMTV_V1_0 || BOARD_SNE_ISMTV_V1_1)
		if (EXTI_GetITStatus(EXTI_Line3) != RESET) {
			EXTI_ClearITPendingBit(EXTI_Line3);
			
			at86rf2xx_isr();
		}
	#endif
}
/******************************************************************************/
/**
 * This function handles External interrupt lines 5 thru 9.
 * @param  None
 * @return None
 */
void EXTI9_5_IRQHandler(void) {
	#if BOARD_SNR
		if (EXTI_GetITStatus(EXTI_Line9) != RESET) {
			EXTI_ClearITPendingBit(EXTI_Line9);
			at86rf2xx_isr();
		}
	#endif
	/* Interrupt routine for cc1101 */
	/* Is GDO0 line activated? */
	/* line 9, for reset button */
	if(EXTI_GetITStatus(EXTI_RESETBUTTON) != RESET){
		/* reset button pressed */
		//button_sensor_interrupt_handler();
		//resetbutton_sensor_handleInterrupt();
		/* clear interrupt */
		EXTI_ClearITPendingBit(EXTI_RESETBUTTON);
	}
}
/******************************************************************************/
/**
 * This function handles TIM4 interrupts.
 * @param  None
 * @return None
 */
//void TIM4_IRQHandler(void) 
// {	
// }
/******************************************************************************/
/**
 * This function handles TIM4 interrupts.
 * @param  None
 * @return None
 */
// void TIM2_IRQHandler(void)
// {
// 	   contiki_rtimer_isr();
// }
/******************************************************************************/
/**
 * This function handles TIM5 interrupts.
 * @param  None
 * @return None
 */
void TIM5_IRQHandler(void)
{
    contiki_rtimer_isr();
}
/******************************************************************************/
/**
 * This function handles RCC interrupt request.
 * @param  None
 * @return None
 */
void RCC_IRQHandler(void) {
	if (RCC_GetITStatus(RCC_IT_HSERDY) != RESET) {
		/* Clear HSERDY interrupt pending bit */
		RCC_ClearITPendingBit(RCC_IT_HSERDY);
		/* Check if the HSE clock is still available */
		if (RCC_GetFlagStatus(RCC_FLAG_HSERDY) != RESET) {
			/* Select HSE as system clock source */
			RCC_SYSCLKConfig(RCC_SYSCLKSource_HSE);
			/* Enable PLL: once the PLL is ready the PLLRDY interrupt is generated */
			RCC_PLLCmd(ENABLE);
		}
	}
	if (RCC_GetITStatus(RCC_IT_PLLRDY) != RESET) {
		/* Clear PLLRDY interrupt pending bit */
		RCC_ClearITPendingBit(RCC_IT_PLLRDY);
		/* Check if the PLL is still locked */
		if (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) != RESET) {
			/* Select PLL as system clock source */
			RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
			/* Disable HSE Ready interrupt */
			RCC_ITConfig(RCC_IT_HSERDY, DISABLE);
			/* Disable PLL Ready interrupt */
			RCC_ITConfig(RCC_IT_PLLRDY, DISABLE);
		}
	}
}
/******************************************************************************/
