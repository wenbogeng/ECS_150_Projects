#define RAPIDJSON_HAS_STDSTRING 1
// @author Wenbo Geng
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#include <iostream>
#include <map>
#include <string>
#include <sstream>

#include "TransferService.h"
#include "ClientError.h"

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;
using namespace std;

TransferService::TransferService() : HttpService("/transfers") { }


void TransferService::post(HTTPRequest *request, HTTPResponse *response) {
    //Authenticated User information
    User* user = getAuthenticatedUser(request);
    // if user empty
    if (user == NULL) {
        response->setStatus(403);
        return;
    }
    
    // take user input
    WwwFormEncodedDict dict = request->formEncodedBody();
    int amount = atoi(dict.get("amount").c_str());
    // if it's negative amount
    if (amount < 0) {
        response->setStatus(401);
        return;
    }
    // user doesn't have enought balance 
    if (user->balance < amount) {
        response->setStatus(401);
        return;
    }
    int is_inDB = 0;
    string to_User = dict.get("to");
    map<string, User*>::iterator iter;// = this->m_db->users.find("em");
    // check the user if in the data base
    for (iter = this->m_db->users.begin(); iter != this->m_db->users.end(); ++iter) {
        // if in the data base then break
        if (iter->second->username == to_User) {
            is_inDB = 1;
            break;
        }
    }
    // if not in the data base then end the error message
    if (is_inDB == 0) {
        response->setStatus(400);
        return;
    }
    // check if it's transfer to myself then send the error message
    if (user->user_id == iter->second->user_id) {
        response->setStatus(400);
        return;
    }
    // transfer vaild
    if (user->balance - amount >= 0 && iter != this->m_db->users.end()) {
        // update the balance
        user->balance = user->balance-amount;
        // add balance to the other user
        iter->second->balance = iter->second->balance + amount;
        // record the transfer data
        Transfer* trans = new Transfer();
        trans->from = user;
        trans->to = iter->second;
        trans->amount = amount;
        this->m_db->transfers.push_back(trans);
    }
    // from github example return the information in Json
    Document doc;
    Document::AllocatorType& data = doc.GetAllocator();
    Value obj;
    obj.SetObject();
    obj.AddMember("balance", user->balance, data);
    Value arr;
    arr.SetArray();
    for (unsigned long int i = 0; i < this->m_db->transfers.size(); ++i) {
        if (this->m_db->transfers[i]->from->username == user->username) {
            Value to;
            to.SetObject();
            to.AddMember("from", this->m_db->transfers[i]->from->username, data);
            to.AddMember("to", this->m_db->transfers[i]->to->username, data);
            to.AddMember("amount", this->m_db->transfers[i]->amount, data);
            arr.PushBack(to, data);
        }
    }
    // update the information to response 
    obj.AddMember("transfers", arr, data);
    doc.Swap(obj);
    StringBuffer buf;
    PrettyWriter<StringBuffer> writer(buf);
    doc.Accept(writer);
    response->setContentType("application/json");
    response->setBody(buf.GetString() + string("\n"));
    response->setStatus(200);
}
