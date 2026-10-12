#include <netinet/in.h> //structure for storing address information
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h> //for socket APIs
#include <sys/types.h>
#include "list.h"

#define PORT 9001
#define ACK "ACK"
#define BUFSZ 1024

static volatile sig_atomic_t stop = 0;

static void handle_sigint(int sig) {
    (void)sig;
    stop = 1;   /* main loop notices (recv/accept return EINTR) and cleans up */
}

/* Parse the next token as an int. Returns 0 on success, -1 if missing/invalid. */
static int next_int(int *out) {
    char *tok = strtok(NULL, " ");
    char *end;
    if (!tok) return -1;
    long v = strtol(tok, &end, 10);
    if (*end != '\0') return -1;
    *out = (int)v;
    return 0;
}

int main(int argc, char const* argv[])
{
    (void)argc; (void)argv;
    int n, val, idx;
    int servSockD = socket(AF_INET, SOCK_STREAM, 0);
    int clientSocket = -1;
    char buf[BUFSZ];
    char sbuf[BUFSZ];
    char *token;
    list_t *mylist;
    struct sockaddr_in servAddr;

    if (servSockD < 0) { perror("socket"); return 1; }

    // allow quick restart of the server on the same port
    int opt = 1;
    setsockopt(servSockD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Ctrl-C: no SA_RESTART so blocking accept()/recv() are interrupted
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(PORT);
    servAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(servSockD, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
        perror("bind");
        close(servSockD);
        return 1;
    }
    if (listen(servSockD, 1) < 0) {
        perror("listen");
        close(servSockD);
        return 1;
    }

    mylist = list_alloc();  // create the list
    if (!mylist) { close(servSockD); return 1; }

    int done = 0;
    while (!stop && !done) {
        clientSocket = accept(servSockD, NULL, NULL);
        if (clientSocket < 0) {
            if (errno == EINTR) continue;
            perror("accept");
            break;
        }

        while (!stop) {
            n = recv(clientSocket, buf, sizeof(buf) - 1, 0);
            if (n < 0 && errno == EINTR) continue;
            if (n <= 0) break;          // client disconnected (or error): wait for next client
            buf[n] = '\0';

            memset(sbuf, '\0', sizeof(sbuf));
            token = strtok(buf, " ");
            if (!token) {
                snprintf(sbuf, sizeof(sbuf), "ERROR: empty command");
            }
            else if (strcmp(token, "exit") == 0) {
                done = 1;
                break;
            }
            else if (strcmp(token, "menu") == 0) {
                snprintf(sbuf, sizeof(sbuf), "%s", ACK);
            }
            else if (strcmp(token, "print") == 0) {
                char *s = listToString(mylist);
                snprintf(sbuf, sizeof(sbuf), "%s", s ? s : "ERROR: out of memory");
                free(s);
            }
            else if (strcmp(token, "get_length") == 0) {
                val = list_length(mylist);
                snprintf(sbuf, sizeof(sbuf), "Length = %d", val);
            }
            else if (strcmp(token, "add_back") == 0) {
                if (next_int(&val) < 0) snprintf(sbuf, sizeof(sbuf), "ERROR: usage: add_back <value>");
                else { list_add_to_back(mylist, val); snprintf(sbuf, sizeof(sbuf), "%s%d", ACK, val); }
            }
            else if (strcmp(token, "add_front") == 0) {
                if (next_int(&val) < 0) snprintf(sbuf, sizeof(sbuf), "ERROR: usage: add_front <value>");
                else { list_add_to_front(mylist, val); snprintf(sbuf, sizeof(sbuf), "%s%d", ACK, val); }
            }
            else if (strcmp(token, "add_position") == 0) {
                if (next_int(&idx) < 0 || next_int(&val) < 0)
                    snprintf(sbuf, sizeof(sbuf), "ERROR: usage: add_position <index> <value>");
                else if (idx < 0 || idx > list_length(mylist))
                    snprintf(sbuf, sizeof(sbuf), "ERROR: index out of range");
                else { list_add_at_index(mylist, val, idx); snprintf(sbuf, sizeof(sbuf), "%s%d", ACK, val); }
            }
            else if (strcmp(token, "remove_back") == 0) {
                if (list_length(mylist) == 0) snprintf(sbuf, sizeof(sbuf), "ERROR: list is empty");
                else { val = list_remove_from_back(mylist); snprintf(sbuf, sizeof(sbuf), "%s%d", ACK, val); }
            }
            else if (strcmp(token, "remove_front") == 0) {
                if (list_length(mylist) == 0) snprintf(sbuf, sizeof(sbuf), "ERROR: list is empty");
                else { val = list_remove_from_front(mylist); snprintf(sbuf, sizeof(sbuf), "%s%d", ACK, val); }
            }
            else if (strcmp(token, "remove_position") == 0) {
                if (next_int(&idx) < 0) snprintf(sbuf, sizeof(sbuf), "ERROR: usage: remove_position <index>");
                else if (idx < 0 || idx >= list_length(mylist))
                    snprintf(sbuf, sizeof(sbuf), "ERROR: index out of range");
                else { val = list_remove_at_index(mylist, idx); snprintf(sbuf, sizeof(sbuf), "%s%d", ACK, val); }
            }
            else if (strcmp(token, "get") == 0) {
                if (next_int(&idx) < 0) snprintf(sbuf, sizeof(sbuf), "ERROR: usage: get <index>");
                else if (idx < 0 || idx >= list_length(mylist))
                    snprintf(sbuf, sizeof(sbuf), "ERROR: index out of range");
                else { val = list_get_elem_at(mylist, idx); snprintf(sbuf, sizeof(sbuf), "%s%d", ACK, val); }
            }
            else {
                snprintf(sbuf, sizeof(sbuf), "ERROR: unknown command");
            }

            // send NUL-terminated reply (not the whole 1024-byte buffer)
            send(clientSocket, sbuf, strlen(sbuf) + 1, 0);
        }

        close(clientSocket);
        clientSocket = -1;
    }

    // graceful shutdown: free the list, close sockets
    list_free(mylist);
    close(servSockD);
    if (stop) printf("\nServer interrupted, cleaned up.\n");
    return 0;
}