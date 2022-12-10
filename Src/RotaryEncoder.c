#include "RotaryEncoder.h"

/* private variables */
static const RotaryEncoder_Driver* encoderDriver;
#if ROTARY_ENCODER_MAX_NUM == -1
    static RotaryEncoder* lastEncoder = ROTARY_ENCODER_NULL;

    #define __encoders()        lastEncoder
    #define __next(S)           S = (S)->Previous
#else
    static RotaryEncoder encoders[ROTARY_ENCODER_MAX_NUM] = {0};

    #define __encoders()        encoders
    #define __next(S)           S++
#endif // ROTARY_ENCODER_MAX_NUM == -1

#if ROTARY_ENCODER_INIT
    #define __initPin(X)        if (encoderDriver->initPin) { encoderDriver->initPin((X)); }
#else
    #define __initPin(X)        
#endif

#if ROTARY_ENCODER_INIT_IRQ
    #define __initIrq(X)        if (encoderDriver->initIrq) { encoderDriver->initIrq((X)); }
#else
    #define __initIrq(X)        
#endif

#if ROTARY_ENCODER_DEINIT
    #define __deInitPin(X)      if (encoderDriver->deInitPin) { encoderDriver->deInitPin((X)); }
#else
    #define __deInitPin(X)        
#endif



/**
 * @brief use for initialize
 * 
 * @param driver 
 */
void RotaryEncoder_init(const RotaryEncoder_Driver* driver) {
    encoderDriver = driver;
}
/**
 * @brief user must place it in timer with 20ms ~ 50ms 
 * all of callbacks handle and fire in this function
 */
void RotaryEncoder_handle(void) {
    RotaryEncoder_Direction direction;
    RotaryEncoder* pEncoder = __encoders();
#if ROTARY_ENCODER_MAX_NUM == -1
    while (ROTARY_ENCODER_NULL != pEncoder) {
#else
    uint8_t len = ROTARY_ENCODER_MAX_NUM;
    while (len-- > 0) {
        if (pEncoder->Configured) {
#endif
    #if ROTARY_ENCODER_ENABLE_FLAG
        if (pEncoder->Enabled) {
    #endif // ROTARY_ENCODER_ENABLE_FLAG
            // update direction
            direction = (RotaryEncoder_Direction) (encoderDriver->readPin((const RotaryEncoder_PinConfig*) &pEncoder->Config->Data) == encoderDriver->readPin((const RotaryEncoder_PinConfig*) &pEncoder->Config->Clock));
        #if ROTARY_ENCODER_INDEX
            pEncoder->Index += direction ? -1 : 1;
            if (pEncoder->Index > pEncoder->Range.Max) {
                pEncoder->Index = pEncoder->Range.Min;
            }
            else if (pEncoder->Index < pEncoder->Range.Min) {
                pEncoder->Index = pEncoder->Range.Max;
            }
        #endif
            if (pEncoder->onChange) {
                pEncoder->onChange(pEncoder, direction);
            }
            pEncoder->LastDirection = direction;
        }
    #if ROTARY_ENCODER_ENABLE_FLAG
        }
    #endif // RotaryEncoder_ENABLE_FLAG
    #if ROTARY_ENCODER_MAX_NUM != -1
        }
    #endif // RotaryEncoder_ENABLE_FLAG
        __next(pEncoder);
    }
}

/**
 * @brief set new pin configuration for encoder
 * 
 * @param encoder address of encoder instance
 * @param config new pin configuration
 * @return uint8_t return 1 if encoder added, 0 if there is no space
 */
void RotaryEncoder_setConfig(RotaryEncoder* encoder, const RotaryEncoder_Pins* config) {
    encoder->Config = config;
}
/**
 * @brief get encoder pin config
 * 
 * @param encoder 
 * @return const RotaryEncoder_Pins* 
 */
const RotaryEncoder_Pins* RotaryEncoder_getConfig(RotaryEncoder* encoder) {
    return encoder->Config;
}
#if ROTARY_ENCODER_MAX_NUM > 0
/**
 * @brief finding a empty space for new encoder in array
 * 
 * @return RotaryEncoder* return null if not found empty space
 */
RotaryEncoder* RotaryEncoder_new(void) {
    uint8_t len = ROTARY_ENCODER_MAX_NUM;
    RotaryEncoder* pEncoder = encoders;
    while (len-- > 0) {
        if (!pEncoder->Configured) {
            return pEncoder;
        }
        pEncoder++;
    }
    return ROTARY_ENCODER_NULL;
}
#endif // ROTARY_ENCODER_MAX_NUM
/**
 * @brief add encoder into list for process
 * 
 * @param encoder address of encoder
 * @param config encoder pin configuration
 */
uint8_t RotaryEncoder_add(RotaryEncoder* encoder, const RotaryEncoder_Pins* config) {
    // check for null
    if (ROTARY_ENCODER_NULL == encoder) {
        return 0;
    }
    // add new encoder to list
    RotaryEncoder_setConfig(encoder, config);
    // init IOs
    encoderDriver->initPin(config);
    encoder->State = (encoderDriver->readPin(encoder->Config) << 1 | encoderDriver->readPin(encoder->Config)) & 3;
#if ROTARY_ENCODER_MAX_NUM == -1
    // add encoder to linked list
    encoder->Previous = lastEncoder;
    lastEncoder = RotaryEncoder;
#endif // ROTARY_ENCODER_MAX_NUM == -1
    encoder->Configured = 1;
    encoder->Enabled = 1;
    return 1;
}
/**
 * @brief remove encoder from list
 * 
 * @param remove address of encoder
 * @return uint8_t return 1 if encoder found, 0 if not found
 */
uint8_t RotaryEncoder_remove(RotaryEncoder* remove) {
#if ROTARY_ENCODER_MAX_NUM == -1
    RotaryEncoder* pEncoder = lastEncoder;
    // check last encoder first
    if (remove == pEncoder) {
        // deinit IO
    #if ROTARY_ENCODER_USE_DEINIT
        if (encoderDriver->deinitPin) {
            encoderDriver->deinitPin(remove->Config);
        }
    #endif
        // remove encoder dropped from link list
        pEncoder->Previous = remove->Previous;
        remove->Previous = ROTARY_ENCODER_NULL;
        remove->Configured = 0;
        remove->Enabled = 0;
        return 1;
    }
    while (ROTARY_ENCODER_NULL != pEncoder) {
        if (remove == pEncoder->Previous) {
            // deinit IO
		#if ROTARY_ENCODER_USE_DEINIT
            if (encoderDriver->deinitPin) {
                encoderDriver->deinitPin(remove->Config);
            }
        #endif
            // remove encoder dropped from link list
            pEncoder->Previous = remove->Previous;
            remove->Previous = ROTARY_ENCODER_NULL;
            remove->Configured = 0;
            remove->Enabled = 0;
            return 1;
        }
        pEncoder = pEncoder->Previous;
    }
#else
    uint8_t len = ROTARY_ENCODER_MAX_NUM;
    RotaryEncoder* pEncoder = encoders;
    while (len--) {
        if (remove == pEncoder && pEncoder->Configured) {
            pEncoder->Configured = 0;
            pEncoder->Enabled = 0;
            return 1;
        }
        pEncoder++;
    }
#endif // ROTARY_ENCODER_MAX_NUM == -1
    return 0;
}
/**
 * @brief finding encoder based on PinConfig in list
 * 
 * @param config 
 * @return RotaryEncoder* 
 */
RotaryEncoder* RotaryEncoder_find(const RotaryEncoder_Pins* config) {
#if ROTARY_ENCODER_MAX_NUM == -1
    RotaryEncoder* pEncoder = lastEncoder;
    while (ROTARY_ENCODER_NULL != pEncoder) {
        if (config == pEncoder->Config) {
            return pEncoder;
        }
        pEncoder = pEncoder->Previous;
    }
#else
    uint8_t len = ROTARY_ENCODER_MAX_NUM;
    RotaryEncoder* pEncoder = encoders;
    while (len--) {
        if (config == pEncoder->Config) {
            return pEncoder;
        }
        pEncoder++;
    }
#endif // ROTARY_ENCODER_MAX_NUM == -1
    return ROTARY_ENCODER_NULL;
}

void RotaryEncoder_onChange(RotaryEncoder* encoder, RotaryEncoder_CallbackFn cb) {
    encoder->onChange = cb;
}

#if ROTARY_ENCODER_ENABLE_FLAG
void RotaryEncoder_setEnabled(RotaryEncoder* RotaryEncoder, uint8_t enabled) {
    encoder->Enabled = enabled;
}
uint8_t RotaryEncoder_isEnabled(RotaryEncoder* encoder) {
    return encoder->Enabled;
}
#endif

#if ROTARY_ENCODER_ARGS
void RotaryEncoder_setArgs(RotaryEncoder*, void* args) {
    RotaryEncoder->Args = args;
}
void* RotaryEncoder_getArgs(RotaryEncoder* encoder) {
    return encoder->Args;
}
#endif

#if ROTARY_ENCODER_INDEX
void RotaryEncoder_setRange(RotaryEncoder* encoder, RotaryEncoder_Index min, RotaryEncoder_Index max) {
    encoder->Range.Max = max;
    encoder->Range.Min = min;
    RotaryEncoder_setIndex(encoder->Index);
}
void RotaryEncoder_setIndex(RotaryEncoder* encoder, RotaryEncoder_Index index) {
    if (encoder->Index > encoder->Range.Max) {
        index = encoder->Range.Max;
    }
    else if (encoder->Index < encoder->Range.Min) {
        index = encoder->Range.Min;
    }
    encoder->Index = index;
}
RotaryEncoder_Index RotaryEncoder_getIndex(RotaryEncoder* encoder) {
    return encoder->Index;
}
#endif

