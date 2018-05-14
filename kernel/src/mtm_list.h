
#ifndef __MTM_LIST_H__
#define __MTM_LIST_H__

#include "mtm_def.h"

struct mtm_list_t {
    struct mtm_list_t *next;
    struct mtm_list_t *prev;
};


#define mtm_list_entry(list_ptr, type, member)  \
    (  (type *)( (char *)(list_ptr) - (unsigned long)( &((type *)0)->member ) )  )

mtm_inline static void mtm_list_init(struct mtm_list_t *list)
{
    list->next = list;
    list->prev = list;
}

mtm_inline static int mtm_list_empty(struct mtm_list_t *list)
{
    int retval;
    if (list->next == list)
    {
        retval =  MTM_TRUE;
    }
    else
    {
        retval = MTM_FALSE;
    }
    return (retval);
}

mtm_inline static void __mtm_list_add(struct mtm_list_t *insert,
                                      struct mtm_list_t *prev,
                                      struct mtm_list_t *next)
{
    prev->next   = insert;
    next->prev   = insert;
    insert->prev = prev;
    insert->next = next;
}


mtm_inline static void mtm_list_add2tail(struct mtm_list_t *insert, struct mtm_list_t *curr)
{
    __mtm_list_add(insert, curr->prev, curr);
}


mtm_inline static void mtm_list_add2head(struct mtm_list_t *insert, struct mtm_list_t *curr)
{
    __mtm_list_add(insert, curr, curr->next);
}


mtm_inline static void mtm_list_del(struct mtm_list_t *list)
{
    list->prev->next = list->next;
    list->next->prev = list->prev;

    list->next = list->prev = list;
}

#endif /* __MTM_LIST_H__ */

/* END OF FILE */
