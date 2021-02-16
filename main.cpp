#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <math.h>
#define PI 3.1415926535
#define DR 0.0174533 // one degree in radians

typedef struct
{
    int z, q, s, d; // Button state (on/off)
} ButtonKeys;

ButtonKeys keys;

float px, py, pdx, pdy, pa; // player position
float pSpeed = 0.05;
float cameraSensitivity = 0.2;
float nbRays = 90;
float frame1, frame2, fps;
int mapX = 8, mapY = 8, mapS = 64;
int map[] = 
{
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 1, 0, 0, 1, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 1, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
};

float dist(float ax, float ay, float bx, float by)
{
    return sqrt((bx - ax) * (bx - ax) + (by - ay) * (by - ay));
}

void drawMap2D() {
    int x, y, xo, yo;
    for (y = 0; y < mapY; y++)
    {
        for (x = 0; x < mapX; x++)
        {
            if (map[y*mapX + x] == 1) 
            {
                glColor3f(1, 1, 1);
            }
            else 
            {
                glColor3f(0, 0, 0);
            }
            xo = x * mapS;
            yo = y * mapS;
            glBegin(GL_QUADS);
            glVertex2i(xo + 1, yo + 1);
            glVertex2i(xo + 1, yo + mapS - 1);
            glVertex2i(xo + mapS - 1, yo + mapS - 1);
            glVertex2i(xo + mapS - 1, yo + 1);
            glEnd();
        }
    }
}

void drawPlayer()
{
    glColor3f(1, 1, 0);
    glPointSize(8);
    glBegin(GL_POINTS);
    glVertex2i(px, py);
    glEnd();

    glLineWidth(3);
    glBegin(GL_LINES);
    glVertex2i(px, py);
    glVertex2i(px + pdx * 5, py + pdy * 5);
    glEnd();
}

void buttonDown(unsigned char key, int x, int y)
{
    if (key == 'z') keys.z = 1;
    if (key == 'q') keys.q = 1;
    if (key == 's') keys.s = 1;
    if (key == 'd') keys.d = 1;
    glutPostRedisplay();
}

void buttonUp(unsigned char key, int x, int y)
{
    if (key == 'z') keys.z = 0;
    if (key == 'q') keys.q = 0;
    if (key == 's') keys.s = 0;
    if (key == 'd') keys.d = 0;
    glutPostRedisplay();
}

void drawRays3D()
{
    int r, mx, my, mp, dof;
    float hx, hy, vx, vy, rx, ry, ra, xo, yo, rDist;
    ra = pa - DR * 45;
    if (ra < 0) ra += 2*PI;
    if (ra > 2*PI) ra -= 2*PI;

    for (r = 0; r < nbRays; r++)
    {
        // Check horizontal lines
        dof = 0;
        float rInvTan = 1 / tan(ra);
        if (ra < PI) {
            // Looking up
            hy = (((int)py >> 6) << 6) - 0.0001; // round to the nearest multiple of 64
            hx = (py - hy) * rInvTan + px;
            yo = -64;
            xo = -yo * rInvTan;
        }
        if (ra > PI) {
            // Looking down
            hy = (((int)py >> 6) << 6) + 64; // round to the nearest multiple of 64
            hx = (py - hy) * rInvTan + px;
            yo = 64;
            xo = -yo * rInvTan;
        }
        if (ra == 0 || ra == PI)
        {
            // Looking straight left or right
            hy = py;
            hx = px;
            dof = 8;
        }

        while (dof < 8)
        {
            mx = (int) (hx) >> 6;
            my = (int) (hy) >> 6;
            mp = my * mapX + mx;
            if (mp > 0 && mp < mapX * mapY && map[mp] == 1)
            {
                // Hit a wall
                dof = 8;
            }
            else
            {
                // Next horizontal line
                hx += xo;
                hy += yo;
                dof += 1;
            }
        }

        // Check vertical lines
        dof = 0;
        float rTan = tan(ra);
        if (ra < PI / 2 || ra > 3 * PI /2) {
            // Looking right
            vx = (((int)px >> 6) << 6) + 64; // round to the nearest multiple of 64
            vy = (px - vx) * rTan + py;
            xo = 64;
            yo = -xo * rTan;
        }
        if (ra > PI / 2 && ra < 3 * PI / 2) {
            // Looking left
            vx = (((int)px >> 6) << 6) - 0.0001; // round to the nearest multiple of 64
            vy = (px - vx) * rTan + py;
            xo = -64;
            yo = -xo * rTan;
        }
        if (ra == PI / 2 || ra == 3 * PI / 2)
        {
            // Looking straight up or down
            vy = py;
            vx = px;
            dof = 8;
        }

        while (dof < 8)
        {
            mx = (int) (vx) >> 6;
            my = (int) (vy) >> 6;
            mp = my * mapX + mx;
            if (mp > 0 && mp < mapX * mapY && map[mp] == 1)
            {
                // Hit a wall
                dof = 8;
            }
            else
            {
                // Next horizontal line
                vx += xo;
                vy += yo;
                dof += 1;
            }
        }

        if (dist(px, py, hx, hy) < dist(px, py, vx, vy))
        {
            // Horizontal
            rx = hx;
            ry = hy;
            rDist = dist(px, py, hx, hy);
            glColor3f(0, 0.9, 0);
        }
        else 
        {
            // Vertical
            rx = vx;
            ry = vy;
            rDist = dist(px, py, vx, vy);
            glColor3f(0, 0.6, 0);
        }

        // Draw 2D ray
        glLineWidth(1);
        glBegin(GL_LINES);
        glVertex2i(px, py);
        glVertex2i(rx, ry);
        glEnd();

        // Draw 3D walls

        // Fix "fisheye effect"
        float ca = pa - ra;
        if (ca < 0) ca += 2 * PI;
        if (ca > 2 * PI) ca -= 2 * PI;
        rDist = rDist * cos(ca);

        float lineH = (mapS * 500) / rDist; // Line height
        if (lineH > 500) lineH = 500;
        float lineO = 256 - lineH / 2; // Line offset (from the top)
        float drawOffset = 530; // 3D view offset (from the left)
        
        glLineWidth(8);
        glBegin(GL_LINES);
        glVertex2i((nbRays - r) * (1024 - drawOffset - 10) / 90 + drawOffset, lineO);
        glVertex2i((nbRays - r) * (1024 - drawOffset - 10) / 90 + drawOffset, lineO + lineH);
        glEnd();

        // Next ray angle
        ra += DR;
        if (ra < 0) ra += 2*PI;
        if (ra > 2*PI) ra -= 2*PI;
    }
}

void display()
{
    // Frames per second
    frame2 = glutGet(GLUT_ELAPSED_TIME);
    fps = frame2 - frame1;
    frame1 = glutGet(GLUT_ELAPSED_TIME);

    // Check buttons (on/off)
    if (keys.q) pa += cameraSensitivity * DR * fps;       
    if (keys.d) pa -= cameraSensitivity * DR * fps;

    if (pa < 0) pa += 2*PI;
    if (pa > 2*PI) pa -= 2*PI;

    pdx = cos(pa) * 5;
    pdy = - sin(pa) * 5;

    // Variables for collision checking
    int colOffset = 10;
    int xo = 0;
    if (pdx < 0) {xo = -colOffset;} else {xo = colOffset;}

    int yo = 0;
    if (pdy < 0) {yo = -colOffset;} else {yo = colOffset;}

    int mpx = px / 64;
    int mpx_add_xo = (px + xo) / 64;
    int mpx_sub_xo = (px - xo) / 64;

    int mpy = py / 64;
    int mpy_add_yo = (py + yo) / 64;
    int mpy_sub_yo = (py - yo) / 64;

    if (keys.z)
    {
        // Check collision
        if (map[mpy * mapX + mpx_add_xo] == 0) px += pdx * fps * pSpeed;
        if (map[mpy_add_yo * mapX + mpx] == 0) py += pdy * fps * pSpeed;
    }
    if (keys.s)
    {
        if (map[mpy * mapX + mpx_sub_xo] == 0) px -= pdx * fps * pSpeed;
        if (map[mpy_sub_yo * mapX + mpx] == 0) py -= pdy * fps * pSpeed;
    }
    glutPostRedisplay();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawMap2D();
    drawPlayer();
    drawRays3D();
    glutSwapBuffers();
}

void init()
{
    glClearColor(0.3, 0.3, 0.3, 0);
    gluOrtho2D(0, 1024, 512, 0);
    px = 1.5 * mapS;
    py = 1.5 * mapS;
    pa = 0;
    pdx = cos(pa) * 5;
    pdy = sin(pa) * 5;
}

void resize(int w, int h)
{
    glutReshapeWindow(1024, 512);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(1024, 512);
    glutCreateWindow("DoomLike");
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(buttonDown);
    glutKeyboardUpFunc(buttonUp);
    glutReshapeFunc(resize);
    glutMainLoop();
}