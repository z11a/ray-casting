#include <GL\glew.h>
#include <GL\freeglut.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>

void display();
void resize(int width, int height);
void init();
void drawPlayer();
float distance(float ax, float ay, float bx, float by);
void drawRays();
void drawBox();
void drawMap(int map[]);
void keyHandler(unsigned char key, int x, int y);
void collision();
void lose_screen();

// vars
const float PI = 3.14159265359;
const float degree1 = 0.0174533;
float px, py, pDeltaX, pDeltaY, pAngle;          // player position
bool lost;                                       // turns true when game is lost

// 3d-screen
const int screenHeight = 320, screenWidth = 160;

// map
int mapWidth = 8;
int mapHeight = 8;
int mapBlockSize = 50;
int map[] =                
{ 1, 1, 1, 1, 1, 1, 1, 1,
  1, 0, 0, 0, 0, 0, 0, 1,
  1, 0, 1, 0, 0, 0, 0, 1,
  1, 0, 0, 0, 0, 0, 0, 1,
  1, 0, 0, 0, 0, 1, 1, 1,
  1, 0, 0, 0, 0, 0, 0, 1,
  1, 0, 0, 0, 0, 0, 0, 1,
  1, 1, 1, 1, 1, 1, 1, 1 };

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE);
    glutInitWindowSize(1280, 512);     // window size
    glutInitWindowPosition(300, 300);  // distance from the top-left screen
    glutCreateWindow("Zea");           // window name
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(resize);
    glutKeyboardFunc(keyHandler);
    glutMainLoop();
    return 0;
}
void display(void) {
    if (!lost) { // havent lost yet
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // buffers = array of memory for pixel info or something

        // draw
        drawMap(map);
        collision();
		drawRays();
        drawPlayer();

        glutSwapBuffers();
    }
    else { // u lost
        glClear(GL_COLOR_BUFFER_BIT);

        //draw
        lose_screen();

        glutSwapBuffers();
    }
}
void resize(int width, int height) {
    glutReshapeWindow(1280, 512);
}
void init() {
    glClearColor(0.3, 0.3, 0.3, 0);
    gluOrtho2D(0, 1280, 512, 0);

    px = 200;
    py = 200;
    pAngle = 0 + PI / 4;
    pDeltaX = cos(pAngle);
    pDeltaY = sin(pAngle);
    lost = false;
}
void drawPlayer() {
    // Player Box
    glColor3f(1, 1, 0);
    glPointSize(8);
    glBegin(GL_POINTS);
    glVertex2i(px, py);
    glEnd();

    // Player Angle Stick
    glLineWidth(3);
    glBegin(GL_LINES);
    glVertex2i(px, py);
    glVertex2i(px + pDeltaX * 25, py + pDeltaY * 25);
    glEnd();
}
void drawMap(int map[]) {
	int x, y;
	for (y = 0; y < mapHeight; y++) {                               // rows
		for (x = 0; x < mapWidth; x++) {                            // each block
			if (map[y * mapWidth + x] == 1) {                       // 1 = black, 0 = white
				glColor3f(255, 255, 255);
			}
			else {
				glColor3f(0, 0, 0);
			}
			glBegin(GL_QUADS);
			glVertex2i(x * mapBlockSize + 1, y * mapBlockSize + 1);           // the '1's are for making the outline
			glVertex2i(x * mapBlockSize + 1, y * mapBlockSize + mapBlockSize - 1);
			glVertex2i(x * mapBlockSize + mapBlockSize - 1, y * mapBlockSize + mapBlockSize - 1);
			glVertex2i(x * mapBlockSize + mapBlockSize - 1, y * mapBlockSize + 1);
			glEnd();
		}
	}
}
void keyHandler(unsigned char key, int x, int y) {
    // Reset
    if (key == 'r') {
        px = 512; 
        py = 256; 
        lost = false; 
    }

    // Movement
    if (key == 'w') {
        px += pDeltaX * 5;
        py += pDeltaY * 5;
    }
    if (key == 's') { 
        px -= pDeltaX * 5;
        py -= pDeltaY * 5; }

    // Rotation
    if (key == 'q') {
        pAngle -= PI / 30;          // turn left

        if (pAngle < 0) {   
            pAngle += 2 * PI;
        }
        pDeltaX = cos(pAngle);
        pDeltaY = sin(pAngle);
    }
    if (key == 'e') {
        pAngle += PI / 30;          // turn right

        if (pAngle > 2 * PI) {
            pAngle -= 2 * PI;
        }
        pDeltaX = cos(pAngle);
        pDeltaY = sin(pAngle);
    }
    glutPostRedisplay();
}
float distance(float ax, float ay, float bx, float by) {
	return (sqrt((bx - ax) * (bx - ax) + (by - ay) * (by - ay)));
}
void drawRays() {
	int rays, mapX, mapY, mapPos, depth;
	float rayX, rayY, rayAngle, xOffset, yOffset, horiDist, vertDist, finalDist, lineHeight, lineOffset;

	rayAngle = pAngle - degree1 * 30;					// starts with left most angle

	if (rayAngle < 0) {
		rayAngle += 2 * PI;
	}
	else if (rayAngle > 2 * PI) {
		rayAngle -= 2 * PI;
	}

	for (rays = 0; rays < 90; rays++) {
		depth = 0;
		float horiDist = 9999999;							 
		float horiX = px, horiY = py;
		float invTan = -1 / tan(rayAngle);

		// Horizontal Lines
		if (rayAngle > PI) {								// looking up, because going down in the screen is higher y coordinate
			rayY = py - ((int)py % mapBlockSize);
			rayX = (py - rayY) * invTan + px;
			yOffset = -mapBlockSize;
			xOffset = -(yOffset * invTan);

			while (depth < 8) {
				mapX = (int)rayX / mapBlockSize;
				mapY = (int)rayY / mapBlockSize - 1;        // for some reason while looking up, the ray stops at the wrong side of the block so we have to subtract 1
				mapPos = mapY * mapWidth + mapX;			

				if (mapPos > 0 && mapPos < sizeof(map) / sizeof(int) && map[mapPos] == 1) {     // hit wall
					horiX = rayX;
					horiY = rayY;
					horiDist = distance(horiX, horiY, px, py);
					depth = 8;
				}
				else {										// doesn't hit a wall, it repeats and checks the next square
					rayX += xOffset;
					rayY += yOffset;
					depth += 1;
				}
			}
		}
		else if (rayAngle < PI) {                   // looking down
			rayY = py - ((int)py % mapBlockSize) + mapBlockSize;
			rayX = (py - rayY) * invTan + px;
			yOffset = mapBlockSize;
			xOffset = -(yOffset * invTan);
		}
		else if (rayAngle == PI) {
			rayX = px;								// - mapBlockSize?
			rayY = py;
			depth = 8;
		}
		else if (rayAngle == 0) {
			rayX = px;								// + mapBlockSize?
			rayY = py;
			depth = 8;
		}

		while (depth < 8) {                        // this runs if the rayAngle is 0, PI, or looking down
			mapX = (int)rayX / mapBlockSize;
			mapY = (int)rayY / mapBlockSize;
			mapPos = mapY * mapWidth + mapX;

			if (mapPos > 0 && mapPos < sizeof(map) / sizeof(int) && map[mapPos] == 1) {     // hit wall
				horiX = rayX;
				horiY = rayY;
				horiDist = distance(horiX, horiY, px, py);
				depth = 8;
			}
			else {
				rayX += xOffset;
				rayY += yOffset;
				depth += 1;
			}
		}

		// Vertical Lines
		depth = 0;
		float vertDist = 9999999;
		float vertX = px, vertY = py;
		float negTan = -tan(rayAngle);

		if (rayAngle > PI / 2 && rayAngle < 3 * PI / 2) {                        // looking left
			rayX = px - ((int)px % mapBlockSize);
			rayY = (px - rayX) * negTan + py;
			xOffset = -mapBlockSize;
			yOffset = -(xOffset * negTan);

			while (depth < 8) {
				mapX = (int)rayX / mapBlockSize - 1;
				mapY = (int)rayY / mapBlockSize;
				mapPos = mapY * mapWidth + mapX;

				if (mapPos > 0 && mapPos < sizeof(map) / sizeof(int) && map[mapPos] == 1) {     // hit wall
					vertX = rayX;
					vertY = rayY;
					vertDist = distance(vertX, vertY, px, py);
					depth = 8;
				}
				else {
					rayX += xOffset;
					rayY += yOffset;
					depth += 1;
				}
			}
		}
		else if (rayAngle < PI / 2 || rayAngle > 3 * PI / 2) {                   // looking right
			rayX = px - ((int)px % mapBlockSize) + mapBlockSize;
			rayY = (px - rayX) * negTan + py;
			xOffset = mapBlockSize;
			yOffset = -(xOffset * negTan);
		}
		else if (rayAngle == PI / 2) {
			rayX = px;
			rayY = py + mapBlockSize;
			depth = 8;
		}
		else if (rayAngle == 3 * PI / 2) {
			rayX = px;
			rayY = py - mapBlockSize;
			depth = 8;
		}

		while (depth < 8) {                        // this runs if the rayAngle is 0, PI, or looking down
			mapX = (int)rayX / mapBlockSize;
			mapY = (int)rayY / mapBlockSize;
			mapPos = mapY * mapWidth + mapX;

			if (mapPos > 0 && mapPos < sizeof(map) / sizeof(int) && map[mapPos] == 1) {     // hit wall 
				vertX = rayX;
				vertY = rayY;
				vertDist = distance(vertX, vertY, px, py);
				depth = 8;
			}
			else {
				rayX += xOffset;
				rayY += yOffset;
				depth += 1;
			}
		}

		// Shortest Ray
		if (vertDist < horiDist) {			
			rayX = vertX;
			rayY = vertY;
			finalDist = vertDist;
			glColor3f(0, 0, 0.7);
		}
		if (horiDist < vertDist) {
			rayX = horiX;
			rayY = horiY;
			finalDist = horiDist;
			glColor3f(0, 0, 0.9);
		}
		glLineWidth(2);
		glBegin(GL_LINES);
		glVertex2i(px, py);
		glVertex2i(rayX, rayY);
		glEnd();

		// testing to make sure rays were calculated properly

		/*std::cout << "playerPos: (" << px << ", " << py << ")" << ", ";		
		std::cout << "ray: (" << rayX << ", " << rayY << ")" << ", ";
		std::cout << "hori: (" << horiX << ", " << horiY << ")" << ", ";
		std::cout << "vert: (" << vertX << ", " << vertY << ")" << ", ";
		std::cout << "horiDist: " << horiDist << ", " << "vertDist: " << vertDist << std::endl;*/ 

		// 3D
		lineHeight = (mapBlockSize * screenHeight) / finalDist;
		if (finalDist > screenHeight) {
			finalDist = screenHeight;
		}

		lineOffset = screenHeight - lineHeight / 2;

		glLineWidth(8);
		glBegin(GL_LINES);
		glVertex2i(rays * 8 + 480, lineOffset);
		glVertex2i(rays * 8 + 480, lineHeight + lineOffset);
		glEnd();

		// Drawing cone
		rayAngle += degree1;
		if (rayAngle < 0) {
			rayAngle += 2 * PI;
		}
		else if (rayAngle > 2 * PI) {
			rayAngle -= 2 * PI;
		}
	}
}
void collision() {
	int mapX = (int)px / mapBlockSize;
	int mapY = (int)py / mapBlockSize;
	int mapPos = mapY * mapWidth + mapX;
}
void lose_screen() {
    glColor3f(255, 0, 0);
    glPointSize(80);
    glBegin(GL_POINTS);
    glVertex2i(512, 256);
    glEnd();
}	