/*
Sources:
https://github.com/KrisKasprzak/96_Graphing/blob/main/OLED_Graphing.ino

*/

#include <cmath>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_WIDTH 128 // OLED display width, in pixels

#define FREQ_PIN 25
#define PHASE_PIN 12

#define MODE_PIN 34

#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int modes = 6;
int mode = 1; // global variable used to pass the value of mode pin, range from 1 to *modes*

const int xmin = 0;
const int xmax = 100;
const int xrange = xmax - xmin;

const int ymin = 0;
const int ymax = 64;
const int yrange = ymax - ymin;

int x = 0; // global variable to track horizontal data position (within xrange)

double gx = 10; // horizontal displacement of graph (0-128), positive is right, starts at left side
double gy = 60; // vertical         "      "   "" of graph (0-64), positive is down, starts at top

double w = 110; // width of graph
double h = 58; // height of graph

// variables for axis markers
int xsteps = 4;
double xinc = (xmax - xmin)/xsteps;

int ysteps = 2;
double yinc = (ymax - ymin)/ysteps;

double dig =  0; // digit for labeled axes

const double max_freq = 10.0;
const double min_freq = 1.0;

const double min_phase = 0.0;
const double max_phase = 1.0;

double phase = 0; // global variable similar to mode but for phase
double freq = 1.0; // global variable   "    "   "    "   "  frequency

// placeholders to draw lines connecting data points
// initialized at (0,0)
double ox = gx;
double oy = gy-h/2; // just gy for x axis at bottom of screen

int triangle_data[xrange];
// possibly introduce a sin_data array to use for sin calculations and use sin(x) = cos(x-pi/2)

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

  // Produce data for triangle wave
  for (int i = 0; i < trunc(xrange/2); i++) triangle_data[i] = 2*(i-xmin)*(ymax-ymin)/xrange; // positive slope
  for (int i = ceil(xrange/2); i < xrange; i++) triangle_data[i] = 2*(xmax-i)*(ymax-ymin)/xrange; // negative slope
}

void loop() {

  mode = map(analogRead(MODE_PIN), 0, 4095, 1, modes);

  freq = map(analogRead(FREQ_PIN), 0, 4095, min_freq, max_freq);
  phase = map(analogRead(PHASE_PIN), 0, 4095, 100000*min_phase, 100000*max_phase)/100000.0;
  
  checkMode();

}

void graph(Adafruit_SSD1306 &d, double y, String title) {

  if (x > xmax) x = 0;
  if (x == 0) {

    ox = gx;

    double temp;
    double i;
    display.clearDisplay();

    // draw y scale
    for (i = ymin; i <= ymax; i += yinc) {
      // compute the transform
      // note: their transform function is the same as the map fucntion, except the map uses long and we need double
      temp = (i - ymin)*(gy - h - gy)/(ymax - ymin) + gy;
      if (i == ((ymax-ymin)/2-(ymax-ymin)%2)) d.drawFastHLine(gx-3, temp, w+3, SSD1306_WHITE); // draw x axis
      else d.drawFastHLine(gx-3+w/2, temp, 3, SSD1306_WHITE);
      d.setCursor(gx-27, temp-3);
    }

    // draw x scale
    for (i = xmin; i <= xmax; i += xinc) {
      //compute the transform
      temp = (i-xmin)*w/(xmax-xmin)+gx;
      if (i == ((xmax-xmin)/2-(xmax-xmin)%2)) d.drawFastVLine(temp, gy-h, h+3, SSD1306_WHITE); // draw y axis
      else d.drawFastVLine(temp, gy-h/2, 3, SSD1306_WHITE);
      d.setCursor(temp, gy+6);
    }
  }

  // graph drawn now plot the data

  double u = (x-xmin)*w/(xmax-xmin) + gx;
  y = (y-ymin)*(gy-h-gy)/(ymax-ymin)+gy;
  d.drawLine(ox, oy, u, y, SSD1306_WHITE);
  d.drawLine(ox, oy, u, y-1, SSD1306_WHITE);
  ox = u;
  oy = y;

  d.display();
}

/* Alternate graphing functions
void graphLabeledAxes(Adafruit_SSD1306 &d, double y, String title) {
  // alter gx/gy and w/h to fit labels
  double gx = 20;
  double gy = 50;
  double w = 90;
  double h = 27;
  double ox = gx;
  double oy = gy;
  if (x > xmax) x = 0;
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

    // draw y scale
    d.setTextSize(1);
    d.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    for (i = ymin; i <= ymax; i += yinc) {
      // compute the transform
      // note: their transform function is the same as the map fucntion, except the map uses long and we need double
      temp = (i - ymin)*(gy - h - gy)/(ymax - ymin) + gy;
      if (i == 0) d.drawFastHLine(gx-3, temp, w+3, SSD1306_WHITE);
      else d.drawFastHLine(gx-3, temp, 3, SSD1306_WHITE);
      d.setCursor(gx-18, temp-3);
      d.println(i, dig);
    }

    // draw x scale
    for (i = xmin; i <= xmax; i += xinc) {
      //compute the transform
      d.setTextSize(1);
      d.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
      temp = (i-xmin)*w/(xmax-xmin)+gx;
      if (i == 0) d.drawFastVLine(temp, gy-h, h+3, SSD1306_WHITE);
      else d.drawFastVLine(temp, gy, 3, SSD1306_WHITE);
      d.setCursor(temp, gy+6);
      d.println(i, dig);
    }
  }

  // graph drawn now plot the data

  double u = (x-xmin)*w/(xmax-xmin) + gx;
  y = (y-ymin)*(gy-h-gy)/(ymax-ymin)+gy;
  d.drawLine(u, y, u, y, SSD1306_WHITE);
  d.drawLine(u, y-1, u, y-1, SSD1306_WHITE);

  d.display();
}

void graphLabelessAxes(Adafruit_SSD1306 &d, double y, String title) {

  if (x > xmax) x = 0;
  if (x == 0) {

    double temp;
    double i;
    display.clearDisplay();

    // draw y scale
    for (i = ymin; i <= ymax; i += yinc) {
      // compute the transform
      // note: their transform function is the same as the map fucntion, except the map uses long and we need double
      temp = (i - ymin)*(gy - h - gy)/(ymax - ymin) + gy;
      if (i == 0) d.drawFastHLine(gx-3, temp, w+3, SSD1306_WHITE);
      else d.drawFastHLine(gx-3, temp, 3, SSD1306_WHITE);
      d.setCursor(gx-27, temp-3);
    }

    // draw x scale
    for (i = xmin; i <= xmax; i += xinc) {
      //compute the transform
      temp = (i-xmin)*w/(xmax-xmin)+gx;
      if (i == 0) d.drawFastVLine(temp, gy-h, h+3, SSD1306_WHITE);
      else d.drawFastVLine(temp, gy, 3, SSD1306_WHITE);
      d.setCursor(temp, gy+6);
    }
  }

  // graph drawn now plot the data

  double u = (x-xmin)*w/(xmax-xmin) + gx;
  y = (y-ymin)*(gy-h-gy)/(ymax-ymin)+gy;
  d.drawLine(u, y, u, y-2, SSD1306_WHITE);
  d.drawLine(u, y-2, u, y, SSD1306_WHITE);

  d.display();
}

void graphCenterY_Axis(Adafruit_SSD1306 &d, double y, String title) {
  if (x > xmax) x = 0;
  if (x == 0) {

    ox = gx;

    double temp;
    double i;
    display.clearDisplay();

    // draw y scale
    for (i = ymin; i <= ymax; i += yinc) {
      // compute the transform
      // note: their transform function is the same as the map fucntion, except the map uses long and we need double
      temp = (i - ymin)*(gy - h - gy)/(ymax - ymin) + gy;
      if (i == 0) d.drawFastHLine(gx-3, temp, w+3, SSD1306_WHITE); // draw x axis
      else d.drawFastHLine(gx-3+w/2, temp, 3, SSD1306_WHITE);
      d.setCursor(gx-27, temp-3);
    }

    // draw x scale
    for (i = xmin; i <= xmax; i += xinc) {
      //compute the transform
      temp = (i-xmin)*w/(xmax-xmin)+gx;
      if (i == ((xmax-xmin)/2-(xmax-xmin)%2)) d.drawFastVLine(temp, gy-h, h+3, SSD1306_WHITE); // draw y axis
      else d.drawFastVLine(temp, gy, 3, SSD1306_WHITE);
      d.setCursor(temp, gy+6);
    }
  }

  // graph drawn now plot the data

  double u = (x-xmin)*w/(xmax-xmin) + gx;
  y = (y-ymin)*(gy-h-gy)/(ymax-ymin)+gy;
  d.drawLine(ox, oy, u, y, SSD1306_WHITE);
  d.drawLine(ox, oy, u, y-1, SSD1306_WHITE);
  ox = u;
  oy = y;

  d.display();
}

void graphCenterX_Axis(Adafruit_SSD1306 &d, double y, String title) {
  if (x > xmax) x = 0;
  if (x == 0) {
    ox = gx;
    oy = gy - h/2;

    double temp;
    double i;
    display.clearDisplay();

    // draw y scale
    for (i = ymin; i <= ymax; i += yinc) {
      // compute the transform
      // note: their transform function is the same as the map fucntion, except the map uses long and we need double
      temp = (i - ymin)*(gy - h - gy)/(ymax - ymin) + gy;
      if (i == ((ymax-ymin)/2-(ymax-ymin)%2)) d.drawFastHLine(gx-3, temp, w+3, SSD1306_WHITE);
      else d.drawFastHLine(gx-3, temp, 3, SSD1306_WHITE);
      d.setCursor(gx-27, temp-3);
    }

    // draw x scale
    for (i = xmin; i <= xmax; i += xinc) {
      //compute the transform
      temp = (i-xmin)*w/(xmax-xmin)+gx;
      if (i == 0) d.drawFastVLine(temp, gy-h, h+3, SSD1306_WHITE);
      else d.drawFastVLine(temp, gy-h/2, 3, SSD1306_WHITE);
      d.setCursor(temp, gy+6);
    }
  }

  // graph drawn now plot the data

  double u = (x-xmin)*w/(xmax-xmin) + gx;
  y = (y-ymin)*(gy-h-gy)/(ymax-ymin)+gy;
  d.drawLine(u, y, u, y-2, SSD1306_WHITE);
  d.drawLine(u, y-2, u, y, SSD1306_WHITE);

  d.display();
}

void graphCenterBothAxis(Adafruit_SSD1306 &d, double y, String title) {
  if (x > xmax) x = 0;
  if (x == 0) {

    ox = gx;

    double temp;
    double i;
    display.clearDisplay();

    // draw y scale
    for (i = ymin; i <= ymax; i += yinc) {
      // compute the transform
      // note: their transform function is the same as the map fucntion, except the map uses long and we need double
      temp = (i - ymin)*(gy - h - gy)/(ymax - ymin) + gy;
      if (i == ((ymax-ymin)/2-(ymax-ymin)%2)) d.drawFastHLine(gx-3, temp, w+3, SSD1306_WHITE); // draw x axis
      else d.drawFastHLine(gx-3+w/2, temp, 3, SSD1306_WHITE);
      d.setCursor(gx-27, temp-3);
    }

    // draw x scale
    for (i = xmin; i <= xmax; i += xinc) {
      //compute the transform
      temp = (i-xmin)*w/(xmax-xmin)+gx;
      if (i == ((xmax-xmin)/2-(xmax-xmin)%2)) d.drawFastVLine(temp, gy-h, h+3, SSD1306_WHITE); // draw y axis
      else d.drawFastVLine(temp, gy-h/2, 3, SSD1306_WHITE);
      d.setCursor(temp, gy+6);
    }
  }

  // graph drawn now plot the data

  double u = (x-xmin)*w/(xmax-xmin) + gx;
  y = (y-ymin)*(gy-h-gy)/(ymax-ymin)+gy;
  d.drawLine(ox, oy, u, y, SSD1306_WHITE);
  d.drawLine(ox, oy, u, y-1, SSD1306_WHITE);
  ox = u;
  oy = y;

  d.display();
}
*/

void checkMode() {
  switch (mode) {
    case 1: // Sin
      graph(display, yrange*(sin(2*M_PI*(1.0*freq*x/xrange - phase))+1)/2, "Sin Graph");
      x++;
      break;
    case 2: // Cos
      graph(display, yrange*(cos(2*M_PI*(1.0*freq*x/xrange - phase))+1)/2, "Cos Graph");
      x++;
      break;
    case 3: // Square
      graph(display, sin(2*M_PI*(1.0*freq*x/xrange - phase)) > 0 ? ymax - 1 : ymin + 1, "Square Graph");
      x++;
      break;
    case 4: // Triangle
      graph(display, triangle_data[((int) (freq*x - phase*xrange))%xrange], "Triangle Graph");
      x++;
      break;
    case 5: // Sawtooth
      graphSawtooth(); // must be a function because it defines an internal variable
      break;
    case 6: // Noise
      graph(display, 1.0*random(ymin, ymax), "Random Number");
      x++;
      break;
    default:
      break;
  }
}

void graphSawtooth() {
  // point slope equation with the modulus
  int val = ((int) (freq*(x-xmin-xrange*phase/freq)*yrange/(xrange)))%ymax;
  if (val < 0) val += yrange; // Check if the phase variable "pushes" it negative and compensate by adding yrange because it can only happen within the phase, so it will always bring it within view
  graph(display, val, "Sawtooth Graph");
  x++;
}

/* Functional definitions of modes
  void graphSin() {
    graph(display, yrange*(sin(2*M_PI*(1.0*freq*x/xrange - phase))+1)/2, "Sin Graph");
    x++;
  }

  void graphCos() {
    graph(display, yrange*(cos(2*M_PI*(1.0*freq*x/xrange - phase))+1)/2, "Cos Graph");
    x++;
  }

  void graphSquare() {
    graph(display, sin(2*M_PI*(1.0*freq*x/xrange - phase)) > 0 ? ymax - 1 : ymin + 1, "Square Graph");
    x++;
  }

  void graphTriangle() {
    graph(display, triangle_data[((int) (freq*x - phase*xrange))%xrange], "Triangle Graph");
    x++;
  }

  void graphSawtooth() {
    // point slope equation with the modulus
    int val = ((int) (freq*(x-xmin-xrange*phase/freq)*yrange/(xrange)))%ymax;
    if (val < 0) val += yrange;
    graph(display, val, "Sawtooth Graph");
    x++;
  }

  void graphNoise() {
    graph(display, 1.0*random(ymin, ymax), "Random Number");
    x++;
  }
*/