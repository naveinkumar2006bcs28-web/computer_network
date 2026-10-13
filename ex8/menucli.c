
/*
 *  Menu-Driven TCP Client (ARP Lookup + Full Duplex Chat)
 *  Compile : gcc menuclient.c -o menu_cli
 *  Run     : ./menu_cli [server_ip] [port]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUF_LEN       1024
#define LOGIN_RETRIES 3

/* ===================== HELPERS ===================== */

/* read a password while printing '*' for every keystroke */
static void masked_input(char *dest, size_t cap)
{
    struct termios orig, raw;
    size_t pos = 0;
    int ch;

    tcgetattr(STDIN_FILENO, &orig);
    raw = orig;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);

    while ((ch = getchar()) != '\n' && ch != EOF) {
        if (ch == 127 || ch == 8) {
            if (pos > 0) {
                pos--;
                printf("\b \b");
                fflush(stdout);
            }
        } else if (pos < cap - 1) {
            dest[pos++] = (char)ch;
            putchar('*');
            fflush(stdout);
        }
    }
    dest[pos] = '\0';

    tcsetattr(STDIN_FILENO, TCSANOW, &orig);
    printf("\n");
}

/* ===================== AUTHENTICATION ===================== */

static int do_login(int sfd)
{
    char name[64], pw[64], pkt[BUF_LEN], resp[BUF_LEN];
    int try, nb;

    for (try = 1; try <= LOGIN_RETRIES; try++) {
        printf("Username : ");
        if (fgets(name, sizeof(name), stdin) == NULL)
            return 0;
        name[strcspn(name, "\n")] = '\0';

        printf("Password : ");
        fflush(stdout);
        masked_input(pw, sizeof(pw));

        snprintf(pkt, sizeof(pkt), "%s %s", name, pw);
        if (send(sfd, pkt, strlen(pkt), 0) < 0) {
            perror("send error");
            return 0;
        }

        nb = recv(sfd, resp, BUF_LEN - 1, 0);
        if (nb <= 0) {
            printf("Server closed the connection.\n");
            return 0;
        }
        resp[nb] = '\0';

        if (strcmp(resp, "AUTH_OK") == 0) {
            printf("Logged in successfully.\n\n");
            return 1;
        }
        printf("Wrong credentials (attempt %d of %d)\n\n",
               try, LOGIN_RETRIES);
    }
    return 0;
}

/* ===================== ARP MODE ===================== */

static int do_arp(int sfd)
{
    char buf[BUF_LEN], resp[BUF_LEN];
    int nb;

    /* wait for server to confirm ARP mode */
    nb = recv(sfd, resp, BUF_LEN - 1, 0);
    if (nb <= 0)
        return -1;
    resp[nb] = '\0';
    if (strcmp(resp, "ARP_READY") != 0) {
        printf("  Server: %s\n", resp);
        return -1;
    }

    printf("\n── ARP Lookup Mode ──\n");
    printf("Enter an IP to get its MAC. Type 'back' for menu, 'exit' to quit.\n\n");

    while (1) {
        printf("Query IP > ");
        if (fgets(buf, BUF_LEN, stdin) == NULL)
            return -1;
        buf[strcspn(buf, "\n")] = '\0';

        if (strlen(buf) == 0)
            continue;

        if (send(sfd, buf, strlen(buf), 0) < 0) {
            perror("send error");
            return -1;
        }

        if (strcmp(buf, "exit") == 0) {
            nb = recv(sfd, resp, BUF_LEN - 1, 0);
            if (nb > 0) {
                resp[nb] = '\0';
            }
            return 1;  /* signal: quit entirely */
        }

        nb = recv(sfd, resp, BUF_LEN - 1, 0);
        if (nb <= 0) {
            printf("Server closed the connection.\n");
            return -1;
        }
        resp[nb] = '\0';

        if (strcmp(resp, "MENU_BACK") == 0) {
            printf("\n");
            return 0;  /* back to menu */
        }

        printf("  >> %s\n\n", resp);
    }
}

/* ===================== CHAT MODE ===================== */

static int do_chat(int sfd)
{
    fd_set rfds;
    char buf[BUF_LEN], resp[BUF_LEN];
    int nb;

    /* wait for server to confirm chat mode */
    nb = recv(sfd, resp, BUF_LEN - 1, 0);
    if (nb <= 0)
        return -1;
    resp[nb] = '\0';
    if (strcmp(resp, "CHAT_READY") != 0) {
        printf("  Server: %s\n", resp);
        return -1;
    }

    printf("\n── Full Duplex Chat ──\n");
    printf("Type freely. Type 'bye' to end, 'back' for menu.\n\n");

    while (1) {
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);
        FD_SET(sfd, &rfds);

        if (select(sfd + 1, &rfds, NULL, NULL, NULL) < 0) {
            perror("select error");
            return -1;
        }

        /* you typed something */
        if (FD_ISSET(STDIN_FILENO, &rfds)) {
            nb = read(STDIN_FILENO, buf, BUF_LEN - 2);
            if (nb <= 0)
                return -1;
            buf[nb] = '\0';

            if (send(sfd, buf, nb, 0) < 0)
                return -1;

            if (strncmp(buf, "back", 4) == 0) {
                /* wait for server ack */
                nb = recv(sfd, resp, BUF_LEN - 1, 0);
                printf("\n");
                return 0;  /* back to menu */
            }
            if (strncmp(buf, "bye", 3) == 0) {
                printf(">> You ended the chat.\n\n");
                return 1;  /* quit entirely */
            }
        }

        /* server sent something */
        if (FD_ISSET(sfd, &rfds)) {
            nb = recv(sfd, buf, BUF_LEN - 1, 0);
            if (nb <= 0) {
                printf(">> [Server disconnected]\n");
                return -1;
            }
            buf[nb] = '\0';

            if (strcmp(buf, "MENU_BACK\n") == 0 || strcmp(buf, "MENU_BACK") == 0) {
                printf(">> Server returned you to menu.\n\n");
                return 0;
            }

            printf("  [Server] %s", buf);
            if (buf[nb - 1] != '\n')
                printf("\n");
            if (strncmp(buf, "bye", 3) == 0) {
                printf(">> [Server ended the chat]\n\n");
                return 1;
            }
        }
    }
}

/* ===================== MENU DISPLAY ===================== */

static void show_menu(void)
{
    printf("=============================\n");
    printf("         MAIN MENU\n");
    printf("=============================\n");
    printf("  1. ARP Lookup\n");
    printf("  2. Full Duplex Chat\n");
    printf("  3. Quit\n");
    printf("=============================\n");
    printf("Choose > ");
    fflush(stdout);
}

/* ===================== MAIN ===================== */

int main(int argc, char *argv[])
{
    int sfd, port, nb, running;
    struct sockaddr_in srv;
    char host[64], line[64], buf[BUF_LEN], resp[BUF_LEN];
    int result;

    /* server IP and port: from command line or asked at runtime */
    if (argc > 2) {
        snprintf(host, sizeof(host), "%s", argv[1]);
        port = atoi(argv[2]);
    } else {
        printf("Enter server IP (press Enter for 127.0.0.1): ");
        if (fgets(host, sizeof(host), stdin) == NULL)
            return 1;
        host[strcspn(host, "\n")] = '\0';
        if (host[0] == '\0')
            snprintf(host, sizeof(host), "%s", "127.0.0.1");
        printf("Enter server port: ");
        if (fgets(line, sizeof(line), stdin) == NULL)
            return 1;
        port = atoi(line);
    }
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid port number\n");
        return 1;
    }

    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    memset(&srv, 0, sizeof(srv));
    srv.sin_family = AF_INET;
    srv.sin_port   = htons(port);
    if (inet_pton(AF_INET, host, &srv.sin_addr) <= 0) {
        fprintf(stderr, "Bad server address\n");
        close(sfd);
        exit(EXIT_FAILURE);
    }

    if (connect(sfd, (struct sockaddr *)&srv, sizeof(srv)) < 0) {
        perror("connect error");
        close(sfd);
        exit(EXIT_FAILURE);
    }
    printf(">> Connected to server %s:%d\n\n", host, port);

    /* must authenticate first */
    if (!do_login(sfd)) {
        printf("Access denied — goodbye.\n");
        close(sfd);
        return 1;
    }

    /* menu loop */
    running = 1;
    while (running) {
        /* wait for server MENU signal */
        nb = recv(sfd, resp, BUF_LEN - 1, 0);
        if (nb <= 0) {
            printf("Server closed the connection.\n");
            break;
        }
        resp[nb] = '\0';

        if (strcmp(resp, "GOODBYE") == 0) {
            printf(">> Server says goodbye.\n");
            break;
        }

        if (strcmp(resp, "MENU") != 0) {
            printf("  Server: %s\n", resp);
            continue;
        }

        show_menu();
        if (fgets(buf, sizeof(buf), stdin) == NULL)
            break;
        buf[strcspn(buf, "\n")] = '\0';

        /* send choice to server */
        if (send(sfd, buf, strlen(buf), 0) < 0) {
            perror("send error");
            break;
        }

        if (strcmp(buf, "1") == 0) {
            result = do_arp(sfd);
            if (result < 0) { running = 0; }
            else if (result == 1) { running = 0; }
            /* result == 0 means back to menu */
        } else if (strcmp(buf, "2") == 0) {
            result = do_chat(sfd);
            if (result < 0) { running = 0; }
            else if (result == 1) { running = 0; }
        } else if (strcmp(buf, "3") == 0) {
            nb = recv(sfd, resp, BUF_LEN - 1, 0);
            if (nb > 0) {
                resp[nb] = '\0';
            }
            printf(">> Goodbye.\n");
            running = 0;
        } else {
            nb = recv(sfd, resp, BUF_LEN - 1, 0);
            if (nb > 0) {
                resp[nb] = '\0';
                printf("  Server: %s\n\n", resp);
            }
        }
    }

    close(sfd);
    return 0;
}

