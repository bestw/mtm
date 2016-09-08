
#ifndef __MTM_DEF_H__
#define __MTM_DEF_H__

#include <stdint.h>

typedef uint32_t    stack_t;            /* Stack type        */
typedef uint32_t    sr_type;            /* Sys register type */
typedef uint32_t    tick_t;             /* System tick type  */

/* mtm_inline define */
#if defined (__CC_ARM)                  /* ARM CC  */
    #define mtm_inline          __inline
#elif defined (__ICCARM__)              /* IAR CC  */
    #define mtm_inline          inline
#elif defined (__GNUC__)                /* GNC GCC */
    #define mtm_inline          inline
#endif

#define MTM_TRUE    1
#define MTM_FALSE   0
#define MTM_NULL    0
#define MTM_PNULL   (void *)0


/* Tick configure */
#define MTM_TICKS_PER_SECOND    100
#define MTM_TICK_MAX            UINT32_MAX

#endif /* __MTM_DEF_H__ */

/* END OF FILE */
