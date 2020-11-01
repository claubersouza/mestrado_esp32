

#ifdef __cplusplus
extern "C" {    
#endif


#define ESP8266_DREG(addr) *((volatile uint32_t *)(0x3FF00000+(addr)))
#define RANDOM_REG32  ESP8266_DREG(0x20E44)


int randomBitRaw2(void);
int randomBitRaw(void);
long calculateRandom(long howBig);
long random(long min, long max);


#ifdef __cplusplus
    }
#endif