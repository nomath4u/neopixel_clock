#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <time.h>                       // time() ctime()
#include <sys/time.h>                   // struct timeval
#include <coredecls.h>                  // settimeofday_cb()
#include "ssid.h" //Needs to be created locally


#define DEBUGGING 0
#define DEBUG_MODE_LED 29

#define LIGHT_PIN D8
#define MODE_PIN 4
#define STRIP_LEN 30
#define HOUR_SHIFT 0
#define MIN_SHIFT 6
#define SEC_SHIFT MIN_SHIFT + 7
#define ADJUST_PIN 10

#define TZ  -8 //PST
#define DST_MN  60 //Summer time DST
#define T_MN  ((TZ)*60)
#define TZ_SEC ((TZ)*3600)
#define DST_SEC ((DST_MN)*60) 

#define MSEC_IN_SEC 1000
#define MSEC_IN_MIN 60000
#define MSEC_IN_HOUR 3600000

#define MAX_BRIGHTNESS 64 //Can go up to 255 but I'm not sure if the arduino could handle it.
#define WHEEL_MAX 255 //All the way around the Wheel
/**********************************
* Function Signatures
**********************************/

int get_h(long*);
int get_m(long*);
int get_se(long*);
void modify_by_mode();
void create_neopixel_chain(int, int, int);
void modify_utime();
void modify_brightness();
void modify_color();
void show_error();
uint8_t Red(uint32_t color);
uint8_t Green(uint32_t color);
uint8_t Blue(uint32_t color);
uint32_t DimBackgroundColor(uint32_t color);

/**********************************
* Strip Setup
**********************************/
Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIP_LEN, LIGHT_PIN, NEO_GRB + NEO_KHZ400);

/**********************************
* Wheel Setup
**********************************/
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos){
    WheelPos = 255 - WheelPos;
    if(WheelPos < 85){
        return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    else if(WheelPos < 170){
        WheelPos -= 85;
        return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    else{
        WheelPos -= 170;
        return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    }
}

/**********************************
* NeoPixel Setup
**********************************/



//uint32_t magenta = strip.Color(0, 100, 150); //Active color These should both be temporary until we have wheel
uint32_t teal = Wheel( 150 );
uint32_t orange = Wheel( 15 ); //Background color for off places
uint32_t green = Wheel(85); //Seperator

unsigned short utime_wheel_pos = 150;
unsigned short background_wheel_pos = 15;
unsigned short seperator_wheel_pos = 85;

uint32_t utime_color = teal;
uint32_t background_color = orange;
uint32_t seperator_color = green;

uint32_t red = strip.Color(255,0,0);
uint32_t yellow = strip.Color(255, 100, 0);
uint32_t blue = strip.Color(0, 0, 255);
uint32_t off = strip.Color(0, 0, 0);

timeval tv;
timespec tp;
time_t now;
timeval cbtime;      // time set in callback

unsigned short bright;

/**********************************
* Setup time callback
**********************************/
void time_is_set(void) {
  gettimeofday(&cbtime, NULL);
  Serial.println("------------------ settimeofday() was called ------------------");
}

/**********************************
* Testing functions
**********************************/
#define PTM(w) \
  Serial.print(":" #w "="); \
  Serial.print(tm->tm_##w);

void printTm(const char* what, const tm* tm) {
  Serial.print(what);
  PTM(isdst); PTM(yday); PTM(wday);
  PTM(year);  PTM(mon);  PTM(mday);
  PTM(hour);  PTM(min);  PTM(sec);
  Serial.println("");
}

/**********************************
* State Machine
**********************************/
enum { PRESS, UNPRESS };
enum { CLOCK, UPDATE_HOUR, UPDATE_MIN, UPDATE_SEC, UPDATE_BRIGHTNESS, UPDATE_COLOR, STATE_END };
int state_color[STATE_END] = { off, green, yellow, blue, orange, teal };
int mode = CLOCK;
int mode_button_state = UNPRESS;
unsigned long adjust = 0; 

void setup(){
  /********************
  * Setup Pins
  ********************/
  pinMode(LIGHT_PIN, OUTPUT);

  
  /********************
  * Setup Neopixels
  ********************/
  bright = MAX_BRIGHTNESS;
  strip.begin();
  strip.show(); //Initialize everything off
  strip.setBrightness(bright); //Current limiting
  Serial.begin(115200);

  /********************
  * Setup Wifi and Time
  ********************/
  settimeofday_cb(time_is_set);
  configTime(TZ_SEC, DST_SEC, "pool.ntp.org");
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, SSIDPWD);
  
}

void loop(){  
  int sec;
  int m;
  int h;
  struct tm * timeinfo;

  /********************
  * Get time from internet
  ********************/
  gettimeofday(&tv, NULL);
  now =time(NULL);
  timeinfo = localtime(&now);
  h = timeinfo->tm_hour;
  m = timeinfo->tm_min;
  sec = timeinfo->tm_sec;
  //Serial.println(ctime(&now));
  
  /********************
  * Populate strip
  ********************/
  strip.clear();
  create_neopixel_chain(h,m,sec);
  if(WiFi.status() != WL_CONNECTED){ show_error(); }
  strip.show();
  delay(100); //Can save power here but probably need to wake up more than once per second.
  
}

void create_neopixel_chain(int h, int m, int sec){
  /* 5 lights for h 6 ms and 6 seconds
  Meaning we need 17 lights. We also need 2 seperators which means 19. Just use the first 0-18 then*/
  add_seperators();
  add_h(h);
  add_m(m);
  add_second(sec);
}

void add_seperators(){
  #define NUM_HOUR 5
  #define NUM_MINUTE (NUM_HOUR + 6 +1)
  #define NUM_SEC ( NUM_MINUTE + 6 +1 )
  strip.setPixelColor(NUM_HOUR, seperator_color);
  strip.setPixelColor(NUM_MINUTE, seperator_color);
  strip.setPixelColor(NUM_SEC, seperator_color);
}

void add_h(int h){
  for(int i = 4; i >=0; i--){
    if( h >= (1 << i) ){
      h = h - ( 1<< i );
      strip.setPixelColor((i + HOUR_SHIFT), utime_color);
    } else {
      strip.setPixelColor(( i + HOUR_SHIFT), DimBackgroundColor(background_color));
    }
  }
  
}

void add_m(int m){
  for(int i = 5; i >=0; i--){
    if( m >= (1 << i) ){
      m = m - ( 1<< i );
      strip.setPixelColor((i + MIN_SHIFT), utime_color);
    } else {
      strip.setPixelColor(( i + MIN_SHIFT), DimBackgroundColor(background_color));
    }
  }
}

void add_second(int sec){
  for(int i = 5; i >=0; i--){
    if( sec >= (1 << i) ){
      sec = sec - ( 1<< i );
      strip.setPixelColor((i + SEC_SHIFT), utime_color);
    } else {
      strip.setPixelColor(( i + SEC_SHIFT), DimBackgroundColor(background_color));
    }
  }
}


/*Assumes starting colors are nicely spaced on the color wheel*/
void modify_color(){
  if(!digitalRead(ADJUST_PIN)){
    utime_wheel_pos++;
    background_wheel_pos++;
    seperator_wheel_pos++;
    if(utime_wheel_pos > WHEEL_MAX){
      utime_wheel_pos = 0;
    }
    if(background_wheel_pos > WHEEL_MAX){
      background_wheel_pos = 0;
    }
    if( seperator_wheel_pos > WHEEL_MAX){
      seperator_wheel_pos = 0;
    }
    utime_color = Wheel(utime_wheel_pos);
    background_color = Wheel(background_wheel_pos);
    seperator_color = Wheel(seperator_wheel_pos);
  } 
}

void show_error(){
  #define ERR_LED 20
  strip.setPixelColor( ERR_LED, red);
}

/*Consider using DimColor function. Haven't looked for it in code but adafruit mentions it*/
void modify_brightness(){
  if(!digitalRead(ADJUST_PIN)){
    bright--;
    if (bright > MAX_BRIGHTNESS ) { //Checking for underflow on unsigned value
      bright = MAX_BRIGHTNESS;
    }
    strip.setBrightness(bright);
  }
}

    // Returns the Red component of a 32-bit color
    uint8_t Red(uint32_t color)
    {
        return (color >> 16) & 0xFF;
    }

    // Returns the Green component of a 32-bit color
    uint8_t Green(uint32_t color)
    {
        return (color >> 8) & 0xFF;
    }

    // Returns the Blue component of a 32-bit color
    uint8_t Blue(uint32_t color)
    {
              return color & 0xFF;
    }

    uint32_t DimBackgroundColor(uint32_t color)
    {
        uint32_t dimColor = strip.Color(Red(color) >> 3, Green(color) >> 3, Blue(color) >> 3);
        return dimColor;
    }
