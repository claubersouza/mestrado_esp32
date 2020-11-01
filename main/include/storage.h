#ifdef __cplusplus
extern "C" {
#endif

#define OPEN_FILE_MSG "Opening file"
#define OPEN_FILE_MSG_ERROR "Failed to open file for writing"

#define READ_FILE_MSG "Reading file"
#define WRITE_FILE_SUCCESS "File written"



void spiff_init();
void writeFile(char* file, char* msg);
void readFile(char* file);
void closeFile();
FILE* openFile(char* file, char* mode);
bool renameFile(char* fileOld,char* fileNew);
void deleteFile(char* file);


    #ifdef __cplusplus
    }
    #endif