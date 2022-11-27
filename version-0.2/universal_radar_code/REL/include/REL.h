
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
} REL_SerialMessageStruct;

int REL_InitialiseSerialMessage( char serialport[] );

int REL_ReadSerialMessage ( REL_SerialMessageStruct *serialmessage);

void REL_CloseSerialMessage (void);


