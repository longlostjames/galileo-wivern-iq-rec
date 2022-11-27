
/* data structure to hold a translated serial message */
typedef struct
{
        double  az;
        double  el;
        int     year;
        int     month;
        int     day;
        int     hour;
        int     min;
        int     sec;
        int     centi_sec;
} RSM_SerialMessageStruct;

int RSM_InitialiseSerialMessage( char serialport[] );

int RSM_ReadSerialMessage ( RSM_SerialMessageStruct *serialmessage);

void RSM_CloseSerialMessage (void);


