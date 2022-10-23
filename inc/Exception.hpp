#ifndef EXCEPTION_HPP_
#define EXCEPTION_HPP_

#include <exception>
#include <string>

class MyException: public std::exception {
  std::string message;

  public:
  MyException(std::string message): message(message){}

  std::string cause() {return message;}
};

#endif