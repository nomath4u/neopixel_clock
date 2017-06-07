#include <Adafruit_NeoPixel.h>

#define LIGHT_PIN 3
#define STRIP_LEN 30
#define HOUR_SHIFT 0
#define MIN_SHIFT 6
#define SEC_SHIFT MIN_SHIFT + 7
#define ADJUST_PIN 10
/**********************************
* Function Signatures
**********************************/

int get_h(long*);
int get_m(long*);
int get_se(long*);
void create_neopixel_chain(int, int, int);

/**********************************
* NeoPixel Setup
**********************************/

Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIP_LEN, LIGHT_PIN, NEO_GRB + NEO_KHZ800);

uint32_t magenta = strip.Color(0, 100, 150); //Active color These should both be temporary until we have wheel
uint32_t light_magenta = strip.Color( 20, 4, 0 ); //Background color for off places
uint32_t green = strip.Color(0, 255, 0); //Seperator
unsigned long adjust = 0; 
void setup(){
  pinMode(LIGHT_PIN, OUTPUT);
  pinMode(ADJUST_PIN, INPUT);
  digitalWrite(ADJUST_PIN, HIGH); //Internal pull up
  //Neopixel init
  strip.begin();
  strip.show(); //Initialize everything off
  strip.setBrightness(64); //Current limiting
  Serial.begin(9600);
  //delay(3000); //Wait for timer to get going?
}
void loop(){
  unsigned long time; //Worried about overflow
  int sec;
  int m;
  int h;
  if(!digitalRead(ADJUST_PIN)){//Active low
    adjust = adjust + 60000;
  }
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
  strip.clear();
  create_neopixel_chain(h,m,sec);
  strip.show();
  delay(100); //Can save power here but probably need to wake up more than once per second.
  
}

int get_h( unsigned long* time){
  int h = *time / (3600000);
  *time = *time - (h * 3600000);
  while( h >= 24 ) {
    h = h - 24; //Wrap around
  }
  return h;
}

int get_m(unsigned long* time){
  int m = *time / (60000);
  //Serial.print("Minute: ");
  //Serial.println(m);
  *time = *time - (m * 60000);
  return m;
}

int get_sec(unsigned long* time){
  int sec = *time / (1000 );
  *time = *time - (sec * 1000);
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
