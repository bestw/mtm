

#include "mtm_def.h"

stack_t *mtm_stack_init(void    (*task_entry)(void *param),
                        void    *param,
                        stack_t *stk_addr,
                        void    *task_return)
{
    stack_t *stk;

    stk = stk_addr;

    *(stk)      = (uint32_t)0x01000000;     /* xPSR            */
    *(--stk)    = (uint32_t)task_entry;     /* Entry Point, PC */
    *(--stk)    = (uint32_t)task_return;    /* LR (R14)        */
    *(--stk)    = (uint32_t)0x0;            /* R12             */
    *(--stk)    = (uint32_t)0x0;            /* R3              */
    *(--stk)    = (uint32_t)0x0;            /* R2              */
    *(--stk)    = (uint32_t)0x0;            /* R1              */
    *(--stk)    = (uint32_t)param;          /* R0 : arguement  */

    *(--stk)    = (uint32_t)0x0;            /* R11             */
    *(--stk)    = (uint32_t)0x0;            /* R10             */
    *(--stk)    = (uint32_t)0x0;            /* R9              */
    *(--stk)    = (uint32_t)0x0;            /* R8              */
    *(--stk)    = (uint32_t)0x0;            /* R7              */
    *(--stk)    = (uint32_t)0x0;            /* R6              */
    *(--stk)    = (uint32_t)0x0;            /* R5              */
    *(--stk)    = (uint32_t)0x0;            /* R4              */

    return (stk);
}

/* END OF FILE */
