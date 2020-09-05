#include "login/user.hpp"
#include <catch2/catch.hpp>
#include <debug/debug.hpp>

TEST_CASE("Creating Usertable and inserting values","[UserTable]") {
  std::error_code ec;
  std::filesystem::remove("users.db",ec);

  {
    UserTable tbl;
    REQUIRE(tbl.Load("users.db") == UserTable::ReturnCodes::OK);
    //Let the destructor run to write changes on disk!
    //
    
    REQUIRE( tbl.AddUser("alex","pw","Alexander Leonhardt",UserAccess::ADMIN) == UserTable::ReturnCodes::OK);

    REQUIRE( tbl.AddUser("alex","pwas","Alexander Leonhardtasd",UserAccess::READ) == UserTable::ReturnCodes::USER_EXISTS);


    REQUIRE( tbl.RemoveUser("alex") == UserTable::ReturnCodes::OK);
    REQUIRE( tbl.AddUser("alex","pwas","Alexander Leonhardtasd",UserAccess::READ) == UserTable::ReturnCodes::OK);

    REQUIRE( tbl.GetNumUsers() == 2 );
  }

  std::filesystem::remove("users.db",ec);
  REQUIRE(ec.value() == 0); 
}


TEST_CASE("Threading user table test","[UserTable]") {
  debug::gLogLevel = debug::LogLevel::DBG_ALWAYS;
  std::error_code ec;
  std::filesystem::remove("users.db",ec);

  {
    UserTable tbl;
    REQUIRE(tbl.Load("users.db") == UserTable::ReturnCodes::OK);
    //Let the destructor run to write changes on disk!
    
    std::thread t1([&tbl](){
        for(int i = 0; i < 500; i++)
          tbl.AddUser(std::to_string(i), "alex", "leo", UserAccess::ADMIN);
        });
    
    std::thread t2([&tbl](){
        for(int i = 2000; i < 2500; i++)
          tbl.AddUser(std::to_string(i), "alex", "leo", UserAccess::ADMIN);
        });

    std::thread t3([&tbl](){
        for(int i = 4000; i < 4500; i++)
          tbl.AddUser(std::to_string(i), "alex", "leo", UserAccess::ADMIN);
        });

    t1.join();
    t2.join();
    t3.join();

    REQUIRE( tbl.GetNumUsers() == 1501 );
    
  }

  std::filesystem::remove("users.db",ec);
  REQUIRE(ec.value() == 0); 
}

TEST_CASE("Check root user","[UserTable]") {
  debug::gLogLevel = debug::LogLevel::DBG_ALWAYS;
  std::error_code ec;
  std::filesystem::remove("users.db",ec);

  {
    UserTable tbl;
    REQUIRE(tbl.Load("users.db") == UserTable::ReturnCodes::OK);
    //Let the destructor run to write changes on disk!
    
    REQUIRE( tbl.GetNumUsers() == 1);
    std::string cookie = tbl.LogIn("root", "password");
    REQUIRE(cookie != "");

    //Must be at least 32 bytes long otherwise its a very weak cookie
    REQUIRE(cookie.length() >= 32);

    auto usr = tbl.GetUserFromCookie(cookie);
    REQUIRE(usr != nullptr);
    REQUIRE(usr->Email() == "root");
    REQUIRE(usr->Access() == UserAccess::ADMIN); 
  }

  std::filesystem::remove("users.db",ec);
  REQUIRE(ec.value() == 0); 
}
