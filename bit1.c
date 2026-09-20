#include <stdio.h>
#include <string.h>

#define MAX 100

int data[800], stuffed[1600], framed[1600], destuffed[800];
char str[MAX], output[MAX];

int flag[] = {0,1,1,1,1,1,1,0};

void printArray(char name[], int a[], int n)
{
    int i;
    printf("%s ", name);

    for(i = 0; i < n; i++)
        printf("%d", a[i]);

    printf("\n");
}

int main()
{
    int choice;

    while(1)
    {
        printf("\n--- MENU ---\n");
        printf("1. String Transmission\n");
        printf("2. Manual Binary Input\n");
        printf("3. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        getchar();

        if(choice == 3)
            break;

        /* CASE 1 */
        if(choice == 1)
        {
            int i, b, ones = 0;
            int dataLen = 0, stuffLen = 0;
            int frameLen = 0, destuffLen = 0;
            int outIndex = 0;

            printf("\nEnter a string: ");
            fgets(str, MAX, stdin);
            str[strcspn(str, "\n")] = '\0';

            /* String to Binary */
            for(i = 0; i < strlen(str); i++)
            {
                for(b = 7; b >= 0; b--)
                {
                    data[dataLen++] = (str[i] >> b) & 1;
                }
            }

            printArray("Original: ", data, dataLen);

            /* Bit Stuffing */
            for(i = 0; i < dataLen; i++)
            {
                stuffed[stuffLen++] = data[i];

                if(data[i] == 1)
                    ones++;
                else
                    ones = 0;

                if(ones == 5)
                {
                    stuffed[stuffLen++] = 0;
                    ones = 0;
                }
            }

            printArray("Stuffed:  ", stuffed, stuffLen);

            /* Framing */
            for(i = 0; i < 8; i++)
                framed[frameLen++] = flag[i];

            for(i = 0; i < stuffLen; i++)
                framed[frameLen++] = stuffed[i];

            for(i = 0; i < 8; i++)
                framed[frameLen++] = flag[i];

            printArray("Framed:   ", framed, frameLen);

            /* Bit Destuffing */
            ones = 0;

            for(i = 8; i < frameLen - 8; i++)
            {
                destuffed[destuffLen++] = framed[i];

                if(framed[i] == 1)
                    ones++;
                else
                    ones = 0;

                if(ones == 5)
                {
                    if(framed[i + 1] != 0)
                    {
                        printf("Error: Invalid stuffing!\n");
                        break;
                    }

                    i++;
                    ones = 0;
                }
            }

            printArray("Destuffed:", destuffed, destuffLen);

            /* Binary to String */
            for(i = 0; i < destuffLen; i += 8)
            {
                unsigned char ch = 0;

                for(b = 0; b < 8; b++)
                    ch = (ch << 1) | destuffed[i + b];

                output[outIndex++] = ch;
            }

            output[outIndex] = '\0';

            printf("Output: %s\n", output);
        }

        /* CASE 2 */
        else if(choice == 2)
        {
            char input[1600];
            int binary[1600];
            int i, len, ones = 0, dLen = 0;

            printf("\nEnter binary stream: ");
            fgets(input, 1600, stdin);
            input[strcspn(input, "\n")] = '\0';

            len = strlen(input);

            /* Convert characters to numbers */
            for(i = 0; i < len; i++)
            {
                if(input[i] == '0')
                    binary[i] = 0;
                else if(input[i] == '1')
                    binary[i] = 1;
                else
                {
                    printf("Error: Enter only 0 and 1.\n");
                    break;
                }
            }

            /* Destuff */
            for(i = 0; i < len; i++)
            {
                destuffed[dLen++] = binary[i];

                if(binary[i] == 1)
                    ones++;
                else
                    ones = 0;

                if(ones == 5)
                {
                    if(binary[i + 1] != 0)
                    {
                        printf("Error: Invalid stuffing!\n");
                        break;
                    }

                    i++;
                    ones = 0;
                }
            }

            printArray("Destuffed:", destuffed, dLen);
        }

        else
        {
            printf("Invalid choice!\n");
        }
    }

    return 0;
}
