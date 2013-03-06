#include <stdio.h>
#include <sys/stat.h>

int main(int argc, char** argv) {
    int ret;
    printf("starting %s...\n", argv[0]);

    //create named fifo.
    mknod("vga-comm", S_IFIFO | 0666, 0);
    return 0;
}
