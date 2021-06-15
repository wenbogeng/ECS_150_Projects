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
    
    if(argc == 1){
        write(STDOUT_FILENO,"wunzip: file1 [file2 ...]\n", 26);
        return 1;
    }else {
            for(int i = 1; i < argc; ++i){
                fd = open(argv[i],O_RDONLY);
                if (fd == -1) {
//                    cerr << "could not open " << argv[i] << endl;
                    write(STDOUT_FILENO,"couldn't open the file", 25);
                    // this signifies an error
//                exit(1);
                    return 1;
                }
                char buf_Read[1];
                int bytesRead;
//                int ret;
                
                while (read(fd, &bytesRead, 4) > 0) {
                    read(fd, buf_Read, sizeof(buf_Read));
                    for(int j = 0; j < bytesRead; ++j)
                    {
                        write(STDOUT_FILENO, buf_Read, sizeof(buf_Read));
                        
                    }
                }
                       
           }
                       
      }
    close(fd);
    return 0;
                       
}

