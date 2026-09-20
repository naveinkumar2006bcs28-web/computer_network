#include <stdio.h>
#include <string.h>

int main()
{
    char data[100], stuffed[200], destuffed[100];
    int i, j = 0, k = 0;

    printf("Enter data: ");
    scanf("%s", data);

    /* Byte Stuffing */
    stuffed[j++] = 'F';

    for(i = 0; i < strlen(data); i++)
    {
        if(data[i] == 'F' || data[i] == 'E')
            stuffed[j++] = 'E';

        stuffed[j++] = data[i];
    }

    stuffed[j++] = 'F';
    stuffed[j] = '\0';

    printf("Stuffed data: %s\n", stuffed);

    /* Byte Destuffing */
    for(i = 1; i < j - 1; i++)
    {
        if(stuffed[i] == 'E')
            i++;

        destuffed[k++] = stuffed[i];
    }

    destuffed[k] = '\0';

    printf("Destuffed data: %s\n", destuffed);

    return 0;
}
