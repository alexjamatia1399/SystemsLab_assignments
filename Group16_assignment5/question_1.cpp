#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
using namespace std;
#define N 5              // capacity of restraunt
#define T 30             // total no of diners
int noDinersOutside = 0; // no of diners waiting outside the restraunt
int noDinersInside = 0;  // no of diners waiting inside the restraunt
pthread_mutex_t outputMutex;
pthread_mutex_t mtx;       // mutex to access shared counter
pthread_mutex_t frontDoor; // lock front door
pthread_mutex_t backDoor;  // lock back door
pthread_cond_t isFull;     // conditional variable to signal restraunt thread that its full
pthread_cond_t isEmpty;    // condition variable to signal restraunt that its empty

// restraunt thread
void *restraunt(void *arg)
{
    int s = T / N;
    while (s--)
    {
        pthread_mutex_lock(&mtx);
        if (noDinersInside < N)
            pthread_cond_wait(&isFull, &mtx);
        pthread_mutex_unlock(&mtx);
        chrono::seconds delay(1);
        cout << "The restraunt is at full capacity now, all N diners are present, service will start\n";
        cout << "Serving all diners\n";
        this_thread::sleep_for(delay);
        cout << "Served\n";
        pthread_mutex_unlock(&backDoor); // unlock back door
        pthread_mutex_lock(&mtx);
        while (noDinersInside != 0)
            pthread_cond_wait(&isEmpty, &mtx); // wait till restraunt is empty
        pthread_mutex_unlock(&mtx);
        cout << "Restraunt is now empty\n";
        // cleanup for the next batch
        pthread_mutex_unlock(&frontDoor); // unlock the front door for the next batch
        pthread_mutex_lock(&backDoor);    // lock the back door
        cout << "Ready for next batch\n";
    }
}
void *diner(void *arg)
{
    pthread_mutex_lock(&frontDoor);
    pthread_mutex_lock(&mtx);
    noDinersInside++;
    if (noDinersInside < N)
        pthread_mutex_unlock(&frontDoor);
    pthread_mutex_unlock(&mtx);
    // so only N diners can enter through the front door, after that it stays locked
    if (noDinersInside == N)
        pthread_cond_signal(&isFull); // signal restraunt that its full

    pthread_mutex_lock(&backDoor); // all the diner threads wait till back door is unlocked
    pthread_mutex_lock(&mtx);
    noDinersInside--;
    if (noDinersInside == 0)
        pthread_cond_signal(&isEmpty); // signal restraunt that last diner has left
    pthread_mutex_unlock(&mtx);
    pthread_mutex_unlock(&backDoor);
    // diner leaves
}

int main()
{
    // FILE* file = freopen("output.txt", "w", stdout);
    pthread_mutex_init(&mtx, NULL);
    pthread_mutex_init(&frontDoor, NULL);
    pthread_mutex_init(&backDoor, NULL);
    pthread_mutex_lock(&backDoor);
    pthread_cond_init(&isFull, NULL);
    pthread_cond_init(&isEmpty, NULL);

    pthread_t restrauntThread;
    if (pthread_create(&restrauntThread, NULL, restraunt, NULL) != 0)
    {
        cout << "error creating restraunt thread\n";
        exit(0);
    }
    pthread_t dinerThread[T];

    for (int i = 0; i < T; i++)
    {
        if (pthread_create(&dinerThread[i], NULL, diner, NULL) != 0)
        {
            cout << "error creating diner thread thread\n";
            exit(0);
        }
    }

    for (int i = 0; i < T; i++)
    {
        pthread_join(dinerThread[i], NULL);
    }
    pthread_join(restrauntThread, NULL);
    // waiting for all threads to come back
    // fclose(file);
}