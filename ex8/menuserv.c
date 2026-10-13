
/*
 *  Menu-Driven TCP Server (ARP Lookup + Full Duplex Chat)
 *  Iterative: one client at a time.
 *  Compile : gcc menuserver.c -o menu_srv
 *  Run     : ./menu_srv [port] [bind_ip]
 *
 *  Files needed:
 *    credentials.txt  -> "username password" per line
 *    arptable.txt     -> "IP MAC" per line
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUF_LEN       1024
#define LOGIN_RETRIES 3
#define PASSWD_FILE   "credentials.txt"
#define TABLE_FILE    "arptable.txt"

/* ===================== AUTHENTICATION ===================== */

static int verify_user(const char *name, const char *pw)
{
    FILE *fp = fopen(PASSWD_FILE, "r");
    char fname[64], fpw[64];
    int found = 0;

    if (!fp) {
        perror("could not open password file");
        return 0;
    }
    while (fscanf(fp, "%63s %63s", fname, fpw) == 2) {
        if (strcmp(fname, name) == 0 && strcmp(fpw, pw) == 0) {
            found = 1;
            break;
        }
    }
    fclose(fp);
    return found;
}

static int handle_login(int cfd, char *uname, size_t usize)
{
    char buf[BUF_LEN], name[64], pw[64];
    int try, nb;

    for (try = 1; try <= LOGIN_RETRIES; try++) {
        nb = recv(cfd, buf, BUF_LEN - 1, 0);
        if (nb <= 0)
            return 0;
        buf[nb] = '\0';

        if (sscanf(buf, "%63s %63s", name, pw) == 2 &&
            verify_user(name, pw)) {
            send(cfd, "AUTH_OK", 7, 0);
            snprintf(uname, usize, "%s", name);
            printf(">> '%s' logged in.\n", name);
            return 1;
        }
        send(cfd, "AUTH_FAIL", 9, 0);
        printf(">> bad credentials (attempt %d/%d)\n", try, LOGIN_RETRIES);
    }
    return 0;
}

/* ===================== ARP LOOKUP ===================== */

static int find_mac(const char *ip, char *mac, size_t mac_sz)
{
    FILE *fp = fopen(TABLE_FILE, "r");
    char tbl_ip[64], tbl_mac[64];
    int hit = 0;

    if (!fp) {
        perror("could not open ARP table file");
        return 0;
    }
    while (fscanf(fp, "%63s %63s", tbl_ip, tbl_mac) == 2) {
        if (strcmp(tbl_ip, ip) == 0) {
            snprintf(mac, mac_sz, "%s", tbl_mac);
            hit = 1;
            break;
        }
    }
    fclose(fp);
    return hit;
}

static void handle_arp(int cfd)
{
    char buf[BUF_LEN], resp[BUF_LEN], mac[64];
    struct in_addr tmp;
    int nb;

    send(cfd, "ARP_READY", 9, 0);
    printf(">> ARP mode started.\n");

    while (1) {
        nb = recv(cfd, buf, BUF_LEN - 1, 0);
        if (nb <= 0) {
            printf(">> Client disconnected during ARP.\n");
            break;
        }
        buf[nb] = '\0';
        buf[strcspn(buf, "\r\n")] = '\0';

        if (strcmp(buf, "back") == 0) {
            send(cfd, "MENU_BACK", 9, 0);
            printf(">> Client returned to menu.\n");
            break;
        }

        if (strcmp(buf, "exit") == 0) {
            send(cfd, "SESSION_END", 11, 0);
            printf(">> Client ended the session.\n");
            break;
        }

        printf("  ARP request : who has %s ?\n", buf);

        if (inet_pton(AF_INET, buf, &tmp) != 1) {
            snprintf(resp, sizeof(resp),
                     "Error: '%s' is not a valid IPv4 address", buf);
        } else if (find_mac(buf, mac, sizeof(mac))) {
            snprintf(resp, sizeof(resp),
                     "ARP Reply : %s is at %s", buf, mac);
        } else {
            snprintf(resp, sizeof(resp),
                     "ARP : no entry found for %s", buf);
        }
        send(cfd, resp, strlen(resp), 0);
    }
}

/* ===================== FULL DUPLEX CHAT ===================== */

static void handle_chat(int cfd, const char *uname)
{
    fd_set rfds;
    char buf[BUF_LEN];
    int nb;

    send(cfd, "CHAT_READY", 10, 0);
    printf(">> Chat with '%s' started. Type 'bye' to end.\n\n", uname);

    while (1) {
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);
        FD_SET(cfd, &rfds);

        if (select(cfd + 1, &rfds, NULL, NULL, NULL) < 0) {
            perror("select error");
            break;
        }

        /* server typed something — send it */
        if (FD_ISSET(STDIN_FILENO, &rfds)) {
            nb = read(STDIN_FILENO, buf, BUF_LEN - 2);
            if (nb <= 0)
                break;
            buf[nb] = '\0';
            if (send(cfd, buf, nb, 0) < 0)
                break;
            if (strncmp(buf, "bye", 3) == 0) {
                printf(">> You ended the chat.\n");
                break;
            }
        }

        /* client sent something — display it */
        if (FD_ISSET(cfd, &rfds)) {
            nb = recv(cfd, buf, BUF_LEN - 1, 0);
            if (nb <= 0) {
                printf(">> [%s disconnected]\n", uname);
                break;
            }
            buf[nb] = '\0';

            /* client wants to go back to menu */
            if (strcmp(buf, "back\n") == 0 || strcmp(buf, "back") == 0) {
                printf(">> Client returned to menu.\n");
                send(cfd, "MENU_BACK\n", 10, 0);
                break;
            }

            printf("  [%s] %s", uname, buf);
            if (buf[nb - 1] != '\n')
                printf("\n");
            if (strncmp(buf, "bye", 3) == 0) {
                printf(">> [%s ended the chat]\n", uname);
                break;
            }
        }
    }
}

/* ===================== MENU LOOP ===================== */

static void serve_client(int cfd, const char *uname)
{
    char buf[BUF_LEN];
    int nb, running = 1;

    while (running) {
        /* send menu to client */
        send(cfd, "MENU", 4, 0);

        nb = recv(cfd, buf, BUF_LEN - 1, 0);
        if (nb <= 0) {
            printf(">> Client disconnected.\n");
            break;
        }
        buf[nb] = '\0';
        buf[strcspn(buf, "\r\n")] = '\0';

        if (strcmp(buf, "1") == 0) {
            handle_arp(cfd);
        } else if (strcmp(buf, "2") == 0) {
            handle_chat(cfd, uname);
        } else if (strcmp(buf, "3") == 0) {
            send(cfd, "GOODBYE", 7, 0);
            printf(">> Client chose to quit.\n");
            running = 0;
        } else {
            send(cfd, "ERR_INVALID", 11, 0);
        }
    }
}

/* ===================== MAIN ===================== */

int main(int argc, char *argv[])
{
    int lfd, cfd, reuse = 1, port;
    char host_ip[64] = "", line[64], uname[64];
    struct sockaddr_in srv, cli;
    socklen_t cli_len = sizeof(cli);

    signal(SIGPIPE, SIG_IGN);

    if (argc > 1) {
        port = atoi(argv[1]);
        if (argc > 2)
            snprintf(host_ip, sizeof(host_ip), "%s", argv[2]);
    } else {
        printf("Enter port to listen on: ");
        if (fgets(line, sizeof(line), stdin) == NULL)
            return 1;
        port = atoi(line);
        printf("Enter IP to bind (press Enter for all interfaces): ");
        if (fgets(host_ip, sizeof(host_ip), stdin) == NULL)
            host_ip[0] = '\0';
        host_ip[strcspn(host_ip, "\n")] = '\0';
    }
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid port number\n");
        return 1;
    }

    if ((lfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    memset(&srv, 0, sizeof(srv));
    srv.sin_family = AF_INET;
    srv.sin_port   = htons(port);
    if (host_ip[0] == '\0')
        srv.sin_addr.s_addr = htonl(INADDR_ANY);
    else if (inet_pton(AF_INET, host_ip, &srv.sin_addr) != 1) {
        fprintf(stderr, "Invalid bind IP address\n");
        close(lfd);
        return 1;
    }

    if (bind(lfd, (struct sockaddr *)&srv, sizeof(srv)) < 0) {
        perror("bind error");
        close(lfd);
        exit(EXIT_FAILURE);
    }
    if (listen(lfd, 5) < 0) {
        perror("listen error");
        close(lfd);
        exit(EXIT_FAILURE);
    }

    printf(">> Menu Server listening on %s:%d\n",
           host_ip[0] ? host_ip : "0.0.0.0", port);

    while (1) {
        cfd = accept(lfd, (struct sockaddr *)&cli, &cli_len);
        if (cfd < 0) {
            perror("accept error");
            continue;
        }
        printf("\n>> Client connected from %s:%d\n",
               inet_ntoa(cli.sin_addr), ntohs(cli.sin_port));

        if (handle_login(cfd, uname, sizeof(uname)))
            serve_client(cfd, uname);
        else
            printf(">> Authentication failed. Dropping client.\n");

        close(cfd);
        printf(">> Waiting for next client...\n");
    }

    close(lfd);
    return 0;
}

