#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX 100

int main() {
    char flag, esc;
    char message[MAX];
    char stuffed[MAX * 2];
    char framed[MAX * 2];
    char destuffed[MAX];
    int choice;
    int i; 

    printf("Enter a character to use as FLAG (e.g., F): ");
    scanf(" %c", &flag);
    printf("Enter a character to use as ESC (e.g., E): ");
    scanf(" %c", &esc);

    while (1) {
        printf("\n--- MENU ---\n");
        printf("1. Standard String Transmission\n");
        printf("2. Manual Stream Input (Test Framing Errors)\n");
        printf("3. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        getchar(); 

        if (choice == 3) {
            printf("Exiting. Goodbye!\n");
            break;
        }

        switch (choice) {
            case 1: {
                int len;
                int j = 0;
                int k = 0;
                int m = 0;
                int error = 0;
                int framedLen;

                printf("\nEnter text message to send: ");
                fgets(message, sizeof(message), stdin);
                message[strcspn(message, "\n")] = '\0'; 

                len = strlen(message);
                printf("\nOriginal Data: %s\n", message);

                for (i = 0; i < len; i++) {
                    if (message[i] == flag || message[i] == esc) {
                        stuffed[j++] = esc;
                        stuffed[j++] = message[i];
                    } else {
                        stuffed[j++] = message[i];
                    }
                }
                stuffed[j] = '\0';
                printf("Stuffed Data : %s\n", stuffed);

                framed[k++] = flag; 
                for (i = 0; i < j; i++) {
                    framed[k++] = stuffed[i];
                }
                framed[k++] = flag; 
                framed[k] = '\0';
                printf("Framed Data  : %s\n", framed);

                framedLen = strlen(framed);

                if (framed[0] != flag || framed[framedLen - 1] != flag) {
                    printf("[ERROR] Missing boundary flags!\n");
                    break;
                }

                for (i = 1; i < framedLen - 1; i++) {
                    if (framed[i] == esc) {
                        if (i + 1 >= framedLen - 1) {
                            printf("[ERROR] Dangling ESC found!\n");
                            error = 1;
                            break;
                        }
                        i++; 
                        destuffed[m++] = framed[i]; 
                    } else if (framed[i] == flag) {
                        printf("[ERROR] Unescaped FLAG found inside payload!\n");
                        error = 1;
                        break;
                    } else {
                        destuffed[m++] = framed[i];
                    }
                }
                destuffed[m] = '\0';

                if (!error) {
                    printf("Received Msg : %s (Success!)\n", destuffed);
                }
                break;
            }

            case 2: {
                char manualStream[MAX];
                int streamLen;
                int m = 0;
                int error = 0;

                printf("\nEnter a raw frame to test receiver logic: ");
                fgets(manualStream, sizeof(manualStream), stdin);
                manualStream[strcspn(manualStream, "\n")] = '\0';

                streamLen = strlen(manualStream);

                if (manualStream[0] != flag || manualStream[streamLen - 1] != flag) {
                    printf("[DISCARDED] Framing Failure: Stream must start and end with '%c'\n", flag);
                    break;
                }

                for (i = 1; i < streamLen - 1; i++) {
                    if (manualStream[i] == esc) {
                        if (i + 1 >= streamLen - 1) {
                            printf("[DISCARDED] Protocol Failure: Dangling ESC character at index %d!\n", i);
                            error = 1;
                            break;
                        }
                        i++; 
                        destuffed[m++] = manualStream[i];
                    } else if (manualStream[i] == flag) {
                        printf("[DISCARDED] Protocol Failure: Unescaped FLAG character found at index %d!\n", i);
                        error = 1;
                        break;
                    } else {
                        destuffed[m++] = manualStream[i];
                    }
                }
                destuffed[m] = '\0';

                if (!error) {
                    printf("[ACCEPTED] Successfully unpacked message: %s\n", destuffed);
                }
                break;
            }

            default:
                printf("Invalid selection. Try again.\n");
        }
    }
    return 0;
}
