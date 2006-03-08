/* File : pyCarmen.i */

%module(directors="1") pyCarmen
%{
#include "pyCarmenMessages.h"
%}

%include "std_string.i"

/* turn on director wrapping Callback */
%feature("director") MessageHandler;
#define __attribute__(x)
%include "carmen.h"
%include "global.h"

%include "pyCarmen.h"
%include "pyCarmenMessages.h"



%include arm_messages.h
%include base_messages.h
%include camera_messages.h
%include gps_nmea_messages.h
%include laser_messages.h
%include localize_messages.h
%include logger_messages.h
%include playback_messages.h
%include map_messages.h
%include navigator_messages.h
%include param_messages.h
%include robot_messages.h
%include simulator_messages.h
//%include hokuyo_messages.h
//%include pantilt_messages.h
//%include proccontrol_messages.h
//segway_messages.h


%include arm_interface.h
%include base_interface.h
%include camera_interface.h
%include gps_nmea_interface.h
%include laser_interface.h
%include localize_interface.h
%include playback_interface.h
%include map_interface.h
%include navigator_interface.h
%include param_interface.h
%include robot_interface.h
%include simulator_interface.h

//%include planner_interface.h
//%include hokuyo_interface.h
//%include pantilt_interface.h
//%include proccontrol_interface.h
//%include segway_interface.h



// This tells SWIG to treat char ** as a special case
%typemap(in) char ** {
  /* Check if is a list */
  if (PyList_Check($input)) {
    int size = PyList_Size($input);
    int i = 0;
    $1 = (char **) malloc((size+1)*sizeof(char *));
    for (i = 0; i < size; i++) {
      PyObject *o = PyList_GetItem($input,i);
      if (PyString_Check(o))
	$1[i] = PyString_AsString(PyList_GetItem($input,i));
      else {
	PyErr_SetString(PyExc_TypeError,"list must contain strings");
	free($1);
	return NULL;
      }
    }
    $1[i] = 0;
  } else {
    PyErr_SetString(PyExc_TypeError,"not a list");
    return NULL;
  }
}

// This cleans up the char ** array we malloc'd before the function call
%typemap(freearg) char ** {
  free((char *) $1);
}



%include "ipc_wrapper.h"
