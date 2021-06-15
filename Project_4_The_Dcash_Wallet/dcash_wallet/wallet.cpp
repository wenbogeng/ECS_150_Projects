#define RAPIDJSON_HAS_STDSTRING 1
// @ author Wenbo Geng
#include <iostream>
#include<iomanip>
#include <fstream>
#include <sstream>
#include <string>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "WwwFormEncodedDict.h"
#include "HttpClient.h"
#include "StringUtils.h"

#include "rapidjson/document.h"

using namespace std;
using namespace rapidjson;

int API_SERVER_PORT = 8080;
string API_SERVER_HOST = "localhost";
string PUBLISHABLE_KEY = "pk_test_51IzVkzIjgYXsVYBBnQoFICf8shdFpqYvM8H8BWt5avl8VrdZQLp7sBwAbhyN376QG94Y8gV0MMpILETuZgoj81rx009oUuK8vf";

string auth_token;
string user_id;

// update user's information and email information
void updateUser(string user_id, string email, int* balance, int* flag) {
    HttpClient *client = new HttpClient(API_SERVER_HOST.c_str(), API_SERVER_PORT);
    string request_conf = "/users/" + user_id;
    WwwFormEncodedDict body;
    body.set("email", email);
    string encoded_body = body.encode();
    client->set_header("x-auth-token", auth_token);
    HTTPClientResponse *client_response = client->put(request_conf, encoded_body);
    Document *d2 = client_response->jsonBody();
    if (!d2->IsObject()) {
        (*flag) = 1;
        delete client;
        return;
    }
    (*flag) = 0;
    (*balance) = (*d2)["balance"].GetInt();
    delete client;
}

// get user ID to the same user information
void getUser(string user_id, string* email, int* balance, int* flag) {
    HttpClient *client = new HttpClient(API_SERVER_HOST.c_str(), API_SERVER_PORT);
    string request_conf = "/users/" + user_id;
    client->set_header("x-auth-token", auth_token);
    HTTPClientResponse *client_response = client->get(request_conf);
    Document *d2 = client_response->jsonBody();
    if (!d2->IsObject()) {
        (*flag) = 1;
        delete client;
        return;
    }
    (*flag) = 0;
    (*email) = (*d2)["email"].GetString();
    (*balance) = (*d2)["balance"].GetInt();
    delete client;
}

// get update in authorizted token
void authTokens(string user_name, string password,
                string* auth_token2, string* user_id2,
                int* flag) {
    HttpClient *client = new HttpClient(API_SERVER_HOST.c_str(), API_SERVER_PORT);
    string request_conf = "/auth-tokens";
    WwwFormEncodedDict body;
    body.set("username", user_name);
    body.set("password", password);
    string encoded_body = body.encode();
    HTTPClientResponse *client_response = client->post(request_conf,encoded_body);
    Document *d2 = client_response->jsonBody();
    if (!d2->IsObject()) {
        (*flag) = 1;
        delete client;
        return;
    }
    (*flag) = 0;
    (*auth_token2) = (*d2)["auth_token"].GetString();
    (*user_id2) = (*d2)["user_id"].GetString();
    delete client;
}

// deleted the user's token
void authTokensDelete(string auth_token2) {
    HttpClient *client = new HttpClient(API_SERVER_HOST.c_str(), API_SERVER_PORT);
    string request_conf = "/auth-tokens/" + auth_token2;
    client->set_header("x-auth-token", auth_token);
    client->del(request_conf);
    delete client;
}


// to do the transfer
void transfer(string to, int amount, int* balance, int* flag) {
    HttpClient *client = new HttpClient(API_SERVER_HOST.c_str(), API_SERVER_PORT);
    client->set_header("Authorization", string("Bearer ") + PUBLISHABLE_KEY);
    string request_conf = "http://";
    request_conf = request_conf + API_SERVER_HOST +  ":" + to_string(API_SERVER_PORT) + "/auth-tokens/transfers";
    WwwFormEncodedDict body;
    body.set("to", to);
    body.set("amount", amount);
    string encoded_body = body.encode();
    client->set_header("x-auth-token", auth_token);
    HTTPClientResponse *client_response = client->post(request_conf, encoded_body);
    Document *d2 = client_response->jsonBody();
    if (!d2->IsObject()) {
        (*flag) = 1;
        delete client;
        return;
    }
    (*flag) = 0;
    (*balance) = (*d2)["balance"].GetInt();
    delete client;
}

// to do the deposit
void deposit(int amount, string stripe_token, int* balance, int* flag) {
    HttpClient *client = new HttpClient(API_SERVER_HOST.c_str(), API_SERVER_PORT);
    client->set_header("Authorization", string("Bearer ") + PUBLISHABLE_KEY);

    string request_conf = "http://";
    request_conf = request_conf + API_SERVER_HOST + ":" + to_string(API_SERVER_PORT) + "/auth-tokens/deposits";
    WwwFormEncodedDict body;
    body.set("amount", amount);
    body.set("stripe_token", stripe_token);
    string encoded_body = body.encode();
    client->set_header("x-auth-token", auth_token);
    HTTPClientResponse *client_response = client->post(request_conf, encoded_body);
    Document *d2 = client_response->jsonBody();
    if (!d2->IsObject()) {
        (*flag) = 1;
        delete client;
        return;
    }
    (*flag) = 0;
    (*balance) = (*d2)["balance"].GetInt();
    delete client;
}

int main(int argc, char *argv[]) {
    stringstream config;
    int fd = open("config.json", O_RDONLY);
    if (fd < 0) {
        cout << "could not open config.json" << endl;
        exit(1);
    }
    int ret;
    char buffer[4096];
    while ((ret = read(fd, buffer, sizeof(buffer))) > 0) {
        config << string(buffer, ret);
    }
    Document d;
    d.Parse(config.str());
    API_SERVER_PORT = d["api_server_port"].GetInt();
    API_SERVER_HOST = d["api_server_host"].GetString();
    PUBLISHABLE_KEY = d["stripe_publishable_key"].GetString();

    ifstream fin(argv[1]);
    string input;
    // check the user input
    while (getline(fin, input)) {
        // cout << "D$> ";
        // if user input is empty then send the error message
        if (input.empty()) {
            cout << "Error" << endl;
            continue;
        }
        //store the user input
        vector<string> vect_User = StringUtils::split(input, ' ');
        if (vect_User.size() <= 0) {
            cout << "Error" << endl;
            continue;
        // take the auth command
        } else if (vect_User[0] == "auth") {
            // have to be within four command edge case
            if (vect_User.size() != 4) {
                cout << "Error" << endl;
                continue;
            } else {
                // check if invald auth tokens
                int flag = 0;
                authTokens(vect_User[1], vect_User[2], &auth_token, &user_id, &flag);
                int balance = 0;
                if (flag == 0) {
                    // update the user information
                    updateUser(user_id, vect_User[3], &balance, &flag);
                    if (flag == 0) {
                        double new_balance = balance / 100.0;
                        cout<<setiosflags(ios::fixed)<<setprecision(2);
                        cout<<"Balance: $"<<new_balance << endl;
                    } else {
                        cout <<"Error" << endl;
                    }
                } else {
                    cout <<"Error" <<endl;
                }
            }
            // check the balance
        } else if (vect_User[0] == "balance") {
            // it's have to be one input
            if (vect_User.size() != 1) {
                cout << "Error" << endl;
                continue;
            } else {
                // get the user informations
                string email;
                int balance = 0;
                int flag = 0;
                getUser(user_id, &email, &balance, &flag);
                if (flag == 0) {
                    // change it to usd
                    double new_balance = balance / 100.0;
                    cout<<setiosflags(ios::fixed)<<setprecision(2);
                    cout<<"Balance: $"<<new_balance << endl;
                } else {
                    cout<<"Error" << endl;
                }
            }
        } else if (vect_User[0] == "deposit") {
            // it have to be 6 input size
            if (vect_User.size() != 6) {
                cout << "Error" << endl;
                continue;
            } else {
                // record the card informations
                double new_amount = atof(vect_User[1].c_str());
                string card = vect_User[2];
                string year = vect_User[3];
                string month = vect_User[4];
                string cvc = vect_User[5];
                int balance = 0;
                int flag = 0;
                // deposit
                deposit(int(100 * new_amount), card, &balance, &flag);
                if (flag == 0) {
                    double new_balance = balance / 100.0;
                    cout<<setiosflags(ios::fixed)<<setprecision(2);
                    cout<<"Balance: $"<<new_balance << endl;
                } else {
                    cout<<"Error" << endl;
                }
            }
            // send the masesage
        } else if (vect_User[0] == "send") {
            if (vect_User.size() != 3) {
                cout << "Error" << endl;
                continue;
            } else {
                string username = vect_User[1];
                int amount = int(100 * atof(vect_User[2].c_str()));
                int balance = 0;
                int flag = 0;
                transfer(username, amount, &balance, &flag);
                if (flag == 0) {
                    double new_balance = balance / 100.0;
                    cout<<setiosflags(ios::fixed)<<setprecision(2);
                    cout<<"Balance: $"<<new_balance << endl;
                } else {
                    cout << "Error" << endl;
                }
            }
            // logout command
        } else if (vect_User[0] == "logout") {
            if (vect_User.size() != 1) {
                cout << "Error" << endl;
                continue;
            } else {
                // delete user permisions
                authTokensDelete(auth_token);
                break;
            }
        }
    }
    return 0;
}
