

#ifndef __MTM_ARCH_H__
#define __MTM_ARCH_H__

#include "mtm_def.h"

stack_t *mtm_stack_init(void    (*task_entry)(void *param),
                        void    *param,
                        stack_t *stk_addr,
                        void    *task_return);

#define MTM_INTERRUPT_DISABLE() {sr=mtm_interrupt_disable();}
#define MTM_INTERRUPT_ENABLE()  {mtm_interrupt_enable(sr);}


void    mtm_interrupt_enable(sr_type sr);
sr_type mtm_interrupt_disable(void);

void mtm_context_switch(void);
void mtm_start_first(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void mtm_systick_init(uint32_t cnts);

void HardFault_Handler(void);

#endif /* __MTM_ARCH_H__ */
