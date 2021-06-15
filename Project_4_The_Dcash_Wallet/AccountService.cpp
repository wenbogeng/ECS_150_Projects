#define RAPIDJSON_HAS_STDSTRING 1
//@ author Wenbo Geng
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "AccountService.h"
#include "ClientError.h"

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/stringbuffer.h"

using namespace std;
using namespace rapidjson;

AccountService::AccountService() : HttpService("/users") {
  
}
/*
 Account Service take two functions get and put
 get: find the user_id and for the same user update the email and return
 put: find the user_id and if it's the same user then output the result
 **/
void AccountService::put(HTTPRequest *request, HTTPResponse *response) {
  // check if user's authentiaceted
  User* user = getAuthenticatedUser(request);
  // if user is empty return error message
  if (user == NULL) {
	response->setStatus(403);
    return;
  }
  // get the user input email from the github example
  WwwFormEncodedDict dict = request->formEncodedBody();
  string email = dict.get("email");
  // get user id update the email and user id
  vector<string> path_User_Id = request->getPathComponents();
  string user_id = path_User_Id[path_User_Id.size()-1];
  if (user_id != user->user_id) {
    response->setStatus(400);
	return;
  }
  // using map to tore the pointer
  map<string, User*>::iterator iter = this->m_db->users.find(user_id);
  // find the user id and update the email then return
  if (iter != this->m_db->users.end() && !email.empty()) {
    iter->second->email = email;
    // from github example
	Document doc;
	Document::AllocatorType& data = doc.GetAllocator();
	Value obj;
	obj.SetObject();
	obj.AddMember("balance", iter->second->balance, data);
	obj.AddMember("email", email, data);
	doc.Swap(obj);
    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
	doc.Accept(writer);
    // return response to json
	response->setContentType("application/json");
	response->setBody(buffer.GetString() + string("\n"));
	response->setStatus(200);
  } else response->setStatus(404);
}

void AccountService::get(HTTPRequest *request, HTTPResponse *response) {
  //check the user input
  User* user = getAuthenticatedUser(request);
  if (user == NULL) {
	response->setStatus(403);
    return;
  }
  // gget user id and take user input id
  vector<string> path_User_Id = request->getPathComponents();
  string user_id = path_User_Id[path_User_Id.size()-1];
  map<string, User*>::iterator iter = this->m_db->users.find(user_id);
  // if it's not match then return eror message
  if (user->user_id != iter->second->user_id) {
    response->setStatus(400);
	return;
  }
 // find the user id a
  if (iter != this->m_db->users.end()) {
	Document doc;
	Document::AllocatorType& data = doc.GetAllocator();
    // from github example
	Value obj;
	obj.SetObject();
	obj.AddMember("balance", iter->second->balance, data);
	obj.AddMember("email", iter->second->email, data);
	doc.Swap(obj);
    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
	doc.Accept(writer);
	// return reposnse to json
	response->setContentType("application/json");
	response->setBody(buffer.GetString() + string("\n"));
    response->setStatus(200);
  } else response->setStatus(404);

    // if not found the user return error message
}
