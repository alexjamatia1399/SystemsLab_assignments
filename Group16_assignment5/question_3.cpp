#include <iostream>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <pthread.h>
#include <chrono>
using namespace std;
int cntG = 0;
struct request
{
    int reqId;
    int serviceNo;
    int resourceNeeded;
    int canBeServed = 1; // if req could never be served
    int didWait = 0;     // if req waited due to high traffic
    std::chrono::time_point<std::chrono::high_resolution_clock> startT;
    std::chrono::time_point<std::chrono::high_resolution_clock> endT;
};
vector<request> incomingReq, outgoingReq;
pthread_mutex_t outputMtx, mtx, shareMtx;
int n;
int m;
struct worker
{
    int workerId;
    int priority;
    int resources;
    int isBusy;
};
vector<vector<int>> workerBusy;
struct passInfo
{
    int a;
    int b;
};
void *serveReq(void *arg)
{
    passInfo pi = *((passInfo *)arg);
    int serviceId = pi.a;
    int wid = pi.b;
    // a request is served
    chrono::seconds delay(1);
    this_thread::sleep_for(delay);
    workerBusy[serviceId][wid] = 0;
}
void *serviceThread(void *arg)
{
    int serviceId = *((int *)arg);
    // now we read the m wokrers for this-- priority, resources
    vector<worker> workers;
    for (int i = 0; i < m; i++)
    {
        worker w;
        cin >> w.priority >> w.resources;
        w.workerId = i;

        workers.push_back(w);
    }
    pthread_mutex_unlock(&mtx); // other threads allowed to read stdin

    int noWorkers = m;
    // now we put the appropriate requests in this que
    queue<request> q;
    for (auto x : incomingReq)
    {
        if (x.serviceNo == serviceId)
            q.push(x);
    }
    // now we have the que ready

    // we service req 1 by 1
    while (!q.empty())
    {
        request r = q.front();
        int id = -1;
        int priority = -1; // higher no more prioirty
        int wid = 0;
        for (auto x : workers)
        {

            if (workerBusy[serviceId][wid])
                continue;
            if (x.resources >= r.resourceNeeded && x.priority > priority)
            {
                id = x.workerId;
                priority = x.priority;
            }
            wid++;
        }
        if (id == -1) // req cant be served
        {

            int cnt = 0;
            int cnt2 = 0;
            for (auto x : workers)
            {
                if (workerBusy[serviceId][wid])
                    cnt2++;
                if (x.resources < r.resourceNeeded)
                    cnt++;
            }
            if (cnt == m) // can never be served
            {
                q.pop();

                cout << "this req with reqid : " << r.reqId << " cant be served\n";
                cntG++;
                r.endT = std::chrono::high_resolution_clock::now();
                outgoingReq.push_back(r);
            }
            else if (cnt2 == m) // can serve in future
            {
                q.pop();
                q.push(r);
                cout << "this req with reqid : " << r.reqId << " blocked due to absence of workers\n";
            }
            else
            {
                q.pop();
                q.push(r);
                cout << "this req with reqid : " << r.reqId << " waits due to absence of resources\n";
            }
        }
        else // req is served
        {
            workerBusy[serviceId][wid] = 1;
            pthread_t workerThread;
            passInfo pi;
            pi.a = serviceId;
            pi.b = wid;
            pthread_create(&workerThread, NULL, serveReq, (void *)&pi);
            q.pop();
            r.endT = std::chrono::high_resolution_clock::now();
            outgoingReq.push_back(r);
        }
    }

    return NULL;
}

int main()
{
    //// reading requests
    ifstream inputFile("requests.txt");
    if (inputFile.is_open())
    {
        string line;
        int id = 0;
        while (getline(inputFile, line))
        {
            int num1, num2;
            stringstream ss(line);
            request s1;
            s1.reqId = id++;
            ss >> s1.serviceNo >> s1.resourceNeeded;
            s1.startT = std::chrono::high_resolution_clock::now();
            incomingReq.push_back(s1);
        }

        inputFile.close();
    }
    else
        cout << "error opening file\n";
    /////
    pthread_mutex_init(&outputMtx, NULL);
    FILE *file1 = freopen("input.txt", "r", stdin);
    FILE *file2 = freopen("output.txt", "w", stdout);
    /////
    cin >> n;
    cin >> m;
    for (int i = 0; i < n; i++)
    {
        workerBusy.push_back(vector<int>(m, 0));
    }
    pthread_t serviceThreads[n];
    for (int i = 0; i < n; i++)
    {
        pthread_mutex_lock(&mtx);
        int id = i;
        pthread_create(&serviceThreads[i], NULL, serviceThread, (void *)&id);
    }
    for (int i = 0; i < n; i++)
    {
        pthread_join(serviceThreads[i], NULL);
    }
    ////
    cout << "order of req being processed is as follows\n";
    for (auto x : outgoingReq)
    {
        cout << x.reqId << " ";
    }
    cout << "\nturn around time for the requests are as follows along with waiting times:\n";
    for (auto x : outgoingReq)
    {
        cout << "ID : " << x.reqId << " ";
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(x.endT - x.startT);
        cout << "TT " << elapsed.count() << " milliseconds ";
        int cnt = elapsed.count();
        cout << "WT " << max(0, cnt - 1000) << " milliseconds ";
        ;
        cout << endl;
    }

    cout << "\nNo of req rejected due to lack of resources " << cntG << " \n";
    /////
    freopen("/dev/tty", "r", stdin);
    freopen("/dev/tty", "w", stdout);
}