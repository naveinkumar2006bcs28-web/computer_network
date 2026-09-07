#include <stdio.h>

#define MAX 50

/* Function for Go-Back-N */
void goBackN(int n, int w, int lost)
{
    int base = 0;
    int i;

    printf("\n====================================\n");
    printf("       GO-BACK-N PROTOCOL\n");
    printf("====================================\n");

    while (base < n)
    {
        printf("\nSender Window: ");

        for (i = base; i < base + w && i < n; i++)
            printf("%d ", i);

        printf("\n");

        /* Send frames */
        for (i = base; i < base + w && i < n; i++)
        {
            printf("Sender  : Sending Frame %d\n", i);

            if (i == lost)
            {
                printf("Receiver: Frame %d lost/corrupted\n", i);
                printf("Receiver: ACK not received\n");
                break;
            }

            printf("Receiver: Frame %d received\n", i);
            printf("Receiver: Sending ACK %d\n", i + 1);
        }

        /* Error occurred */
        if (i < base + w && i < n && i == lost)
        {
            printf("\n*** TIMEOUT ***\n");
            printf("Sender  : Go back to Frame %d\n", lost);

            printf("Sender  : Retransmitting: ");

            for (i = lost; i < base + w && i < n; i++)
                printf("%d ", i);

            printf("\n");

            /* Assume retransmitted frames are received */
            for (i = lost; i < base + w && i < n; i++)
            {
                printf("Receiver: Frame %d received\n", i);
                printf("Receiver: Sending ACK %d\n", i + 1);
            }

            lost = -1;
        }

        base += w;
    }

    printf("\nTransmission completed successfully.\n");
}


/* Function for Selective Repeat */
void selectiveRepeat(int n, int w, int lost)
{
    int base = 0;
    int i;

    printf("\n====================================\n");
    printf("      SELECTIVE REPEAT PROTOCOL\n");
    printf("====================================\n");

    while (base < n)
    {
        printf("\nSender Window: ");

        for (i = base; i < base + w && i < n; i++)
            printf("%d ", i);

        printf("\n");

        /* Send frames */
        for (i = base; i < base + w && i < n; i++)
        {
            printf("Sender  : Sending Frame %d\n", i);

            if (i == lost)
            {
                printf("Receiver: Frame %d lost/corrupted\n", i);
                printf("Receiver: ACK %d not received\n", i + 1);
            }
            else
            {
                printf("Receiver: Frame %d received\n", i);
                printf("Receiver: Sending ACK %d\n", i + 1);
            }
        }

        /* Retransmit only lost frame */
        if (lost >= base && lost < base + w)
        {
            printf("\n*** TIMEOUT for Frame %d ***\n", lost);
            printf("Sender  : Retransmitting Frame %d\n", lost);

            printf("Receiver: Frame %d received\n", lost);
            printf("Receiver: Sending ACK %d\n", lost + 1);

            lost = -1;
        }

        base += w;
    }

    printf("\nTransmission completed successfully.\n");
}


/* Main Function */
int main()
{
    int choice;
    int n, w, lost;

    printf("====================================\n");
    printf("       SLIDING WINDOW PROTOCOL\n");
    printf("====================================\n");

    printf("\n1. Go-Back-N\n");
    printf("2. Selective Repeat\n");

    printf("\nEnter your choice: ");
    scanf("%d", &choice);

    printf("Enter number of frames: ");
    scanf("%d", &n);

    printf("Enter window size: ");
    scanf("%d", &w);

    printf("Enter lost frame number (-1 for no loss): ");
    scanf("%d", &lost);

    if (n <= 0 || n > MAX || w <= 0 || w > n)
    {
        printf("\nInvalid input!\n");
        return 0;
    }

    if (lost >= n)
    {
        printf("\nInvalid lost frame number!\n");
        return 0;
    }

    switch (choice)
    {
        case 1:
            goBackN(n, w, lost);
            break;

        case 2:
            selectiveRepeat(n, w, lost);
            break;

        default:
            printf("\nInvalid choice!\n");
    }

    return 0;
}
