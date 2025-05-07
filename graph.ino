/*
Sources:
https://github.com/KrisKasprzak/96_Graphing/blob/main/OLED_Graphing.ino

*/

/* TODO:
Make square, triangle, sawtooth
Make graph functions for different kinds of axes
Make sin graph in middle labeless x-axis graph 
Make it continuous data (for non-integer valued frequencies make it continue data at looping)
Find out how to fade brightness of individual sections, then make fading graph (fades out the beginning of the signal as it finishes before looping, so maximum amount of signal can be seen constantly) so it loops less abrubtly
*/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_WIDTH 128 // OLED display width, in pixels

#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

double x = 0;
double y = 0;
double gx = 30;
double gy = 50;
double w = 75;
double h = 30;
double dig = 0;
double randomNum = 0.0;
double ox, oy;
bool redraw = true;

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
  // display.drawBitmap(0, 0, image, 32, 13, WHITE);
  // display.display();
  delay(2000);

  // drawMandle();
}

void loop() {

  graphSin();

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
void graph(Adafruit_SSD1306 &d, double x, double y, double gx, double gy, double w, double h, double xlo, double xhi, double xinc, double ylo, double yhi, double yinc, double dig, String title, boolean &Redraw) {
  
  double i;
  double temp;

  if (Redraw == true) {
    Redraw = false;
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
      // note: their transform function is the same as teh map fucntion, except the map uses long and we need double
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

  x = (x-xlo)*w/(xhi-xlo) + gx;
  y = (y-ylo)*(gy-h-gy)/(yhi-ylo)+gy;
  d.drawLine(ox, oy, x, y, SSD1306_WHITE);
  d.drawLine(ox, oy-1, x, y-1, SSD1306_WHITE);
  ox = x;
  oy = y;

  d.display();
}

void graphLabelessAxes() {

}

void graphCenterY_Axis() {

}

void graphCenterX_Axis() {

}

void graphCenterBothAxis() {

}

void graphNoise() {
  int rand = random(ymin, ymax);
  randomNum = 1.0*rand;
  
  graph(display, x++, randomNum, gx, gy, w, h, xmin, xmax, xinc, ymin, ymax, yinc, dig, "Random Number", redraw);
  if (x == 100) {
    x = 0;
    display.clearDisplay();
    redraw = true;
  }
}

void graphSin() {
  double freq = 2;
  double phase = 0; // [0, 2PI] for true loop, [0, PI/2] is only important, [PI/2, PI] is cos(), [PI, 3PI/2] for -sin(), [3PI/2, 2PI] for -cos()
  double val = ymax*(sin(2*M_PI*freq*x/xmax + phase)+1)/2;
  //Serial.println(x, val);
  graph(display, x++, val, gx, gy, w, h, xmin, xmax, xinc, ymin, ymax, yinc, dig, "Sin Graph", redraw);
  if (x == 100) {
    x = 0;
    display.clearDisplay();
    redraw = true;
  }
}

void graphCos() {
  double freq = 2;
  double phase = 0;
  double val = ymax*(cos(2*M_PI*freq*x/xmax + phase)+1)/2;
  //Serial.println(x, val);
  graph(display, x++, val, gx, gy, w, h, xmin, xmax, xinc, ymin, ymax, yinc, dig, "Cos Graph", redraw);
  if (x == 100) {
    x = 0;
    display.clearDisplay();
    redraw = true;
  }
}
