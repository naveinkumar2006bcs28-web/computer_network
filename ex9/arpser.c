
/*
 *  TCP ARP Simulation Server (iterative, with authentication)
 *  Compile : gcc arpserver.c -o arp_srv
 *  Run     : ./arp_srv [port] [bind_ip]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUF_LEN       1024
#define LOGIN_RETRIES 3
#define PASSWD_FILE   "credentials.txt"
#define TABLE_FILE    "arptable.txt"

/* verify a user/pass pair against the stored file */
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

/* search the arp table for a given IP and return the MAC */
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

/* handle login for a connected client; returns 1 on success */
static int handle_login(int cfd)
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
            printf(">> '%s' logged in.\n", name);
            return 1;
        }
        send(cfd, "AUTH_FAIL", 9, 0);
        printf(">> bad credentials (attempt %d/%d)\n", try, LOGIN_RETRIES);
    }
    return 0;
}

/* serve ARP queries from one authenticated client */
static void handle_queries(int cfd)
{
    char buf[BUF_LEN], resp[BUF_LEN], mac[64];
    struct in_addr tmp;
    int nb;

    while (1) {
        nb = recv(cfd, buf, BUF_LEN - 1, 0);
        if (nb <= 0) {
            printf(">> Client disconnected.\n");
            break;
        }
        buf[nb] = '\0';
        buf[strcspn(buf, "\r\n")] = '\0';

        if (strcmp(buf, "exit") == 0) {
            send(cfd, "Goodbye", 7, 0);
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

int main(int argc, char *argv[])
{
    int lfd, cfd, reuse = 1, port;
    char host_ip[64] = "", line[64];
    struct sockaddr_in srv, cli;
    socklen_t cli_len = sizeof(cli);

    signal(SIGPIPE, SIG_IGN);

    /* port and bind IP: from command line or asked at runtime */
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

    printf(">> ARP Server listening on %s:%d\n",
           host_ip[0] ? host_ip : "0.0.0.0", port);

    /* iterative loop: one client at a time */
    while (1) {
        cfd = accept(lfd, (struct sockaddr *)&cli, &cli_len);
        if (cfd < 0) {
            perror("accept error");
            continue;
        }
        printf("\n>> Client connected from %s:%d\n",
               inet_ntoa(cli.sin_addr), ntohs(cli.sin_port));

        if (handle_login(cfd))
            handle_queries(cfd);
        else
            printf(">> Authentication failed. Dropping client.\n");

        close(cfd);
        printf(">> Waiting for next client...\n");
    }

    close(lfd);
    return 0;
}

