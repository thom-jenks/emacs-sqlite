#ifndef SQLITE_HPP
#define SQLITE_HPP

#include <iostream>
#include <vector>
#include <sqlite3.h>
#include <string>


typedef std::vector<std::string> row_t;

class SQLException: public std::exception{
private:
  std::string _msg;
public:
  SQLException(): _msg("anon"){};
  SQLException(std::string msg): _msg(msg){}
  virtual const char* what() const throw(){
    return _msg.c_str();
  }
};


class SQLiteConnection{
private:
  static sqlite3* _connection;
public:
  SQLiteConnection(const std::string& file);
  SQLiteConnection();
  ~SQLiteConnection();

  static sqlite3* get_instance(){ return _connection; };
  bool is_connected(){ return _connection != nullptr; }

  void open(const std::string& fp);
  void close();
};

class SQLiteQuery{
private:
  sqlite3* _connection;
  std::vector<row_t> query_data;
  
public:
  SQLiteQuery();
  SQLiteQuery(std::string query);
  ~SQLiteQuery();

  std::vector<row_t> data(){ return query_data; }
};

#endif /* SQLITE_HPP */
