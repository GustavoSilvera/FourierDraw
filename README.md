# FourierDraw
Graphical animation for Fourier Transform Series approximations of complex relations. 

# Basically
  Imagine a single moon orbiting at a fixed angular velocity and radius. Now imagine another moon orbiting around that moon.
  Now imagine another moon orbiting aound that moon orbiting around the first moon... and on and on and on...
  Now what if there was a pencil on the tip of the very last moon in the sequence? What kind of image would it draw?
  Well, by applying a [[Complex Fourier Transform Series](https://en.wikipedia.org/wiki/Fourier_series)] to the sequence, 
  it is possible to tweak the initial angles and velocities of all the 'moons' in order to match a nontrivial function, like a drawing outline.
  Below I have a gallery of some examples of this program, where an image was downloaded and traced, and then the fourier series was used to outline it using ~800 moons.

![](images/butterfly.gif) ![](images/chungus.gif) ![](images/GoPlaid.gif)  

# How To Install
  This program was written in C++ using the open source CinderC++ graphics libraries and compiled on a Windows Machine. 
  To get the simplest installation, simply install [[CinderC++ 0.8.6](https://www.libcinder.org/download)] for either OSX or Windows. I am currently working on porting the code to work with OpenFrameworks (For Linux Support) and creating a makefile to compile this on any machine regardless of OS.
  
# How To Use
  Simply add any image to the /Assets directory (for best tracing, make sure it has a great contrast with a dark border) and let the program automatically trace. 
  Or, for those who want to make custom drawings, simply use the mouse buttons to place 'orange dots' as coordinates that will be traced by the program after initializing.
  
# Controls
  Press the **del** key to remove an 'orange dot' coordinate, and press **spacebar** to begin the moon orbiting as soon as the orange dots are placed.

[![Alt text](https://img.youtube.com/vi/VI8btEeCVeE/0.jpg)](https://www.youtube.com/watch?v=VI8btEeCVeE)\

Youtube Link: https://www.youtube.com/watch?v=VI8btEeCVeE

