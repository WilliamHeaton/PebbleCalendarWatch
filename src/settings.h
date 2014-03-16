
//Send Keys
#define GET_EVENT_DAYS 1
#define GET_SETTINGS 2
//Recieve Keys
#define MONTHYEAR_KEY 1
#define EVENT_DAYS_DATA_KEY 3
#define SETTINGS_KEY 4
#define EVENT_DETAILS_KEY 5
#define EVENT_DETAILS_LINE1_KEY 6
#define EVENT_DETAILS_LINE2_KEY 7
#define EVENT_DETAILS_CLEAR_KEY 2


//Storage Keys
#define BLACK_KEY 1
#define GRID_KEY 2
#define INVERT_KEY 3
#define SHOWTIME_KEY 4
#define START_OF_WEEK_KEY 5
#define SHOWWEEKNO_KEY 6
#define HIDELASTPREV_KEY 7
#define BOLDEVENTS_KEY 8
#define WEEKSTOSHOW_KEY 9


extern bool black;
extern bool grid;
extern bool invert;
extern bool showtime;
extern bool hidelastprev;
extern bool boldevents;

// First day of the week. Values can be between -6 and 6 
// 0 = weeks start on Sunday
// 1 =  weeks start on Monday
extern int start_of_week;
extern int showweekno;
extern int weekstoshow;

extern char weekno_form[4][3];
extern char daysOfWeek[7][3];
extern char months[12][12];

extern int offset;



extern int curHour;
extern int curMin;
extern int curSec;

extern bool calEvents[32];

void get_settings();

void processSettings(uint8_t encoded[2]);



