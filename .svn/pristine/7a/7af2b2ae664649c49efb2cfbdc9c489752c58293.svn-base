
#include "/home/chilbolton_software/universal_radar_code/RSP/include/RSP.h"

int RLC_GPIBCommunication( int pad, char *msg, char *reply, int ctrl);

int RLC_LecroyErrorCheck( int serialport_fd ); 

int RLC_InitialiseSerialPort( char serialport[] );

void RLC_CloseSerialPort (int serialport_fd);

int RLC_LecroyReset( int serialport_fd );

int RLC_LecroySelectChannel( int serialport_fd, int channel );

int RLC_LecroySequenceOn( int serialport_fd );

int RLC_LecroyUserOn( int serialport_fd );

int RLC_LecroySyncOn( int serialport_fd);

int RLC_LecroyOutputOn( int serialport_fd);

int RLC_LecroyLoadCode( int serialport_fd, int *lecroy_code, RSP_ParamStruct *param, int length_lecroy_code, float vmax, float vmin );

