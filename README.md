# TDT4230 - Graphics and Visualization - Final Project

This is my final project in the course TDT4230. I have attempted to simulate a black hole by making use of a screen-space effect using deferred rendering techniques.

The code is based on the "glowbox" application code, made by Bart Iver van Blokland and Peder Bergebakken Sundt. This application is found at https://github.com/bartvbl/TDT4230-Assignment-1.git.


## How to run:

	git clone --recursive https://github.com/klovjan/TDT4230_Project.git

Should you forget the `--recursive` bit, just run:

	git submodule update --init


### Windows

Install Microsoft Visual Studio Express and CMake.
You may use CMake-gui or the command-line cmake to generate a Visual Studio solution.

### Linux:

Make sure you have a C/C++ compiler such as  GCC, CMake and Git.

	make run

which is equivalent to

	git submodule update --init
	cd build
	cmake ..
	make
	./glowbox
