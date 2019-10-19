// A header file for helpers.c
// Declare any additional functions in this file
#include "shell_util.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>

volatile sig_atomic_t sigusr1_flag;
volatile sig_atomic_t sigchld_flag;

int timeComparator(void *entry1, void *entry2);
void pipe_helper(char * args[], int numTokens, int index);
void r_in(char * args[], int index);
void r_out(char * args[], int index);
void r_two(char * args[], int indeces[]);
void r_three(char * args[], int indeces[]);
void sigchld_handler(int sig);
void sigusr1_handler(int sig);
