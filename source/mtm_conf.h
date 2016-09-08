/* vim: set syntax=c: */
#ifndef __MTM_CONF_H__
#define __MTM_CONF_H__

#include "mtm_def.h"

#define MTM_MUTEX_ENABLE
//#define MTM_SEM_ENABLE
//#define MTM_MBOX_ENABLE
//#define MTM_MQUEUE_ENABLE

/* Task number define */
#define MTM_MAX_TASK_NBR            16u

#ifdef MTM_MUTEX_ENABLE
#define MTM_MAX_MUTEX_NBR           8u
#endif

#ifdef MTM_SEM_ENABLE
#define MTM_MAX_SEM_NBR             8u
#endif

#ifdef MTM_MBOX_ENABLE
#define MTM_MAX_MBOX_NBR            8u
#endif

#ifdef MTM_MQUEUE_ENABLE
#define MTM_MAX_MQUEUE_NBR          8u
#endif

#endif /* __MTM_CONF_H__ */
