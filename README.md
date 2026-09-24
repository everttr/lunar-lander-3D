# Lunar Lander 3D: *Lunar Lander Goes to Space!*

A game about precariously landing a lunar module without blowing up. Written (nearly) from scratch in C++ & OpenGL. Made as my final project for Mike Bailey's CS 450 course at OSU. Try it! It's (almost) fun!

All game logic, shaders, and models by Reed Evertt. No rights reserved on that front. The [GLUT](https://github.com/freeglut/freeglut) & [GLM](https://github.com/g-truc/glm) libraries are used, and some CS 450 template code remains (this is also why the main file is "sample.cpp" and why there's so many unused template files littering the repo; I wasn't graded on file neatness!).

**A Windows-playable built version of the game is available as a release on the repository.**

## Features/Functionality
- Procedurally generated terrain geometry
- Procedural texturing for the environment
	- Perlin implementation for the "moon"
	- Voronoi implementation for the stars
- Physics-based gameplay
	- Linear and rotational momentum operable by player
	- Triangle-sphere collision which detects whether a landing is safe
- Multiple camera views
	- The "first-person" view includes camera shake at high speeds *(does this imply the moon has an atmosphere?)*

## Controls
- `Q`/`A`, `W`/`S`, `E`/`D`: rotational thrust
- `Space`/`Z`: upwards/downwards thrust
- `F`/`R`: anti-momentum rotational & linear thrust respectively
- `1`, `2`, `3`: camera view
- `Tab`: pause/resume
Note: holding down a button only triggers repeat presses after a delay, like a word processor (sorry! again, this is a visual/computational showcase more than a true arcade game)

## Video + Screenshots
Below is my original video demo from CS 450:

https://github.com/user-attachments/assets/b769bfe8-8ff1-4994-944e-520218073851

Below are some more screenshots from the game:

<img src="https://raw.githubusercontent.com/everttr/lunar-lander-3D/main/images/lander_flying.png" alt="Lunar Lander Flying" width="350"/>

<img src="https://raw.githubusercontent.com/everttr/lunar-lander-3D/main/images/lander_crash.png" alt="Lunar Lander Crashing" width="350"/>

## Building
Load the .sln file in Visual Studio, then build the solution!
