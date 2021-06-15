#define RAPIDJSON_HAS_STDSTRING 1
// @author Wenbo Geng
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "AuthService.h"
#include "StringUtils.h"
#include "ClientError.h"

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/stringbuffer.h"

using namespace std;
using namespace rapidjson;

AuthService::AuthService() : HttpService("/auth-tokens") {
    
}

void AuthService::post(HTTPRequest *request, HTTPResponse *response) {
  // take user inputs
  WwwFormEncodedDict dict = request->formEncodedBody();
  string user_Name = dict.get("username");
  // check the user input if it's invaild
  for (unsigned long int i = 0; i< user_Name.size(); ++i) {
    if (user_Name[i] >= 'a' && user_Name[i] <= 'z') continue;
	else {
	   response->setStatus(400);
	   return;
	}
  }
  // store the password
  string password = dict.get("password");
  // check if the user is in the data base if user is in the data base check if the password is correct
  if (!user_Name.empty() && !password.empty()) {
    map<string, User*>::iterator iter = this->m_db->users.find(user_Name);
	for (iter = this->m_db->users.begin(); iter != this->m_db->users.end(); ++iter) {
	  if (iter->second->username == user_Name) break;
	}
    // check the password if it's correct
	if (iter != this->m_db->users.end()) {
        User* user = iter->second;
        if (password == user->password) {
            string auth_token = StringUtils::createAuthToken();
            this->m_db->auth_tokens.insert(make_pair(auth_token, user));
            string user_id = user->user_id;
            // take the input to the json format from github example
            Document doc;
            Document::AllocatorType& data = doc.GetAllocator();
            Value obj;
            obj.SetObject();
            obj.AddMember("auth_token", auth_token, data);
            obj.AddMember("user_id", user_id, data);
            doc.Swap(obj);
            
            StringBuffer buf;
            PrettyWriter<StringBuffer> writer(buf);
            
            doc.Accept(writer);
            // return json messages
            response->setContentType("application/json");
            response->setBody(buf.GetString() + string("\n"));
            response->setStatus(200);

        } else response->setStatus(403);
        
       // if user is not in the data base
	} else {
        // create a user
        User* user = new User();
		user->username = user_Name;
		user->password = password;
		user->user_id = StringUtils::createUserId();
		user->balance = 0;
        // return the user informations
		string auth_token = StringUtils::createAuthToken();
		this->m_db->auth_tokens.insert(make_pair(auth_token, user));
		this->m_db->users.insert(make_pair(user->user_id, user));
        
        // take the input to the json format from github example
		Document doc;
		Document::AllocatorType& data = doc.GetAllocator();
		Value obj;
		obj.SetObject();
		obj.AddMember("auth_token", auth_token, data);
		obj.AddMember("user_id", user->user_id, data);
		doc.Swap(obj);
        StringBuffer buf;
		PrettyWriter<StringBuffer> writer(buf);
		doc.Accept(writer);
		
		response->setContentType("application/json");
		response->setBody(buf.GetString() + string("\n"));
		response->setStatus(201);
    }
  }
}

void AuthService::del(HTTPRequest *request, HTTPResponse *response) {
  User* user = getAuthenticatedUser(request);
  // delete the user token and check the user information
  if (user != NULL) {
    vector<string> path_User_ID = request->getPathComponents();
    string auth = path_User_ID[path_User_ID.size()-1];
    map<string, User*>::iterator iter = this->m_db->auth_tokens.find(auth);
    if (iter != this->m_db->auth_tokens.end()) this->m_db->auth_tokens.erase(iter);
    response->setStatus(200);
  } else response->setStatus(403);
}
