
#define MAX_AGENDA_LENGTH 50
#define MAX_AGENDA_TITLE 55
#define MAX_AGENDA_DATE 25
#define MAX_AGENDA_TIME 20
//Send Keys
#define GET_EVENT_DAYS 1
#define GET_SETTINGS 2
#define GET_EVENT_DETAILS 3
#define APP_VERSION_MAJOR 4
#define APP_VERSION_MINOR 5

//Recieve Keys
#define MONTHYEAR_KEY 1
#define EVENT_DAYS_DATA_KEY 3
#define SETTINGS_KEY 4
#define EVENT_DETAILS_KEY 5
#define EVENT_DETAILS_LINE1_KEY 6
#define EVENT_DETAILS_LINE2_KEY 7
#define EVENT_DETAILS_DATEL_KEY 8
#define EVENT_DETAILS_LENGTH_KEY 2
#define EVENT_DAYS_NEXT_DATA_KEY 9
#define EVENT_DAYS_LAST_DATA_KEY 10

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
#define AGENDA_TITLE_KEY 10
#define HIDEAGENDA_KEY 11
#define NOTINVERTAGENDA_KEY 12
#define SHOWLABELS_KEY 13
#define SHOWDATE_KEY 14
#define AGENDALENGTH_KEY 99
#define AGENDADATE_KEY 100
#define AGENDATITLE_KEY 200
#define AGENDATIME_KEY 300


#define PEB_Y 168
#define PEB_X 144
#define LABEL_H 15

#define TIME_H 42
#define TIME_OFFSET_WO 7
#define TIME_OFFSET_W 3

#define DATE_H 22
#define DATE_H_WO 24
#define DATE_OFF 7

#define AGENDA_ROWS 1
#define AGENDA_PAD_L 7

#define AGENDA_DATE_H 6
#define AGENDA_DATE_OFF 2

#define AGENDA_BOTTOM_PAD 3


extern bool black;
extern bool grid;
extern bool invert;
extern bool showtime;
extern bool hidelastprev;
extern bool boldevents;
extern bool showdate;
extern bool showlabels;
extern bool hideagenda;
extern bool notinvertagenda;

extern bool watchmode;

extern int start_of_week;
extern int showweekno;
extern int weekstoshow;
extern int weekstoshow;
extern int agenda_title_rows;

extern char weekno_form[4][3];
extern char daysOfWeek[7][3];
extern char months[12][12];

extern int offset;



extern int curHour;
extern int curMin;
extern int curSec;

extern bool calEvents[32];
extern bool nextEvents[32];
extern bool lastEvents[32];

extern int agendaLength;
extern char agendaTitle[MAX_AGENDA_LENGTH][MAX_AGENDA_TITLE];
extern char agendaDate[MAX_AGENDA_LENGTH][MAX_AGENDA_DATE];
extern char agendaTime[MAX_AGENDA_LENGTH][MAX_AGENDA_TIME];

void get_top_event();
void log_int(int i);
void readSettings();
void get_settings();
void getmode();
void processSettings(uint8_t encoded[2]);
