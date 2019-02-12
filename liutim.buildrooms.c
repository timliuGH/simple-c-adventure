#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

enum bool {false, true};

enum RoomType {START_ROOM, MID_ROOM, END_ROOM};

struct Room
{
    char *name;
    enum RoomType type;
    int numConnections;
    struct Room *connections[6];
    char *filename;
};

void AddRandomConnection(struct Room * []);
struct Room *GetRandomRoom(struct Room * []);
void ConnectRooms(struct Room *, struct Room *);
enum bool IsSameRoom(struct Room *, struct Room *);
enum bool AlreadyConnected(struct Room *, struct Room *);
enum bool IsGraphFull(struct Room * []);

int main(int argc, char *argv[])
{
    /* Get process ID of current program*/
    int pid;
    pid = getpid();
    /* Create string to name directory */
    char *dir = NULL;

    /* Allocate space for directory's name */
    dir = malloc(25 * sizeof(char));
    if (dir == 0)
        printf("malloc() failed\n");
    memset(dir, '\0', 25);

    /* Create directory's name */
    sprintf(dir, "liutim.rooms.%d", pid);

    /* Make directory */
    mkdir(dir, 0755);


    /* Create array of numbers to be shuffled for randomization purposes */
    int shuffled[10];
    int i;
    for (i = 0; i < 10; ++i)
        shuffled[i] = i;

    /* Shuffle array using Durstenfeld's Fisher-Yates shuffle */
    int j;
    int tmp;
    srand(time(0));
    for (i = 9; i > 0; --i)
    {
        j = rand() % (i + 1 - 0) + 0;
        tmp = shuffled[j];
        shuffled[j] = shuffled[i];
        shuffled[i] = tmp;
    }
    /* Hold array of struct Rooms */
    struct Room *rooms[7];

    for (i = 0; i < 7; ++i)
    {
        /* Allocate memory for a struct Room */
        rooms[i] = malloc(sizeof(struct Room));
        if (rooms[i] == 0)
            printf("malloc() failed\n");

        /* Set numConnections to 0 */
        rooms[i]->numConnections = 0;

        /* Allocate memory for the room's name */
        rooms[i]->name = malloc(9 * sizeof(char));
        if (rooms[i]->name == 0)
            printf("malloc() failed\n");

        /* Clear memory for the room's name */
        memset(rooms[i]->name, '\0', 9);

        /* Allocate memory for the room's filename */
        rooms[i]->filename = malloc(35 * sizeof(char));
        if (rooms[i]->filename == 0)
            printf("malloc() failed\n");

        /* Clear memory for the room's filename */
        memset(rooms[i]->filename, '\0', 35);

        /* Assign room names in order of 1st 7 numbers in shuffled array */
        switch (shuffled[i])
        {
            case 0:
                sprintf(rooms[i]->name, "HyrCastl");
                sprintf(rooms[i]->filename, "%s/HyrCastl_Room", dir);
                break;
            case 1:
                sprintf(rooms[i]->name, "TempTime");
                sprintf(rooms[i]->filename, "%s/TempTime_Room", dir);
                break;
            case 2:
                sprintf(rooms[i]->name, "LonRanch");
                sprintf(rooms[i]->filename, "%s/LonRanch_Room", dir);
                break;
            case 3:
                sprintf(rooms[i]->name, "Kokiri");
                sprintf(rooms[i]->filename, "%s/Kokiri_Room", dir);
                break;
            case 4:
                sprintf(rooms[i]->name, "LostWds");
                sprintf(rooms[i]->filename, "%s/LostWds_Room", dir);
                break;
            case 5:
                sprintf(rooms[i]->name, "Kakariko");
                sprintf(rooms[i]->filename, "%s/Kakariko_Room", dir);
                break;
            case 6:
                sprintf(rooms[i]->name, "DeathMtn");
                sprintf(rooms[i]->filename, "%s/DeathMtn_Room", dir);
                break;
            case 7:
                sprintf(rooms[i]->name, "ZorasDom");
                sprintf(rooms[i]->filename, "%s/ZorasDom_Room", dir);
                break;
            case 8:
                sprintf(rooms[i]->name, "LakeHyli");
                sprintf(rooms[i]->filename, "%s/LakeHyli_Room", dir);
                break;
            case 9:
                sprintf(rooms[i]->name, "GerudoV");
                sprintf(rooms[i]->filename, "%s/GerudoV_Room", dir);
                break;
        }
    }
    /* Select 2 unique rooms to be START_ROOM and END_ROOM */
    int randStart = rand() % 7;
    int randEnd = rand() % 7;
    while (randEnd == randStart)
        randEnd = rand() % 7;
    rooms[randStart]->type = START_ROOM;
    rooms[randEnd]->type = END_ROOM;

    /* Assign remaining rooms to be END_ROOMs */
    for (i = 0; i < 7; ++i)
    {
        if (i != randStart && i != randEnd)
            rooms[i]->type = MID_ROOM;
    }
    /* Make random connections between rooms */
    while (IsGraphFull(rooms) != true)
    {
        AddRandomConnection(rooms);
    }
    /* Iterate over each room */
    int oFile;
    for (i = 0; i < 7; ++i)
    {
        /* Open a new file to write to */
        oFile = open(rooms[i]->filename, O_WRONLY | O_CREAT, 0600);
        if (oFile < 0)
        {
            fprintf(stderr, "Could not open %s\n", rooms[i]->filename);
            perror("Error in main()");
            exit(1);
        }
        /* Write room name */
        write(oFile, "ROOM NAME: ", 11 * sizeof(char));
        write(oFile, rooms[i]->name, strlen(rooms[i]->name)*sizeof(char));

        /* Iterate over each room's connections */
        int numCons = rooms[i]->numConnections;
        for (j = 0; j < numCons; ++j)
        {
            /* Write each connection's room name */
            char *str = malloc(20 * sizeof(char));
            if (str == 0)
                printf("malloc() failed\n");
            memset(str, '\0', 20);
            sprintf(str, "\nCONNECTION %d: ", j + 1);
            write(oFile, str, strlen(str) * sizeof(char));
            free(str);
            write(oFile, rooms[i]->connections[j]->name,
                    strlen(rooms[i]->connections[j]->name) * sizeof(char));
        }
        /* Write each room's room type */
        char *typeStr = malloc(25 * sizeof(char));
        if (typeStr == 0)
            printf("malloc() failed\n");
        memset(typeStr, '\0', 25);
        switch (rooms[i]->type)
        {
            case 0:
                sprintf(typeStr, "\nROOM TYPE: START_ROOM\n");
                break;
            case 1:
                sprintf(typeStr, "\nROOM TYPE: MID_ROOM\n");
                break;
            case 2:
                sprintf(typeStr, "\nROOM TYPE: END_ROOM\n");
                break;
        }
        write(oFile, typeStr, strlen(typeStr) * sizeof(char));
        free(typeStr);
        close(oFile);
    }

    /*
    printf("After connections:\n");
    for (i = 0; i < 7; ++i)
    {
        printf("Name: %s, Type: %d, Connections: %d\n", rooms[i]->name, rooms[i]->type, rooms[i]->numConnections);
    }
    printf("List of connections:\n");
    for (i = 0; i < 7; ++i)
    {
        printf("ROOM NAME: %s\n", rooms[i]->name);
        int cons = rooms[i]->numConnections;
        for (j = 0; j < cons; ++j)
            printf("CONNECTION %d: %s\n", j + 1, rooms[i]->connections[j]->name);
        printf("ROOM TYPE: ");
        switch (rooms[i]->type)
        {
            case 0:
                printf("START_ROOM\n");
                break;
            case 1:
                printf("MID_ROOM\n");
                break;
            case 2:
                printf("END_ROOM\n");
                break;
        }
    }
    */


    /* DEBUG: Check if rooms have names and types */
    /*
    for (i = 1; i < 7; i++)
        ConnectRooms(rooms[0], rooms[i]);
    for (i = 0; i < 7; ++i)
    {
        printf("Name: %s, Type: %d, Connections: %d\n", rooms[i]->name, rooms[i]->type, rooms[i]->numConnections);
    }
    for (i = 0; i < rooms[0]->numConnections; i++)
        printf("%s\n", rooms[0]->connections[i]->name);
    struct Room *temp = GetRandomRoom(rooms);
        printf("Random Name: %s, Type: %d, Connections: %d\n", temp->name, temp->type, temp->numConnections);
    */

    
    /* Free memory used to make directory name */
    free(dir);

    /* Free dynamically allocated memory */
    for (i = 0; i < 7; ++i)
    {
        free(rooms[i]->name);
        free(rooms[i]->filename);
        free(rooms[i]);
    }

    return 0;
}

void AddRandomConnection(struct Room * arr[])
{
    struct Room *temp1;
    struct Room *temp2;
    temp1 = GetRandomRoom(arr);
    temp2 = GetRandomRoom(arr);
    while (IsSameRoom(temp1, temp2) == true ||
           AlreadyConnected(temp1, temp2) == true)
        temp2 = GetRandomRoom(arr);
    ConnectRooms(temp1, temp2);
}

struct Room *GetRandomRoom(struct Room * arr[])
{
    /* Get random number from 0 to 6 representing each room in arr */
    int num;
    num = rand() % 7;

    /* Check that the chosen room has space to make connections */
    while (arr[num]->numConnections >= 6)
        num = rand() % 7;
    return arr[num];
}

void ConnectRooms(struct Room *x, struct Room *y)
{
    /* Add the rooms to each room's array of connections */
    x->connections[x->numConnections] = y;
    y->connections[y->numConnections] = x;

    /* Update each room's number of connections */
    x->numConnections++;
    y->numConnections++;
}

enum bool IsSameRoom(struct Room *x, struct Room *y)
{
    /* Check if room names match */
    return strcmp(x->name, y->name) == 0;
}

enum bool AlreadyConnected(struct Room *x, struct Room *y)
{
    /* Iterate over connections to room x */
    int i;
    for (i = 0; i < x->numConnections; ++i)
    {
        /* Check if connection matches room y */
        if (IsSameRoom(x->connections[i], y))
            return true;
    }
    return false;
}

enum bool IsGraphFull(struct Room * arr[])
{
    int i;
    /* Iterate over all rooms */
    for (i = 0; i < 7; ++i)
    {
        /* Check if room has at least 3 connections */
        if (arr[i]->numConnections < 3)
            return false;
    }
    return true;
}
