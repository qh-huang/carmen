typedef struct {
  double    y;
  double    u;
  double    v;
} YUV;

typedef struct {
  double    r;
  double    g;
  double    b;
} RGB;

typedef struct {
  double    h;
  double    s;
  double    v;
} HSV;


YUV           convert_from_rgb( RGB color );

RGB           convert_from_yuv( YUV color );

RGB           hsv_to_rgb( HSV color );

RGB           val_to_rgb( double val );

RGB           val_to_gray( double val );



typedef void (*logtools_draw_function_t)(int x,int y);

void          logtools_draw_set_function( logtools_draw_function_t funct );

void          logtools_draw_ellipse( logtools_ivector2_t p, const int a,const int b);

void          logtools_draw_line( logtools_ivector2_t p1, logtools_ivector2_t p2 );
     
void          logtools_draw_circle( logtools_ivector2_t p, const int r );

void          logtools_draw_filled_circle( logtools_ivector2_t p, const int r );

