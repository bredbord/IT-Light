#include <FastLED.h>
#include "EEPROM.h"

#define FASTLED_ALLOW_INTERRUPTS 0
FASTLED_USING_NAMESPACE

#include <Button.h>
#include <ButtonEventCallback.h>
#include <PushButton.h>
#include <Bounce2.h>    // https://github.com/thomasfredericks/Bounce-Arduino-Wiring

// Create an instance of PushButton reading digital pin 5
PushButton button = PushButton(2);

// Pride2015
// Animated, ever-changing rainbows.
// by Mark Kriegsman

#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    6
//#define CLK_PIN   4
#define LED_TYPE    WS2811
#define COLOR_ORDER BRG
#define NUM_LEDS    25

#define MAX_MODE 11

CRGB leds[NUM_LEDS];

int mode, brightness;
bool dimmerDirection = false;
bool EEHoldWrite = false;

// Prototypes
void pride();
void configurePushButton(Bounce&);
void onButtonPressed(Button&);


void setup() {

  mode = EEPROM.read(1);
  brightness = EEPROM.read(2);

  if (brightness < 127) dimmerDirection = true;

  // Configure the button as you'd like - not necessary if you're happy with the defaults
  button.configureButton(configurePushButton);
  // When the button is first pressed, call the function onButtonPressed (further down the page)
  button.onPress(onButtonPressed);
  // Once the button has been held for 1 second (1000ms) call onButtonHeld. Call it again every 0.5s (500ms) until it is let go
  button.onHoldRepeat(700, 12, onButtonHeld);
  // When the button is released, call onButtonReleased
  button.onRelease(onButtonReleased);
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(brightness);

  //Serial.begin(9600);
  
}


void loop()
{
  button.update();  // read new switch input

  //Modes
  if (mode == 1) { FastLED.setTemperature(UncorrectedTemperature); for (int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Red; } // RED
  if (mode == 2) { FastLED.setTemperature(UncorrectedTemperature); for (int i = 0; i < NUM_LEDS; i++) leds[i] = 0xFF0066; }  // Pink
  if (mode == 3) { FastLED.setTemperature(UncorrectedTemperature); for (int i = 0; i < NUM_LEDS; i++) leds[i] = 0xDD00FF; }  // Purple
  if (mode == 4) { FastLED.setTemperature(UncorrectedTemperature); for (int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Blue; }  // BLUE
  if (mode == 5) { FastLED.setTemperature(UncorrectedTemperature); for (int i = 0; i < NUM_LEDS; i++) leds[i] = 0x00FFFF; }  // Light Blue
  if (mode == 6) { FastLED.setTemperature(UncorrectedTemperature); for (int i = 0; i < NUM_LEDS; i++) leds[i] = 0x00FF91; }  // Aqua
  if (mode == 7) { FastLED.setTemperature(UncorrectedTemperature); for (int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Green; }  // Green

  // Temps
  if (mode == 8) { FastLED.setTemperature(Candle); for (int i = 0; i < NUM_LEDS; i++) leds[i] = Candle; }
  if (mode == 9) { FastLED.setTemperature(OvercastSky); for (int i = 0; i < NUM_LEDS; i++) leds[i] = OvercastSky; }

  //Animations
  if (mode == 10) { FastLED.setTemperature(UncorrectedTemperature); pride(); }
  if (mode == 11) { FastLED.setTemperature(UncorrectedTemperature); EVERY_N_MILLISECONDS(20) {pacifica_loop(); FastLED.show();} }

  // LED updater
  if (mode != 11) FastLED.show();
}

// Pride-----------------------------------------------------------------------------------------------------
// This function draws rainbows with an ever-changing,
// widely-varying set of parameters.
void pride() 
{
  static uint16_t sPseudotime = 0;
  static uint16_t sLastMillis = 0;
  static uint16_t sHue16 = 0;
 
  uint8_t sat8 = beatsin88( 87, 220, 250);
  uint8_t brightdepth = beatsin88( 341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 1, 3000);
  
  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis ;
  sLastMillis  = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * beatsin88( 400, 5,9);
  uint16_t brightnesstheta16 = sPseudotime;
  
  for( uint16_t i = 0 ; i < NUM_LEDS; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);
    
    CRGB newcolor = CHSV( hue8, sat8, bri8);
    
    uint16_t pixelnumber = i;
    pixelnumber = (NUM_LEDS-1) - pixelnumber;
    
    nblend( leds[pixelnumber], newcolor, 64);
  }
}



// Pacifica----------------------------------------------------------------------------------------------------------------
CRGBPalette16 pacifica_palette_1 = 
    { 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117, 
      0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x14554B, 0x28AA50 };
CRGBPalette16 pacifica_palette_2 = 
    { 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117, 
      0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x0C5F52, 0x19BE5F };
CRGBPalette16 pacifica_palette_3 = 
    { 0x000208, 0x00030E, 0x000514, 0x00061A, 0x000820, 0x000927, 0x000B2D, 0x000C33, 
      0x000E39, 0x001040, 0x001450, 0x001860, 0x001C70, 0x002080, 0x1040BF, 0x2060FF };


void pacifica_loop()
{
  // Increment the four "color index start" counters, one for each wave layer.
  // Each is incremented at a different speed, and the speeds vary over time.
  static uint16_t sCIStart1, sCIStart2, sCIStart3, sCIStart4;
  static uint32_t sLastms = 0;
  uint32_t ms = GET_MILLIS();
  uint32_t deltams = ms - sLastms;
  sLastms = ms;
  uint16_t speedfactor1 = beatsin16(3, 179, 269);
  uint16_t speedfactor2 = beatsin16(4, 179, 269);
  uint32_t deltams1 = (deltams * speedfactor1) / 256;
  uint32_t deltams2 = (deltams * speedfactor2) / 256;
  uint32_t deltams21 = (deltams1 + deltams2) / 2;
  sCIStart1 += (deltams1 * beatsin88(1011,10,13));
  sCIStart2 -= (deltams21 * beatsin88(777,8,11));
  sCIStart3 -= (deltams1 * beatsin88(501,5,7));
  sCIStart4 -= (deltams2 * beatsin88(257,4,6));

  // Clear out the LED array to a dim background blue-green
  fill_solid( leds, NUM_LEDS, CRGB( 2, 6, 10));

  // Render each of four layers, with different scales and speeds, that vary over time
  pacifica_one_layer( pacifica_palette_1, sCIStart1, beatsin16( 3, 11 * 256, 14 * 256), beatsin8( 10, 70, 130), 0-beat16( 301) );
  pacifica_one_layer( pacifica_palette_2, sCIStart2, beatsin16( 4,  6 * 256,  9 * 256), beatsin8( 17, 40,  80), beat16( 401) );
  pacifica_one_layer( pacifica_palette_3, sCIStart3, 6 * 256, beatsin8( 9, 10,38), 0-beat16(503));
  pacifica_one_layer( pacifica_palette_3, sCIStart4, 5 * 256, beatsin8( 8, 10,28), beat16(601));

  // Add brighter 'whitecaps' where the waves lines up more
  pacifica_add_whitecaps();

  // Deepen the blues and greens a bit
  pacifica_deepen_colors();
}

// Add one layer of waves into the led array
void pacifica_one_layer( CRGBPalette16& p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff)
{
  uint16_t ci = cistart;
  uint16_t waveangle = ioff;
  uint16_t wavescale_half = (wavescale / 2) + 20;
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    waveangle += 250;
    uint16_t s16 = sin16( waveangle ) + 32768;
    uint16_t cs = scale16( s16 , wavescale_half ) + wavescale_half;
    ci += cs;
    uint16_t sindex16 = sin16( ci) + 32768;
    uint8_t sindex8 = scale16( sindex16, 240);
    CRGB c = ColorFromPalette( p, sindex8, bri, LINEARBLEND);
    leds[i] += c;
  }
}

// Add extra 'white' to areas where the four layers of light have lined up brightly
void pacifica_add_whitecaps()
{
  uint8_t basethreshold = beatsin8( 9, 55, 65);
  uint8_t wave = beat8( 7 );
  
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    uint8_t threshold = scale8( sin8( wave), 20) + basethreshold;
    wave += 7;
    uint8_t l = leds[i].getAverageLight();
    if( l > threshold) {
      uint8_t overage = l - threshold;
      uint8_t overage2 = qadd8( overage, overage);
      leds[i] += CRGB( overage, overage2, qadd8( overage2, overage2));
    }
  }
}

// Deepen the blues and greens
void pacifica_deepen_colors()
{
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    leds[i].blue = scale8( leds[i].blue,  145); 
    leds[i].green= scale8( leds[i].green, 200); 
    leds[i] |= CRGB( 2, 5, 7);
  }
}

// Buttons--------------------------------------------------------------------------------------------------------------------

void configurePushButton(Bounce& bouncedButton){
  // Set the debounce interval to 15ms - default is 10ms
  bouncedButton.interval(10);
}

void onButtonPressed(Button& btn){
  mode++;
  if (mode > MAX_MODE) mode = 1;
  EEPROM.write(1, mode);
}

void onButtonHeld(Button& btn, uint16_t duration, uint16_t repeatCount){
  digitalWrite(13, HIGH);
  EEHoldWrite = true;

  if (dimmerDirection) brightness++;  //light is dim, so raise
  else brightness--;  //otherwise its bright so lower
  
  if (brightness > 255) { brightness = 255; dimmerDirection = false; }
  else if(brightness < 0) { brightness = 0; dimmerDirection = true; }

  //Serial.println(brightness);
  
  FastLED.setBrightness(brightness);
}

void onButtonReleased(Button& btn, uint16_t duration){
  if (EEHoldWrite) {
    EEPROM.write(2, brightness);
    EEHoldWrite = false;
  }
}
