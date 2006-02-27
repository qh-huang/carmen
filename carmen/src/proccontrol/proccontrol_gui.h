#include <qframe.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcursor.h>
#include <qwidget.h>
#include <qpopupmenu.h>
#include <qmultilinedit.h>
#include <qtextview.h>
#include <qfont.h>

#define MAX_NUM_GROUPS   20
#define MAX_NUM_MODULES  40
#define NUM_STATES       4
#define MAX_NAME_LENGTH  256

typedef struct {
  char   group_name[MAX_NAME_LENGTH];
  int    group;
  char   module_name[MAX_NAME_LENGTH];
  int    module;
  int    active;
  int    requested_state;
  int    pid;
  int    output;
} process_type;

typedef struct {
  int           numprocesses;
  int           numgrps;
  int           procingrp[MAX_NUM_GROUPS];
  process_type  process[MAX_NUM_GROUPS][MAX_NUM_MODULES];
  int           output;
} process_table_type;


void  carmen_output( int state );


class QDisplay : public QWidget {
  Q_OBJECT
  
public:
  QDisplay( QWidget *parent = 0, const char *name = 0 );
  void showStatus( int group, int button, int status );
  void setGroup( int group, char * group_name );
  void setModule( int group, int module, char * module_name, int pid );
  void hideButton( int group, int module );
  void showLine( char * line );

private:
  QPushButton  *but[MAX_NUM_GROUPS][MAX_NUM_MODULES];
  QHBoxLayout  *box[MAX_NUM_GROUPS];
  QButtonGroup *bgrp[MAX_NUM_GROUPS];
  QTextView *output;

private slots:
  void startClicked( int );
  void stopClicked( int );
  void showClicked( int );
  void noClicked( int );


protected:
  void closeEvent( QCloseEvent *ev );
  
protected slots:
};


