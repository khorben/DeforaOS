/* inetd.c */



#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>


/* inetd */
static int _inetd_error(char const * message, int ret);
static int _inetd_do(void);
static void _inetd_sigchld(int signum);
static void _inetd_accept(int fd, struct sockaddr_in addr, int addrlen);
static int _inetd(void)
{
	return _inetd_do();
}

static int _inetd_do(void)
{
	int fd;
	struct sockaddr_in sa;
	struct sockaddr_in sa_conn;
	int sa_size = sizeof(struct sockaddr_in);
	int conn;

	if(signal(SIGCHLD, _inetd_sigchld) == SIG_ERR)
		_inetd_error("signal", 0);
	if((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		return _inetd_error("socket", 1);
	sa.sin_family = AF_INET;
	sa.sin_port = htons(10007);
	sa.sin_addr.s_addr = INADDR_ANY;
	if(bind(fd, &sa, sizeof(sa)) != 0)
		return _inetd_error("bind", 1);
	if(listen(fd, 5) != 0)
		return _inetd_error("listen", 1);
	while((conn = accept(fd, &sa_conn, &sa_size)) != 0)
		_inetd_accept(conn, sa_conn, sa_size);
	return 0;
}

static int _inetd_error(char const * message, int ret)
{
	fprintf(stderr, "%s", "inetd: ");
	perror(message);
	return ret;
}

static void _inetd_sigchld(int signum)
{
	pid_t pid;
	int status;

	if(signal(SIGCHLD, _inetd_sigchld) == SIG_ERR)
		_inetd_error("signal", 0);
	if((pid = waitpid(-1, &status, WNOHANG)) == -1)
	{
		_inetd_error("waitpid", 0);
		return;
	}
	fprintf(stderr, "%s%d%s%s%d%s", "Child ", pid,
			WIFEXITED(status) ? " exited" : " was terminated",
			" with error code ", WEXITSTATUS(status), "\n");
}

static void _inetd_log(struct sockaddr_in * addr, int addrlen);
static void _inetd_client(int fd, struct sockaddr_in * addr, int addrlen);
static void _inetd_accept(int fd, struct sockaddr_in addr, int addrlen)
{
	pid_t pid;

	if((pid = fork()) == -1)
	{
		_inetd_error("fork", 0);
		return;
	}
	if(pid > 0)
	{
		_inetd_log(&addr, addrlen);
		close(fd);
		return;
	}
	_inetd_client(fd, &addr, addrlen);
}

static void _inetd_log(struct sockaddr_in * addr, int addrlen)
{
	uint8_t * ip = (uint8_t*)&(addr->sin_addr.s_addr);

	fprintf(stderr, "%s%d.%d.%d.%d:%d\n", "Connection from ",
			ip[0], ip[1], ip[2], ip[3], ntohs(addr->sin_port));
}

static void _inetd_client(int fd, struct sockaddr_in * addr, int addrlen)
{
	if(close(0) != 0 || close(1) != 0
			|| dup2(fd, 0) != 0 || dup2(fd, 1) != 1)
	{
		_inetd_error("dup2", 0);
		return;
	}
	execl("./echo", "echo", NULL);
	_inetd_error("exec", 0);
	exit(2);
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: inetd\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	if(argc != 1)
		return _usage();
	return _inetd() ? 2 : 0;
}
