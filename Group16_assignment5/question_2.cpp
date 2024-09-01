#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#define CAP 5
#define CROSSTIME 1

using namespace std;

pthread_mutex_t bridgeMutex; // mutex to secure critical section containing people on bridge
pthread_cond_t emptyBridge;  // conditional mutex check for bridge emptyBridge or people on bridge is less

int peopleOnBridge = 0;  // indicates people crossing the bridge
int bridgeDirection = 1; // direction of the vehicles on the bridge => 2 - south, 1 - north

// CASE 1
void enterBridge1(int direction)
{
    // lock mutex
    pthread_mutex_lock(&bridgeMutex);
    // wait when bridge capaciy exceeds or there are people coming from other direction
    while (peopleOnBridge > 0 && (peopleOnBridge >= CAP || direction != bridgeDirection))
    {
        pthread_cond_wait(&emptyBridge, &bridgeMutex);
    }
    // set direction
    if (peopleOnBridge == 0)
        bridgeDirection = direction;
    peopleOnBridge++;
    // unlock mutex
    pthread_mutex_unlock(&bridgeMutex);
}

void exitBridge1()
{
    // lock mutex
    pthread_mutex_lock(&bridgeMutex);
    peopleOnBridge--;
    // signal to threads waiting in case capacity exceeds or opposite direction traffic
    pthread_cond_signal(&emptyBridge);
    // unlock mutex
    pthread_mutex_unlock(&bridgeMutex);
}

void *northWorker1(void *arg)
{
    enterBridge1(1);
    cout << "Crossing bridge...Number of people on bridge " << peopleOnBridge << "\n";
    sleep(CROSSTIME); // sleep thread to imitate crossing
    exitBridge1();
}

void *southWorker1(void *arg)
{
    enterBridge1(2);
    cout << "Crossing bridge...Number of people on bridge " << peopleOnBridge << "\n";
    sleep(CROSSTIME); // sleep thread to imitate crossing
    exitBridge1();
}

int case1(int numOfNorthPerson, int numOfSouthPerson)
{
    pthread_t northThread[numOfNorthPerson], southThread[numOfSouthPerson];

    // mutex intialisation
    pthread_mutex_init(&bridgeMutex, NULL);
    pthread_cond_init(&emptyBridge, NULL);

    // thread creation
    for (int i = 0; i < max(numOfNorthPerson, numOfSouthPerson); i++)
    {
        if (i < numOfNorthPerson)
            if (pthread_create(&northThread[i], NULL, northWorker1, NULL))
            {
                printf("thread creation failed\n");
                return EXIT_FAILURE;
            }
        if (i < numOfSouthPerson)
            if (pthread_create(&southThread[i], NULL, southWorker1, NULL))
            {
                printf("thread creation failed\n");
                return EXIT_FAILURE;
            }
    }
    // Waiting for threads to join
    for (int i = 0; i < max(numOfNorthPerson, numOfSouthPerson); i++)
    {
        if (i < numOfNorthPerson)
            if (northThread[i])
                pthread_join(northThread[i], NULL);
        if (i < numOfSouthPerson)
            if (southThread[i])
                pthread_join(southThread[i], NULL);
    }
    return 2;
}

// CASE 2
// Entry to critical section
void enterBridge2()
{
    // lock mutex
    pthread_mutex_lock(&bridgeMutex);

    // wait condition when capacity of bridge is full
    while (peopleOnBridge > 0 && (peopleOnBridge >= CAP))
    {
        pthread_cond_wait(&emptyBridge, &bridgeMutex);
    }
    peopleOnBridge++;
    // unlock mutex
    pthread_mutex_unlock(&bridgeMutex);
}

void exitBridge2()
{
    // lock mutex
    pthread_mutex_lock(&bridgeMutex);
    peopleOnBridge--;
    // signal threads which sleeping because bridge capacity exceeded
    pthread_cond_signal(&emptyBridge);
    // unlock mutex
    pthread_mutex_unlock(&bridgeMutex);
}

// Worker Function for north person
void *northWorker2(void *arg)
{
    enterBridge2();
    cout << "Crossing bridge from north...Number of people on bridge " << peopleOnBridge << "\n";
    sleep(CROSSTIME); // sleep thread to imitate crossing
    exitBridge2();
}

// Worker Function for south person
void *southWorker2(void *arg)
{
    enterBridge2();
    cout << "Crossing bridge from south...Number of people on bridge " << peopleOnBridge << "\n";
    sleep(CROSSTIME); // sleep thread to imitate crossing
    exitBridge2();
}

int case2(int numOfNorthPerson, int numOfSouthPerson)
{
    pthread_t northThread[numOfNorthPerson], southThread[numOfSouthPerson];

    // Intialise mutex
    pthread_mutex_init(&bridgeMutex, NULL);
    pthread_cond_init(&emptyBridge, NULL);

    // Thread creation
    for (int i = 0; i < max(numOfNorthPerson, numOfSouthPerson); i++)
    {
        if (i < numOfNorthPerson)
            if (pthread_create(&northThread[i], NULL, northWorker2, NULL))
            {
                printf("thread creation failed\n");
                return EXIT_FAILURE;
            }
        if (i < numOfSouthPerson)
            if (pthread_create(&southThread[i], NULL, southWorker2, NULL))
            {
                printf("thread creation failed\n");
                return EXIT_FAILURE;
            }
    }

    // Waiting for threads to join
    for (int i = 0; i < max(numOfNorthPerson, numOfSouthPerson); i++)
    {
        if (i < numOfNorthPerson)
            if (northThread[i])
                pthread_join(northThread[i], NULL);
        if (i < numOfSouthPerson)
            if (southThread[i])
                pthread_join(southThread[i], NULL);
    }
    return 2;
}

int main()
{
    int numOfNorthPerson, numOfSouthPerson;
    int t;

    // User Inputs
    cout << "Enter Number of North Person\n";
    cin >> numOfNorthPerson;
    cout << "Enter Number of South Person\n";
    cin >> numOfSouthPerson;
    cout << "Enter Case\n";
    cin >> t;

    switch (t)
    {
    case 1:
        case1(numOfNorthPerson, numOfSouthPerson);
        break;
    case 2:
        case2(numOfNorthPerson, numOfSouthPerson);
        break;
    }
    return 0;
}