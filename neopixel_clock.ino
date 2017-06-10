#include <Adafruit_NeoPixel.h>

#define DEBUGGING 1
#define DEBUG_MODE_LED 29

#define LIGHT_PIN 3
#define MODE_PIN 4
#define STRIP_LEN 30
#define HOUR_SHIFT 0
#define MIN_SHIFT 6
#define SEC_SHIFT MIN_SHIFT + 7
#define ADJUST_PIN 10

#define MSEC_IN_SEC 1000
#define MSEC_IN_MIN 60000
#define MSEC_IN_HOUR 3600000

#define MAX_BRIGHTNESS 64 //Can go up to 255 but I'm not sure if the arduino could handle it.
/**********************************
* Function Signatures
**********************************/

int get_h(long*);
int get_m(long*);
int get_se(long*);
void modify_by_mode();
void create_neopixel_chain(int, int, int);
void modify_time();
void modify_brightness();
void modify_color();


/**********************************
* NeoPixel Setup
**********************************/

Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIP_LEN, LIGHT_PIN, NEO_GRB + NEO_KHZ800);

uint32_t magenta = strip.Color(0, 100, 150); //Active color These should both be temporary until we have wheel
uint32_t light_magenta = strip.Color( 20, 4, 0 ); //Background color for off places
uint32_t green = strip.Color(0, 255, 0); //Seperator

uint32_t yellow = strip.Color(255, 100, 0);
uint32_t blue = strip.Color(0, 0, 255);
uint32_t off = strip.Color(0, 0, 0);

unsigned long time; //Worried about overflow
unsigned short bright;

/**********************************
* State Machine
**********************************/
enum { PRESS, UNPRESS };
enum { CLOCK, UPDATE_HOUR, UPDATE_MIN, UPDATE_SEC, UPDATE_BRIGHTNESS, UPDATE_COLOR, STATE_END };
int state_color[STATE_END] = { off, green, yellow, blue, light_magenta, magenta };
int mode = CLOCK;
int mode_button_state = UNPRESS;
unsigned long adjust = 0; 
void setup(){
  /********************
  * Setup Pins
  ********************/
  pinMode(LIGHT_PIN, OUTPUT);
  pinMode(ADJUST_PIN, INPUT);
  pinMode(MODE_PIN, INPUT);
  digitalWrite(ADJUST_PIN, HIGH); //Internal pull up
  digitalWrite(MODE_PIN, HIGH);
  
  /********************
  * Setup Neopixels
  ********************/
  bright = MAX_BRIGHTNESS;
  strip.begin();
  strip.show(); //Initialize everything off
  strip.setBrightness(bright); //Current limiting
  Serial.begin(9600);
  //delay(3000); //Wait for timer to get going?
}
void loop(){
  
  int sec;
  int m;
  int h;
  static boolean blnk = 0; //Blnk counter
  blnk = !blnk; //Blink it
  //if(!digitalRead(ADJUST_PIN)){//Active low
  //  adjust = adjust + 60000;
  //}
  //Get the time in msec
  time = millis(); //Overflows after 50 days :(
  time = time + adjust;
  
  //Serial.println(time);
  h = get_h(&time);
  m = get_m(&time);
  sec = get_sec(&time);
  //Serial.println(h);
  //Serial.println(m);
  //Serial.println(sec);
  //Serial.println();
  //Serial.println();
  if(!digitalRead(MODE_PIN)){//Active Low //Works but poorly, should be made to interrupt
    if(mode_button_state == UNPRESS){
      mode = mode + 1;
      mode_button_state = PRESS;
      if(mode >= STATE_END){ mode = CLOCK; }
    }
  } else {
    mode_button_state = UNPRESS;
  }
  
  
  
  
  strip.clear();
  create_neopixel_chain(h,m,sec);
  if(DEBUGGING){
      strip.setPixelColor(DEBUG_MODE_LED, state_color[mode]);
  }
  modify_by_mode(blnk);
  
  strip.show();
  delay(100); //Can save power here but probably need to wake up more than once per second.
  
}

int get_h( unsigned long* time){
  int h = *time / (MSEC_IN_HOUR);
  *time = *time - (h * MSEC_IN_HOUR);
  while( h >= 24 ) {
    h = h - 24; //Wrap around
  }
  return h;
}

int get_m(unsigned long* time){
  int m = *time / (MSEC_IN_MIN);
  //Serial.print("Minute: ");
  //Serial.println(m);
  *time = *time - (m * MSEC_IN_MIN);
  return m;
}

int get_sec(unsigned long* time){
  int sec = *time / (MSEC_IN_SEC );
  *time = *time - (sec * MSEC_IN_SEC);
  return sec;
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
  strip.setPixelColor(NUM_HOUR, green);
  strip.setPixelColor(NUM_MINUTE, green);
  strip.setPixelColor(NUM_SEC, green);
}

void add_h(int h){
  //boolean leds[5];
  //memset(leds, 0, sizeof(leds));
  for(int i = 4; i >=0; i--){
    if( h >= (1 << i) ){
      h = h - ( 1<< i );
      //leds[i] = 1;
      strip.setPixelColor((i + HOUR_SHIFT), magenta);
    } else {
      strip.setPixelColor(( i + HOUR_SHIFT), light_magenta);
    }
  }
  
}

void add_m(int m){
  //boolean leds[6];
  //memset(leds, 0, sizeof(leds));
  for(int i = 5; i >=0; i--){
    if( m >= (1 << i) ){
      m = m - ( 1<< i );
      //leds[i] = 1;
      strip.setPixelColor((i + MIN_SHIFT), magenta);
    } else {
      strip.setPixelColor(( i + MIN_SHIFT), light_magenta);
    }
  }
}

void add_second(int sec){
  //boolean leds[6];
  //memset(leds, 0, sizeof(leds));
  for(int i = 5; i >=0; i--){
    if( sec >= (1 << i) ){
      sec = sec - ( 1<< i );
      //leds[i] = 1;
      strip.setPixelColor((i + SEC_SHIFT), magenta);
    } else {
      strip.setPixelColor(( i + SEC_SHIFT), light_magenta);
    }
  }
}

void modify_by_mode(boolean blnk){
  switch(mode){
    case CLOCK:
      break;
    case UPDATE_HOUR:
    case UPDATE_MIN:
    case UPDATE_SEC:
      modify_time(blnk);
      break;
    case UPDATE_BRIGHTNESS:
      modify_brightness();
      break;
    case UPDATE_COLOR:
      modify_color();
      break;
  }
}

void modify_time(boolean b){
  static boolean adjust_pressed = UNPRESS;
  blink_section(b);
  if(!digitalRead(ADJUST_PIN)){
    if( adjust_pressed == UNPRESS ){    
      switch( mode ){
        case UPDATE_HOUR:
          adjust += MSEC_IN_HOUR;
          break;
        case UPDATE_MIN:
          adjust += MSEC_IN_MIN;
          break;
        case UPDATE_SEC:
          adjust += MSEC_IN_SEC;
          break;
        default://This shouldn't happen it wouldn't make any sense
        break;
      }
    }
    adjust_pressed = PRESS;
  } else {
    adjust_pressed = UNPRESS;
  }
}

void modify_color(){
  //Stubbed
}

void blink_section(boolean b){
  int i;
  if(b){
    switch( mode ){
      case UPDATE_HOUR:
        for( i = HOUR_SHIFT; i < MIN_SHIFT-1; i++){
          strip.setPixelColor(i, off);
        }
        break;
      case UPDATE_MIN:
        for( i = MIN_SHIFT; i < SEC_SHIFT-1; i++){
          strip.setPixelColor(i, off);
        }
        break;
      case UPDATE_SEC:
        for( i = SEC_SHIFT; i < SEC_SHIFT + 7; i++){
          strip.setPixelColor(i,off);
        }
        break;
      default:
        break;
      //Shouldn't ever happen. This would be confusing
    }
  }
}

void modify_brightness(){
  if(!digitalRead(ADJUST_PIN)){
    bright--;
    if (bright > MAX_BRIGHTNESS ) { //Checking for underflow on unsigned value
      bright = MAX_BRIGHTNESS;
    }
    strip.setBrightness(bright);
  }
}
