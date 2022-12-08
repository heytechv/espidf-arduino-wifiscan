#include <xtensa/coreasm.h>
#include <xtensa/corebits.h>
#include <xtensa/config/system.h>
#include "freertos/xtensa_context.h"
#include "esp_private/panic_reason.h"
#include "sdkconfig.h"
#include "soc/soc.h"
#include "soc/gpio_reg.h"
#include "soc/dport_reg.h"

#define L5_INTR_STACK_SIZE  12


.text

.literal PIN_WS2812B, (1<<27)

.literal LED_PIN, (1<<2)
.literal GPIO_REG_W1TS, 0x3FF44008  # set PIN (high)  [REGISTER addr]
.literal GPIO_REG_W1TC, 0x3FF4400C  # clear PIN (low) [REGISTER addr]

// WS2812B times
.literal T0Hx40ns, 10     # 400 ns +- 150 ns -> 10 * 40 ns
.literal T0Lx40ns, 20     # 850 ns +- 150 ns -> 22 * 40 ns (daje 20 bo jeszcze czas wykonania innych instrukcji w C, a LOW zatrzaskuje)
.literal T1Hx40ns, 20     # 800 ns +- 150 ns -> 20 * 40 ns
.literal T1Lx40ns, 10     # 450 ns +- 150 ns -> 12 * 40 ns (daje 10 bo jeszcze czas wykonania innych instrukcji w C, a LOW zatrzaskuje)
.literal TRESET,   2000   # > 50 us          -> 2000 * 40 ns


/**
 * Reset LED matrix
 */
.global WS2812B_RESET
.align 4
WS2812B_RESET:
    entry a1, 32

    l32r a15, GPIO_REG_W1TC
    l32r a14, PIN_WS2812B
    //memw
    s32i a14, a15, 0

    l32r a2, TRESET
    call4 _DELAY_x40ns
    
    retw


/**
 * Set bit to LED matrix
 */
.global WS2812B_SEND_BIT
.align 4
WS2812B_SEND_BIT:
    entry a1, 32

    bnez a2, w2sb_send_1        // a2 != 0 -> jump to "w2sb_send_1"

    w2sb_send_0:                // a2 == 0 -> send 0
    l32r a15, GPIO_REG_W1TS     // SET pin (HIGH)
    l32r a14, PIN_WS2812B       //
    s32i a14, a15, 0            //
                                //
    l32r a2, T0Hx40ns           // wait
    call4 _DELAY_x40ns          //
                                //
    l32r a15, GPIO_REG_W1TC     // CLEAR pin (LOW)    
    l32r a14, PIN_WS2812B       //
    s32i a14, a15, 0            //
                                //
    l32r a2, T0Lx40ns           // wait
    call4 _DELAY_x40ns          //
                                //
    retw                        // return

    w2sb_send_1:                // a2 != 0 -> send 1
    l32r a15, GPIO_REG_W1TS     // SET pin (HIGH)
    l32r a14, PIN_WS2812B       //
    s32i a14, a15, 0            //
                                //
    l32r a2, T1Hx40ns           // wait
    call4 _DELAY_x40ns          //
                                //
    l32r a15, GPIO_REG_W1TC     // CLEAR pin (LOW)
    l32r a14, PIN_WS2812B       //
    s32i a14, a15, 0            //
                                //
    l32r a2, T1Lx40ns           // wait
    call4 _DELAY_x40ns          //
                                //
    retw                        // return

/**
* Local function (works only in asm)
* call it using call4
*
* @param a2 ile razy czekac (4.167 ns) * (4+2+2+2 [szacowana ilosc instrukcji]) = 41.67 ns
*            (zmierzone recznie 39 ns) -> okolo 40 ns
*
* f_crystal = 240 MHz
* 1 okres zegara na 1 cykl maszynowy (w esp32), wiec jeden cykl maszynowy trwa:
*     T = 1s / 240 MHz = 4.16666667e-9 s = 4.167 ns
*/
_DELAY_x40ns:

    dx40ns_loop:
    addi a2, a2, -1
    bnez a2, dx40ns_loop

    jx a4   // return


