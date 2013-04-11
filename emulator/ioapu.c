#include "tools.h"
#include "apu.h"

static unsigned short apu_addr;
static unsigned char apu_data;

#define APU_IO_SIZE 0x20

/*
 * apucore r/w func ptr.
 * */
void set_dma_data(unsigned char data);
void set_joypad_data(unsigned char data);
unsigned char get_joypad_data(void);
void release_bus(void);
int get_rw_pin(void);

typedef void (apu_write_t)(unsigned char);
typedef unsigned char (apu_read_t)(void);

static apu_write_t *apu_write_func[APU_IO_SIZE];
static apu_read_t *apu_read_func[APU_IO_SIZE];

void set_apu_addr(unsigned short addr) {
    //dprint("set_apu_addr: %02x\n", addr);
    apu_addr = addr;
}

unsigned char get_apu_data(void) {
    return apu_data;
}

void set_apu_data(unsigned char data) {
    //dprint("set_apu_data: %02x\n", data);
    apu_data = data;
}

void set_apu_start(int ce) {
    //let ram i/o on the bus.
    if (ce) {
        if (get_rw_pin()) {
            //write cycle
            apu_write_func[apu_addr](apu_data);
        }
        else {
            //read cycle
            apu_data = apu_read_func[apu_addr]();
        }
        release_bus();
    }
}


/*dummy I/O func*/
static void null_write(unsigned char d){}
static unsigned char null_read(void){return 0;}

int init_apu(void) {
    int i;
    apu_addr = 0;
    apu_data = 0;

    //fill null function
    for (i = 0; i < APU_IO_SIZE; i++) {
        apu_write_func[i] = null_write;
    }
    for (i = 0; i < APU_IO_SIZE; i++) {
        apu_read_func[i] = null_read;
    }
    //dma func
    apu_write_func[0x14] = set_dma_data;
    //joypad func
    apu_write_func[0x16] = set_joypad_data;
    apu_read_func[0x16] = get_joypad_data;

    return TRUE;
}

void clean_apu(void) {
}

