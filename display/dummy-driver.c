#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

#include "tools.h"
#include "vga.h"

int main_fd(int argc, char** argv) {
    int ret;
    int fd;
    char buf[100];

    printf("fd ver %s...\n", argv[0]);

    //create named fifo.
    ret = mknod(VGA_FIFO, S_IFIFO | 0666, 0);
    if (ret != RT_OK && errno != EEXIST) {
        fprintf(stderr, "error creating pipe!\n");
        return -1;
    }

    fd = open(VGA_FIFO, O_RDWR | O_NONBLOCK );
    if (fd == -1) {
        fprintf(stderr, "error opening fifo!\n");
        return -1;
    }
    while (1) {

        printf("send...\n");
        fflush(stdout);
        write (fd, buf, sizeof(buf));

        sleep(1);
    }
    close(fd);

    return 0;
}

int main_file(int argc, char** argv) {
    int ret;
    FILE* fifo_o;
    char buf[100];

    printf("file ver %s...\n", argv[0]);

    //create named fifo.
    ret = mknod(VGA_FIFO, S_IFIFO | 0666, 0);
    if (ret != RT_OK && errno != EEXIST) {
        fprintf(stderr, "error creating pipe!\n");
        return -1;
    }


    fifo_o = fopen(VGA_FIFO, "w");
    if (fifo_o == NULL) {
        fprintf(stderr, "error opening fifo!\n");
        return -1;
    }

    while (1) {
        printf("send...\n");
        fflush(stdout);
        fwrite (buf, 1, sizeof(buf), fifo_o);

        sleep(1);
    }

    fclose(fifo_o);

    return 0;
}


void pipe_sig_handler(int p) {
    printf("sigpipe!\n");
}

int main(int argc, char** argv) {
    //signal(SIGPIPE, SIG_IGN);
    signal(SIGPIPE, pipe_sig_handler);
    
    //sender can repeat using the same pipe.
    return main_fd(argc, argv);
    //return main_file(argc, argv);
}

