//
//  ECS 150 gunrock.cpp
//  proj3
//
//  Created by Wenbo Geng on 5/1/21.
//
// Following the code for this project is using the example on professor's lecture slides psesutocode consumer and producer

#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <sstream>
#include <deque>

#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "HttpService.h"
#include "HttpUtils.h"
#include "FileService.h"
#include "MySocket.h"
#include "MyServerSocket.h"
#include "dthread.h"

using namespace std;

int PORT = 8080;
int THREAD_POOL_SIZE = 1;
int BUFFER_SIZE = 1;
int running; // check if the thread is in the running state
int length = 0; // length of the queue
string BASEDIR = "static";
string SCHEDALG = "FIFO";
string LOGFILE = "/dev/null";

vector<HttpService *> services;
vector<pthread_t> thread_ID; // store the thread IDs

// Thread locks
// check if the conditions are true or false
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_lock(&mutex);
//pthread_cond_wait(cond, mutex);
//pthread_mutex_unlock(&mutex);

// producer and customr condition variables
pthread_cond_t c_cond= PTHREAD_COND_INITIALIZER;
pthread_cond_t p_cond = PTHREAD_COND_INITIALIZER;

HttpService *find_service(HTTPRequest *request) {
   // find a service that is registered for this path prefix
  for (unsigned int idx = 0; idx < services.size(); idx++) {
    if (request->getPath().find(services[idx]->pathPrefix()) == 0) {
      return services[idx];
    }
  }

  return NULL;
}

void invoke_service_method(HttpService *service, HTTPRequest *request, HTTPResponse *response) {
  stringstream payload;
  
  // invoke the service if we found one
  if (service == NULL || request->getPath().find("..") != string::npos) {
    // not found status
    response->setStatus(404);
  } else if (request->isHead()) {
    payload << "HEAD " << request->getPath();
    sync_print("invoke_service_method", payload.str());
    cout << payload.str() << endl;
    service->head(request, response);
  } else if (request->isGet()) {
    payload << "GET " << request->getPath();
    sync_print("invoke_service_method", payload.str());
    cout << payload.str() << endl;
    service->get(request, response);
  } else {
    // not implemented status
    response->setStatus(405);
  }
}

void handle_request(MySocket *client) {
  HTTPRequest *request = new HTTPRequest(client, PORT);
  HTTPResponse *response = new HTTPResponse();
  stringstream payload;
  // read in the request
  bool readResult = false;
  try {
    payload << "client: " << (void *) client;
    sync_print("read_request_enter", payload.str());
    readResult = request->readRequest();
    sync_print("read_request_return", payload.str());
  } catch (...) {
    // swallow it
  }
    
  if (!readResult) {
    // there was a problem reading in the request, bail
    delete response;
    delete request;
    sync_print("read_request_error", payload.str());
    return;
  }
    
  HttpService *service = find_service(request);
  invoke_service_method(service, request, response);

  // send data back to the client and clean up
  payload.str(""); payload.clear();
  payload << " RESPONSE " << response->getStatus() << " client: " << (void *) client;
  sync_print("write_response", payload.str());
  cout << payload.str() << endl;
  client->write(response->response());
    
  delete response;
  delete request;

  payload.str(""); payload.clear();
  payload << " client: " << (void *) client;
  sync_print("close_connection", payload.str());
  client->close();
  delete client;
}

//Threat queue API
struct thread_args {
    MySocket *client;
    struct thread_args *next;
};

// set front and back for the queue
struct thread_args *front = NULL;

struct thread_args *back = NULL;

// get the top element
int peek(thread_args &arg){
    // if it's empty
    if(length == 0) return -1;
    else{ // get the top elements
        arg = *front;
        // change the pointers
        if(length == 1) front = NULL;
        else if(length ==2){
            front = front->next;
            back = NULL;
        }else front = front->next;
        --length;
        return 0;
    }
}

// push back to the queue
void push_back(thread_args *arg){
    length++;
    // if it's empty
    if (front == NULL) front = arg;
    else if (front != NULL && back == NULL){
        back = arg;
        front->next = arg;
    } else{
        // change the pointers
        back->next = arg;
        back = arg;
    }
}

void *handle_thread(void *arg){
    struct thread_args *thread_args = new struct thread_args;
    while(1){
        dthread_mutex_lock(&mutex);
        while(length == 0){
            --running;
            dthread_cond_wait(&c_cond, &mutex);
            ++running;
        }
        peek(*thread_args);
        dthread_mutex_unlock(&mutex);
        handle_request(thread_args->client);
        dthread_cond_signal(&p_cond);
    }
}

// Create the threads
void create_thread(){
    for (int i = 0; i < THREAD_POOL_SIZE; i++){
        pthread_t tid = dthread_create(&tid, NULL, handle_thread, NULL); //creating the thread
        pthread_join(tid, NULL); //waiting for thread tid to end
        thread_ID.push_back(tid);
    }
}

int main(int argc, char *argv[]) {

  signal(SIGPIPE, SIG_IGN);
  int option;

  while ((option = getopt(argc, argv, "d:p:t:b:s:l:")) != -1) {
    switch (option) {
    case 'd':
      BASEDIR = string(optarg);
      break;
    case 'p':
      PORT = atoi(optarg);
      break;
    case 't':
      THREAD_POOL_SIZE = atoi(optarg);
      break;
    case 'b':
      BUFFER_SIZE = atoi(optarg);
      break;
    case 's':
      SCHEDALG = string(optarg);
      break;
    case 'l':
      LOGFILE = string(optarg);
      break;
    default:
      cerr<< "usage: " << argv[0] << " [-p port] [-t threads] [-b buffers]" << endl;
      exit(1);
    }
  }

  set_log_file(LOGFILE);

  sync_print("init", "");
  MyServerSocket *server = new MyServerSocket(PORT);
  MySocket *client;

  services.push_back(new FileService(BASEDIR));

  // here is the main thread

  //init all conditional variables and lock
  pthread_cond_init(&p_cond, NULL);
  pthread_cond_init(&c_cond, NULL);
  pthread_mutex_init(&mutex, NULL);
  running = THREAD_POOL_SIZE;

  // creat all thread asked
  create_thread();
  
    while(true) { // always run
        sync_print("waiting_to_accept", "");
        client = server->accept();
        sync_print("client_accepted", "");
        dthread_mutex_lock(&mutex);
        
        // if the buffer is full then unlock the mutex and waiting for p_cond to change
        while(length == BUFFER_SIZE) dthread_cond_wait(&p_cond, &mutex);
        // Initialize the queue
        struct thread_args *thread_args = new struct thread_args;
        thread_args->client = client;
        push_back(thread_args);
        // release the mutex lock
        dthread_mutex_unlock(&mutex);
        // change the condition and send the signal and let the thread know
        if (running < THREAD_POOL_SIZE) dthread_cond_signal(&c_cond);
    }
    
    // waiting for the conditions and destory the locks
    // Failed in Test 4: Concurrent requests (0.0/12.5)
//    pthread_mutex_destroy(&mutex);
//    pthread_cond_destroy(&c_cond);
//    pthread_cond_destroy(&p_cond);
}
