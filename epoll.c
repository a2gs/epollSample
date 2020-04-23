/* Andre Augusto Giannotti Scota (https://sites.google.com/view/a2gs/) */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>

int createOpenNamedPipe(char *name)
{
	int fd = 0;

	if(mkfifo(name, S_IRUSR|S_IWUSR) == -1){
		printf("mkfifo(%s) error: [%s]\n", name, strerror(errno));
		return(-1);
	}

	fd = open(name, O_RDWR, S_IRUSR|S_IWUSR);
	if(fd == -1){
		printf("open(%s) error: [%s]\n", name, strerror(errno));
		return(-1);
	}

	return(fd);
}

char * printDescriptorEvent(uint32_t event)
{
	if(event & EPOLLIN)        return("EPOLLIN - The associated file is available for read(2) operations");
	if(event & EPOLLOUT)       return("EPOLLOUT -  The associated file is available for write(2) operations");
	if(event & EPOLLRDHUP)     return("EPOLLRDHUP - Stream socket peer closed connection, or shut down writing half of connection");
	if(event & EPOLLPRI)       return("EPOLLPRI - There is an exceptional condition on the file descriptor");
	if(event & EPOLLERR)       return("EPOLLERR - Error  condition happened on the associated file descriptor");
	if(event & EPOLLHUP)       return("EPOLLHUP - Hang up happened on the associated file descriptor");
	if(event & EPOLLET)        return("EPOLLET - Sets  the  Edge  Triggered  behavior  for  the associated file descriptor");
	if(event & EPOLLONESHOT)   return("EPOLLONESHOT - Sets the one-shot behavior for the associated file descriptor");
	if(event & EPOLLWAKEUP)    return("EPOLLWAKEUP - RTFM");
	if(event & EPOLLEXCLUSIVE) return("EPOLLEXCLUSIVE - Sets an exclusive wakeup mode for the epoll file descriptor that is being attached to the  target  file  descriptor");

	return(NULL);
}

#define MAX_EVENTS_FD (3)
#define MAX_MSG_SZ (50)
#define EVENT_FD_TIMEOUT (30000)

int main(int argc, char *argv[])
{
	int fd1 = 0, fd2 = 0, fd3 = 0;
	int epollFd = 0;
	int eventCount = 0;
	int ret = 0, i = 0;
	ssize_t bytesRead = 0;
	char msg[MAX_MSG_SZ + 1] = {0};
	struct epoll_event waitingEvents[MAX_EVENTS_FD];
	struct epoll_event event;

	/* ------------------------------ */
	ret = 0;

	fd1 = createOpenNamedPipe("AAA");
	if(fd1 == -1) return(-1);

	fd2 = createOpenNamedPipe("BBB");
	if(fd2 == -1) return(-1);

	fd3 = createOpenNamedPipe("CCC");
	if(fd3 == -1) return(-1);

	printf("Descriptors: AAA [%d] | BBB [%d] | CCC [%d]\n", fd1, fd2, fd3);

	/* ------------------------------ */

	epollFd = epoll_create1(0);
	if(epollFd == -1){
		printf("epoll_create1() error: [%s]\n", strerror(errno));
		ret = -1;
		goto clean_all;
	}

	/* ------------------------------ */

	event.events  = EPOLLIN;
	event.data.fd = fd1;

	if(epoll_ctl(epollFd, EPOLL_CTL_ADD, fd1, &event) == -1){
		printf("epoll_ctl(event) error: [%s]\n", strerror(errno));
		ret = -1;
		goto clean_all;
	}

	event.events  = EPOLLIN;
	event.data.fd = fd2;

	if(epoll_ctl(epollFd, EPOLL_CTL_ADD, fd2, &event) == -1){
		printf("epoll_ctl(event2) error: [%s]\n", strerror(errno));
		ret = -1;
		goto clean_all;
	}

	event.events  = EPOLLIN;
	event.data.fd = fd3;

	if(epoll_ctl(epollFd, EPOLL_CTL_ADD, fd3, &event) == -1){
		printf("epoll_ctl(event3) error: [%s]\n", strerror(errno));
		ret = -1;
		goto clean_all;
	}

	/* ------------------------------ */

	while(1){
		printf("Listening the poll...\n");

		eventCount = epoll_wait(epollFd, waitingEvents, MAX_EVENTS_FD, EVENT_FD_TIMEOUT);
		if(eventCount == -1){
			printf("epoll_ctl(event3) error: [%s]\n", strerror(errno));
			ret = -1;
			goto clean_all;
		}else if(eventCount == 0){
			printf("Timeout.\n");
			continue;
		}

		for(i = 0; i < eventCount; i++){
			memset(msg, 0, sizeof(msg));

			printf("Reading file descriptor [%d] Event: [%s]\n",
			       waitingEvents[i].data.fd, printDescriptorEvent(waitingEvents[i].events));

			bytesRead = read(waitingEvents[i].data.fd, msg, MAX_MSG_SZ);

			if(bytesRead == -1){
				printf("read() error: [%s]\n", strerror(errno));
				ret = -1;
				goto clean_all;
			}else if(bytesRead == 0){
				printf("There is no more bytes...\n");
			}else{
				printf("Read [%lu] bytes. Msg: [%s]\n", bytesRead, msg);

				if(strncmp(msg, "quit", 4) == 0){
					printf("Quit command! Exit.\n");
					goto clean_all;
				}
			}
		}
	}

	/* ------------------------------ */

clean_all:
	close(epollFd);
	close(fd1);    close(fd2);    close(fd3);
	unlink("AAA"); unlink("BBB"); unlink("CCC");

	return(ret);
}
