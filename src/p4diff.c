#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <math.h>
#define BLOCK_READ 64

/*
  A small program , witch only purpose is to compare byte by byte 
  an entire output of a stdout of a file with the given rigth output.
  Compares every 64 bytes the same bytes from the files and adds them to find the total % of the comparison.
  If system calls fail return 42!
  If all good return total % .
*/

/*
  Usefull function to avoid any anomalies with the call system read.
  If it doesnt read the amount of bytew requested, be sure it does!
*/
int read_file (int fd, void *buf, int length){
    int  curr, n=0;

    do {
        if ((curr = read (fd, &((char*)buf)[n], length-n)) == -1){
            if (errno ==EINTR){
                continue;
            }
            else {
                return -1;
            }
        }
        
        if (curr == 0){
            return n;
        }

        n += curr;
    }
    while (n < length);

    return n;
}

/*return the bigger integer*/
int max (int std_in, int fsize){

    if (fsize >= std_in){
        return fsize;
    }
    else {
        return std_in;
    }
}

/*
  After succesfully opening the right stdout file read in packs of 64 
  and make sure to calculate total % from the most bytes read of the two.
*/
int main (int argc, char* argv[]){
    int system_fail = 0, result=0, fd_prog_out=0, fsize=0, same_bytes=0, std_in=0, i=0, total=0, file=0, input=0;
    struct stat info;
    char buf[BLOCK_READ+1] = {'\0'}, str[BLOCK_READ+1] = {'\0'};

    fd_prog_out = open (argv[1], O_RDONLY);
    if (fd_prog_out == -1){
        perror("open ");
        return 42;
    }

    system_fail = fstat (fd_prog_out, &info);
    if (system_fail == -1){
        perror("fsize ");
        return 42;
    }

    fsize = info.st_size;

    while (1){

        system_fail = read_file (fd_prog_out, buf, BLOCK_READ);
        if (system_fail == -1){
            perror("read ");
            return 42;
        }
        fsize -= system_fail;
        file = system_fail;

        system_fail = read_file (STDIN_FILENO, str, BLOCK_READ);
        if (system_fail == -1){
            perror("read_stdout ");
            return 42;
        }
        input = system_fail;

        if (file < input){
            result = file;
        }
        else {
            result = input;
        }

        std_in += input; 
        if (fsize == 0 && input == 0){
            break;
        }

        for (i=0; i < result; i++){
            if (str[i] == buf[i]){
               same_bytes++;
            }
        }
    }

    if (info.st_size == 0 && std_in == 0){
        total = 100;
        return total;
    }

    total = (same_bytes* 100)/ max (std_in , info.st_size);
    
    return total;
}