// @author Wenbo Geng
#include <iostream>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

using namespace std;

int main(int argc, char *argv[]) {
    int fd;
//    int ret_Read = 0;
    
    int counter = 0;
    
    char buf_Read [1];
    char current [1];
//    char buf_Write[4096];
    
    // see if the user passed in an argument
    if(argc == 1)
    {
        write(STDOUT_FILENO,"wzip: file1 [file2 ...]\n", 24);
        return 1;
        
    }else {
            for(int i = 1; i < argc; ++i){
                fd = open(argv[i],O_RDONLY);
                if (fd == -1) {
                    write(STDOUT_FILENO,"wcat: cannot open file\n", 23);
                    // this signifies an error
//                exit(1);
                    return 1;
            }
                while(read(fd, buf_Read, sizeof(buf_Read))> 0){
                    if(*current == '\0') *current = *buf_Read;
                    if(*current == *buf_Read) counter++;
                    else
                    {
                        write(STDOUT_FILENO,&counter,sizeof(int)); // check the with int range
                        write(STDOUT_FILENO,current,1);
                        *current = *buf_Read;
                        counter = 1;
                    }
                }
        }
    }
    if(*current != '\0'){
        write(STDOUT_FILENO, &counter, 4);
        write(STDOUT_FILENO, current, 1);
    }
    close(fd);
    return 0;
}
