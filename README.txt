README

This is my project for a 1306 128x64 OLED screen [displayed] with an ESP32 over I2C.

It is a fork or KrisKasprzak's OLED_Graphing.ino file, found through his youtube video: https://www.youtube.com/watch?v=13PFOwcK3-I&ab_channel=KrisKasprzak.

It displays an animation of any function, I have it set to display a sin graph with adjustable parameters for amplitude, frequency, and phase. These could be assigned to inputs on the ESP32 with potentiometers attached for live adjustment.

Currently, the file also has a noise and cos function, I intend to add square, triangle, and sawtooth wave functions. This is included in the TODO section at the top of the .ino file.