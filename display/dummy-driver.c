#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "tools.h"
#include "vga.h"

static struct rgb15 disp_data[VGA_WIDTH][VGA_HEIGHT];

struct timespec sleep_inteval = {0, 1};

static void pipe_sig_handler(int p) {
    printf("sigpipe!\n");
}

static void send_data (int fd, const void* buf, size_t size) {
    //write(fd, buf, size);
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(DISPLAY_PORT);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    sendto(fd, buf, size, 0, (struct sockaddr *)&addr, sizeof(addr));
}

static void send_line(int fd, int line) {
    struct vga_pulse data;
    int x;

    //send preamble
    data.h_sync = 0;
    data.v_sync = 1;
    send_data (fd, &data, sizeof(data));
    nanosleep(&sleep_inteval, NULL);
    for (x = 0; x < VGA_WIDTH; x++) {
        data.h_sync = 1;
        data.v_sync = 1;
        data.r = disp_data[x][line].r;
        data.g = disp_data[x][line].g;
        data.b = disp_data[x][line].b;

        send_data (fd, &data, sizeof(data));
        nanosleep(&sleep_inteval, NULL);
    }

    //send postamble
    data.h_sync = 0;
    data.v_sync = 1;
    send_data (fd, &data, sizeof(data));
    nanosleep(&sleep_inteval, NULL);
}

static void send_vsync_line(int fd, int cnt) {
    struct vga_pulse data;
    int x;

    //vsync blank data.
    data.h_sync = 0;
    data.v_sync = 0;
    for (x = 0; x < cnt; x++) {
        send_data (fd, &data, sizeof(data));
        nanosleep(&sleep_inteval, NULL);
    }
}

static void init_color1(void) {
    int x,y;
    for (y = 0; y < VGA_HEIGHT; y++) {
        for (x = 0; x < VGA_WIDTH; x++) {
            if (x < VGA_WIDTH / 7) {
                //75% white
                disp_data[x][y].r = to5bit(0xffff);
                disp_data[x][y].g = to5bit(0xffff);
                disp_data[x][y].b = to5bit(0xffff);
            }
            else if (x < VGA_WIDTH * 2 / 7) {
                //yellow
                disp_data[x][y].r = to5bit(0xffff);
                disp_data[x][y].g = to5bit(0xffff);
                disp_data[x][y].b = to5bit(0xffff);
            }
            else if (x < VGA_WIDTH * 2 / 7) {
                //cian
                disp_data[x][y].r = to5bit(0xffff);
                disp_data[x][y].g = to5bit(0xffff);
                disp_data[x][y].b = to5bit(0xffff);
            }
            else if (x < VGA_WIDTH * 2 / 7) {
                //green
                disp_data[x][y].r = 0;
                disp_data[x][y].g = to5bit(0xffff);
                disp_data[x][y].b = 0;
            }
            else if (x < VGA_WIDTH * 2 / 7) {
                //magenda
                disp_data[x][y].r = to5bit(0xffff);
                disp_data[x][y].g = to5bit(0xffff);
                disp_data[x][y].b = to5bit(0xffff);
            }
            else if (x < VGA_WIDTH * 2 / 7) {
                //red
                disp_data[x][y].r = to5bit(0xffff);
                disp_data[x][y].g = 0;
                disp_data[x][y].b = 0;
            }
            else if (x < VGA_WIDTH * 2 / 7) {
                //blue
                disp_data[x][y].r = 0;
                disp_data[x][y].g = 0;
                disp_data[x][y].b = to5bit(0xffff);
            }

            disp_data[x][y].r = to5bit(0xffff);
            disp_data[x][y].g = to5bit(0xffff);
            disp_data[x][y].b = to5bit(0xffff);
        }
    }
}

int main(int argc, char** argv) {
    int ret;
    int fd;

    //signal(SIGPIPE, SIG_IGN);
    signal(SIGPIPE, pipe_sig_handler);

    memset(&disp_data, 0, sizeof(disp_data));
    init_color1();
    
    //create named fifo.
    ret = mknod(VGA_FIFO, S_IFIFO | 0666, 0);
    if (ret != RT_OK && errno != EEXIST) {
        fprintf(stderr, "error creating pipe!\n");
        return -1;
    }

    //fd = open(VGA_FIFO, O_RDWR | O_NONBLOCK );
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        fprintf(stderr, "error opening fifo!\n");
        return -1;
    }
    while (1) {
        int i;
        printf("send...\n");
        /*
        send_vsync_line(fd, 10);
        for (i = 0; i < VGA_HEIGHT; i++) {
            send_line(fd, i);
        }
        send_vsync_line(fd, 10);
        */
        //max udp size is 64k!!
        //bug disp_data has 650k bytes!!
        send_data(fd, &disp_data, sizeof(disp_data));
        //send_data(fd, &disp_data, 10);
        //send_data (fd, &data, sizeof(data));

        sleep(1);
    }
    close(fd);

    return 0;
}


