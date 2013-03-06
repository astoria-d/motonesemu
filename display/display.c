#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "tools.h"
#include "vga.h"

int main_file(int argc, char** argv) {
    int ret;
    FILE* fifo;
    char buf[100];
    int cnt = 0;

    dprint("starting FILE %s...\n", argv[0]);

    //create named fifo.
    ret = mknod(VGA_FIFO, S_IFIFO | 0666, 0);
    if (ret != RT_OK && errno != EEXIST) {
        fprintf(stderr, "error creating pipe!\n");
        return -1;
    }

    while (1) {
    dprint("fifo open...");
    
    fifo= fopen(VGA_FIFO, "r");
    if (fifo == NULL) {
        fprintf(stderr, "error opening fifo!\n");
        return -1;
    }
    dprint(" ok.\n");


        int i;

        dprint("cnt: %d\n", cnt++);
        fflush(stdout);
        ret = fread (buf, 1, sizeof(buf), fifo);
        if (ret == 0)
            continue;

        for (i = 0; i < sizeof(buf); i++) {
            printf ("%02x ", buf[i]);
            fflush(stdout);
        }
        dprint("\n");

    dprint("fd close...");
    fclose(fifo);
    dprint(" ok.\n");

    }
    return 0;
}

int main_fd(int argc, char** argv) {
    int ret;
    int fd;
    char buf[100];
    int cnt = 0;

    dprint("starting FD %s...\n", argv[0]);

    //create named fifo.
    ret = mknod(VGA_FIFO, S_IFIFO | 0666, 0);
    if (ret != RT_OK && errno != EEXIST) {
        fprintf(stderr, "error creating pipe!\n");
        return -1;
    }

    dprint("fifo open...");
    fd = open(VGA_FIFO, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "error opening fifo!\n");
        return -1;
    }
    dprint(" ok.\n");

    while (1) {
        int i;

        dprint("cnt: %d\n", cnt++);
        fflush(stdout);
        ret = read (fd, buf, sizeof(buf));
        if (ret == 0)
            continue;

        for (i = 0; i < sizeof(buf); i++) {
            printf ("%02x ", buf[i]);
            fflush(stdout);
        }
        dprint("\n");

    }

    dprint("fd close...");
    close(fd);
    dprint(" ok.\n");

    return 0;
}

int main(int argc, char** argv) {
    //receiver both file/fd ok if pipe is closed each transaction!!
    return main_file(argc, argv);
    //return main_fd(argc, argv);
}
