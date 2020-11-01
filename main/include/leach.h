

#ifdef __cplusplus
extern "C" {    
#endif


#define OPERATING_NODES 10
#define CH 1
#define NORMAL 0
void setup_wifi(void);
void electionCH();
float  generateRandom();
float reelection();
void initLeach(void *pvParameters);

#ifdef __cplusplus
    }
#endif