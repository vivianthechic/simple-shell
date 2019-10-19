#include "shell_util.h"
#include "linkedList.h"
#include "helpers.h"

// Library Includes
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
	int i; //loop counter
	char *args[MAX_TOKENS + 1];
	int exec_result;
	int exit_status;
	pid_t pid;
	pid_t wait_result;
	List_t bg_list;
	

    //Initialize the linked list
    bg_list.head = NULL;
    bg_list.length = 0;
    bg_list.comparator = timeComparator;  // Don't forget to initialize this to your comparator!!!

	// Setup segmentation fault handler
	if(signal(SIGSEGV, sigsegv_handler) == SIG_ERR)
	{
		perror("Failed to set signal handler");
		exit(-1);
	}

	if(signal(SIGCHLD, sigchld_handler) == SIG_ERR)
	{
		perror("Failed to set signal handler");
		exit(-1);
	}

	if(signal(SIGUSR1, sigusr1_handler) == SIG_ERR)
	{
		perror("Failed to set signal handler");
		exit(-1);
	}

	while(1) {

		char pipe = 0;
		char redirect_in = 0;
		char redirect_out = 0;
		int index = 0;
		int rdcount = 0;
		// DO NOT MODIFY buffer
		// The buffer is dynamically allocated, we need to free it at the end of the loop
		char * const buffer = NULL;
		size_t buf_size = 0;
		int indeces[4];
		// Print the shell prompt
		display_shell_prompt();
		
		// Read line from STDIN
		ssize_t nbytes = getline((char **)&buffer, &buf_size, stdin);

		// No more input from STDIN, free buffer and terminate
		if(nbytes == -1) {
			free(buffer);
			break;
		}

		// Remove newline character from buffer, if it's there
		if(buffer[nbytes - 1] == '\n')
			buffer[nbytes- 1] = '\0';

		// Handling empty strings
		if(strcmp(buffer, "") == 0) {
			free(buffer);
			continue;
		}
		
		char *buf_cpy = malloc(buf_size+1);
		buf_cpy = strcpy(buf_cpy,buffer);

		// Parsing input string into a sequence of tokens
		size_t numTokens;
		*args = NULL;
		numTokens = tokenizer(buffer, args);

		if(strcmp(args[0],"exit") == 0) {
			// Terminating the shell
			free(buffer);
			free(buf_cpy);
			if(bg_list.length != 0){
				node_t * n;
				for(n = bg_list.head; n != NULL;){
					ProcessEntry_t* entry = n -> value;
					printf(BG_TERM, entry->pid, entry->cmd);
					kill(entry->pid, SIGKILL);
					n = n->next;
					removeByPid(&bg_list,entry->pid);
				}
			}
			return 0;
		}
		
		
		//built-in command cd	
		else if(strcmp(args[0],"cd") == 0){
			int cdreturn;
			if (numTokens == 1) 
				cdreturn = chdir("/home");
			else 
				cdreturn = chdir(args[1]);
			if(cdreturn != 0){
				fprintf(stderr,DIR_ERR);
			}
			else{
				char cwd[100];
				printf("%s\n", getcwd(cwd,100));
			}
			continue;
		}

		//built-in command estatus
		else if (strcmp(args[0],"estatus") == 0){
			printf("%d\n",WEXITSTATUS(exit_status));
			fflush(stdout);
			continue;
		}
		
		int temp;
		for(temp = 0; temp < numTokens; temp++){
			if(strcmp(args[temp],">")==0 || strcmp(args[temp],"2>")==0 || strcmp(args[temp],">>")==0){
				redirect_out = 1;
				index = temp;
				indeces[rdcount] = temp;
				rdcount++;
			}
			else if(strcmp(args[temp],"<")==0){
				redirect_in = 1;
				index = temp;
				indeces[rdcount] = temp;
				rdcount++;
			}
			else if(strcmp(args[temp],"|")==0){
				pipe = 1;
				index = temp;
			}
		}
	
		pid = fork();   //In need of error handling......
		if(pid < 0){
			free(buf_cpy);
			free(buffer);
			return 0;
		}
		if (pid == 0){ //If zero, then it's the child process
		if(strcmp(args[numTokens-1], "&")==0){
				args[numTokens-1] = NULL;
				numTokens--;
			}
			if(pipe)
				pipe_helper(args, numTokens, index);
			else if(redirect_in && rdcount == 1){
				r_in(args, index);
			}
			else if(redirect_out && rdcount == 1){
				r_out(args, index);
			}
			else if(rdcount == 2){
				r_two(args,indeces);
			}
			else if(rdcount == 3){
				r_three(args,indeces);
			}
			else if(rdcount > 3)
				fprintf(stderr,RD_ERR);
			else{
				exec_result = execvp(args[0], &args[0]);
				if(exec_result == -1){ //Error checking
					printf(EXEC_ERR, args[0]);
					exit(EXIT_FAILURE);
				}
			}
			exit(EXIT_SUCCESS);
		}
		else{ // Parent Process
            		if(sigchld_flag){
                		node_t * n;
                		for(n = bg_list.head; n != NULL;){
                    			ProcessEntry_t* entry = n->value;
                    			if(waitpid(entry->pid, &exit_status ,WNOHANG) > 0){
                        			fprintf(stdout, BG_TERM, entry->pid, entry->cmd);
                        			n = n->next;
                        			removeByPid(&bg_list,entry->pid);
                    			}
                    			else n = n->next;
                		}
                		sigchld_flag = 0;
            		}
            		if(sigusr1_flag){
                		node_t * n;
                		for(n = bg_list.head; n != NULL; n = n->next){
                    			ProcessEntry_t* entry = n->value;
                    			printBGPEntry(entry);
                		}
                		sigusr1_flag = 0;
            		}
            
            		if(strcmp(args[numTokens-1],"&")==0){
                		ProcessEntry_t * entry = malloc(sizeof(ProcessEntry_t));
                		entry->cmd = malloc(buf_size+1);
                		entry->cmd = strcpy(entry->cmd,buf_cpy);
                		entry->pid = pid;
                		entry->seconds = time(NULL);
                		insertInOrder(&bg_list,entry);
                		continue;
            		}
            
			wait_result = waitpid(pid, &exit_status, 0);
			if(wait_result == -1){
				printf(WAIT_ERR);
				exit(EXIT_FAILURE);
			}
		}
		
		// Free the buffer allocated from getline
		free(buffer);
		free(buf_cpy);
	}
	deleteList(&bg_list);
	return 0;
}

