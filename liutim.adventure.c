#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;

enum bool {false, true};
enum RoomType {START_ROOM, MID_ROOM, END_ROOM};

struct Room
{
    char *name;
    enum RoomType type;
    int numCons;
    struct Room *cons[6];
    char *filename;
};

void AddConnection(char str[], struct Room *arr[], int idx);
void *GetTime();

int main(int argc, char *argv[])
{
    /* Have main thread lock mutex */
    int status = pthread_mutex_lock(&myMutex);
    if (status != 0)
        printf("lock failed\n");

    /* Hold timestamp of newest subdir of room files */
    int newestDirTime = -1;

    /* Hold prefix of subdir being seached for */
    char targetDirPrefix[32] = "liutim.rooms.";

    /* Hold name of newest subdir that contains the prefix */
    char newestDirName[256];
    memset(newestDirName, '\0', sizeof(newestDirName));

    /* Hold starting directory */
    DIR *dirToCheck;

    /* Hold current subdir of starting directory */
    struct dirent *fileInDir;

    /* Hold information about subdir */
    struct stat dirAttributes;

    /* Start search in current directory */
    dirToCheck = opendir(".");

    if (dirToCheck > 0)
    {
        /* Iterate over entries in current directory */
        while ( (fileInDir = readdir(dirToCheck)) != NULL )
        {
            /* Check if prefix is in entry */
            if (strstr(fileInDir->d_name, targetDirPrefix) != NULL)
            {
                stat(fileInDir->d_name, &dirAttributes);

                /* Compare latest found entry with current newest entry */
                if ((int)dirAttributes.st_mtime > newestDirTime)
                {
                    /* Update info for newest entry */
                    newestDirTime = (int)dirAttributes.st_mtime;
                    memset(newestDirName, '\0', sizeof(newestDirName));
                    strcpy(newestDirName, fileInDir->d_name);
                }
            }
        }
    }
    closedir(dirToCheck);

    /* Hold array of struct Rooms */
    struct Room *rooms[7];
    
    /* Hold index of struct Rooms array */
    int rIdx = 0;

    /* Open newest directory to add room names to struct Rooms */
    DIR *newestDir;
    newestDir = opendir(newestDirName);
    struct dirent *fileInNewest;
    if (newestDir > 0)
    {
        while ((fileInNewest = readdir(newestDir)) != NULL)
        {
            /* Check if file is a room file */
            if (strstr(fileInNewest->d_name, "Room") != NULL)
            {
                /* Allocate memory for a struct Room */
                rooms[rIdx] = malloc(sizeof(struct Room));
                if (rooms[rIdx] == 0)
                    printf("malloc() failed\n");

                /* Set the room's connections to 0 */
                rooms[rIdx]->numCons = 0;

                /* Allocate memory for the room's filename */
                rooms[rIdx]->filename = malloc(52 * sizeof(char));
                if (rooms[rIdx]->filename == 0)
                    printf("malloc() failed\n");

                /* Clear memory for room's filename */
                memset(rooms[rIdx]->filename, '\0', 52);

                /* Set room's filename to current entry in directory */
                sprintf(rooms[rIdx]->filename, "%s/%s", newestDirName,
                        fileInNewest->d_name);

                /* Open current room file for reading */
                FILE *roomFile = fopen(rooms[rIdx]->filename, "r");
                if (roomFile == NULL)
                {
                    fprintf(stderr, "Could not open %s\n", 
                            rooms[rIdx]->filename);
                    exit(1);
                }
                
                /* Allocate memory for the room's name */
                rooms[rIdx]->name = malloc(9 * sizeof(char));
                if (rooms[rIdx]->name == 0)
                    printf("malloc() failed\n");

                /* Clear memory for the room's name */
                memset(rooms[rIdx]->name, '\0', 9);

                /* Grab first line from room file */
                char *buffer = NULL;
                size_t bufferSize = 0;
                int numChars = getline(&buffer, &bufferSize, roomFile);

                /* Extract room name (line starts with "ROOM NAME: ")*/
                int c;
                int idx = 0;
                char inputName[9] = {'\0'};
                for (c = 11; c < numChars - 1; c++)
                {
                    inputName[idx] = buffer[c];
                    idx++;
                }
                /* Add room name to struct Room */
                strcpy(rooms[rIdx]->name, inputName);
                rIdx++;

                /* Reset buffer for next getline command */
                free(buffer);
                buffer = NULL;

                fclose(roomFile);
            }
        }
    }
    closedir(newestDir);

    /* Add connections and room types for each room, and find START_ROOM */
    struct Room *start = NULL;

    for (rIdx = 0; rIdx < 7; ++rIdx)
    {
        /* Open current room's room file */
        FILE *roomFile = fopen(rooms[rIdx]->filename, "r");
        if (roomFile == NULL)
        {
            fprintf(stderr, "Could not open %s\n", rooms[rIdx]->filename);
            exit(1);
        }
        char *buffer = NULL;
        size_t bufferSize = 0;

        /* Skip first line (which is the name of the room) */
        getline(&buffer, &bufferSize, roomFile);
        free(buffer);
        buffer = NULL;

        /* Get lines in room file that are connections to other rooms */
        getline(&buffer, &bufferSize, roomFile);
        while (strstr(buffer, "CONNECTION"))
        {
            /* Check if line in file matches a recognized room */
            if (strstr(buffer, "HyrCastl"))
                AddConnection("HyrCastl", rooms, rIdx);
            else if (strstr(buffer, "TempTime"))
                AddConnection("TempTime", rooms, rIdx);
            else if (strstr(buffer, "LonRanch"))
                AddConnection("LonRanch", rooms, rIdx);
            else if (strstr(buffer, "Kokiri"))
                AddConnection("Kokiri", rooms, rIdx);
            else if (strstr(buffer, "LostWds"))
                AddConnection("LostWds", rooms, rIdx);
            else if (strstr(buffer, "Kakariko"))
                AddConnection("Kakariko", rooms, rIdx);
            else if (strstr(buffer, "DeathMtn"))
                AddConnection("DeathMtn", rooms, rIdx);
            else if (strstr(buffer, "ZorasDom"))
                AddConnection("ZorasDom", rooms, rIdx);
            else if (strstr(buffer, "LakeHyli"))
                AddConnection("LakeHyli", rooms, rIdx);
            else if (strstr(buffer, "GerudoV"))
                AddConnection("GerudoV", rooms, rIdx);
            else
                printf("Room not recognized\n");

            /* Reset buffer to retrieve next line in file */
            free(buffer);
            buffer = NULL;
            getline(&buffer, &bufferSize, roomFile);
        }
        /* Assign room type based on info in room file */
        if (strstr(buffer, "START"))
        {
            rooms[rIdx]->type = START_ROOM;

            /* Save starting room */
            start = rooms[rIdx];
        }
        else if (strstr(buffer, "END"))
            rooms[rIdx]->type = END_ROOM;
        else
            rooms[rIdx]->type = MID_ROOM;

        free(buffer);
        buffer = NULL;
        
        fclose(roomFile);
    }

    /* Start the actual game */
    struct Room *current = start;   /* Hold player's current position */
    struct Room *path[32];          /* Hold path player has taken */
    int pathIdx = 0;                /* Index of player's path */

    /* Run game until player has reached END_ROOM */
    while (current->type != END_ROOM)
    {
        /* Output current location and possible connections */
        printf("CURRENT LOCATION: %s\n", current->name);
        printf("POSSIBLE CONNECTIONS: ");
        int i;
        int count = current->numCons - 1;
        for (i = 0; i < count; ++i)
            printf("%s, ", current->cons[i]->name);
        printf("%s.\n", current->cons[current->numCons-1]->name);

        /* Prompt user for location to move to */
        printf("WHERE TO? >");
        char *userInputBuffer = NULL;
        size_t userInputSize = 0;
        int userChars = getline(&userInputBuffer, &userInputSize, stdin);
        printf("\n");

        /* Extract user input without input's newline */
        char input[9] = {'\0'};
        for (i = 0; i < userChars - 1; ++i)
            input[i] = userInputBuffer[i];

        /* Check if user is asking for current time */
        while (strcmp(input, "time") == 0)
        {
            /* Create new thread */
            int resCode;
            pthread_t threadID;
            resCode = pthread_create(&threadID, NULL, GetTime, NULL);
            if (resCode != 0)
                printf("pthread_create failed\n");

            /* Have main thread unlock its lock on mutex */
            status = pthread_mutex_unlock(&myMutex);
            if (status != 0)
                printf("unlock failed\n");

            /* Have main thread wait for new thread to complete */
            resCode = pthread_join(threadID, NULL);

            /* Have main thread relock mutex */
            status = pthread_mutex_lock(&myMutex);
            if (status != 0)
                printf("lock failed\n");

            /* Open currentTime.txt file created by new thread */
            FILE *oFile;
            oFile = fopen("currentTime.txt", "r");
            if (oFile == NULL)
            {
                fprintf(stderr, "Could not open %s\n", "currentTime.txt");
                exit(1);
            }
            /* Get time from file */
            size_t timeFileSize = 0;
            char *timeFile = NULL;
            getline(&timeFile, &timeFileSize, oFile);

            /* Output time to screen */
            printf("%s\n\n", timeFile);
            free(timeFile);
            fclose(oFile);

            /* Prompt user for location to move to */
            printf("WHERE TO? >");
            free(userInputBuffer);
            userInputBuffer = NULL;
            userInputSize = 0;
            userChars = getline(&userInputBuffer, &userInputSize, stdin);
            printf("\n");

            /* Reset input array to hold user input without newline */
            for (i = 0; i < 9; ++i)
                input[i] = '\0';
            /* Extract user input without newline */
            for (i = 0; i < userChars - 1; ++i)
                input[i] = userInputBuffer[i];
        }
        /* Check if input exists in current room's connections */
        count = current->numCons;
        enum bool found = false;
        for (i = 0; i < count; ++i)
        {
            if (!found && strcmp(current->cons[i]->name, input) == 0)
            {
                /* Update player's current position */
                current = current->cons[i];

                /* Update player's path */
                path[pathIdx] = current;
                pathIdx++;
                found = true;
            }
        }
        /* Handle if user input is not a recognized room */
        if (!found)
            printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
        free(userInputBuffer);
    }
    /* Handle if user has reached END_ROOM */
    printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
    printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", pathIdx);

    /* Output list of rooms (path) visited by user */
    int i;
    for (i = 0; i < pathIdx; ++i)
        printf("%s\n", path[i]->name);

    /* Free dynamically allocated memory for each room */
    for (rIdx = 0; rIdx < 7; ++rIdx)
    {
        free(rooms[rIdx]->name);
        free(rooms[rIdx]->filename);
        free(rooms[rIdx]);
    }

    return 0;
}

/* This function takes a room name, an array of pointers to struct Rooms, 
 * and the index of the current room being populated with data. It loops
 * through the array to find the struct Room that matches the passed-in
 * room name, then adds that room as a connection to the passed-in room
 */
void AddConnection(char str[], struct Room *arr[], int idx)
{
    /* Start with first room in array of struct Room pointers */
    int i = 0;
    struct Room *temp = arr[i];

    /* Iterate through array until reach room with matching name */
    while (strcmp(temp->name, str) != 0)
    {
        i++;
        temp = arr[i];
    }
    /* Add the room as a connection */
    arr[idx]->cons[arr[idx]->numCons] = temp;
    arr[idx]->numCons++;
}

/* This function is called by a new thread to create a new file 
 * "currentTime.txt" and write to it the current time and date. 
 */
void *GetTime()
{
    /* Have the new thread lock the mutex */
    int status = pthread_mutex_lock(&myMutex);
    if (status != 0)
        printf("lock failed\n");

    /* Create a new file to write to */
    int oFile;
    oFile = open("currentTime.txt", O_WRONLY | O_CREAT, 0600);
    if (oFile < 0)
    {
        fprintf(stderr, "Could not open %s\n", "currentTime.txt");
        perror("Error in main()");
        exit(1);
    }
    /* Hold the formatted time and date */
    char timeRes[52];

    /* Get the current time and date */
    time_t t;
    struct tm *tmp;
    t = time(0);
    tmp = localtime(&t);

    /* Format the time and date as 00:00am, Day, Month 00, 0000 */
    strftime(timeRes, sizeof(timeRes), "%l:%M%P, %A, %B %d, %Y", tmp);

    /* Write the formatted time and date to the new file */
    write(oFile, timeRes, strlen(timeRes) * sizeof(char));
    close(oFile);

    /* Have the new thread unlock the mutex */
    status = pthread_mutex_unlock(&myMutex);
    if (status != 0)
        printf("unlock failed\n");

    return NULL;
}
