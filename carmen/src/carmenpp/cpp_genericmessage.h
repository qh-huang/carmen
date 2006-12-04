#ifndef CARMEN_CPP_GENERIC_MESSAGE_H
#define CARMEN_CPP_GENERIC_MESSAGE_H

#include "cpp_carmen.h"

class GenericMessage {
 public: 
  GenericMessage() {}
  virtual ~GenericMessage() {}
  GenericMessage(const GenericMessage& x) {
    *this = x;
  }
 
  virtual double getTimestamp() const= 0;
  virtual const char* getMessageID() const= 0;
  virtual void save(carmen_FILE *logfile, double logger_timestamp) = 0;
  virtual char*  fromString(char* s) = 0;
  virtual GenericMessage* clone() const = 0;
};

#endif
