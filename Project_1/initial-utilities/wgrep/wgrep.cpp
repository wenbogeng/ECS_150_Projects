// @author Wenbo Geng
#include <iostream>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string>

using namespace std;

int main(int argc, char *argv[]) {
    int fd;
    
    bool isFound = false;
    
    char buf_Read[1];
    char buf_Write[8192];
    

    int counter = 0;
    unsigned int index = 0;
    
    // see if the user passed in an argument
    if (argc == 1) {
        write(STDOUT_FILENO, "wgrep: searchterm [file ...]\n", 29);
        return 1;
    }else if (argc == 2) fd = STDIN_FILENO;// it's already open
    else {
        fd = open(argv[2], O_RDONLY);
        if (fd == -1) {
            write(STDOUT_FILENO, "wgrep: cannot open file\n", 24);
            return 1;
        }
    }
    
    //    string term;
    string target = argv[1];
    while (read(fd, buf_Read, sizeof(buf_Read)) > 0){
        buf_Write[counter] = *buf_Read;
        ++counter;
        if(!isFound){
            if(*buf_Read == target[index]) ++index;
            else index = 0;
            if(index == target.size()) isFound = true;
        }
        if (*buf_Read == '\n') {
            if(isFound){
                // clear the result and buffer
                write(STDOUT_FILENO, buf_Write, counter);
            }
            isFound = false;
            counter = 0;
            index = 0;
        }

    }
    close(fd);
    return 0;
}
