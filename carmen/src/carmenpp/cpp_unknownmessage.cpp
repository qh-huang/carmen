#include "cpp_unknownmessage.h"

UnknownMessage::UnknownMessage() {
  s=NULL;
}

UnknownMessage::~UnknownMessage() {
  if (this->s!=NULL)
    delete [] this->s;
}


UnknownMessage::UnknownMessage(const UnknownMessage& x) : AbstractMessage(x) {
  if (this->s!=NULL)
    delete [] this->s;
  this->s = new char[strlen(x.s)+1];
  strcpy(this->s, x.s);
}
  
UnknownMessage::UnknownMessage(char* s) {
  fromString(s);
//   if (this->s!=NULL)
//     delete [] this->s;
//   this->s = new char[strlen(s)+1];
//   strcpy(this->s, s);
}

double UnknownMessage::getTimestamp() const { 
  return 0; 
}

void UnknownMessage::save(carmen_FILE *logfile, double ) {
  carmen_fprintf(logfile, s);
}

char*  UnknownMessage::fromString(char* s) {
  if (this->s!=NULL)
    delete [] this->s;
  this->s = new char[strlen(s)+1];
  strcpy(this->s, s);
  return (char*) &(s[strlen(s)-1]);
}


AbstractMessage* UnknownMessage::clone() const {
  return new UnknownMessage(*this);
}

char*  UnknownMessage::getString() {
  return s;
}
