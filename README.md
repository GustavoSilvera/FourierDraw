# FourierDraw
Graphical Windows animation for Fourier Transform Series approximations of complex relations. 

# Basically
  Imagine a single moon orbiting at a fixed angular velocity and radius. Now imagine another moon orbiting around that moon.
  Now imagine another moon orbiting around that moon orbiting around the first moon... and on and on and on...
  Now what if there was a pencil on the tip of the very last moon in the sequence? What kind of image would it draw?
  Well, by applying a [[Complex Fourier Transform Series](https://en.wikipedia.org/wiki/Fourier_series)] to the sequence, 
  it is possible to tweak the initial angles and velocities of all the 'moons' in order to match a nontrivial function, like a drawing outline.
  Below I have a gallery of some examples of this program, where an image was downloaded and traced, and then the Fourier series was used to outline it using ~800 moons.

![](images/butterfly.gif) ![](images/chungus.gif) ![](images/GoPlaid.gif)  

# How To Install
  This program was written in C++ using the open source CinderC++ graphics libraries and compiled on a Windows Machine. 
  To get the simplest installation, simply install [[CinderC++ 0.8.6](https://www.libcinder.org/download)] for either OSX or Windows (No linux Support yet).
  
# How To Use
  Simply add any image to the /Assets directory (for best tracing, make sure it has a great contrast with a dark border) and change the source file (FourierDraw.cpp) to read whatever name you gave the file, then let the program automatically trace. 
  Or, for those who want to make custom drawings, simply use the right mouse buttons to place 'orange dots' as coordinates that will be traced by the program after initializing.
  
# Controls
  Press the **del** key to remove an 'orange dot' coordinate, and press **spacebar** to begin the moon orbiting as soon as the orange dots are placed.

# YouTube Video

[![Alt text](https://img.youtube.com/vi/VI8btEeCVeE/0.jpg)](https://www.youtube.com/watch?v=VI8btEeCVeE)\

Youtube Link: https://www.youtube.com/watch?v=VI8btEeCVeE

