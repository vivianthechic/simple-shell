// Your helper functions need to be here.
#include "helpers.h"

int timeComparator(void *entry1, void *entry2){
	ProcessEntry_t * e1 = (ProcessEntry_t *)entry1;
	ProcessEntry_t * e2 = (ProcessEntry_t *)entry2;
	if (e1->seconds < e2->seconds) return -1;
	else if (e1->seconds > e2->seconds) return 1;
	else return 0;
}




void pipe_helper(char * args[], int numTokens, int index){
	char *args1[numTokens];
	char *args2[numTokens];
	pid_t pid1;
	pid_t pid2;
	int pipefd[2];

	int i;
	for(i = 0; i < numTokens; i++){
		args1[i] = NULL;
		args2[i] = NULL;
	}

	if(pipe(pipefd) == -1 || index == 0 || index == numTokens-1){
		fprintf(stderr,PIPE_ERR);
		exit(EXIT_FAILURE);
	}

	for(i = 0;i < index; i++){
		args1[i] = malloc(strlen(args[i])+1);
		args1[i] = strcpy(args1[i],args[i]);
	}
	for(i = index+1; i < numTokens; i++){
		args2[i-index-1] = malloc(strlen(args[i])+1);
		args2[i-index-1] = strcpy(args2[i-index-1],args[i]);
	}

	pid1 = fork(); //first command

	if(pid1 < 0) exit(EXIT_FAILURE);
	if(pid1 == 0){ //child for first command
		close(1);
		dup(pipefd[1]);
		close(pipefd[0]);
		close(pipefd[1]);
		if(execvp(args1[0],args1) < 0){
			printf(EXEC_ERR, args1[0]);
			exit(EXIT_FAILURE);
		}
	}
	else{
		if(waitpid(pid1,NULL,0) == -1){
			perror(WAIT_ERR);
			exit(EXIT_FAILURE);
		}
	}

	pid2 = fork(); //second command

	if(pid2 < 0) exit(EXIT_FAILURE);
	if(pid2 == 0){ //child for second command
		close(0);
		dup(pipefd[0]);
		close(pipefd[0]);
		close(pipefd[1]);
		if(execvp(args2[0],args2) < 0){
			printf(EXEC_ERR,args2[0]);
			exit(EXIT_FAILURE);
		}
	}
	else{
		close(pipefd[0]);
		close(pipefd[1]);
		if(waitpid(pid2,NULL,0) == -1){
			perror(WAIT_ERR);
			exit(EXIT_FAILURE);
		}
	}
	return;
}


     
void r_in(char * args[], int index){
	char * a[index+1];
	char * filename = args[index + 1];
	pid_t pid;
	int fd;

	int i;
	for(i = 0; i < index + 1; i++)
		a[i] = NULL;
	for(i = 0; i < index; i++){
		a[i] = malloc(strlen(args[i])+1);
		a[i] = strcpy(a[i],args[i]);
	}

	pid = fork();

	if(pid < 0) exit(EXIT_FAILURE);
	if(pid == 0){
		if((fd = open(filename, O_RDONLY)) < 0){
			fprintf(stderr,RD_ERR);
			return;
		}
		dup2(fd,0);
		close(fd);
		if(execvp(a[0],a) < 0){
			printf(EXEC_ERR,a[0]);
			exit(EXIT_FAILURE);
		}
	}
	else{
		if(waitpid(pid,NULL,0) == -1){
			perror(WAIT_ERR);
			exit(EXIT_FAILURE);
		}
	}
	return;
}

void r_out(char * args[], int index){
    	char * a[index+1];
    	char * filename = args[index + 1];
    	pid_t pid;
    	int fd;
    
    	int i;
    	for(i = 0; i < index + 1; i++)
        	a[i] = NULL;
    	for(i = 0; i < index; i++){
        	a[i] = malloc(strlen(args[i])+1);
        	a[i] = strcpy(a[i],args[i]);
    	}
    
    	pid = fork();
    
    	if(pid < 0) exit(EXIT_FAILURE);
    	if(pid == 0){
        	if(strcmp(args[i],">") == 0){
			if((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU)) < 0){
				fprintf(stderr,RD_ERR);
				return;
			}
			dup2(fd,1);
		}
		else if(strcmp(args[i],">>") == 0){
			if((fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, S_IRWXU)) < 0){
                                fprintf(stderr,RD_ERR);
                        	return;
			}
                        dup2(fd,1);
		}
		else{ // strcmp(args[i],"2>") == 0
			if((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU)) < 0){
                                fprintf(stderr,RD_ERR);
                                return;
                        }
                        dup2(fd,2);
		}
        	close(fd);
        	if(execvp(a[0],a) < 0){
            		printf(EXEC_ERR,a[0]);
           	 	exit(EXIT_FAILURE);
        	}
    	}
   	else{
        close(fd);
        	if(waitpid(pid,NULL,0) == -1){
            		perror(WAIT_ERR);
            		exit(EXIT_FAILURE);
		}
	}
	return;
}


void r_two( char * args[], int indeces[]){
    	char * filename = args[indeces[1]+1];
	if(indeces[0] == indeces[1]-1 || strncmp(args[indeces[0]], args[indeces[1]], 1) == 0 || strcmp(filename, args[indeces[0]+1]) == 0){
		fprintf(stderr,RD_ERR);
		return;
	}
	char * a[indeces[1]+1];
    	int fd;
    
    	int i;
    	for(i = 0; i < indeces[1] + 1; i++)
        	a[i] = NULL;
    	for(i = 0; i < indeces[1]; i++){
        	a[i] = malloc(strlen(args[i])+1);
        	a[i] = strcpy(a[i],args[i]);
    	}
    
    	if(strcmp(args[indeces[1]],"<") == 0){
        	if((fd = open(filename, O_RDONLY)) < 0){
            		fprintf(stderr,RD_ERR);
            		return;
        	}
        	dup2(fd,0);
        	close(fd);
       	 	r_out(a,indeces[0]);
    	}
    	else{ // the second is >, >>, or 2>
        	if(strcmp(args[i],">") == 0){
            		if((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU)) < 0){
                		fprintf(stderr,RD_ERR);
                		return;
            		}
            		dup2(fd,1);
        	}
        	else if(strcmp(args[i],">>") == 0){
            		if((fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, S_IRWXU)) < 0){
                		fprintf(stderr,RD_ERR);
                		return;
            		}
            		dup2(fd,1);
        	}
        	else{ // strcmp(args[i],"2>") == 0
            		if((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU)) < 0){
                		fprintf(stderr,RD_ERR);
                		return;
            		}
            		dup2(fd,2);
        	}
        	close(fd);
        	if(strcmp(args[indeces[0]],"<") == 0)
        		r_in(a,indeces[0]);
        	else // the first is also redirection out
            		r_out(a,indeces[0]);
    	}
	return;
}

void r_three(char * args[], int indeces[]){
	char * filename = args[indeces[2]+1];
	if(indeces[2] == indeces[1] + 1 || strcmp(filename,args[indeces[1]+1]) == 0 || strncmp(args[indeces[2]],args[indeces[1]],1) == 0 || strncmp(args[indeces[2]],args[indeces[0]],1) == 0){
		fprintf(stderr,RD_ERR);
		return;
	}
	
	char * a[indeces[2]+1];
	int fd;

	int i;
	for(i = 0; i < indeces[2]+1; i++)
		a[i] = NULL;
	for(i = 0; i < indeces[2]; i++){
		a[i] = malloc(strlen(args[i])+1);
		a[i] = strcpy(a[i],args[i]);
	}

	if(strcmp(args[indeces[2]],"<") == 0){
		if((fd = open(filename, O_RDONLY)) < 0){
			fprintf(stderr,RD_ERR);
			return;
		}
		dup2(fd,0);
	}
	else if(strcmp(args[indeces[2]],">") == 0){
		if((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU)) < 0){
			fprintf(stderr,RD_ERR);
			return;
		}
		dup2(fd,1);
	}
	else if(strcmp(args[indeces[2]],">>") == 0){
		if((fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, S_IRWXU)) < 0){
			fprintf(stderr,RD_ERR);
			return;
		}
		dup2(fd,1);
	}
	else{
		if((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU)) < 0){
			fprintf(stderr,RD_ERR);
			return;
		}
		dup2(fd,2);
	}
	close(fd);
	r_two(a,indeces);
	return;
}


void sigchld_handler(int sig){
	sigchld_flag = 1;
	return;
}

void sigusr1_handler(int sig){
	sigusr1_flag = 1;
	return;
}
