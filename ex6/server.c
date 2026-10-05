#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>

#define PORT 9090
#define BUFFER_SIZE 1024
#define CREDENTIALS_FILE "users.txt"

int isPalindrome(char *str) {
    int len = strlen(str);

    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
        len--;
    }

    int left = 0;
    int right = len - 1;

    while (left < right) {
        if (tolower(str[left]) != tolower(str[right]))
            return 0;

        left++;
        right--;
    }

    return 1;
}

int authenticateUser(char *username, char *password) {

    FILE *fp = fopen(CREDENTIALS_FILE, "r");

    if (fp == NULL) {
        printf("Error opening users.txt\n");
        return 0;
    }

    char fileUser[BUFFER_SIZE];
    char filePass[BUFFER_SIZE];

    while (fscanf(fp, "%s %s", fileUser, filePass) == 2) {

        if (strcmp(username, fileUser) == 0 &&
            strcmp(password, filePass) == 0) {

            fclose(fp);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}

int main() {

    int server_fd, client_fd;

    struct sockaddr_in server_addr, client_addr;

    socklen_t addr_len = sizeof(client_addr);

    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    char username[BUFFER_SIZE];
    char password[BUFFER_SIZE];

    /* Create socket */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
               &opt, sizeof(opt));

    /* Server address */
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    /* Bind */
    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {

        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    /* Listen */
    if (listen(server_fd, 5) < 0) {

        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("\n=================================\n");
    printf("          TCP SERVER\n");
    printf("=================================\n");
    printf("Server started on port %d\n", PORT);
    printf("Waiting for client...\n\n");

    while (1) {

        /* Accept client */
        client_fd = accept(server_fd,
                           (struct sockaddr *)&client_addr,
                           &addr_len);

        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }

        printf("\n---------------------------------\n");
        printf("Client connected\n");
        printf("IP Address : %s\n", inet_ntoa(client_addr.sin_addr));
        printf("Port       : %d\n",
               ntohs(client_addr.sin_port));
        printf("---------------------------------\n\n");

        /* Ask username */
        send(client_fd,
             "SEND_USERNAME",
             strlen("SEND_USERNAME"),
             0);

        memset(username, 0, BUFFER_SIZE);

        recv(client_fd,
             username,
             BUFFER_SIZE - 1,
             0);

        username[strcspn(username, "\n")] = '\0';

        printf("Username received : %s\n", username);

        usleep(100000);

        /* Ask password */
        send(client_fd,
             "SEND_PASSWORD",
             strlen("SEND_PASSWORD"),
             0);

        memset(password, 0, BUFFER_SIZE);

        recv(client_fd,
             password,
             BUFFER_SIZE - 1,
             0);

        password[strcspn(password, "\n")] = '\0';

        /* Display password as stars */
        printf("Password received : ");

        for (int i = 0; i < strlen(password); i++) {
            printf("*");
        }

        printf("\n");

        usleep(100000);

        /* Authentication */
        if (authenticateUser(username, password)) {

            send(client_fd,
                 "LOGIN_SUCCESS",
                 strlen("LOGIN_SUCCESS"),
                 0);

            printf("\n---------------------------------\n");
            printf("Login successful!\n");
            printf("User : %s\n", username);
            printf("---------------------------------\n\n");

        } else {

            send(client_fd,
                 "LOGIN_FAILED",
                 strlen("LOGIN_FAILED"),
                 0);

            printf("\n---------------------------------\n");
            printf("Login failed!\n");
            printf("User : %s\n", username);
            printf("---------------------------------\n\n");

            close(client_fd);
            continue;
        }

        /* Client operations */
        while (1) {

            memset(buffer, 0, BUFFER_SIZE);
            memset(response, 0, BUFFER_SIZE);

            int bytes_read = recv(client_fd,
                                  buffer,
                                  BUFFER_SIZE - 1,
                                  0);

            if (bytes_read <= 0)
                break;

            int choice = atoi(buffer);

            /* Exit */
            if (choice == 3) {

                printf("[%s] Exit selected\n", username);
                break;
            }

            /* Display selected operation */
            if (choice == 1) {

                printf("\n[%s] Echo\n", username);

            } else if (choice == 2) {

                printf("\n[%s] Palindrome Check\n", username);
            }

            /* Receive input */
            memset(buffer, 0, BUFFER_SIZE);

            bytes_read = recv(client_fd,
                              buffer,
                              BUFFER_SIZE - 1,
                              0);

            if (bytes_read <= 0)
                break;

            buffer[strcspn(buffer, "\n")] = '\0';

            printf("Input    : %s\n", buffer);

            /* Echo */
            if (choice == 1) {

                snprintf(response,
                         BUFFER_SIZE,
                         "%s",
                         buffer);

                printf("Response : %s\n", response);
            }

            /* Palindrome */
            else if (choice == 2) {

                if (isPalindrome(buffer)) {

                    snprintf(response,
                             BUFFER_SIZE,
                             "%s is a Palindrome",
                             buffer);

                } else {

                    snprintf(response,
                             BUFFER_SIZE,
                             "%s is NOT a Palindrome",
                             buffer);
                }

                printf("Response : %s\n", response);
            }

            /* Invalid choice */
            else {

                snprintf(response,
                         BUFFER_SIZE,
                         "Invalid choice");

                printf("Response : %s\n", response);
            }

            printf("---------------------------------\n");

            /* Send response */
            send(client_fd,
                 response,
                 strlen(response),
                 0);
        }

        printf("\n---------------------------------\n");
        printf("%s disconnected\n", username);
        printf("---------------------------------\n");
        printf("Waiting for client...\n\n");

        close(client_fd);
    }

    close(server_fd);

    return 0;
}
