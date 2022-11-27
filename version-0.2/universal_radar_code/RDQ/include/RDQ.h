
#define DMA_BUFFER_SIZE (32*1024*1024*4)

/* Macros to manipulate DMA bank data extraction pointers */
#define GET_POINTER(buffer, num_samples, num_channels, pulse, sample) (buffer + num_channels*(num_samples*pulse + sample))
#define INC_POINTER(buffer, num_channels) buffer += num_channels

/* Macro to perform de-multiplexing of channels */
#define GET_CHANNEL(buffer, channel) (data[ dmux_table[ channel ] ])

void RDQ_InitialiseISACTRL( int pulses_per_daq_cycle, int samples_per_pulse, int clock_divfactor, int delayclocks );

int RDQ_InitialisePCICARD( caddr_t *dma_buffer ); 

void RDQ_ClosePCICARD ( int amcc_fd, caddr_t *dma_buffer );

int RDQ_InitialisePCICARD_New( caddr_t *dma_buffer, size_t dma_buffer_size ); 

void RDQ_ClosePCICARD_New ( int amcc_fd, caddr_t *dma_buffer, size_t dma_buffer_size );

void RDQ_StartAcquisition( int amcc_fd, int dma_bank, short int *data, int tcount );

int RDQ_WaitForAcquisitionToComplete( int amcc_fd );
