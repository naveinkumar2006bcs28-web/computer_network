#include <stdio.h>
#define MAX 10
#define INF 999

void showNetwork(int routerCount, int network[MAX][MAX]) {
    int row, col;

    printf("\n--- CURRENT BIDIRECTIONAL NETWORK MAP ---\n\t");

    for (row = 0; row < routerCount; row++)
        printf("%c\t", 'A' + row);

    printf("\n");

    for (row = 0; row < routerCount; row++) {
        printf("%c\t", 'A' + row);

        for (col = 0; col < routerCount; col++) {
            if (network[row][col] == INF)
                printf("INF\t");
            else
                printf("%d\t", network[row][col]);
        }

        printf("\n");
    }
}

/* Displays the final converged cost matrix */
void showCostTable(int routerCount, int costDist[MAX][MAX]) {
    int row, col;

    printf("\n========================================\n");
    printf("         FINAL CONVERGED COST MATRIX\n");
    printf("========================================\n\t");

    for (row = 0; row < routerCount; row++)
        printf("%c\t", 'A' + row);

    printf("\n");

    for (row = 0; row < routerCount; row++) {
        printf("%c\t", 'A' + row);

        for (col = 0; col < routerCount; col++) {
            if (costDist[row][col] == INF)
                printf("INF\t");
            else
                printf("%d\t", costDist[row][col]);
        }

        printf("\n");
    }
}

void runDistanceVector(int routerCount, int cost[MAX][MAX]) {
    int costDist[MAX][MAX], nextRouter[MAX][MAX];
    int row, col, mid, changed, rounds = 0;

    for (row = 0; row < routerCount; row++) {
        for (col = 0; col < routerCount; col++) {

            costDist[row][col] = cost[row][col];

            if (cost[row][col] != INF && row != col)
                nextRouter[row][col] = col;
            else
                nextRouter[row][col] = -1;
        }
    }

    do {
        changed = 0;
        rounds++;

        for (row = 0; row < routerCount; row++) {
            for (col = 0; col < routerCount; col++) {
                for (mid = 0; mid < routerCount; mid++) {

                    if (costDist[row][mid] != INF &&
                        costDist[mid][col] != INF &&
                        nextRouter[row][mid] != -1) {

                        if (costDist[row][mid] + costDist[mid][col]
                            < costDist[row][col]) {

                            costDist[row][col] =
                                costDist[row][mid] + costDist[mid][col];

                            nextRouter[row][col] =
                                nextRouter[row][mid];

                            changed = 1;
                        }
                    }
                }
            }
        }

    } while (changed);

    printf("\n*** RIP DISTANCE VECTOR RESULTS ***\n");
    printf("Total convergence rounds taken: %d\n", rounds);

    for (row = 0; row < routerCount; row++) {

        printf("\n[ RIP TABLE FOR ROUTER %c ]\n", 'A' + row);
        printf("End Router\tNext Step\tTotal Cost\n");
        printf("-----------------------------------\n");

        for (col = 0; col < routerCount; col++) {

            printf("%c\t\t", 'A' + col);

            if (row == col)
                printf("-\t\t0\n");

            else if (costDist[row][col] == INF)
                printf("-\t\tINF\n");

            else
                printf("%c\t\t%d\n",
                       'A' + nextRouter[row][col],
                       costDist[row][col]);
        }
    }

    showCostTable(routerCount, costDist);
}

int getClosestNode(int costDist[MAX], int done[MAX], int routerCount) {
    int smallest = INF;
    int selected = -1;
    int row;

    for (row = 0; row < routerCount; row++) {

        if (!done[row] && costDist[row] < smallest) {
            smallest = costDist[row];
            selected = row;
        }
    }

    return selected;
}

void printRoute(int previous[MAX], int destination) {

    if (previous[destination] == -1) {
        printf("%c", 'A' + destination);
        return;
    }

    printRoute(previous, previous[destination]);
    printf(" > %c", 'A' + destination);
}

void runLinkState(int routerCount, int network[MAX][MAX]) {

    int costDist[MAX], done[MAX], previous[MAX];
    int resultTable[MAX][MAX];

    int startNode, row, col, step, active;
    char startChar;

    printf("\nEnter starting OSPF router (A-%c): ",
           'A' + routerCount - 1);

    scanf(" %c", &startChar);

    startNode = startChar - 'A';

    if (startNode < 0 || startNode >= routerCount) {
        printf("\nInvalid choice!\n");
        return;
    }

    /* Prepare matrix for displaying final results */
    for (row = 0; row < routerCount; row++) {
        for (col = 0; col < routerCount; col++) {

            if (row == col)
                resultTable[row][col] = 0;
            else
                resultTable[row][col] = INF;
        }
    }

    for (row = 0; row < routerCount; row++) {
        costDist[row] = INF;
        done[row] = 0;
        previous[row] = -1;
    }

    costDist[startNode] = 0;

    for (step = 0; step < routerCount - 1; step++) {

        active = getClosestNode(costDist, done, routerCount);

        if (active == -1)
            break;

        done[active] = 1;

        for (row = 0; row < routerCount; row++) {

            if (!done[row] &&
                network[active][row] != INF &&
                costDist[active] != INF) {

                int trialCost =
                    costDist[active] + network[active][row];

                if (trialCost < costDist[row]) {
                    costDist[row] = trialCost;
                    previous[row] = active;
                }
            }
        }
    }

    printf("\n*** OSPF LINK STATE RESULTS ***\n");
    printf("OSPF Root Source: %c\n\n", 'A' + startNode);
    printf("End Router\tOSPF Cost\tFull SPF Route\n");
    printf("-----------------------------------\n");

    for (row = 0; row < routerCount; row++) {

        printf("%c\t\t", 'A' + row);

        if (costDist[row] == INF) {
            printf("INF\tNo Route Found\n");
        }
        else {
            printf("%d\t\t", costDist[row]);
            printRoute(previous, row);
            printf("\n");
        }

        resultTable[startNode][row] = costDist[row];
    }

    /* Calculate shortest paths from the remaining routers */
    for (row = 0; row < routerCount; row++) {

        if (row == startNode)
            continue;

        int workDist[MAX], workDone[MAX];

        for (col = 0; col < routerCount; col++) {
            workDist[col] = INF;
            workDone[col] = 0;
        }

        workDist[row] = 0;

        for (step = 0; step < routerCount - 1; step++) {

            int workNode =
                getClosestNode(workDist, workDone, routerCount);

            if (workNode == -1)
                break;

            workDone[workNode] = 1;

            for (col = 0; col < routerCount; col++) {

                if (!workDone[col] &&
                    network[workNode][col] != INF &&
                    workDist[workNode] != INF) {

                    int candidate =
                        workDist[workNode] + network[workNode][col];

                    if (candidate < workDist[col])
                        workDist[col] = candidate;
                }
            }
        }

        for (col = 0; col < routerCount; col++)
            resultTable[row][col] = workDist[col];
    }

    showCostTable(routerCount, resultTable);
}

void getShortestPairPath(int routerCount, int network[MAX][MAX]) {

    char startChar, endChar;
    int startNode, endNode;

    int costDist[MAX], done[MAX], previous[MAX];
    int row, step, active;

    printf("\nEnter starting router (A-%c): ",
           'A' + routerCount - 1);
    scanf(" %c", &startChar);

    printf("Enter destination router (A-%c): ",
           'A' + routerCount - 1);
    scanf(" %c", &endChar);

    startNode = startChar - 'A';
    endNode = endChar - 'A';

    if (startNode < 0 || startNode >= routerCount ||
        endNode < 0 || endNode >= routerCount) {

        printf("\nInvalid router options chosen!\n");
        return;
    }

    for (row = 0; row < routerCount; row++) {
        costDist[row] = INF;
        done[row] = 0;
        previous[row] = -1;
    }

    costDist[startNode] = 0;

    for (step = 0; step < routerCount - 1; step++) {

        active =
            getClosestNode(costDist, done, routerCount);

        if (active == -1)
            break;

        done[active] = 1;

        for (row = 0; row < routerCount; row++) {

            if (!done[row] &&
                network[active][row] != INF &&
                costDist[active] != INF) {

                int trialCost =
                    costDist[active] + network[active][row];

                if (trialCost < costDist[row]) {
                    costDist[row] = trialCost;
                    previous[row] = active;
                }
            }
        }
    }

    printf("\n=== MINIMUM COST PATH ROUTE ===\n");

    if (costDist[endNode] == INF) {

        printf("No path exists between %c and %c.\n",
               startChar, endChar);
    }
    else {

        printf("Minimum Total Path Cost: %d\n",
               costDist[endNode]);

        printf("Shortest Structural Route: ");
        printRoute(previous, endNode);
        printf("\n");
    }
}

int main() {

    int routerCount = 0;
    int network[MAX][MAX];

    int row, col, menuChoice, linkCost;
    int configured = 0;

    char editStart, editEnd;
    int editStartId, editEndId;

    while (1) {

        printf("\n=== CHOOSE AN ALGORITHM ===\n");
        printf("1. RIP (Distance Vector Protocol)\n");
        printf("2. OSPF (Link State Protocol)\n");
        printf("3. Run Both Protocols\n");
        printf("4. Modify Shared Link Cost\n");
        printf("5. Find Minimum Cost Path for a Pair\n");
        printf("6. Exit\n\nYour selection: ");

        if (scanf("%d", &menuChoice) != 1) {

            while (getchar() != '\n');

            printf("\nWrong option choice!\n");
            continue;
        }

        if (menuChoice == 6) {
            printf("\nGoodbye.\n");
            break;
        }

        if (menuChoice < 1 || menuChoice > 6) {
            printf("\nWrong option choice!\n");
            continue;
        }

        if (!configured && menuChoice != 6) {

            printf("\n[ NETWORK NOT CONFIGURED ]\n");
            printf("Enter total routers (max %d): ", MAX);

            scanf("%d", &routerCount);

            if (routerCount <= 0 || routerCount > MAX) {

                printf("\nInvalid number!\n");
                routerCount = 0;
                continue;
            }

            for (row = 0; row < routerCount; row++) {
                for (col = 0; col < routerCount; col++) {

                    if (row == col)
                        network[row][col] = 0;
                    else
                        network[row][col] = INF;
                }
            }

            printf("\n--- ENTER SHARED TWO-WAY LINK COSTS ---\n");
            printf("Type %d if there is no direct connection link.\n\n",
                   INF);

            for (row = 0; row < routerCount; row++) {

                for (col = row + 1; col < routerCount; col++) {

                    printf("Link Cost between %c and %c: ",
                           'A' + row, 'A' + col);

                    scanf("%d", &linkCost);

                    if (linkCost == 0)
                        linkCost = INF;

                    network[row][col] = linkCost;
                    network[col][row] = linkCost;
                }
            }

            configured = 1;

            showNetwork(routerCount, network);
        }

        switch (menuChoice) {

            case 1:
                printf("\n>> Running RIP Protocol\n");
                runDistanceVector(routerCount, network);
                break;

            case 2:
                printf("\n>> Running OSPF Protocol\n");
                runLinkState(routerCount, network);
                break;

            case 3:
                printf("\n>> Running Both Protocols\n");
                runDistanceVector(routerCount, network);
                runLinkState(routerCount, network);
                break;

            case 4:

                showNetwork(routerCount, network);

                printf("\n--- MODIFY SHARED LINK COST ---\n");

                printf("Enter first router (A-%c): ",
                       'A' + routerCount - 1);
                scanf(" %c", &editStart);

                printf("Enter second router (A-%c): ",
                       'A' + routerCount - 1);
                scanf(" %c", &editEnd);

                editStartId = editStart - 'A' + 1;
                editEndId = editEnd - 'A' + 1;

                if (editStartId < 1 || editStartId > routerCount ||
                    editEndId < 1 || editEndId > routerCount) {

                    printf("\nInvalid router options selected!\n");
                    break;
                }

                if (editStartId == editEndId) {

                    printf("\nInternal loop metrics cannot change from 0!\n");
                    break;
                }

                printf("Enter new symmetric cost (use %d for link down): ",
                       INF);

                scanf("%d", &linkCost);

                if (linkCost == 0)
                    linkCost = INF;

                network[editStartId - 1][editEndId - 1] = linkCost;
                network[editEndId - 1][editStartId - 1] = linkCost;

                printf("\nShared link values updated dynamically!\n");

                showNetwork(routerCount, network);

                break;

            case 5:
                getShortestPairPath(routerCount, network);
                break;

            default:
                break;
        }
    }

    printf("\n======\n FINISHED RUNNING\n=====\n");

    return 0;
}

