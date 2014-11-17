ClothPhysics
blaws

Simple C++ program that draws a particle-based animated cloth square. To 
use a bitmap texture, specify it on the command line:
	./cloth <BMP texture>

Click and drag to orient view.
Right-click on any position of the screen to add a force to the
	corresponding place on the cloth.
'i', 'o' to zoom in/out.
'w', 'a', 's', 'd' to move view.
'+', '-' to increase/decrease gravity.
'v', 'f' to increase/decrease cloth stiffness.
'[', ']' to increase/decrease cloth size.
'ENTER' to toggle wireframe/solid view.
'r' to reset cloth position.
''' (quote) to toggle reset position between horizontal and vertical.
'ESC' or 'q' to quit.

Written on Mac OSX 10.9 using OpenGL/GLUT.
