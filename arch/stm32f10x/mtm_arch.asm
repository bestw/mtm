;
; Context switch and others
;

    EXPORT mtm_interrupt_disable
    EXPORT mtm_interrupt_enable
    EXPORT mtm_context_switch
    EXPORT mtm_start_first
    EXPORT PendSV_Handler

    EXTERN mtm_next_task
    EXTERN mtm_cur_task
    EXTERN mtm_cur_prio
    EXTERN mtm_highest_prio

NVIC_INT_CTRL   EQU     0xE000ED04
NVIC_SYSPRI14   EQU     0xE000ED20
NVIC_PENDSV_PRI EQU     0x00FF0000
NVIC_PENDSVSET  EQU     0x10000000


    AREA |.text|, CODE, READONLY, ALIGN=2
    THUMB
    REQUIRE8
    PRESERVE8

;
mtm_interrupt_disable       PROC
    MRS     R0, PRIMASK
    CPSID   I
    BX      LR
    ENDP

;
mtm_interrupt_enable        PROC
    MSR     PRIMASK, R0
    BX      LR
    ENDP

;
mtm_context_switch          PROC
    LDR     R0, =NVIC_INT_CTRL
    LDR     R1, =NVIC_PENDSVSET
    STR     R1, [R0]
    BX      LR
    ENDP

;
mtm_start_first             PROC
    LDR     R0, =NVIC_SYSPRI14
    LDR     R1, =NVIC_PENDSV_PRI
    STR     R1, [R0]

    MOVS    R0, #0x00
    MSR     PSP, R0

    LDR     R0, =NVIC_INT_CTRL
    LDR     R1, =NVIC_PENDSVSET
    STR     R1, [R0]
    CPSIE   I

    ENDP

;
PendSV_Handler              PROC
    CPSID   I
    MRS     R0, PSP
    CBZ     R0, PendSV_Handler_Restore      ; If PSP is 0, do not save

    SUBS    R0, R0, #0x20
    STMFD   R0, {R4-R11}
    LDR     R1, =mtm_cur_task               ; mtm_cur_task->stk
    LDR     R1, [R1]
    STR     R0, [R1]

PendSV_Handler_Restore
    LDR     R0, =mtm_highest_prio           ; mtm_cur_prio = mtm_highest_prio
    LDR     R1, =mtm_cur_prio
    LDRB    R2, [R0]
    STRB    R2, [R1]

    LDR     R0, =mtm_cur_task               ; mtm_cur_task = mtm_next_task
    LDR     R1, =mtm_next_task
    LDR     R2, [R1]
    STR     R2, [R0]

    LDR     R0, [R2]
    LDMFD   R0, {R4-R11}
    ADDS    R0, R0, #0x20
    MSR     PSP, R0
    ORR     LR, LR, #0x04
    CPSIE   I
    BX      LR

    ENDP

    END
