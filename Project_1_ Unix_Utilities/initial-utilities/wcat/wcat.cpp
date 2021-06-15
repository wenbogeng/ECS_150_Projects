// @author Wenbo Geng
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

using namespace std;

int main(int argc, char *argv[]) {
    
    int fd;
    int ret_Read;
    
    char buf_Read[4096];
    // see if the user passed in an argument
    if(argc == 1) return 0; // it's already open
        else {
            for(int i = 1; i < argc; ++i){
                fd = open(argv[i],O_RDONLY);
                if (fd == -1) {
                    write(STDOUT_FILENO, "wcat: cannot open file\n", 23);
                    // this signifies an error
                    return 1;
            }
                while((ret_Read = read(fd, buf_Read, sizeof(buf_Read))) > 0) write(STDOUT_FILENO,buf_Read,ret_Read);
        }
    }
    close(fd);
    return 0;
}


