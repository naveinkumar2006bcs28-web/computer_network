#include <stdio.h>
#include <string.h>

#define MAX 100

int data[800], stuffed[1600], destuffed[800], framed[1600];
char str[MAX];
char output_str[MAX];
int flag[] = {0, 1, 1, 1, 1, 1, 1, 0};

void print_array(const char *label, int arr[], int size) {
    int i;
    printf("%s ", label);
    for (i = 0; i < size; i++) {
        printf("%d", arr[i]);
    }
    printf("\n");
}

int main(void) {
    int choice;

    while (1) {
        printf("___ Menu ___\n");
        printf("1. Standard String Transmission\n");
        printf("2. Manual Binary Input\n");
        printf("3. Exit\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input type. Exiting.\n");
            break;
        }
        getchar();

        if (choice == 3) {
            printf("\nExiting>>>>\n");
            break;
        }

        if (choice == 1) {
            int i, b, ones = 0, data_len = 0, stuffed_len = 0, framed_len = 0, destuffed_len = 0, str_idx = 0;
            
            printf("\n--- Case 1 ---\n");
            printf("enter a string: ");
            fgets(str, sizeof(str), stdin);
            str[strcspn(str, "\n")] = '\0';

            int len = strlen(str);
            for (i = 0; i < len; i++) {
                unsigned char ch = (unsigned char)str[i];
                for (b = 7; b >= 0; b--) {
                    data[data_len++] = (ch >> b) & 1;
                }
            }
            print_array("original data:", data, data_len);

            for (i = 0; i < data_len; i++) {
                stuffed[stuffed_len++] = data[i];
                if (data[i] == 1) {
                    ones++;
                } else {
                    ones = 0;
                }
                if (ones == 5) {
                    stuffed[stuffed_len++] = 0;
                    ones = 0;
                }
            }
            print_array("stuffed:      ", stuffed, stuffed_len);

            for (i = 0; i < 8; i++) framed[framed_len++] = flag[i];
            for (i = 0; i < stuffed_len; i++) framed[framed_len++] = stuffed[i];
            for (i = 0; i < 8; i++) framed[framed_len++] = flag[i];
            print_array("framed:       ", framed, framed_len);

            ones = 0;
            int error = 0;
            for (i = 8; i < framed_len - 8; i++) {
                destuffed[destuffed_len++] = framed[i];
                if (framed[i] == 1) {
                    ones++;
                } else {
                    ones = 0;
                }
                if (ones == 5) {
                    if (framed[i + 1] != 0) {
                        printf("\n[ERROR] Invalid stuffing!\n");
                        error = 1;
                        break;
                    }
                    i++;
                    ones = 0;
                }
            }

            if (error) continue;
            print_array("destuffed:    ", destuffed, destuffed_len);

            for (i = 0; i < destuffed_len; i += 8) {
                unsigned char ch = 0;
                for (b = 0; b < 8; b++) {
                    ch = (ch << 1) | destuffed[i + b];
                }
                output_str[str_idx++] = ch;
            }
            output_str[str_idx] = '\0';
            printf("output string: %s\n", output_str);

        } else if (choice == 2) {
            char binary_str[1600];
            int input_framed[1600];
            int i, ones = 0, destuffed_len = 0, error = 0;

            printf("\n--- Case 2 ---\n");
            printf("Enter raw binary stream: ");
            fgets(binary_str, sizeof(binary_str), stdin);
            binary_str[strcspn(binary_str, "\n")] = '\0';

            int len = strlen(binary_str);
            for (i = 0; i < len; i++) {
                if (binary_str[i] == '1') input_framed[i] = 1;
                else if (binary_str[i] == '0') input_framed[i] = 0;
                else {
                    printf("Error: Input must only contain 0s and 1s.\n");
                    error = 1;
                    break;
                }
            }

            if (error) continue;
            print_array("Input stream: ", input_framed, len);

            for (i = 0; i < len; i++) {
                destuffed[destuffed_len++] = input_framed[i];
                if (input_framed[i] == 1) {
                    ones++;
                } else {
                    ones = 0;
                }

                if (ones == 5) {
                    if (i + 1 >= len) {
                        printf("\n[DISCARDED] Stream ended abruptly after 5 ones.\n");
                        error = 1;
                        break;
                    }
                    if (input_framed[i + 1] != 0) {
                        printf("\n[DISCARDED] Index %d must be a stuffed 0.\n", i + 1);
                        error = 1;
                        break;
                    }
                    i++;
                    ones = 0;
                }
            }

            if (error) continue;
            print_array("destuffed:    ", destuffed, destuffed_len);

        } else {
            printf("Invalid selection.\n");
        }
    }
    return 0;
}

