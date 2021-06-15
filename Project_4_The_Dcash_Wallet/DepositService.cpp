#define RAPIDJSON_HAS_STDSTRING 1
//@ author Wenbo Geng
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#include <iostream>
#include <map>
#include <string>
#include <sstream>

#include "DepositService.h"
#include "Database.h"
#include "ClientError.h"
#include "HTTPClientResponse.h"
#include "HttpClient.h"

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;
using namespace std;

DepositService::DepositService() : HttpService("/deposits") { }

void DepositService::post(HTTPRequest *request, HTTPResponse *response) {
    // take the user request from authenticated user
    User* user = getAuthenticatedUser(request);
    if (user == NULL) {
        response->setStatus(403);
        return;
    }
    // Get the user input
    WwwFormEncodedDict dict = request->formEncodedBody();
    int amount = atoi(dict.get("amount").c_str());
    string stripe_token = dict.get("stripe_token");
    // check if it's empty
    if (stripe_token.empty()) {
        response->setStatus(401);
        return;
    }
    // check if it's negative amount
    if (amount < 0) {
        response->setStatus(401);
        return;
    }
    // If the amount is greater than 0 then the deposit is successful and record it else return a error response
//    if (amount >= 0) {
//      user->balance = user->balance + amount;
//      Deposit* dep = new Deposit();
//      dep->to = user;
//      dep->amount = amount;
//      dep->stripe_charge_id = stripe_token;
//      this->m_db->deposits.push_back(dep);
//    } else {
//      response->setStatus(400);
//      return;
//    }
    // conect to stripe using the github example
    HttpClient client("api.stripe.com", 443, true);
    client.set_basic_auth(m_db->stripe_secret_key, "");
    WwwFormEncodedDict body;
    body.set("amount", amount);
    body.set("source", stripe_token);
    body.set("currency","usd");
    string encoded_body = body.encode();
    HTTPClientResponse *client_Response = client.post("/v1/charges",encoded_body);

    // returned into JSON format in Gunrock
    Document *doc = client_Response->jsonBody();
    string value = (*doc)["id"].GetString();
    
    user->balance = user->balance + amount;
    Deposit* dep = new Deposit();
    dep->to = user;
    dep->amount = amount;
    dep->stripe_charge_id = value;
    this->m_db->deposits.push_back(dep);

    // Also returned into JSON format
    Document doc_Gun;
    Document::AllocatorType& data = doc_Gun.GetAllocator();
    Value obj;
    obj.SetObject();
    obj.AddMember("balance", user->balance, data);
    Value arr;
    arr.SetArray();
    for (unsigned long int i = 0; i < this->m_db->deposits.size(); ++i) {
        if (this->m_db->deposits[i]->to->username == user->username) {
            Value to;
            to.SetObject();
            to.AddMember("to", this->m_db->deposits[i]->to->username, data);
            to.AddMember("amount", this->m_db->deposits[i]->amount, data);
            to.AddMember("stripe_charge_id", value, data);
            arr.PushBack(to, data);
        }
    }
    //update the json information to reponse
    obj.AddMember("deposits", arr, data);
    doc_Gun.Swap(obj);
    StringBuffer buf;
    PrettyWriter<StringBuffer> writer(buf);
    doc_Gun.Accept(writer);
    response->setContentType("application/json");
    response->setBody(buf.GetString() + string("\n"));
    response->setStatus(200);
}
