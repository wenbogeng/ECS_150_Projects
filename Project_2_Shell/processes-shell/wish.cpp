//
//  wish.cpp
//  test-wish
//
//  Created by Wenbo Geng on 4/12/21.
//
#include <iostream>
#include <fstream>
#include <sstream>


#include <cstdio>
#include <cstdlib>


#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <string>
#include <vector>
#include <cstring>

#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include<unistd.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

int size = 512;
// vector <char> saved_Path;
// number of path
int num_Path = 1;
char ERROR_MESSAGE[128] = "An error has occurred\n";
int path_Changed = 0;
// saving the path
char * path[100];
char * path_init;

bool isBatch;
bool pathEmpty;
char multi_Path[512][512];
int number_MultiPath=0;

// check if it's in the interactive mode.
void interactive_Mode();
// check if it's in the batch mode and takes the files
void batch_Mode(const string& file);
// print the prompt
void print_Prompt();
// print the errors
void print_Error();
// pre process the command and takes length of the command and numbers of the command and user's input
void pre_Process(int &len, int &num, string cmd, char * args[]);
// path takes length of the command and user's input
void pwd(int len, char *args[]);
// cd takes length of the command and user's input
int cd(int len, char *args[]);
// checking if there are any space
int check_Only_Space(char* buffer);
// process the command takes the length of the command the user's input
int process(int len, int num, char *args[]);
// check if it's redirection return 1 then its redirectioned
int execute(int len,char *args[]);
// redirection takes length of the command and user's input
int Redirection(int &len, char *args[]);
// Child process
int new_Process(char *args[]);

// check if its having the space since it doesn't have the space
int check_Only_Space(char* buffer){
    int checked = 0;
    for(unsigned int i = 0; i < sizeof(buffer); ++i){
        if(isspace(buffer[i]) == 0){
            checked = 1;
            break;
        }
    }
    // return 1 if there is space else return 0
    return checked;
}

void print_Error(){
    write(STDERR_FILENO, ERROR_MESSAGE, strlen(ERROR_MESSAGE));
    exit(1);
}

void print_Prompt(){
    write(STDOUT_FILENO, "wish> ", strlen("wish> "));
}

// store all the path to array
void pwd(int len, char *args[]){
    num_Path = len - 1;
    for(int i = 0; i < len - 1; ++i){
        // saved_Path.append(path[i]);
        path[i] = args[i + 1];
    }
}

// check with multiPath
int cd(int len, char *args[]){
    if (len == 0 || len != 2){
        write(STDERR_FILENO, ERROR_MESSAGE, strlen(ERROR_MESSAGE));
        return -1;
    }
    int dir = chdir(reinterpret_cast<const char *>(args[1]));
    if (dir == -1){
        write(STDERR_FILENO, ERROR_MESSAGE, strlen(ERROR_MESSAGE));
        return -1;
    }
    return 0;
}

// start a new process
int new_Process(char *args[]) {
    int fpid = fork();
    if(fpid < 0){ // error return negative
        write(STDERR_FILENO, ERROR_MESSAGE, strlen(ERROR_MESSAGE));
        exit(1);
        
    }
    else if(fpid == 0 && pathEmpty){ // child
        if(!path_Changed){
            path_init = strdup("/bin/");
            path_init =strcat(path_init,args[0]);
            if(access(path_init ,X_OK)!=0 && path_Changed == 0){
                path_init = strdup("/usr/bin/");
                path_init =strcat(path_init,args[0]);
                if(access(path_init, X_OK) != 0){
                    print_Error();
                }
            }
            
        }
        else if(path_Changed==1 && number_MultiPath==0){ // parent
            path_init = strcat(path_init,args[0]);
            if(access(path_init, X_OK)!=0){
                write(STDERR_FILENO, ERROR_MESSAGE, strlen(ERROR_MESSAGE));
                exit(0);
                
            }
            
        }
        else{
            for(int i = 0; i < number_MultiPath; ++i){
                strcat(multi_Path[i],args[0]);
                if(access(multi_Path[i],X_OK) ==0){
                    strcpy(path_init,multi_Path[i]);
                    break;
                    
                }
                
            }
            
        }
        if(execv(path_init,args) == -1){
            write(STDERR_FILENO, ERROR_MESSAGE, strlen(ERROR_MESSAGE));
            exit(0);
        }
    }

    return fpid;
}

void pre_Process(int &len, int &num, string cmd, char * args[]){
    string str;
    stringstream in(cmd);
    // find a parallel command without space
    while (in >> str){
        unsigned long found = str.find('&', 0);
        unsigned long fd = 0;
        if (str == "&"){
            ++num;
            char *parallel = new char [str.length()-1];
            
            strcpy(parallel, str.c_str());
            args[len] = parallel;
            
            ++len;
        }else if((int)found != -1){
            while((int)found != -1){
                ++num;
                char *temp = new char [found];
                str.copy(temp, found, 0);
                args[len] = temp;
                ++len;
                
                char *end = new char [1];
                strcpy(end, "&");
                args[len] = end;
                ++len;
                
                fd = found;
                str.erase(0, found + 1);
                found = str.find('&',0);
            }
            // adding new commands
            char *new_cmd = new char [str.size() - fd];
            strcpy(new_cmd, str.c_str());
            args[len] = new_cmd;
            len++;
            
        }else{
            char *new_cmd = new char [str.length()-1];
            strcpy(new_cmd, str.c_str());
            args[len] = new_cmd;
            ++len;
        }
    }
    // add "&" to the end of the command
    if (len == 0 or strcmp(args[len - 1], "&") != 0){
        char *end_cmd = new char [1];
        strcpy(end_cmd, "&");
        args[len] = end_cmd;
    }else{
        --num;
    }
}

void interactive_Mode() {
    int num = 0;
    int len = 0;
    
    string cmd;
    // take input util exit
    while(1) {
        write(STDOUT_FILENO, "wish> ", 6);
//       string cmd = getline(cin, cmd);
        getline(cin, cmd);
        num = 1;
        char * cmd_Buf[100];
        pre_Process(len, num, cmd, cmd_Buf);
        
        string str = cmd_Buf[0];

        if (str == "exit") {
            if (len > 1){
                print_Error();
                continue;
            } else{
                return;
            }
        }else if (str == "cd") cd(len, cmd_Buf);
        else if (str == "path") pwd(len, cmd_Buf);
        else if(str == "&")continue;
        else{
            if(process(len, num, cmd_Buf)){
                print_Error();
            }
        }
//        cmd_Buf[100] = {0};
        // clear the buffer
        for(int i = 0; i < len; ++i){
            char *p = new char[1];
            strcpy(p, "");
            cmd_Buf[i] = p;
        }
    }
}

//simillar with intervative mode just changed the while condition to scan each line of the file
void batch_Mode(const string& file){
    int num = 0;
    int len = 0;
    string cmd;
    
    ifstream fd(file);
    // run every command each line
    while (getline(fd, cmd)){
        len = 0;
        num = 1;
        char * cmd_Buf[100];
        pre_Process(len, num, cmd, cmd_Buf);
        
        string str = cmd_Buf[0];
        if (str == "exit") {
            if (len > 1){
                write(STDERR_FILENO, ERROR_MESSAGE, strlen(ERROR_MESSAGE));
                continue;
            } else{
                return;
            }
        }else if (str == "cd") cd(len, cmd_Buf);
        else if (str == "path") pwd(len, cmd_Buf);
        else if(str == "&")continue;
        else{
            if(process(len, num, cmd_Buf)){
                write(STDERR_FILENO, ERROR_MESSAGE, strlen(ERROR_MESSAGE));
            }
        }
//        cmd_Buf[100] = {0};
        // clear the buffer
        for(int i = 0; i < len; ++i){
            char *p = new char[1];
            strcpy(p, "");
            cmd_Buf[i] = p;
        }
    }
}
int execute(int len, char *args[]){
    string cmd = args[0];
    string temp_Path = args[0];
    // check all the path and adding / make it dir
    for (int i = 0; i < num_Path; ++i){
        temp_Path = path[i];
        temp_Path += "/";
        temp_Path += cmd;
        
        char *temp = new char [temp_Path.length()-1];
        strcpy(temp, temp_Path.c_str());
        
        if (access(temp, X_OK) == 0) {
            args[len + 1] = temp;
            return 0;
        }
    }
    return -1;
}
// from piazza professor answered the question using for loop
int Redirection(int &len, char * args[]){
    // check the spaces and > for each lane
    for (int i = 0; i < len; ++i){
        if (strcmp(args[i], ">") == 0){
            if (i + 2 == len && i != 0) return i + 1;
            else return -1;
        }else {
            string str = args[i];
            unsigned long found = str.find('>',0);
            // separate the commands
            if ((int)found != -1){
                char *p = new char [found];
                str.copy(p, found, 0);
                
                args[i] = p;
                args[i + 1] = NULL;
                
                char *q = new char [str.size() - found];
                str.copy(q, str.size()- found, found + 1);
                
                args[i + 2] = q;
                len +=2;
                
                return i + 2;
            }
        }
    }
    return 0;
}

int process(int len, int num, char *args[]){
    vector<pid_t> child_Process;// store all the child
    int error = 0;
    
    // check the path if it's empty
    if (num_Path == 0){
        error = 1;
        return error;
    }else {
        int counter = 0;
        
        for(int i = 0; i < num; ++i){
            char * cmd[100];
            int len = 0;
            //adding the &
            while(strcmp(args[counter], "&") != 0){
                cmd[len] = args[counter];
                ++len;
                ++counter;
            }
            ++counter;
            cmd[len] = NULL;
            
            //checking the redirections
            int fd = -1;
            int redir = Redirection(len, cmd);
            // check if it can open the file
            if (redir == -1){
                -- num;
                error = 1;
                
                continue;
            }else if (redir > 0){
                fd = open(cmd[redir], O_WRONLY | O_TRUNC | O_CREAT, 0644);
                cmd[len - 2] = NULL;
            }
            if (execute(len, cmd) == -1){
                -- num;
                error = 1;
                
                continue;
            }
            int cv = 0;
            pid_t child_Pid;
            
            // run the command
            child_Pid = fork();
            child_Process.push_back(child_Pid);
            
            if(child_Pid == 0) {
                // print the output using dup2()
                if(fd != -1){
                    dup2(fd, STDOUT_FILENO);
                    dup2(fd, STDERR_FILENO);
                }
                
                cv = execv(cmd[len+1], cmd);
                
                if(cv == -1){
                    error = 1;
                    exit(0);
                }
            }
        }
        // waiting for all the process to end
        for(int i = 0; i < num; ++i){
            waitpid(child_Process[i], NULL, 0);
        }
        // clear the buffer
        child_Process.clear();
    }
    return error;
}

int main(int argc, char *argv[]) {
    string initialPath = "/bin";
    path_init = strdup("/bin");
    
    char *buffer = const_cast<char *>(initialPath.c_str());
    
    int fd;

    path[0] = buffer;

    // Not batch mode
    if(argc == 1){
        isBatch = false;
        print_Prompt();
    }else if(argc == 2){ //batch mode
        isBatch = true;
        fd = open(argv[1], O_RDONLY);
        // Cant open the file or file is null
        if (fd == -1 || !fd) print_Error();
    }else print_Error();
    if (isBatch) batch_Mode(argv[1]);
    else interactive_Mode();
    return 0;
}
