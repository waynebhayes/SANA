char USAGE[] = "\n\
USAGE: parallel [-s [shell-name]] {n} \n\
parallel(1) reads command lines from the standard input, and keeps\n\
{n} of them running concurrently.  It defaults to using the SHELL\n\
environment variable as the executor; '-s' by itself will use the\n\
Bourne Shell, and -s shell-name will use whatever shell you prefer\n\
like rc(1).  To distinguish '-s {n}' from '-s {shell-name}', it assumes\n\
your shell name doesn't start with a digit.";

#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

int parallel;

void SigHandler_changeParallel(int sig)
{
    switch(sig)
    {
	case SIGUSR1: fprintf(stderr, "parallel: increased parallel by 1 to %d\n", ++parallel); break;
	case SIGUSR2: fprintf(stderr, "parallel: decreased parallel by 1 to %d\n", --parallel);
	    if(parallel < 1)
	    {
		fprintf(stderr, "parallel: WARNING: parallel cannot be 0, resetting to 1, \n");
		parallel = 1;
	    }
	    break;
    }
    signal(SIGUSR1, SigHandler_changeParallel);
    signal(SIGUSR2, SigHandler_changeParallel);
}

void Fatal(char *s)
{
    fprintf(stderr, "%s\n", s);
    exit(1);
}


int main(int argc, char *argv[])
{
    int n=0, n_arg = 1, status;
    char line[1024000];
    char *SHELL = getenv("SHELL"), *SH_argv0;

    switch(argc)
    {
    case 2:	/* no args other than n */
	n_arg = 1;
	break;

    case 3:	/* parallel -s n */
	if(strcmp(argv[1], "-s"))
	    Fatal(USAGE);
	n_arg = 2;
	SHELL = "/bin/sh";
	break;

    case 4:	/* parallel -s SHELL n */
	if(strcmp(argv[1], "-s"))
	    Fatal(USAGE);
	SHELL = argv[2];
	n_arg = 3;
	break;

    default:
	Fatal(USAGE);
	break;
    }

    signal(SIGUSR1, SigHandler_changeParallel);
    signal(SIGUSR2, SigHandler_changeParallel);

    SH_argv0 = rindex(SHELL, '/');	/* basename of SHELL */
    if(SH_argv0 == NULL)
	SH_argv0 = SHELL;

    parallel = atoi(argv[n_arg]);
    while(fgets(line, sizeof(line), stdin))
    {
	while(n >= parallel)
	{
	    wait(&status);
	    --n;
	}
	if(fork() == 0)  /* child */
	{
	    execlp(SHELL, SH_argv0, "-c", line, NULL);
	    perror(SHELL);
	    exit(1);
	}
	else
	{
	    if(++n >= parallel)
	    {
		wait(&status);
		--n;
	    }
	}
    }
#if 1
    while(n-- > 0)  /* wait for everyone to finish */
	wait(&parallel); // we no longer need the value of parallel so nuke it with wait status.
#endif
    return 0;
}
