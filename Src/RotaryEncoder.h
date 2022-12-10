/**
 * @file RotaryEncoder.h
 * @author Ali Mirghasemi (ali.mirghasemi1376@gmail.com)
 * @brief This libary help you to work with rotary encoders
 * @version 0.1
 * @date 2022-12-10
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _ROTARY_ENCODER_H_
#define _ROTARY_ENCODER_H_


#ifdef __cpluplus
extern "C" {
#endif

#include <stdint.h>

/******************************************************************************/
/*                                Configuration                               */
/******************************************************************************/

/**
 * @brief Enable feature for process index of encoder
 * 
 */
#define ROTARY_ENCODER_INDEX            1
/**
 * @brief Enable user arguments in RotaryEncoder
 * 
 */
#define ROTARY_ENCODER_ARGS             1
/**
 * @brief Enable IO field in PinConfig
 */
#define ROTARY_ENCODER_IO               1
/**
 * @brief Enable Init Pin function in driver
 */
#define ROTARY_ENCODER_INIT             1
/**
 * @brief Enable Init IRQ function in driver
 */
#define ROTARY_ENCODER_INIT_IRQ         1
/**
 * @brief Enable deinit function in driver
 */
#define ROTARY_ENCODER_DEINIT           0
/**
 * @brief Define maximum number of encoder, -1 for unlimited with linked list
 */
#define ROTARY_ENCODER_MAX_NUM          -1

#if ROTARY_ENCODER_IO
/**
 * @brief Rotary encoder IO type, user can change it
 */
typedef void* RotaryEncoder_IO;
#endif
/**
 * @brief Rotary encoder Pin type
 */
typedef uint16_t RotaryEncoder_Pin;
/**
 * @brief Rotary encoder index type
 */
typedef int16_t RotaryEncoder_Index;

/******************************************************************************/

#define ROTARY_ENCODER_NULL            ((RotaryEncoder*) 0)
#define ROTARY_ENCODER_CONFIG_NULL     ((RotaryEncoder_PinConfig*) 0)

/* Predefined data types */
struct _RotaryEncoder;
typedef struct _RotaryEncoder RotaryEncoder;

/**
 * @brief Shows which direction encoder going
 */
typedef enum {
    RotaryEncoder_Direction_ClockWise       = 0,
    RotaryEncoder_Direction_AntiClockWise   = 1,
} RotaryEncoder_Direction;

typedef struct {
#if ROTARY_ENCODER_IO
    RotaryEncoder_IO        IO;
#endif
    RotaryEncoder_Pin       Pin;
} RotaryEncoder_PinConfig;

typedef struct {
    RotaryEncoder_PinConfig Data;
    RotaryEncoder_PinConfig Clock;
} RotaryEncoder_Pins;

typedef struct {
    RotaryEncoder_Index     Min;
    RotaryEncoder_Index     Max;
} RotaryEncoder_IndexRange;

/**
 * @brief initialize pin in input mode, remember if your pin is pull-up, or pull-down 
 * must configured in init function
 * this function call when new RotaryEncoder add to queue
 */
typedef void (*RotaryEncoder_InitPinFn)(const RotaryEncoder_PinConfig* config);
/**
 * @brief this function must return value of a pin
 * 0 -> LOW, 1 -> HIGH
 */
typedef uint8_t (*RotaryEncoder_ReadPinFn)(const RotaryEncoder_PinConfig* config);
/**
 * @brief de-initialize pin and change pin to reset mode
 * this function call on remove RotaryEncoder
 */
typedef void (*RotaryEncoder_DeInitPinFn)(const RotaryEncoder_PinConfig* config);
/**
 * @brief this function initialize falling or rising edge interrupt
 */
typedef void (*RotaryEncoder_InitIRQFn)(const RotaryEncoder_PinConfig* config);

/**
 * @brief Rotary Encoder Driver
 * Hold necessary functions
 */
typedef struct {
    RotaryEncoder_ReadPinFn         readPin;
#if ROTARY_ENCODER_INIT_IRQ
    RotaryEncoder_InitPinFn         initPin;
#endif
#if ROTARY_ENCODER_INIT_IRQ
    RotaryEncoder_InitIRQFn         initIrq;
#endif
#if ROTARY_ENCODER_DEINIT
    RotaryEncoder_DeInitPinFn       deInitPin;
#endif
} RotaryEncoder_Driver;

/**
 * @brief Rotary encoder callback, call when encoder change
 */
typedef void (*RotaryEncoder_CallbackFn)(RotaryEncoder* encoder, RotaryEncoder_Direction direction);


struct _RotaryEncoder {
#if ROTARY_ENCODER_MAX_NUM < 0
    struct _RotaryEncoder*          Previous;
#endif // ROTARY_ENCODER_MAX_NUM
#if ROTARY_ENCODER_ARGS
    void*                           Args;
#endif // ROTARY_ENCODER_ARGS
    const RotaryEncoder_Pins*       Config;
    RotaryEncoder_CallbackFn        onChange;
#if ROTARY_ENCODER_INDEX
    RotaryEncoder_IndexRange        Range;
    RotaryEncoder_Index             Index;
#endif
    uint8_t                         LastDirection   : 1;
    uint8_t                         NotActive       : 1;    			/**< show other states will be ignore or not */
    uint8_t                         IsClockWise     : 1;    			/**< this parameters use only when Activestate Enabled */
    uint8_t                         Configured      : 1;                /**< this flag shows RotaryEncoder is configured or not, just useful fo fixed RotaryEncoder num */
    uint8_t                         Enabled         : 1;                /**< check this flag in irq */
    uint8_t                         Reserved        : 3;
};

void RotaryEncoder_init(const RotaryEncoder_Driver* driver);
void RotaryEncoder_handle(void);

void RotaryEncoder_setConfig(RotaryEncoder* encoder, const RotaryEncoder_Pins* config);
const RotaryEncoder_Pins* RotaryEncoder_getConfig(RotaryEncoder* encoder);

#if ROTARY_ENCODER_MAX_NUM > 0
    RotaryEncoder* RotaryEncoder_new(void);
#endif // ROTARY_ENCODER_MAX_NUM

uint8_t  RotaryEncoder_add(RotaryEncoder* encoder, const RotaryEncoder_Pins* config);
uint8_t  RotaryEncoder_remove(RotaryEncoder* remove);
RotaryEncoder*  RotaryEncoder_find(const RotaryEncoder_Pins* config);

void RotaryEncoder_onChange(RotaryEncoder* RotaryEncoder, RotaryEncoder_CallbackFn cb);

#if ROTARY_ENCODER_INDEX
    void RotaryEncoder_setRange(RotaryEncoder* encoder, RotaryEncoder_Index min, RotaryEncoder_Index max);
    void RotaryEncoder_setIndex(RotaryEncoder* encoder, RotaryEncoder_Index index);
    RotaryEncoder_Index RotaryEncoder_getIndex(RotaryEncoder* encoder);
#endif

#if ROTARY_ENCODER_ENABLE_FLAG
    void RotaryEncoder_setEnabled(RotaryEncoder* encoder, uint8_t enabled);
    uint8_t RotaryEncoder_isEnabled(RotaryEncoder* encoder);
#endif

#if ROTARY_ENCODER_ARGS
    void  RotaryEncoder_setArgs(RotaryEncoder*, void* args);
    void* RotaryEncoder_getArgs(RotaryEncoder* encoder);
#endif


/* Helper macros */
#if ROTARY_ENCODER_IO
    #define ROTARY_ENCODER_INIT_PIN(IO, PIN)        {(IO), (PIN)}
#else
    #define ROTARY_ENCODER_INIT_PIN(IO, PIN)        {(PIN)}
#endif

#define ROTARY_ENCODER_INIT_PINS(DATA, CLK)         {(DATA), (CLK)}

#ifdef __cpluplus
};
#endif

#endif // _ROTARY_ENCODER_H_
