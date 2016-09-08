

#include "mtm_def.h"
#include "stm32f10x.h"

/**
 * @brief  stack or memory overflow 
 */
void HardFault_Handler(void)
{
    __IO uint32_t ReturnAddr;
    ReturnAddr = __get_PSP();
    ReturnAddr = *(uint32_t *)(ReturnAddr + 32);

    while (1){ReturnAddr = ReturnAddr;}
}

/* END OF FILE */
