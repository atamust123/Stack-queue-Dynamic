#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct Stack
{ //frame
    int top;
    char **data;
};

struct Queue
{
    int rear,front;
    struct Stack *frame;
};

struct Queue *enqueue(struct Queue *queue, struct Stack *frame, int currentFrame)
{

    queue->frame = (struct Stack *)realloc(queue->frame, sizeof(struct Stack) * (currentFrame + 1));
    queue->frame[++queue->rear] = frame[currentFrame];
    return queue;
}

struct PC
{
    char *ID; //created to hold clients.dat infos
    char *IP;
    char *MAC;
    struct Queue *inqueue;
    struct Queue *outqueue;
    int hops;
    int success;
};

struct Stack *frame;

void push(char *data, int currentFrame)
{
    frame[currentFrame].data[++frame[currentFrame].top] = (char *)realloc(frame[currentFrame].data[frame[currentFrame].top], sizeof(char) * (strlen(data)));
    strcpy(frame[currentFrame].data[frame[currentFrame].top], data);
}

char *pop(struct Stack *frame,int currentFrame)
{
    free(frame[currentFrame].data[frame[currentFrame].top]);
    frame[currentFrame].data[frame[currentFrame].top--] = NULL;
}

char *peek(int currentFrame)
{
    return frame[currentFrame].data[frame[currentFrame].top];
}

void SEND(char **intendedArray, char **nextArray, char receiverID[2], char senderID[2], char clientID, struct PC *clientsArray, int client_number, int framenumber, int *hoppa);

int main(int argc, char *argv[])
{
    FILE *clientsF = fopen(argv[1], "r");
    FILE *routingF = fopen(argv[2], "r");
    FILE *commandF = fopen(argv[3], "r");
    int maxMessageSize = atoi(argv[4]);

    int client_number, i = 0, j;
    char s[256], *token;

    struct PC *clientsArray; //= malloc(sizeof(struct PC));

    fscanf(clientsF, "%d\n", &client_number);

    clientsArray = malloc(sizeof(struct PC) * client_number); //holds clients infos

    //clients.dat processing...

    while (i < client_number) //clients passing to struct
    {
        fgets(s, 22, clientsF);

        token = strtok(s, " ");
        clientsArray[i].ID = (char *)malloc(1);
        strcpy(clientsArray[i].ID, token);

        token = strtok(NULL, " ");
        clientsArray[i].IP = (char *)malloc(7);
        strcpy(clientsArray[i].IP, token);

        token = strtok(NULL, "\n");
        clientsArray[i].MAC = (char *)malloc(10);
        strcpy(clientsArray[i].MAC, token);
        clientsArray[i].inqueue = (struct Queue *)malloc(sizeof(struct Queue));
        clientsArray[i].inqueue->frame = (struct Stack *)malloc(sizeof(struct Stack)); //Queue is created DYNAMICALLY...
        clientsArray[i].inqueue->rear = -1;
        clientsArray[i].inqueue->front=-1;
        clientsArray[i].outqueue = (struct Queue *)malloc(sizeof(struct Queue));
        clientsArray[i].outqueue->frame = (struct Stack *)malloc(sizeof(struct Stack)); //Queue is created DYNAMICALLY...
        clientsArray[i].outqueue->rear = -1;
        clientsArray[i].outqueue->front=-1;
        clientsArray[i].success = 1;
        i++;
    }

    //routing.dat files are processing...
    char routes[5];
    int counter = 0, row = 0;
    char **intendedArray;
    char **nextArray;

    intendedArray = (char **)malloc(sizeof(char *) * client_number);

    for (i = 0; i < client_number; i++)
        *(intendedArray + i) = (char *)malloc(sizeof(char) * 2);

    nextArray = (char **)malloc(sizeof(char *) * (client_number));

    for (i = 0; i < client_number; i++)
        *(nextArray + i) = (char *)malloc(sizeof(char) * 2);

    for (i = 1; i <= ((client_number - 1) * (client_number + 1)); i++) //iterate until file ends.
    {
        fgets(routes, 5, routingF);
        if (i % client_number == 0) //continue if there is a hyphen.
        {
            counter++;
            row = 0;
            continue;
        }
        else
        {
            intendedArray[counter][row] = routes[0]; //intendedarray[which client][which intended]
            nextArray[counter][row] = routes[2];     //nextArray[which client][which direction]
            row++;
        }
    }

    //Commands.dat file processing...
    int commandNumber, frameCounter, framenumber, currentFrame, receiverCounter, WhichRoute, stackTopCounter = 0, a, b;
    int currentCommand = 0;
    char commands[20], senderID[2], clientID[2], whichQueue[3], receiverID[2], routeDirection[2] = "\0";
    char **tempStackArray, **partedMessage;
    int routeCounter;
    fscanf(commandF, "%d", &commandNumber);
    char string1[1000];
    char message[] = "";

    int whichFrameNumber, hoppa = 0;

    while (currentCommand < commandNumber)
    {
        fscanf(commandF, "%s", commands);
        if (strcmp(commands, "MESSAGE") == 0) //Message command is processing...
        {
            printf("---------------------------------------------------------------------------------------\n");
            printf("Command: %s ", commands);
            fscanf(commandF, "%s", senderID); //Sender ID taken from message command
            printf("%s ", senderID);
            fscanf(commandF, "%s ", receiverID); //Receiver Id taken
            printf("%s ", receiverID);
            fgets(string1, 1000, commandF); //Message taken by fgets from commands.dat

            token = strtok(string1, "#"); //message parsing from delimiters

            strcpy(message, token);

            printf("#%s#", message);
            printf("\n---------------------------------------------------------------------------------------\n");
            printf("Message to be sent: %s", message);

            framenumber = strlen(message) / maxMessageSize + (strlen(message) % maxMessageSize != 0); //frame number calculated.

            //message parsing by max message size then message passing temp 2D array.
            partedMessage = (char **)malloc(sizeof(char *) * framenumber);
            for (i = 0; i < framenumber; i++)
                partedMessage[i] = (char *)malloc(sizeof(char *) * (maxMessageSize + 1));

            //temp. frame array .
            tempStackArray = (char **)malloc(sizeof(char *) * 9);
            for (i = 0; i < 9; i++)
                tempStackArray[i] = (char *)malloc(sizeof(char));

            frameCounter = 0;
            i = 0, j = 0;
            //message stored by temp. 2D array
            while (i < strlen(message))
            {
                partedMessage[frameCounter][j++] = message[i++];
                if (j == maxMessageSize || i == strlen(message)) //go next frame after end of string.
                {
                    partedMessage[frameCounter][j] = '\0';
                    frameCounter++;
                    j = 0;
                }
            }
            printf("\n\n");

            frame = (struct Stack *)malloc(sizeof(struct Stack)); //stack is created.

            currentFrame = 0, receiverCounter = 0, routeCounter = 0;
            i = 0;
            while (currentFrame < framenumber)
            {
                for (i = 0; i < client_number; i++)
                {
                    if (strcmp(clientsArray[i].ID, senderID) == 0)
                    {
                        if (strcmp(clientsArray[receiverCounter].ID, receiverID) == 0)
                        {
                            for (WhichRoute = 0; WhichRoute < (client_number - 1); WhichRoute++)
                            {
                                if (intendedArray[i][WhichRoute] == receiverID[0]) //which direction is intended
                                    routeDirection[0] = nextArray[i][WhichRoute];  //we find which route should we go first
                            }

                            if (strcmp(clientsArray[routeCounter].ID, routeDirection) == 0)
                            {
                                stackTopCounter = 0;
                                frame = realloc(frame, sizeof(struct Stack) * (currentFrame + 1)); //frame reallocated frame by frame
                                frame[currentFrame].data = (char **)malloc(sizeof(char *) * 9);
                                frame[currentFrame].top = -1;
                                for (a = 0; a < 9; a++)
                                    frame[currentFrame].data[a] = (char *)malloc(sizeof(char));
                                //infos are pushing into stack...

                                push(partedMessage[currentFrame], currentFrame);
                                push(clientsArray[receiverCounter].ID, currentFrame);
                                push(clientsArray[i].ID, currentFrame);

                                push(argv[6], currentFrame);
                                push(argv[5], currentFrame);

                                push(clientsArray[receiverCounter].IP, currentFrame);
                                push(clientsArray[i].IP, currentFrame);

                                push(clientsArray[routeCounter].MAC, currentFrame);
                                push(clientsArray[i].MAC, currentFrame);

                                printf("Frame #%d\nSender MAC address: %s", currentFrame + 1, peek(currentFrame));

                                tempStackArray[stackTopCounter] = (char *)realloc(tempStackArray[stackTopCounter], sizeof(char) * (strlen(peek(currentFrame))));
                                strcpy(tempStackArray[stackTopCounter++], peek(currentFrame));
                                pop(frame,currentFrame);

                                printf(", Receiver MAC address: %s\n", peek(currentFrame));

                                tempStackArray[stackTopCounter] = (char *)realloc(tempStackArray[stackTopCounter], sizeof(char) * (strlen(peek(currentFrame))));
                                strcpy(tempStackArray[stackTopCounter++], peek(currentFrame));
                                pop(frame,currentFrame);

                                printf("Sender IP address: %s,", peek(currentFrame));

                                tempStackArray[stackTopCounter] = (char *)realloc(tempStackArray[stackTopCounter], sizeof(char) * (strlen(peek(currentFrame))));
                                strcpy(tempStackArray[stackTopCounter++], peek(currentFrame));
                                pop(frame,currentFrame);

                                printf(" Receiver IP address: %s\n", peek(currentFrame));

                                tempStackArray[stackTopCounter] = (char *)realloc(tempStackArray[stackTopCounter], sizeof(char) * (strlen(peek(currentFrame))));
                                strcpy(tempStackArray[stackTopCounter++], peek(currentFrame));
                                pop(frame,currentFrame);

                                printf("Sender port number: %s,", peek(currentFrame));

                                tempStackArray[stackTopCounter] = (char *)realloc(tempStackArray[stackTopCounter], sizeof(char) * (strlen(peek(currentFrame))));
                                strcpy(tempStackArray[stackTopCounter++], peek(currentFrame));
                                pop(frame,currentFrame);

                                printf(" Receiver port number: %s\n", peek(currentFrame));

                                tempStackArray[stackTopCounter] = (char *)realloc(tempStackArray[stackTopCounter], sizeof(char) * (strlen(peek(currentFrame))));
                                strcpy(tempStackArray[stackTopCounter++], peek(currentFrame));
                                pop(frame,currentFrame);

                                printf("Sender ID: %s,", peek(currentFrame));

                                tempStackArray[stackTopCounter] = (char *)realloc(tempStackArray[stackTopCounter], sizeof(char) * (strlen(peek(currentFrame))));
                                strcpy(tempStackArray[stackTopCounter++], peek(currentFrame));
                                pop(frame,currentFrame);

                                printf(" Receiver ID: %s\n", peek(currentFrame));

                                tempStackArray[stackTopCounter] = (char *)realloc(tempStackArray[stackTopCounter], sizeof(char) * (strlen(peek(currentFrame))));
                                strcpy(tempStackArray[stackTopCounter], peek(currentFrame));
                                pop(frame,currentFrame);

                                printf("Message chunk carried: %s\n", peek(currentFrame));
                                printf("--------\n");

                                while (stackTopCounter >= 0)
                                {
                                    push(tempStackArray[stackTopCounter--], currentFrame);
                                }

                                clientsArray[i].outqueue = enqueue(clientsArray[i].outqueue, frame, currentFrame); //frames pass to queue...

                                currentFrame++;
                            }
                            else
                                routeCounter++;
                        }
                        else
                            receiverCounter++;
                    }
                }
            }

            currentCommand++;
        }
        else if (strcmp(commands, "SHOW_FRAME_INFO") == 0)
        {
            printf("--------------------------------\n");
            printf("Command: %s ", commands);
            fscanf(commandF, "%s ", clientID);
            printf("%s ", clientID);
            fscanf(commandF, "%s ", whichQueue);
            printf("%s ", whichQueue);
            fscanf(commandF, "%d", &whichFrameNumber);
            printf("%d\n", whichFrameNumber);
            printf("--------------------------------\n");
            if (whichFrameNumber > framenumber || whichFrameNumber <= 0)
            {
                printf("No such frame.\n");
                currentCommand++;
                continue;
            }
            else if (strcmp(whichQueue, "out") == 0)
            {
                for (i = 0; i < client_number; i++)
                {
                    if (strcmp(clientsArray[i].ID, clientID) == 0)
                    {
                        if (clientsArray[i].outqueue->rear == -1)
                        {
                            printf("No such frame.\n");
                        }
                        else
                        {
                            printf("Current Frame #%d on the outgoing queue of client %s\n", whichFrameNumber, clientID);
                            printf("Carried Message: \"%s\"\n", clientsArray[i].outqueue->frame[(whichFrameNumber - 1)].data[0]);
                            printf("Layer 0 info: Sender ID: %s, Receiver ID: %s\n", clientsArray[i].outqueue->frame[(whichFrameNumber - 1)].data[2],
                                   clientsArray[i].outqueue->frame[(whichFrameNumber - 1)].data[1]);
                            printf("Layer 1 info: Sender port number: %s, Receiver port number: %s\n", clientsArray[i].outqueue->frame[(whichFrameNumber - 1)].data[4],
                                   clientsArray[i].outqueue->frame[(whichFrameNumber - 1)].data[3]);
                            printf("Layer 2 info: Sender IP address: %s, Receiver IP address: %s\n", clientsArray[i].outqueue->frame[(whichFrameNumber - 1)].data[6],
                                   clientsArray[i].outqueue->frame[(whichFrameNumber - 1)].data[5]);
                            printf("Layer 3 info: Sender MAC address: %s, Receiver MAC address: %s\n", clientsArray[i].outqueue->frame[(whichFrameNumber - 1)].data[8],
                                   clientsArray[i].outqueue->frame[(whichFrameNumber - 1)].data[7]);
                            printf("Number of hops so far: %d\n", hoppa);
                        }
                    }
                }
            }
            else if (strcmp(whichQueue, "in") == 0)
            {
                for (i = 0; i < client_number; i++)
                {
                    if (strcmp(clientsArray[i].ID, clientID) == 0)
                    {
                        if (clientsArray[i].inqueue->rear == -1)
                        {
                            printf("No such frame.\n");
                        }
                        else
                        {
                            printf("Current Frame #%d on the incoming queue of client %s\n", whichFrameNumber, clientID);
                            printf("Carried Message: \"%s\"\n", clientsArray[i].inqueue->frame[(whichFrameNumber - 1)].data[0]);
                            printf("Layer 0 info: Sender ID: %s, Receiver ID: %s\n", clientsArray[i].inqueue->frame[(whichFrameNumber - 1)].data[2],
                                   clientsArray[i].inqueue->frame[(whichFrameNumber - 1)].data[1]);
                            printf("Layer 1 info: Sender port number: %s, Receiver port number: %s\n", clientsArray[i].inqueue->frame[(whichFrameNumber - 1)].data[4],
                                   clientsArray[i].inqueue->frame[(whichFrameNumber - 1)].data[3]);
                            printf("Layer 2 info: Sender IP address: %s, Receiver IP address: %s\n", clientsArray[i].inqueue->frame[(whichFrameNumber - 1)].data[6],
                                   clientsArray[i].inqueue->frame[(whichFrameNumber - 1)].data[5]);
                            printf("Layer 3 info: Sender MAC address: %s, Receiver MAC address: %s\n", clientsArray[i].inqueue->frame[(whichFrameNumber - 1)].data[8],
                                   clientsArray[i].inqueue->frame[(whichFrameNumber - 1)].data[7]);
                            printf("Number of hops so far: %d\n", hoppa);
                        }
                    }
                }
            }
            currentCommand++;
        }
        else if (strcmp(commands, "SHOW_Q_INFO") == 0)
        {
            printf("---------------------------\n");
            fscanf(commandF, "%s", clientID);

            for (i = 0; i < client_number; i++)
            {
                if (strcmp(clientID, clientsArray[i].ID) == 0)
                {

                    fscanf(commandF, "%s", whichQueue);
                    printf("Command: %s %s %s\n", commands, clientID, whichQueue);
                    printf("---------------------------\n");
                    printf("Client %s ", clientID);

                    if (strcmp(whichQueue, "out") == 0)
                    {
                        printf("Outgoing Queue Status\n");
                        printf("Current total number of frames: %d\n", (clientsArray[i].outqueue->rear + 1)); //we add to rear variable +1 because it starts 0
                    }
                    else if (strcmp(whichQueue, "in") == 0)
                    {
                        printf("Incoming Queue Status\n");
                        printf("Current total number of frames: %d\n", (clientsArray[i].inqueue->rear + 1));
                    }
                }
            }
            currentCommand++;
        }
        else if (strcmp(commands, "SEND") == 0)
        {
            printf("----------------\n");
            fscanf(commandF, "%s", clientID);
            printf("Command: %s %s\n", commands, clientID);
            printf("----------------\n");
            SEND(intendedArray, nextArray, receiverID, senderID, clientID[0], clientsArray, client_number, framenumber, &hoppa);
            currentCommand++;
        }
        else if (strcmp(commands, "PRINT_LOG") == 0)
        {
            printf("---------------------\n");
            fscanf(commandF, "%s", clientID);
            printf("Command: %s %s\n", commands, clientID);
            printf("---------------------\n");
            printf("Client %s Logs:\n", clientID);

            i = 0;
            while (clientsArray[i].ID[0] != clientID[0])
            {
                i++;
            }
            time_t now;
            j = 0;
            struct tm ts; //timestampt.
            char buf[21];

            if (clientsArray[i].inqueue->frame[0].top == 8)
            {
                printf("--------------\n");
                printf("Log Entry #%d:\n", ++j);
                printf("Timestamp: ");
                time(&now);
                ts = *localtime(&now);
                strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S ", &ts);
                printf("%s\n", buf);
                printf("Message: %s\n", message);
                printf("Number of frames: %d\n", framenumber);
                printf("Number of hops: %d\n", clientsArray[i].hops);
                printf("Sender ID: %s\n", senderID);
                printf("Receiver ID: %s\n", receiverID);
                printf("Activity: Message Received\n");
                printf("Success: Yes\n");
            }

            if (clientsArray[i].outqueue->frame[0].top == 8)
            {
                printf("--------------\n");
                printf("Log Entry #%d:\n", ++j);
                printf("Timestamp: ");
                time(&now);
                ts = *localtime(&now);
                strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S ", &ts);
                printf("%s\n", buf);
                printf("Message: %s\n", message);
                printf("Number of frames: %d\n", framenumber);
                printf("Number of hops: %d\n", clientsArray[i].hops);
                printf("Sender ID: %s\n", senderID);
                printf("Receiver ID: %s\n", receiverID);
                printf("Activity: Message Forwarded\n");
                printf("Success: ");
                if (clientsArray[i].success == 1)
                    printf("Yes\n");
                else
                    printf("No\n");
            }

            currentCommand++;
        }
        else
        {
            printf("---------------------\n");
            printf("Command: %s ", commands);
            fscanf(commandF, "%s ", commands);
            printf("%s\n", commands);
            printf("---------------------\n");
            printf("Invalid command.\n");
            currentCommand++;
        }
    }

    for (i = 0; i < client_number; i++)
    {
        free(clientsArray[i].ID);
        free(clientsArray[i].IP);
        free(clientsArray[i].MAC);
        free(clientsArray[i].inqueue);
        free(clientsArray[i].outqueue);
    }
    free(clientsArray);

    for (a = 0; a < framenumber; a++)
        for (b = 0; b < 9; b++)
            free(frame[a].data[b]);
    free(frame);

    for (i = 0; i < 9; i++)
        free(tempStackArray[i]);
    free(tempStackArray);

    for (i = 0; i < client_number; i++)
        free(intendedArray[i]);
    free(intendedArray);

    for (i = 0; i < client_number; i++)
        free(nextArray[i]);
    free(nextArray);

    for (i = 0; i < framenumber; i++)
        free(partedMessage[i]);
    free(partedMessage);
    fclose(clientsF);
    fclose(commandF);
    fclose(routingF);
    return 0;
}

void SEND(char **intendedArray, char **nextArray, char receiverID[2], char senderID[2], char clientID, struct PC *clientsArray, int client_number, int framenumber, int *hoppa)
{ //Comments show only first send recursion operations.
    int i = 0, j = 0, senderTemp, receiverTemp, forwarder, nextforwarder,currentFrame;
    while (clientsArray[i].ID[0] != clientID)
    { //c
        i++;
    }
    senderTemp = i;

    while (intendedArray[i][j] != receiverID[0])
    { //e
        j++;
    }
    receiverTemp = j;
    if (nextArray[senderTemp][receiverTemp] == '-')
    { //if there is a hyphen print error.
        //printf("Error: Unreachable destination. Packets are dropped after %d hops!", *hoppa);
    }
    else
    {
        i = 0;
        while (clientsArray[i].ID[0] != nextArray[senderTemp][receiverTemp]) //to find b mac
        {                                                                    //b
            i++;
        }
        forwarder = i; // sender is changed New sender is forwarder
        printf("A message received by client %s, but intended for client %s. Forwarding...\n", clientsArray[forwarder].ID, receiverID);
        i = 0;
        while (intendedArray[forwarder][i] != receiverID[0])
        { //to find d mac
            i++;
        }
        nextforwarder = i;
        if (nextArray[forwarder][nextforwarder] == '-')
        { //if there is a hyphen print error.
            printf("Error: Unreachable destination. Packets are dropped after %d hops!\n", ++*hoppa);
            clientsArray[forwarder].hops = *hoppa;
        }
        else
        {
            i = 0;
            while (clientsArray[i].ID[0] != nextArray[forwarder][nextforwarder]) //first receiver found.
            {
                i++;
            }
            j = i;
            ++*hoppa;
            clientsArray[forwarder].hops = *hoppa;
            for (i = 0; i < framenumber; i++)
            {

                //senderout queue- >>forwarder inqueue
                //mac addr. changes
                //forwarder inqueue ->>forwarder outqueue

                clientsArray[forwarder].inqueue = enqueue(clientsArray[forwarder].inqueue, clientsArray[senderTemp].outqueue->frame, i);

                strcpy(clientsArray[forwarder].inqueue->frame[i].data[8], clientsArray[forwarder].inqueue->frame[i].data[7]);
                strcpy(clientsArray[forwarder].inqueue->frame[i].data[7], clientsArray[j].MAC);
                clientsArray[forwarder].success = 1; //message is successfully operating...
                clientsArray[forwarder].outqueue = enqueue(clientsArray[forwarder].outqueue, clientsArray[forwarder].inqueue->frame, i);

                printf("        Frame #%d MAC address change: New sender MAC %s, new receiver MAC %s\n", i + 1,
                       clientsArray[forwarder].outqueue->frame[i].data[8], clientsArray[forwarder].outqueue->frame[i].data[7]);
            }
        }
        if (nextArray[forwarder][nextforwarder] != receiverID[0]) //Recursion is processing...
            SEND(intendedArray, nextArray, receiverID, senderID, clientsArray[forwarder].ID[0], clientsArray, client_number, framenumber, hoppa);
        else
        {
            i = 0;
            while (clientsArray[i].ID[0] != receiverID[0]) //Final destination.
            {
                i++;
            }
            ++*hoppa;
            clientsArray[i].hops = *hoppa;

            for(j=0;j<framenumber;j++)
                clientsArray[i].inqueue=enqueue(clientsArray[i].inqueue,clientsArray[forwarder].outqueue->frame,j);
            j=i;

            printf("A message received by client %s from client %s after a total of %d hops.\n", receiverID, senderID, *hoppa);
            printf("Message: ");
            
            for (currentFrame=0;currentFrame<framenumber;currentFrame++)
                for (i=8;i>0;i--)
                    pop(clientsArray[j].inqueue->frame,currentFrame);       //frames of queue destroyed to print message.
                
            for (i = 0; i < framenumber; i++)
                printf("%s", clientsArray[j].inqueue->frame[i].data[clientsArray[j].inqueue->frame[i].top]);                
            printf("\n");
        }
    }
}