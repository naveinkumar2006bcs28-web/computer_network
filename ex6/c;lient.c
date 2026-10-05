#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <termios.h>

#define PORT 9090
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024

int main() {

    int sock_fd;
    struct sockaddr_in server_addr;

    char buffer[BUFFER_SIZE];
    char input[BUFFER_SIZE];
    char choice_str[10];
    char username[BUFFER_SIZE];

    /* Client heading */
    printf("\n=================================\n");
    printf("          TCP CLIENT\n");
    printf("=================================\n");

    /* Create socket */
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    /* Server address */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP,
                  &server_addr.sin_addr) <= 0) {

        perror("Invalid address");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    /* Connect to server */
    if (connect(sock_fd,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0) {

        perror("Connection failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server successfully!\n\n");


    /* ================= USERNAME ================= */

    memset(buffer, 0, BUFFER_SIZE);

    recv(sock_fd,
         buffer,
         BUFFER_SIZE - 1,
         0);

    if (strcmp(buffer, "SEND_USERNAME") == 0) {

        printf("Enter username : ");

        fgets(username,
              BUFFER_SIZE,
              stdin);

        username[strcspn(username, "\n")] = '\0';

        send(sock_fd,
             username,
             strlen(username),
             0);
    }


    /* ================= PASSWORD ================= */

    memset(buffer, 0, BUFFER_SIZE);

    recv(sock_fd,
         buffer,
         BUFFER_SIZE - 1,
         0);

    if (strcmp(buffer, "SEND_PASSWORD") == 0) {

        printf("Enter password : ");

        struct termios oldt, newt;

        int i = 0;
        char ch;

        /* Save terminal settings */
        tcgetattr(STDIN_FILENO, &oldt);

        newt = oldt;

        /* Disable normal input display */
        newt.c_lflag &= ~(ICANON | ECHO);

        tcsetattr(STDIN_FILENO,
                  TCSANOW,
                  &newt);


        /* Read password character by character */
        while (i < BUFFER_SIZE - 1) {

            ch = getchar();

            /* Enter key */
            if (ch == '\n') {
                break;
            }

            /* Backspace */
            if (ch == 127 || ch == 8) {

                if (i > 0) {

                    i--;

                    printf("\b \b");
                    fflush(stdout);
                }

            } else {

                input[i++] = ch;

                printf("*");
                fflush(stdout);
            }
        }

        input[i] = '\0';


        /* Restore terminal settings */
        tcsetattr(STDIN_FILENO,
                  TCSANOW,
                  &oldt);

        printf("\n");


        /* Send password */
        send(sock_fd,
             input,
             strlen(input),
             0);
    }


    /* ================= LOGIN RESULT ================= */

    memset(buffer, 0, BUFFER_SIZE);

    recv(sock_fd,
         buffer,
         BUFFER_SIZE - 1,
         0);

    if (strcmp(buffer, "LOGIN_SUCCESS") == 0) {

        printf("\n---------------------------------\n");
        printf("Login successful!\n");
        printf("Welcome, %s!\n", username);
        printf("---------------------------------\n\n");

    } else {

        printf("\n---------------------------------\n");
        printf("Login failed!\n");
        printf("Invalid username or password.\n");
        printf("---------------------------------\n");

        close(sock_fd);

        exit(EXIT_FAILURE);
    }


    /* ================= MAIN MENU ================= */

    while (1) {

        printf("\n========== MENU ==========\n");
        printf("1. Echo\n");
        printf("2. Palindrome Check\n");
        printf("3. Exit\n");
        printf("===========================\n");

        printf("Enter your choice : ");

        int choice;

        scanf("%d", &choice);

        getchar();


        /* Convert choice to string */
        snprintf(choice_str,
                 sizeof(choice_str),
                 "%d",
                 choice);


        /* Send choice to server */
        send(sock_fd,
             choice_str,
             strlen(choice_str),
             0);

        usleep(100000);


        /* ================= EXIT ================= */

        if (choice == 3) {

            printf("\nGoodbye, %s!\n",
                   username);

            break;
        }


        /* ================= INVALID CHOICE ================= */

        if (choice < 1 || choice > 3) {

            printf("\nInvalid choice. Please try again.\n");

            continue;
        }


        /* ================= USER INPUT ================= */

        printf("Enter a string or number : ");

        fgets(input,
              BUFFER_SIZE,
              stdin);

        input[strcspn(input, "\n")] = '\0';


        /* Send input to server */
        send(sock_fd,
             input,
             strlen(input),
             0);


        /* ================= SERVER RESPONSE ================= */

        memset(buffer,
               0,
               BUFFER_SIZE);

        int bytes_read = recv(sock_fd,
                              buffer,
                              BUFFER_SIZE - 1,
                              0);

        if (bytes_read <= 0) {

            printf("\nServer disconnected.\n");

            break;
        }


        printf("\nServer Response : %s\n",
               buffer);

        printf("---------------------------\n");
    }


    /* Close socket */
    close(sock_fd);

    return 0;
}
