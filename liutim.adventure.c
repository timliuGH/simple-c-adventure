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
    int status = pthread_mutex_lock(&myMutex);
    if (status != 0)
        printf("lock failed\n");

    /* Hold timestamp of newest subdir */
    int newestDirTime = -1;

    /* Hold prefix of subdir being seached for */
    char targetDirPrefix[32] = "liutim.rooms.";

    /* Hold name of newest subdir that contains the prefix */
    char newestDirName[256];
    memset(newestDirName, '\0', sizeof(newestDirName));

    /* Holds starting directory */
    DIR *dirToCheck;

    /* Holds current subdir of starting directory */
    struct dirent *fileInDir;

    /* Holds information about subdir */
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
                //printf("Found the prefix: %s\n", fileInDir->d_name);
                stat(fileInDir->d_name, &dirAttributes);

                /* Compare latest found entry with current newest entry */
                if ((int)dirAttributes.st_mtime > newestDirTime)
                {
                    /* Update info for newest entry */
                    newestDirTime = (int)dirAttributes.st_mtime;
                    memset(newestDirName, '\0', sizeof(newestDirName));
                    strcpy(newestDirName, fileInDir->d_name);
                    //printf("Newer subdir: %s, new time: %d\n",
                           //fileInDir->d_name, newestDirTime);
                }
            }
        }
    }
    closedir(dirToCheck);

    //printf("Newest entry found is: %s\n", newestDirName);

    /* Hold array of struct Rooms */
    struct Room *rooms[7];
    int rIdx = 0;

    DIR *newestDir;
    /* Open newest directory to add room names to struct Rooms */
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
                /* Set numCons to 0 */
                rooms[rIdx]->numCons = 0;

                //printf("DEBUG File name: %s\n", fileInNewest->d_name);
                /* Allocate memory for the room's filename */
                rooms[rIdx]->filename = malloc(52 * sizeof(char));
                if (rooms[rIdx]->filename == 0)
                    printf("malloc() failed\n");
                /* Clear memory for room's filename */
                memset(rooms[rIdx]->filename, '\0', 52);
                sprintf(rooms[rIdx]->filename, "%s/%s", newestDirName,
                        fileInNewest->d_name);
                /* Open room file */
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

                /* Grab a line */
                char *buffer = NULL;
                size_t bufferSize = 0;
                int numChars = getline(&buffer, &bufferSize, roomFile);
                /* Extract room name */
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

                free(buffer);
                buffer = NULL;

                fclose(roomFile);
            }
        }
    }
    closedir(newestDir);

    /* Add connections, room types, and find start room */
    struct Room *start = NULL;

    for (rIdx = 0; rIdx < 7; ++rIdx)
    {
        FILE *roomFile = fopen(rooms[rIdx]->filename, "r");
        if (roomFile == NULL)
        {
            fprintf(stderr, "Could not open %s\n", rooms[rIdx]->filename);
            exit(1);
        }
        char *buffer = NULL;
        size_t bufferSize = 0;
        /* Skip first line */
        getline(&buffer, &bufferSize, roomFile);
        free(buffer);
        buffer = NULL;

        getline(&buffer, &bufferSize, roomFile);
        while (strstr(buffer, "CONNECTION"))
        {
            if (strstr(buffer, "HyrCastl"))
                AddConnection("HyrCastl", rooms, rIdx);
            else if (strstr(buffer, "TempTime"))
                AddConnection("TempTime", rooms, rIdx);
            else if (strstr(buffer, "LonRanch"))
                AddConnection("LonRanch", rooms, rIdx);
            else if (strstr(buffer, "Kokiri"))
                AddConnection("Kokiri", rooms, rIdx);
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

            free(buffer);
            buffer = NULL;
            getline(&buffer, &bufferSize, roomFile);
        }
        if (strstr(buffer, "START"))
        {
            rooms[rIdx]->type = START_ROOM;
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

    /* Start game */

    struct Room *current = start;
    struct Room *path[32];
    int pathIdx = 0;

    while (current->type != END_ROOM)
    {
        printf("CURRENT LOCATION: %s\n", current->name);
        printf("POSSIBLE CONNECTIONS: ");
        int i;
        int count = current->numCons - 1;
        for (i = 0; i < count; ++i)
            printf("%s, ", current->cons[i]->name);
        printf("%s.\n", current->cons[current->numCons-1]->name);
        printf("WHERE TO? >");
        /* Get user input */
        char *userInputBuffer = NULL;
        size_t userInputSize = 0;
        int userChars = getline(&userInputBuffer, &userInputSize, stdin);
        printf("\n");

        /* Extract user input without newline */
        char input[9] = {'\0'};
        for (i = 0; i < userChars - 1; ++i)
            input[i] = userInputBuffer[i];

        while (strcmp(input, "time") == 0)
        {
            int resCode;
            pthread_t threadID;
            resCode = pthread_create(&threadID, NULL, GetTime, NULL);
            if (resCode != 0)
                printf("pthread_create failed\n");
            status = pthread_mutex_unlock(&myMutex);
            if (status != 0)
                printf("unlock failed\n");
            resCode = pthread_join(threadID, NULL);
            status = pthread_mutex_lock(&myMutex);
            if (status != 0)
                printf("lock failed\n");
            FILE *oFile;
            oFile = fopen("currentTime.txt", "r");
            if (oFile == NULL)
            {
                fprintf(stderr, "Could not open %s\n", "currentTime.txt");
                exit(1);
            }
            size_t timeFileSize = 0;
            char *timeFile = NULL;
            getline(&timeFile, &timeFileSize, oFile);
            printf("%s\n\n", timeFile);
            free(timeFile);
            fclose(oFile);

            printf("WHERE TO? >");
            /* Get user input */
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
                current = current->cons[i];
                path[pathIdx] = current;
                pathIdx++;
                found = true;
            }
        }
        if (!found)
            printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
        free(userInputBuffer);
    }

    printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
    printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", pathIdx);
    int i;
    for (i = 0; i < pathIdx; ++i)
    {
        printf("%s\n", path[i]->name);
    }



/*DEBUG
    for (rIdx = 0; rIdx < 7; ++rIdx)
    {
        printf("Room name: %s\n", rooms[rIdx]->name);
        printf("Num cons: %d\n", rooms[rIdx]->numCons);
        int j;
        for (j = 0; j < rooms[rIdx]->numCons; ++j)
        {
            printf("%s\n", rooms[rIdx]->cons[j]->name);
        }
        printf("Type: %d\n", rooms[rIdx]->type);
    }
*/
    for (rIdx = 0; rIdx < 7; ++rIdx)
    {
        free(rooms[rIdx]->name);
        free(rooms[rIdx]->filename);
        free(rooms[rIdx]);
    }

    return 0;
}

void AddConnection(char str[], struct Room *arr[], int idx)
{
    int i = 0;
    struct Room *temp = arr[i];
    while (strcmp(temp->name, str) != 0)
    {
        i++;
        temp = arr[i];
    }
    arr[idx]->cons[arr[idx]->numCons] = temp;
    arr[idx]->numCons++;
}

void *GetTime()
{
    int status = pthread_mutex_lock(&myMutex);
    if (status != 0)
        printf("lock failed\n");
    int oFile;
    oFile = open("currentTime.txt", O_WRONLY | O_CREAT, 0600);
    if (oFile < 0)
    {
        fprintf(stderr, "Could not open %s\n", "currentTime.txt");
        perror("Error in main()");
        exit(1);
    }
    char timeRes[52];
    time_t t;
    struct tm *tmp;

    t = time(0);
    tmp = localtime(&t);

    strftime(timeRes, sizeof(timeRes), "%l:%M%P, %A, %B %d, %Y", tmp);
    write(oFile, timeRes, strlen(timeRes) * sizeof(char));
    close(oFile);
    status = pthread_mutex_unlock(&myMutex);
    if (status != 0)
        printf("unlock failed\n");

    return NULL;
}
