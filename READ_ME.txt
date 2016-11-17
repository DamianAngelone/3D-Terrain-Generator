Name:   Damian Angelone
ID:     001408211
macID:  angelodp
E-mail: angelodp@mcmaster.ca

SPECIAL FEATURES THAT ARE IMPLEMENTED:

1) When in colour mode, the program will colour the terrain in regards to differnt heights.
	-This works for both circles/fault algorithm, and drawing wit quad and triangle strips.
		-Water is lowest.
		-Grass is next.
		-Moutain is next.
		-Snow is highest.

2) Implemented the fault algorithm.
	-This can be toggled with the circle's algorithm using the 'c' key.
	-Works with lighting/colour mode, flat/smooth shading, and drawing with triangle/quad strips.

3) Advanced camera / original idea (BONUS).
	-I've wanted to incorporate the mouse into one of my programs for a while now.
	-Rather than following the guidelines for advanced camera, I've redefined my own.
	-When pressing the left/right arrow keys, the terrain will spin around it's center.
		-This looks like the camera is orbitting around the terrain, while looking at the center.
	-Rather than using the up/down keys, I've implemented the mouse instead.
		-When holding a pressing the left button, you have full control of the camera direction.
		-It will reset back to zero upon the release of the button and rotating the terrain (avoid glitching).

CONTROLS:

Use left and right arrow keys to rotate the camera around the grid. 
Use the mouse to control the camera's view.                      
Press 'r' to randomize the grid.                                 
Press 'R' to reset the camera and board.                                   
Press 'c' to switch between Circle and Fault algorithms.         
Press 'f' to switch between glat and smooth shading.             
Press 'w' to cycle through mesh, wireframe, and mesh + wireframe.
Press 'l' to toggle between light and colour mode.
Press 'q' to quit the program.                  
Press 't' to draw terrain with triangles.       
Press 'y' to draw terrain with quads.           
Press 'y' to draw terrain with quads.           
Control light #1 (yellow) with keys 'u' and 'i'.
Control light #2 (red) with keys 'o' and 'p'.