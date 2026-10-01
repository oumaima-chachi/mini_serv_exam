#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>//!add
#include <stdio.h>//!add
#include <stdlib.h>//!add

int sockfd, nfds, id = 0, ids[1024], idx[1024];
char buf[600020], msg[600000], clients[1024][600000];
fd_set fds, rfds;

void err(char *s)
{
    write(2, s, strlen(s));
    exit(1);
}

void send_all(int author)
{
    for (int i = 0; i < nfds; i++)
    {
        if (i != sockfd && i != author && FD_ISSET(i, &fds))
            send(i, buf, strlen(buf), 0);

    }
}
int main(int ac, char **av)
{
    if (ac != 2)
        err("Wrong number of arguments\n");

    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(2130706433); // 127.0.0.1
    addr.sin_port = htons(atoi(av[1]));

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        err("Fatal error\n");

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        err("Fatal error\n");

    if (listen(sockfd, 10) < 0)
        err("Fatal error\n");

    FD_ZERO(&fds);
    FD_SET(sockfd, &fds);
    nfds = sockfd + 1;

    while (1)
    {
        rfds = fds;
        if (select(nfds, &rfds, NULL, NULL, NULL) < 0) err("Fatal error\n");
 
        for (int fd = 0; fd < nfds; fd++)
        {
            if (!FD_ISSET(fd, &rfds)) continue;

            if (fd == sockfd)
            {
                int cli = accept(sockfd, NULL, NULL);
                if (cli >= 0)
                {
                    FD_SET(cli, &fds);
                    if (cli >= nfds) nfds = cli + 1;
                    ids[cli] = id++;
                    idx[cli] = 0;
                    sprintf(buf, "server: client %d just arrived\n", ids[cli]);
                    send_all(cli);
                }
            }
            else
            {
                int r = recv(fd, msg, sizeof(msg) - 1, 0);
                if (r <= 0) {
                    sprintf(buf, "server: client %d just left\n", ids[fd]);
                    send_all(fd);
                    FD_CLR(fd, &fds);
                    close(fd);
                }
                else
                {
                    for (int i = 0; i < r; i++)
                    {
                        clients[fd][idx[fd]++] = msg[i];
                        if (msg[i] == '\n')
                        {
                            clients[fd][idx[fd]] = '\0';
                            sprintf(buf, "client %d: %s", ids[fd], clients[fd]);
                            send_all(fd);
                            idx[fd] = 0;
                        }
                    }
                }
            }
        }
    }
}