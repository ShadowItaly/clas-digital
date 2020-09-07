#include "server.hpp"
#include <mutex>
#include "util/URLParser.hpp"
#include "debug/debug.hpp"


CLASServer &CLASServer::GetInstance()
{
  //Returns the only valid instance of the server
  static CLASServer server;
  return server;
}

void CLASServer::HandleLogin(const httplib::Request &req, httplib::Response &resp)
{
  //First parse the url encoded body of the post request
  URLParser parser(req.body);

  // Try to do a login with the obtained data
  auto cookie = users_.LogIn(parser["email"], parser["password"]);

  //No cookie created deny access
  if(cookie == "") {
    resp.status = 403;
  }
  else {
    //Create the Set Cookie id to make the browser set the cookie on receiving
    //this.
    std::string set_cookie = "SESSID=" + cookie;
    set_cookie += "; SECURE";
    resp.status = 200;
    resp.set_header("Set-Cookie", std::move(set_cookie));
  }
}

void CLASServer::SendUserList(const httplib::Request &req, httplib::Response &resp)
{
  //Send back the user list if the user is an admin user, first check if there
  //is an user associated with the cookie and if it is. Check if the user is an
  //admin user
  const User *usr = GetUserFromCookie(req.get_header_value("Cookie"));
  if(!usr || usr->Access() != UserAccess::ADMIN)
    resp.status = 403;
  else
    resp.set_content(users_.GetAsJSON().dump(),"application/json");
}

void CLASServer::UpdateUserList(const httplib::Request &req, httplib::Response &resp)
{
  //Update the user list based on the json commands received
  const User *usr = GetUserFromCookie(req.get_header_value("Cookie"));

  // Check if the user has the required access
  if(!usr || usr->Access() != UserAccess::ADMIN)
    resp.status = 403;
  else
  {
    try
    {
      auto js = nlohmann::json::parse(req.body);
      //Iterate through the given commands and execute them based on the action
      //they provided
      for(auto &it : js)
      {
        if(it["action"]=="DELETE")
	      {
		      //Remove the user if the action is delete if one of the necessary variables does not exist then throw an error and return an error not found
          users_.RemoveUser(it["email"]);
	      }
	      else if(it["action"]=="CREATE")
	      {
		      //Create the user with the specified email password and access
          users_.AddUser(it["email"], it["password"], "Unknown", it["access"]);
	      }
      }
    }
    catch(...)
    {
      //Something went wrong tell the callee about this
      resp.status = 400;
    }
  }
}


const User *CLASServer::GetUserFromCookie(const std::string &cookie_ptr)
{
  //Try to extract the SESSID from the cookie field
  //If the cookie field is empty return 0 as there is no way an user is logged
  //in without a cookie
  if(cookie_ptr=="") return nullptr;

  //Find the SESSID in the cookie string. A cookie string looks like: 
  //Cookie: BREAD=123jasdklasd; SESSID=Whatever
  //First try to find the start of the cookie used by our authentification
  //system then determine the length of the cookie.
  auto pos = cookie_ptr.find("SESSID=");
  auto pos2 = cookie_ptr.find(";",pos);
  std::string cookie = "";

  // Extract the cookie from the string based on start and end of the cookie
  if(pos2==std::string::npos)
	  cookie = cookie_ptr.substr(pos+7);
  else
	  cookie = cookie_ptr.substr(pos+7,pos2-(pos+7));

  return users_.GetUserFromCookie(cookie);
}


debug::Error<CLASServer::ReturnCodes> CLASServer::Start(std::string listenAddress)
{
  if(!initialised_)
    return debug::Error(ReturnCodes::ERR_SERVER_NOT_INITIALISED,"The server was not initialised yet");

  //Register all the URI Handler.
  server_.Post("/api/v2/server/login",[this](const httplib::Request &req, httplib::Response &resp){this->HandleLogin(req, resp);});

  server_.Get("/api/v2/server/userlist",[this](const httplib::Request &req, httplib::Response &resp){this->SendUserList(req,resp);});

  server_.Post("/api/v2/server/userlist",[this](const httplib::Request &req, httplib::Response &resp){this->UpdateUserList(req,resp);});

  for(auto &it : cfg_.mount_points_)
  {
    server_.set_mount_point("/", it.string().c_str());
  }

  // Check how many times we tried to bind the port
  int port_binding_tries = 0;
  
  //Try to bind the port 3 times always waiting 50 milliseconds in between. If
  //this fails return ERR_PORT_BINDING
  while(!server_.listen(listenAddress.c_str(),cfg_.server_port_))
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    if(port_binding_tries++ > 3)
      return debug::Error(ReturnCodes::ERR_PORT_BINDING);
  }
  return debug::Error(ReturnCodes::OK);
}


debug::Error<CLASServer::ReturnCodes> CLASServer::InitialiseFromFile(std::filesystem::path config_file, std::filesystem::path user_db_file)
{
  std::string config;
  std::ifstream ifs(config_file.string(), std::ios::in);
  if(ifs.is_open())
    return Error(ReturnCodes::ERR_CONFIG_FILE_INITIALISE,"Could not load config file at " + config_file.string());
  ifs>>config;
  ifs.close();

  return InitialiseFromString(config,user_db_file);
}

debug::Error<CLASServer::ReturnCodes> CLASServer::InitialiseFromString(std::string config_file, std::filesystem::path user_db_file)
{
  {
    auto err = cfg_.LoadFromString(config_file);
    if(err)
    {
      return Error(ReturnCodes::ERR_CONFIG_FILE_INITIALISE,"Could not load config file.").Next(err);
    }
  }

  // Try to load the user table, if this fails notify the user and stop
  // initialising
  auto err = users_.Load(user_db_file);
  if(err.GetErrorCode() != UserTable::ReturnCodes::OK)
  {
    debug::log(debug::LOG_ERROR,"Could not create user table in RAM!\n");
    return debug::Error(ReturnCodes::ERR_USERTABLE_INITIALISE,"Could not initialise user table subsytem").Next(err);
  }

  initialised_ = true;

  return Error(ReturnCodes::OK);
}



void CLASServer::Stop()
{
  // Stop the server and tell the status bit about the changed status
  server_.stop();
}


bool CLASServer::IsRunning()
{
  return server_.is_running();
}

CLASServer::CLASServer()
{
  initialised_ = false;
}
