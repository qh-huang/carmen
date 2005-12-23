//
// This Program is provided by Duke University and the authors as a service to the
// research community. It is provided without cost or restrictions, except for the
// User's acknowledgement that the Program is provided on an "As Is" basis and User
// understands that Duke University and the authors make no express or implied
// warranty of any kind.  Duke University and the authors specifically disclaim any
// implied warranty or merchantability or fitness for a particular purpose, and make
// no representations or warranties that the Program will not infringe the
// intellectual property rights of others. The User agrees to indemnify and hold
// harmless Duke University and the authors from and against any and all liability
// arising out of User's use of the Program.
//
// slam.cpp
// Copyright 2005 Austin Eliazar, Ronald Parr, Duke University
//
// Main slam loop
//

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <math.h>
#include <strings.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "learn.h"
#include "mt-rand.h"

// The initial seed used for the random number generated can be set here.
#define SEED 1

// Default names for printing map files.
// File types are automatically appended.
#define MAP_PATH_NAME "map"
#define PARTICLES_PATH_NAME "particles"


//
// Globals
//

// The current commands being given to the robot for movement. 
// Not used when the robot is reading data from a log file.
double RotationSpeed, TranslationSpeed;

// The means by which the slam thread can be told to halt, either by user command or by the end of a playback file.
int continueSlam;
int PLAYBACK_COMPLETE = 0;


//
//
// InitializeRobot
//
// Calls the routines in 'ThisRobot.c' to initialize the necessary hardware and software.
// Each call has an opportunity to return a value of -1, which indicates that the initialization of that
// part of the robot has failed. In that event, Initialize robot itself will return a general error of -1.
//
//
int InitializeRobot(int argc, char *argv[]) {
  if (InitializeThisRobot(argc, argv) == -1) {
    fprintf(stderr, "Start up initialization of the robot has failed.\n");
    return -1;
  }

  fprintf(stderr, "Connect Odometry.\n");
  if (ConnectOdometry(argc, argv) == -1) {
    fprintf(stderr, "Unable to connect to the robot's odometry.\n");
    return -1;
  }

  fprintf(stderr, "Connect Laser.\n");
  if (ConnectLaser(argc, argv) == -1) {
    fprintf(stderr, "Unable to connect to the robot's laser.\n");
    return -1;
  }

  fprintf(stderr, "Connect Drive.\n");
  if (ConnectDrive(argc, argv) == -1) {
    fprintf(stderr, "Unable to connect to the robot's drive motors.\n");
    return -1;
  }

  return 0;
}


//
// WriteLog
//
// Prints to file the data that we would normally be getting from sensors, such as the laser and the odometry.
// This allows us to later play back the exact run, with different parameters.
// All readings are in meters or radians.
//
void WriteLog(FILE *logFile, TSense sense) 
{ 
  int i;

  fprintf(logFile, "Odometry %.6f %.6f %.6f \n", odometry.x, odometry.y, odometry.theta);
  fprintf(logFile, "Laser %d ", SENSE_NUMBER);
  for (i = 0; i < SENSE_NUMBER; i++)
    fprintf(logFile, "%.6f ", sense[i].distance/MAP_SCALE);
  fprintf(logFile, "\n");
}
 


//
// This calls the procedures in the other files which do all the real work. 
// If you wanted to not use hierarchical SLAM, you could remove all references here to High*, and make
// certain to set LOW_DURATION in low.h to some incredibly high number.
//
void *Slam(void *a __attribute__ ((unused)))
{
  TPath *path, *trashPath;
  TSenseLog *obs, *trashObs;
  double oldmC_D = 0;
  double oldmC_T = 0;
  double oldvC_D = 0;
  double oldvC_T = 0;
  double oldmD_D = 0;
  double oldmD_T = 0;
  double oldvD_D = 0;
  double oldvD_T = 0;
  double oldmT_D = 0;
  double oldmT_T = 0;
  double oldvT_D = 0;
  double oldvT_T = 0;

  outFile = fopen("motionModel.txt", "w");
  fprintf(outFile, "This file lists the motion model parameters in the following order : \n");
  fprintf(outFile, "    - mean value from observed translation\n");
  fprintf(outFile, "    - mean value from observed rotation\n");
  fprintf(outFile, "    - variance contributed from observed translation\n");
  fprintf(outFile, "    - variance contributed from observed rotation\n");
  fprintf(outFile, "\n");
  fprintf(outFile, "------------Intermediate Models------------------\n");
  fprintf(outFile, "   m_dist m_turn   v_dist v_turn\n");
  fprintf(outFile, "C: %.4f %.4f   %.4f %.4f\n", meanC_D, meanC_T, varC_D, varC_T);
  fprintf(outFile, "D: %.4f %.4f   %.4f %.4f\n", meanD_D, meanD_T, varD_D, varD_T);
  fprintf(outFile, "T: %.4f %.4f   %.4f %.4f\n", meanT_D, meanT_T, varT_D, varT_T);
  fprintf(outFile, "\n");
  fclose(outFile);

  while ((fabs(oldmC_D-meanC_D) > 0.01)||(fabs(oldmC_T-meanC_T) > 0.03)||(fabs(oldvC_D-varC_D) > 0.01)||(fabs(oldvC_T-varC_T) > 0.1) || 
	 (fabs(oldmD_D-meanD_D) > 0.01)||(fabs(oldmD_T-meanD_T) > 0.03)||(fabs(oldvD_D-varD_D) > 0.01)||(fabs(oldvD_T-varD_T) > 0.1) || 
	 (fabs(oldmT_D-meanT_D) > 0.03)||(fabs(oldmT_T-meanT_T) > 0.01)||(fabs(oldvT_D-varT_D) > 0.03)||(fabs(oldvT_T-varT_T) > 0.01)) {

    readFile = carmen_fopen(PLAYBACK, "r");
    if(readFile == NULL)
      carmen_die("Error: could not open file %s for reading.\n", PLAYBACK);
    logfile_index = carmen_logfile_index_messages(readFile);

    InitLowSlam();
    LowSlam(continueSlam, &path, &obs);
    carmen_fclose(readFile);

    oldmC_D = meanC_D;    
    oldmC_T = meanC_T;
    oldvC_D = varC_D;    
    oldvC_T = varC_T;
    oldmD_D = meanD_D;    
    oldmD_T = meanD_T;
    oldvD_D = varD_D;    
    oldvD_T = varD_T;
    oldmT_D = meanT_D;    
    oldmT_T = meanT_T;
    oldvT_D = varT_D;    
    oldvT_T = varT_T;

    outFile = fopen("motionModel.txt", "a");
    Learn(path);
    fclose(outFile);

    // Get rid of the path and log of observations
    while (path != NULL) {
      trashPath = path;
      path = path->next;
      free(trashPath);
    }
    while (obs != NULL) {
      trashObs = obs;
      obs = obs->next;
      free(trashObs);
    }
  }


  outFile = fopen("motionModel.txt", "a");
  fprintf(outFile, "\n");
  fprintf(outFile, "------------Final Model------------------\n");
  fprintf(outFile, "   m_dist m_turn   v_dist v_turn\n");
  fprintf(outFile, "C: %.4f %.4f   %.4f %.4f\n", meanC_D, meanC_T, varC_D, varC_T);
  fprintf(outFile, "D: %.4f %.4f   %.4f %.4f\n", meanD_D, meanD_T, varD_D, varD_T);
  fprintf(outFile, "T: %.4f %.4f   %.4f %.4f\n", meanT_D, meanT_T, varT_D, varT_T);

  fprintf(outFile, "\n");
  fprintf(outFile, "#define meanC_D %.4f\n#define meanC_T %.4f\n#define varC_D %.4f\n#define varC_T %.4f\n\n", meanC_D, meanC_T, varC_D, varC_T);
  fprintf(outFile, "#define meanD_D %.4f\n#define meanD_T %.4f\n#define varD_D %.4f\n#define varD_T %.4f\n\n", meanD_D, meanD_T, varD_D, varD_T);
  fprintf(outFile, "#define meanT_D %.4f\n#define meanT_T %.4f\n#define varT_D %.4f\n#define varT_T %.4f\n\n", meanT_D, meanT_T, varT_D, varT_T);
  fclose(outFile);

  CloseLowSlam();
  return NULL;
}



//
// Start of main program.
// IF things are set to read from a robot's sensors and not a data log, then this would be the best place
// to actually put in controls for the robot's behaviors and actions. The main SLAM process is called as a
// seperate thread off of this function.
//
int main (int argc, char *argv[])
{
  //char command[256], tempString[20];
  int x;
  //int y;
  //double maxDist, tempDist, tempAngle;
  int WANDER, EXPLORE, DIRECT_COMMAND;
  pthread_t slam_thread;
    
  RECORDING = "";
  PLAYBACK = "";
  for (x = 1; x < argc; x++) {
    if (!strncmp(argv[x], "-R", 2))
      RECORDING = "current.log";
    if (!strncmp(argv[x], "-r", 2)) {
      x++;
      RECORDING = argv[x];
    }
    else if (!strncmp(argv[x], "-p", 2)) {
      x++;
      PLAYBACK = argv[x];
    }
    else if (!strncmp(argv[x], "-P", 2))
      PLAYBACK = "current.log";
  }

  fprintf(stderr, "********** Localization Example *************\n");
  if (PLAYBACK == "")
    if (InitializeRobot(argc, argv) == -1)
      return -1;

  fprintf(stderr, "********** World Initialization ***********\n");

  seedMT(SEED);
  // Spawn off a seperate thread to do SLAM
  //
  // Should use semaphores or similar to prevent reading of the map
  // during updates to the map.
  //
  continueSlam = 1;
  pthread_create(&slam_thread, (pthread_attr_t *) NULL, Slam, &x);

  fprintf(stderr, "*********** Main Loop (Movement) **********\n");


  // This is the spot where code should be inserted to control the robot. You can go ahead and assume
  // that the robot is localizing and mapping.
  WANDER = 0;
  EXPLORE = 0;
  DIRECT_COMMAND = 0;
  RotationSpeed = 0.0;
  TranslationSpeed = 0.0;

  // Some very crude commands designed to give manual control over our ATRV Jr
  // Removed now for convenience and efficiency, since we're running from data logs right now.
  /*
  while (1) {
    // Was there a character pressed?
    //    gets(command);
    scanf("%s", command);
    if (command != NULL) {
      if ((PLAYBACK_COMPLETE) || (strncmp(command, "quit", 4) == 0)) {
	if (PLAYBACK == "") {
	  //stop the robot
	  TranslationSpeed = 0.0;
	  RotationSpeed = 0.0;
	  Drive(TranslationSpeed, RotationSpeed);
	}

	// kill the other thread
	continueSlam = 0;
	pthread_join(slam_thread, NULL);

        return 0;
      }

      else if (strncmp(command, "speed", 5) == 0) {
        strncpy(tempString, index(command, ' '), 10);
        TranslationSpeed = atof(tempString);
	if (TranslationSpeed > 0.5)
	  TranslationSpeed = 0.5;
	RotationSpeed = 0;
	DIRECT_COMMAND = 1;
      }

      else if (strncmp(command, "turn", 4) == 0) {
        strncpy(tempString, index(command, ' '), 10);
        RotationSpeed = atof(tempString);
	if (RotationSpeed > 0.6)
	  RotationSpeed = 0.6;
	TranslationSpeed = 0;
	DIRECT_COMMAND = 1;
      }

      else if (strncmp(command, "stop", 4) == 0) {
	TranslationSpeed = 0.0;
	RotationSpeed = 0.0;
	Drive(TranslationSpeed, RotationSpeed);
	DIRECT_COMMAND = 0;
      }

      else if (strncmp(command, "print", 5) == 0) {
	y = 0;
	for (x = 0; x < cur_particles_used; x++)
	  if (particle[x].probability > particle[y].probability)
	    y = x;
	PrintMap(MAP_PATH_NAME, particle[y].parent, FALSE, -1, -1, -1);
      }

      else if (strncmp(command, "particles", 9) == 0) {
	y = 0;
	for (x = 0; x < cur_particles_used; x++)
	  if (particle[x].probability > particle[y].probability)
	    y = x;
	PrintMap(PARTICLES_PATH_NAME, particle[y].parent, TRUE, -1, -1, -1);
      }

      else if (strncmp(command, "overlay", 7) == 0) {
	y = 0;
	for (x = 0; x < cur_particles_used; x++)
	  if (particle[x].probability > particle[y].probability)
	    y = x;
	PrintMap(MAP_PATH_NAME, particle[y].parent, FALSE, particle[y].x, particle[y].y, particle[y].theta);
      }

      else if (strncmp(command, "centerx ", 8) == 0) {
        strncpy(tempString, index(command, ' '), 10);
        scat_center_x = atof(tempString);
      }

      else if (strncmp(command, "centery ", 8) == 0) {
        strncpy(tempString, index(command, ' '), 10);
        scat_center_y = atof(tempString);
      }

      else if (strncmp(command, "radius ", 7) == 0) {
        strncpy(tempString, index(command, ' '), 10);
        scat_radius = atof(tempString);
      }

      //else {
      //fprintf(stderr, "I don't understand you.\n");
      //}
    }

    if ((DIRECT_COMMAND == 1) && (PLAYBACK == "")) {
      Drive(TranslationSpeed, RotationSpeed);
    }
  
    else if (PLAYBACK == "") {
      // stop the robot
      TranslationSpeed = 0.0;
      RotationSpeed = 0.0;
      Drive(TranslationSpeed, RotationSpeed);
    }
    
  }
  */

  pthread_join(slam_thread, NULL);
  return 0;
}

