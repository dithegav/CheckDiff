/*
  Create a program witch will work as a test compile and test machine of another program.
  In  detail  project4.c will compile the given code from another program bu checking for errors or warnings of any kind (points will be conducted in each case
  Next by using program p4diff.c it will compare the stdout of the program and the correct output in the given file (filename).out.
  This program will calculate sum and print all points according to its evaluation of the program given.
  If system fails occur use perror and return 42.
  Only if the given arguments from running project4 are incorrect will the program return 1.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <math.h>
#define P_ERROR 8
#define MAX_BYTES

typedef struct {
    char *name;
    char *name_c;
    char *name_args, *print_argum;
    char *name_in;
    char *name_out;
    char *name_err;
    int timeout;
}prog_t;

/*a struct only to save the amount of points earned or lost*/
typedef struct {
    int compilation;
    int time_out;
    int output;
    int memory;
    int score;
}results_t;

static void my_handler (int pid){
}

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

/*
  Read all the arguments from (filename).args , 
  eliminate the character '\n' given as the end of the file and add a '\0' to the end to seal the string.
*/
int identify_argum (prog_t *prog2){
    struct stat info;
    char *help_ptr;
    int system_fail=0, fd_args=-1;

    fd_args = open (prog2->name_args, O_RDONLY);
    if (fd_args == -1){
        perror("open_args ");
        return 42;
    }

    system_fail = fstat (fd_args, &info);
    if (system_fail == -1){
        perror("fsize_args ");
        return (42);
    }

    prog2->print_argum = (char*)calloc(info.st_size+1, sizeof(char));
    if (prog2->print_argum == NULL){
        perror("memory_argum ");
        return(42);
    }

    system_fail = read_file (fd_args, prog2->print_argum, info.st_size);
    if (system_fail == -1){
        free(prog2->print_argum);
        perror("read_args ");
        return (42);
    }

    prog2->print_argum[info.st_size+1] = '\0';

    help_ptr = strstr (prog2->print_argum, "\n");
    if (help_ptr != NULL){
        *help_ptr = '\0';
    } 

    return 0;
    /*success*/
}

/*return the bigger integer*/
int max (int begin, int total){

    if (total >= begin){
        return total;
    }
    else {
        return begin;
    }
}

/*print in correct order all the information gathered from the program*/
void get_results (results_t results){

    printf ("\nCompilation: %d\n", results.compilation);
    printf ("\nTimeout: %d\n", results.time_out);
    printf ("\nOutput: %d\n", results.output);
    printf ("\nMemory access: %d\n", results.memory);
    printf ("\nScore: %d\n", results.score);
}

/*helpfull function to free many arguments and to avoid repeating the same code*/
void free_all (prog_t *prog2, int all){

    free(prog2->name_c);
    free(prog2->name_args);
    free(prog2->name_in);
    free(prog2->name_out);
    free(prog2->name);
    free(prog2->name_err);
    if (all == 1){
        free(prog2->print_argum);
    }
}

int main (int argc, char *argv[]){
    struct stat info;
    prog_t prog2;
    results_t results = {0,0,0,0,0};
    struct sigaction action = {{'\0'}};
    char **assist_ptr, *help_ptr, *help;
    char error[7]={' ', 'e','r','r','o','r',':'}, warning[9]={' ', 'w','a','r','n','i','n','g',':'}, buf[P_ERROR+1]={'\0'};
    int pid=0, result=0, fd=-1, search=0, system_fail=0, success=0, i=0, fd_pipe[2] = {-1}, pid_pipe2=0, pid_pipe3=0, idiot=0;

    if (argc != 6 || argv[5]<=0){
        return 1;
    }

    help_ptr = (char*)calloc(strlen(argv[1])+1, sizeof(char));
    if (help_ptr != NULL){
        prog2.name_c = help_ptr;
        strcpy (prog2.name_c , argv[1]);
    }
    else {
        perror("memory_c ");
        return 42;
    }

    help_ptr = (char*)calloc(strlen(argv[2])+1, sizeof(char));
    if (help_ptr != NULL){
        prog2.name_args = help_ptr;
        strcpy (prog2.name_args , argv[2]);
    }
    else {
        free(prog2.name_c);
        perror("memory_args ");
        return 42;
    }

    help_ptr = (char*)calloc(strlen(argv[3])+1, sizeof(char));
    if (help_ptr != NULL){
        prog2.name_in = help_ptr;
        strcpy (prog2.name_in , argv[3]);
    }
    else {
        free(prog2.name_c);
        free(prog2.name_args);
        perror("memory_in ");
        return 42;
    }

    help_ptr = (char*)calloc(strlen(argv[4])+1, sizeof(char));
    if (help_ptr != NULL){
        prog2.name_out = help_ptr;
        strcpy (prog2.name_out , argv[4]);
    }
    else {
        free(prog2.name_c);
        free(prog2.name_args);
        free(prog2.name_in);
        perror("memory_out ");
        return 42;
    }

    prog2.timeout = atoi (argv[5]);

    help_ptr = (char*)calloc(strlen(argv[1])+3, sizeof(char));
    if (help_ptr != NULL){
        prog2.name = help_ptr;
    }
    else {
        free(prog2.name_c);
        free(prog2.name_args);
        free(prog2.name_in);
        free(prog2.name_out);
        perror("memory_name ");
        return 42;
    }

    help_ptr = (char*)calloc(strlen(argv[1])+3, sizeof(char));
    if (help_ptr != NULL){
        prog2.name_err = help_ptr;
        strncpy (prog2.name_err , prog2.name_c, strlen(prog2.name_c)-2);
        strcpy (prog2.name, prog2.name_err);
        strcat (prog2.name_err, ".err");
    }
    else {
        free(prog2.name_c);
        free(prog2.name_args);
        free(prog2.name_in);
        free(prog2.name_out);
        free(prog2.name);
        perror("memory_err ");
        return 42;
    }

    /*open a child process in order to properly and safely compile the given file and redirect its stderr to (filename).err */
    pid = fork();

    if (pid == 0){
        fd = open (prog2.name_err, O_RDWR|O_CREAT|O_TRUNC, 0700);
        if (fd == -1){
            free_all(&prog2, 0);
            perror("open_err ");
            return 42;
        }

        system_fail = dup2(fd, STDERR_FILENO);
        if (system_fail == -1){
            free_all(&prog2, 0);
            perror("dup2_pid_stderr ");
            return 42;
        }

        system_fail = close(fd);
        if (system_fail == -1){
            free_all(&prog2, 0);
            perror("close_pid_err ");
            return 42;
        }

        system_fail = execlp ("gcc", "gcc", "-Wall", "-g", prog2.name_c, "-o", prog2.name, (char*)NULL);
        if (system_fail == -1){
            free_all(&prog2, 0);
            perror("execlp_pid ");
            return 42;
        }
        return 1;
    }

    system_fail = waitpid (pid, &result, 0);
    if (system_fail == -1){
        free_all(&prog2, 0);
        perror("waitpid_fork1 ");
        return 42;
    }

    if (WEXITSTATUS(result) == 42){
        /*system_fail of child */
        return 42;
    }

    if (WIFEXITED(result)){
        fd = open (prog2.name_err, O_RDONLY, 0700);
        if (fd == -1){
            free_all(&prog2, 0);
            perror("open_err_parent ");
            return 42;
        }

        system_fail = fstat (fd, &info);

        if (system_fail == -1){
            /*system failed*/
            free_all(&prog2, 0);
            perror("fsize_err_parent ");
            return 42;
        }

        success = 1;
        /*In bothe loops search the first letter e and w of the word error and warning 
          and go back and forth in order to find them using lseek and read. */
        for (search = 0; search < info.st_size-6; search++){
            system_fail = read (fd, buf, sizeof(char));
            if (system_fail == -1){
                free_all(&prog2, 0);
                perror("read_e ");
                return 42;
            }

            if (strcmp(&buf[0], "e") == 0){
                system_fail = lseek (fd, -2, SEEK_CUR);
                if (system_fail == -1){
                    free_all(&prog2, 0);
                    perror("lseek_e ");
                    return 42;
                }

                system_fail = read (fd, buf, 7);
                if (system_fail == -1){
                    free_all(&prog2, 0);
                    perror("read_error ");
                    return 42;
                }
                result = strncmp(buf, error, 7);
                if (result == 0){
                    success = 0;
                    break;
                }

                system_fail = lseek (fd, -5, SEEK_CUR);
                if (system_fail == -1){
                    free_all(&prog2, 0);
                    perror("lseek_error_back ");
                    return 42;
                }
                
                for (i=0; i < 9; i++){
                    buf[i] = '\0';
                }
            }
        }

        if (success == 0){
            results.compilation = -100;
            get_results(results);
            free_all(&prog2, 0);
            return 42;
        }

        for (i=0; i < 9; i++){
            buf[i] = '\0';
        }

        success = 0;
        if (success == 0){
            system_fail = lseek (fd, 0, SEEK_SET);
            if (system_fail == -1){
                free_all(&prog2, 0);
                perror("lseek_wanring ");
                return 42;
            }
            for (search = 0; search < info.st_size-7; search++){
                system_fail = read (fd, buf, sizeof(char));
                if (system_fail == -1){
                    free_all(&prog2, 0);
                    perror("read_w ");
                    return 42;
                }

                if (strcmp(&buf[0], "w") == 0){
                    system_fail = lseek (fd, -2, SEEK_CUR);
                    if (system_fail == -1){
                        free_all(&prog2, 0);
                        perror("lseek_w ");
                        return 42;
                    }

                    system_fail = read (fd, buf, 9);
                    if (system_fail == -1){
                        free_all(&prog2, 0);
                        perror("read_warning ");
                        return 42;
                    }
                    result = strncmp(buf, warning, 9);
                    for (i=0; i < 9; i++){
                        buf[i] = '\0';
                    }
                    if (result == 0){
                        success += 1;
                        search+=7;
                        continue;
                    }

                    system_fail = lseek (fd, -7, SEEK_CUR);
                    if (system_fail == -1){
                        free_all(&prog2, 0);
                        perror("lseek_warning_back ");
                        return 42;
                    }
                    
                }
                
            }
        }
    }
    
    if (success != -1) {
        results.compilation = (-5) * success;
    }

    result = identify_argum(&prog2);
    if (result == 42){
        free_all(&prog2, 0);
        return 42;
    }
    
    /*creat a table of pointers to prepare the right pointers to be put on the call of execv and safely transfer the arguments*/
    i=1;
    assist_ptr = (char**)malloc(i*sizeof(char*));
    assist_ptr[0] = prog2.name;

    /*use strtok to detect " " that are between the arguments from the string prog2.print_argum*/
    help = strtok (prog2.print_argum, " ");
    while (help != NULL){
        i++;
        assist_ptr = (char**)realloc(assist_ptr, i*sizeof(char*));
        assist_ptr[i-1] = help;

        help = strtok (NULL, " ");
    }

    i+=1;
    assist_ptr = (char**)realloc (assist_ptr, i * sizeof(char*));
    assist_ptr[i-1] = NULL;
    
    system_fail = pipe (fd_pipe);
    if (system_fail == -1){
        free_all(&prog2, 1);
        free(assist_ptr);
        perror("pipe ");
        return 42;
    }

    action.sa_handler = my_handler;
    system_fail = sigaction (SIGALRM, &action, NULL);
    if (system_fail == -1){
        free_all(&prog2, 1);
        free(assist_ptr);
        perror("sigaction ");
        return 42;
    }
    /*Both process (pid_pipe2, pid_pipe3) will be wokring together sharing information about the situation of the program.*/
    pid_pipe2 = fork ();
    if (pid_pipe2 == -1){
        free_all(&prog2, 1);
        free(assist_ptr);
        perror("pipe2 ");
        return 42;
    }

    if (pid_pipe2 == 0){

        system_fail = close(fd_pipe[0]);/*not read needed from pipe*/
        if (system_fail == -1){
            free_all(&prog2, 1);
            free(assist_ptr);
            perror("close_pipe2 ");
            return 42;
        }

        fd = open (prog2.name_in, O_RDONLY);
        if (fd == -1){
            free_all(&prog2, 1);
            free(assist_ptr);
            perror("open_in ");
            return 42;
        }

        system_fail = dup2(fd, STDIN_FILENO);
        if (system_fail == -1){
            perror("dup2_pipe2_stdin ");
            return 42;
        }

        system_fail = dup2(fd_pipe[1], STDOUT_FILENO);
        if (system_fail == -1){
            free_all(&prog2, 1);
            free(assist_ptr);
            perror("dup2_pipe2_stdout ");
            return 42;
        }
        system_fail = close (fd_pipe[1]);
        if (system_fail == -1){
            free_all(&prog2, 1);
            free(assist_ptr);
            perror("close_pipe2[1] ");
            return 42;
        } 
        
        system_fail = execv (prog2.name, assist_ptr);
        if (system_fail == -1){
            free_all(&prog2, 1);
            free(assist_ptr);
            perror("execv_name ");
            return 42;
        }

        
    }
    system_fail = close(fd_pipe[1]);
    if (system_fail == -1){
        free_all(&prog2, 1);
        free(assist_ptr);
        perror("close_pipe2_parent ");
        return 42;
    }

    pid_pipe3 = fork();
    if (pid_pipe3 == -1){
        free_all(&prog2, 1);
        free(assist_ptr);
        perror("pid_pipe3 ");
        return 42;
    }

    if (pid_pipe3 == 0){

        system_fail = dup2(fd_pipe[0], STDIN_FILENO);
        if (system_fail == -1){
            free_all(&prog2, 1);
            free(assist_ptr);
            perror("dup2_pipe3_stdin ");
            return 42;
        }
        system_fail = close(fd_pipe[0]);
        if (system_fail == -1){
            free_all(&prog2, 1);
            free(assist_ptr);
            perror ("close_pipe3[0] ");
            return 42;
        }

        system_fail = execl ("p4diff","p4diff", prog2.name_out, (char*)NULL);
        if (system_fail == -1){
            free_all(&prog2, 1);
            free(assist_ptr);
            perror("execl_pipe3 ");
            return 42;
        }

    }
    system_fail = close(fd_pipe[0]);
    if (system_fail == -1){
        free_all(&prog2, 1);
        free(assist_ptr);
        perror ("close_pipe3_parent ");
        return 42;
    }

    alarm(prog2.timeout);
    i = waitpid(pid_pipe2, &result, 0);
    /*if the alarm was set off before child process's was finished , kill child, if not stopm the timer and move on.*/
    if (i == -1 && errno == EINTR){
        kill (pid_pipe2, SIGKILL);
        results.time_out = -100;
    }
    alarm (0);
    if (WEXITSTATUS(result) == 42){
        /*system_fail inside child*/
        return 42;
    }

    if (WIFSIGNALED(result)){

        if ((WTERMSIG(result) == SIGSEGV) || (WTERMSIG(result) == SIGABRT) || (WTERMSIG(result) == SIGBUS)){
            /*signal error*/
            results.memory = -15;
            /*penalnty -15*/
        }
    }
    
    system_fail = waitpid (pid_pipe3, &idiot, 0);
    if (system_fail == -1){
        free_all(&prog2, 1);
        free(assist_ptr);
        perror ("waitpid_pipe3 ");
        return 42;
    }

    if (WEXITSTATUS(result) == 42){
        /*system_fail inside child*/
        return 42;
    }

    if (WIFEXITED(idiot)){
        /*result is the total that p4diff returned from its comparisons*/
        results.output = WEXITSTATUS(idiot);
        if (results.output == 42){
            free_all(&prog2, 1);
            free(assist_ptr);
            return 42;
        }
    }
    
    results.score = max (0, results.compilation+results.memory+results.output+results.time_out);

    get_results(results);
    free_all(&prog2, 1);
    free(assist_ptr);

    return 0;
}
