/*
Sources:
https://github.com/KrisKasprzak/96_Graphing/blob/main/OLED_Graphing.ino

*/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_WIDTH 128 // OLED display width, in pixels

#define FREQ_PIN 32
#define PHASE_PIN 33

#define MODE_PIN 34

#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int modes = 4;

int mode = 1;
double x = 0; // global variable
double y = 0;
double gx = 30;
double gy = 50;
double w = 75;
double h = 30;
double dig = 0;
int max_freq = 10;
int min_freq = 1;
double min_phase = 0.0;
double max_phase = 1.0;
double phase = 0;
double freq = 1;
double randomNum = 0.0;
double ox, oy;

int xmin = 0;
int xmax = 100;
int ymin = 0;
int ymax = 64;
int xsteps = 4;
double xinc = (xmax - xmin)/xsteps;
int ysteps = 2;
double yinc = (ymax - ymin)/ysteps;

void setup() {
  Serial.begin(9600);

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.display();
  delay(2000);

  display.clearDisplay();
  delay(2000);
}

void loop() {

  mode = map(analogRead(MODE_PIN), 0, 4095, 1, modes);

  freq = map(analogRead(FREQ_PIN), 0, 4095, min_freq, max_freq);
  phase = map(analogRead(PHASE_PIN), 0, 4095, 100000*min_phase, 100000*max_phase)/100000.0;
  
  checkMode();

}

/*

  function to draw a cartesian coordinate system and plot whatever data you want
  just pass x and y and the graph will be drawn huge arguement list

  &display to pass the display object, mainly used if multiple displays are connected to the MCU
  x = x data point
  y = y datapont
  gx = x graph location (lower left)
  gy = y graph location (lower left)
  w = width of graph
  h = height of graph
  xlo = lower bound of x axis
  xhi = upper bound of x asis
  xinc = division of x axis (distance not count)
  ylo = lower bound of y axis
  yhi = upper bound of y asis
  yinc = division of y axis (distance not count)
  title = title of graph
  &redraw = flag to redraw graph on fist call only

*/
void graph(Adafruit_SSD1306 &d, double y, double gx, double gy, double w, double h, double xlo, double xhi, double xinc, double ylo, double yhi, double yinc, double dig, String title) {

  if (x > 100) x = 0;
  if (x == 0) {

    double temp;
    double i;
    display.clearDisplay();

    // draw title
    d.fillRect(0, 0, 127, 16, SSD1306_WHITE);
    d.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    d.setTextSize(1);
    d.setCursor(2, 4);
    d.println(title);
    ox = (x - xlo) * ( w) / (xhi - xlo) + gx;
    oy = (y - ylo) * (gy - h - gy) / (yhi - ylo) + gy;

    // draw y scale
    d.setTextSize(1);
    d.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    for (i = ylo; i <= yhi; i += yinc) {
      // compute the transform
      // note: their transform function is the same as the map fucntion, except the map uses long and we need double
      temp = (i - ylo)*(gy - h - gy)/(yhi - ylo) + gy;
      if (i == 0) d.drawFastHLine(gx-3, temp, w+3, SSD1306_WHITE);
      else d.drawFastHLine(gx-3, temp, 3, SSD1306_WHITE);
      d.setCursor(gx-27, temp-3);
      d.println(i, dig);
    }

    // draw x scale
    for (i = xlo; i <= xhi; i += xinc) {
      //compute the transform
      d.setTextSize(1);
      d.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
      temp = (i-xlo)*w/(xhi-xlo)+gx;
      if (i == 0) d.drawFastVLine(temp, gy-h, h+3, SSD1306_WHITE);
      else d.drawFastVLine(temp, gy, 3, SSD1306_WHITE);
      d.setCursor(temp, gy+6);
      d.println(i, dig);
    }
  }

  // graph drawn now plot the data

  double u = (x-xlo)*w/(xhi-xlo) + gx;
  y = (y-ylo)*(gy-h-gy)/(yhi-ylo)+gy;
  d.drawLine(ox, oy, u, y, SSD1306_WHITE);
  d.drawLine(ox, oy-1, u, y-1, SSD1306_WHITE);
  ox = u;
  oy = y;

  d.display();
}

void checkMode() {
  switch (mode) {
    case 1:
      graphSin();
      break;
    case 2: 
      graphCos();
      break;
    case 3:
      graphSquare();
      break;
    case 4:
      graphNoise();
      break;
    default:
      break;
  }
}

void graphNoise() {
  int rand = random(ymin, ymax);
  randomNum = 1.0*rand;
  graph(display, randomNum, gx, gy, w, h, xmin, xmax, xinc, ymin, ymax, yinc, dig, "Random Number");
  x++;
}

void graphSin() {
  double val = ymax*(sin(2*M_PI*(freq*x/xmax + phase))+1)/2;
  graph(display, val, gx, gy, w, h, xmin, xmax, xinc, ymin, ymax, yinc, dig, "Sin Graph");
  x++;
}

void graphCos() {
  double val = ymax*(cos(2*M_PI*(freq*x/xmax + phase))+1)/2;
  graph(display, val, gx, gy, w, h, xmin, xmax, xinc, ymin, ymax, yinc, dig, "Cos Graph");
  x++;
}

void graphSquare() {
  double val = sin(2*M_PI*(freq*x/xmax + phase)) > 0 ? ymax - 1 : ymin + 1;
  graph(display, val, gx, gy, w, h, xmin, xmax, xinc, ymin, ymax, yinc, dig, "Square Graph");
  x++;
}