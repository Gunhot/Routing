#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAXNODE 100
#define NOTEXIST -1
#define INF 999
#define MAXLENGTH 1000
FILE *topo_fd;
FILE *msg_fd;
FILE *change_fd;
FILE *output_fd;

typedef struct _link
{
    int from;
    int to;
    int cost;
} link;
link linktable[MAXNODE];
int linkn;
int noden;

typedef struct _route
{
    int past;
    int next;
    int cost;
} route;
route routetable[MAXNODE][MAXNODE];

int change;

int init(int argc, char **argv);
void printRoutingTable();
void distanceVector();
void init_table();
void read_topo();
void message_dealer();
void naive_update(int from, int to, int newCost);
void accept_change();

void printRoutingTable()
{
    for (int i = 0; i < noden; i++)
    {
        for (int j = 0; j < noden; j++)
        {
            if (routetable[i][j].cost != INF)
                fprintf(output_fd, "%d %d %d\n", j, routetable[i][j].next, routetable[i][j].cost);
        }
        fprintf(output_fd, "\n");
    }
}

void distanceVector()
{
    change = 0;
    int i, j, k;
    for (k = 0; k < noden; k++)
    {
        for (i = 0; i < noden; i++)
        {
            for (j = 0; j < noden; j++)
            {
                if (routetable[i][k].cost + routetable[k][j].cost <= routetable[i][j].cost)
                {
                    if (routetable[i][k].cost + routetable[k][j].cost == routetable[i][j].cost)
                    {
                        if (routetable[i][k].next < routetable[i][j].next && i != k)
                        {
                            routetable[i][j].next = routetable[i][k].next;
                            change = 1;
                        }
                    }
                    else
                    {
                        routetable[i][j].cost = routetable[i][k].cost + routetable[k][j].cost;
                        routetable[i][j].next = routetable[i][k].next;
                        change = 1;
                    }
                }
            }
        }
    }
}

void init_table()
{
    for (int i = 0; i < noden; i++)
    {
        for (int j = 0; j < noden; j++)
        {
            if (i != j)
            {
                routetable[i][j].cost = INF;
                routetable[i][j].next = NOTEXIST;
            }
            else
            {
                routetable[i][j].cost = 0;
                routetable[i][j].next = i;
            }
        }
    }
    for (int i = 0; i < linkn; i++)
    {
        int from = linktable[i].from;
        int to = linktable[i].to;
        int cost = linktable[i].cost;
        if (routetable[from][to].cost > cost)
        {
            routetable[from][to].cost = cost;
            routetable[from][to].next = to;
            from = linktable[i].to;
            to = linktable[i].from;
            routetable[from][to].cost = cost;
            routetable[from][to].next = to;
        }
    }
}

void read_topo()
{
    fscanf(topo_fd, "%d", &noden);
    int i = 0;
    while (fscanf(topo_fd, "%d %d %d", &linktable[i].from, &linktable[i].to, &linktable[i].cost) == 3)
    {
        linkn++;
        i++;
    }
}

void message_dealer()
{
    rewind(msg_fd);
    int from, to;
    char message[MAXLENGTH];
    while (fscanf(msg_fd, "%d %d %[^\n]", &from, &to, message) == 3)
    {
        fprintf(output_fd, "from %d to %d cost ", from, to);
        int next = routetable[from][to].next;
        if (next == -1)
        {
            fprintf(output_fd, "infinite hops unreachable ");
        }
        else
        {
            fprintf(output_fd, "%d hops ", routetable[from][to].cost);
            fprintf(output_fd, "%d ", from);

            while (next != to)
            {
                fprintf(output_fd, "%d ", next);
                next = routetable[next][to].next;
            }
        }
        fprintf(output_fd, "message %s\n", message);
    }
    fprintf(output_fd, "\n");
}

void naive_update(int from, int to, int newCost)
{
    // Update the cost of the link from 'from' to 'to' with 'newCost'
    int found = 0;
    for (int i = 0; i < linkn; i++)
    {
        if (linktable[i].from == from && linktable[i].to == to)
        {
            linktable[i].to = to;
            if (newCost == -999)
                newCost = INF;
            linktable[i].cost = newCost;
            found = 1;
        }
    }
    if (found == 0)
    {
        linkn++;
        linktable[linkn - 1].from = from;
        linktable[linkn - 1].to = to;
        linktable[linkn - 1].cost = newCost;
    }
}
/*
void update(int from, int to, int newCost)
{
    // Update the cost of the link from 'from' to 'to' with 'newCost'
    routetable[from][to].cost = newCost;

    // Update the next hop for affected destinations
    for (int i = 0; i < noden; i++)
    {
        if (routetable[i][to].next == to && i != to)
        {
            routetable[i][to].cost = routetable[from][to].cost + routetable[i][from].cost;
            routetable[i][to].next = from;
        }
    }

    // Update the routing table for other nodes
    for (int i = 0; i < noden; i++)
    {
        if (i != to && routetable[i][to].next != to)
        {
            for (int j = 0; j < noden; j++)
            {
                if (routetable[i][j].next == to)
                {
                    routetable[i][j].cost = routetable[i][to].cost + routetable[to][j].cost;
                    routetable[i][j].next = from;
                }
            }
        }
    }
}
*/

void accept_change()
{
    int from, to, cost;
    while (fscanf(change_fd, "%d %d %d", &from, &to, &cost) == 3)
    {
        naive_update(from, to, cost);
        init_table();
        do
            distanceVector();
        while (change != 0);
        printRoutingTable();
        if (msg_fd)
            message_dealer();
    }
}

int main(int argc, char **argv)
{
    if (init(argc, argv) == -1)
        return (-1);
    read_topo();
    init_table();
    do
        distanceVector();
    while (change != 0);
    printRoutingTable();

    msg_fd = fopen(argv[2], "r");
    if (msg_fd != NULL)
    {
        message_dealer();
    }
    change_fd = fopen(argv[3], "r");
    if (change_fd != NULL)
    {
        accept_change();
    }
    return (0);
}
int init(int argc, char **argv)
{

    if (argc != 4)
    {
        printf("usage: distvec topologyfile messagesfile changesfile\n");
        return (-1);
    }

    topo_fd = fopen(argv[1], "r");
    if (topo_fd == NULL)
    {
        printf("Error: open input file.\n");
        return (-1);
    }

    output_fd = fopen("output_dv.txt", "w");
    if (output_fd == NULL)
    {
        return (-1);
    }

    return (0);
}