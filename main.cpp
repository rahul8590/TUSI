
#include <GL/glut.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.147159265358

int W, H;
float qW, qH;
float bW, bH;

float gearFactor;

int draggin;
int stopGame;
int gameRunnin;

float timeRunnin;

float distanceBetweenPoints( float, float, float, float );


// glut fonts :
LPVOID glutFonts[7] = {
    GLUT_BITMAP_9_BY_15,
    GLUT_BITMAP_8_BY_13,
    GLUT_BITMAP_TIMES_ROMAN_10,
    GLUT_BITMAP_TIMES_ROMAN_24,
    GLUT_BITMAP_HELVETICA_10,
    GLUT_BITMAP_HELVETICA_12,
    GLUT_BITMAP_HELVETICA_18
};

// print text :
void glutPrint(float x, float y, LPVOID font, char* text, float r, float g, float b, float a)
{
    if(!text || !strlen(text)) return;
    bool blending = false;
    if(glIsEnabled(GL_BLEND)) blending = true;
    glEnable(GL_BLEND);
    glColor4f(r,g,b,a);
    glRasterPos2f(x,y);
    while ( *text ) {
        glutBitmapCharacter(font, *text);
        text++;
    }
    if(!blending) glDisable(GL_BLEND);
}


struct Circle{
    float cenX, cenY;
    float radius;
    float cI, cJ, cK;

    //movemen :
    float m, c;
    float theta;
    int LR;

    static int count;

    void drawCircle( );

    void translateCircle( );
    void translateCircle_Bonus( );

    int clickedWithin( int, int );
    int collisionDetector( );

    // void initialize( centreX, centreY, radius, red, green, blue, theta in degrees ( 0<abs(theta)<90 ), y intercept );
    void initialize( float X, float Y, float R, float I, float J, float K, float Theta, float C ){
        count++;
        cenX = X; cenY = Y;
        radius = R;
        cI = I; cJ = J; cK = K;
        theta = Theta;
        m = tan( Theta * 3.14159265358 / 180.0 );   c = C;

        return;
    }
}circle[5], myBall;
int Circle::count;

void Circle::drawCircle( ){
    glColor3f( cI, cJ, cK );

    glBegin( GL_TRIANGLE_FAN );
        glVertex2f( cenX, cenY );

        glColor3f( cI/5, cJ/5, cK/5 );
        for( float x=-radius; x<=radius; x+=1 ){
            float y = sqrt( radius*radius - x*x );
            glVertex2f( x+cenX, y+cenY );
        }
        for( float x=-radius; x<=radius; x+=1 ){
            float y = sqrt( radius*radius - x*x );
            glVertex2f( x+cenX, -y+cenY );
        }
    glEnd( );

    //glFlush();
    return;
}

#define TRANSLATEDISTANCE 8
void Circle::translateCircle( ){
    float d = TRANSLATEDISTANCE + gearFactor;

    cenY = d * sin( theta * PI / 180 ) + cenY;
    cenX = d * cos( theta * PI / 180 ) + cenX;

    return;
}

#define TRANSLATE_BONUS_DISTANCE 9
void Circle::translateCircle_Bonus( ){
    float d = TRANSLATE_BONUS_DISTANCE + gearFactor;

    cenY = d * sin( theta * PI / 180 ) + cenY;
    cenX = d * cos( theta * PI / 180 ) + cenX;

    return;
}

int Circle::clickedWithin( int X, int Y ){
    X = (int)( X - qW/2 );
    Y = -(int)( Y - qH/2 );
    if( distanceBetweenPoints( cenX, cenY, X, Y ) <= radius ){
        return 1;
    }
    return 0;
}

int Circle::collisionDetector( ){
    if( cenX + radius >= qW/2 || cenX - radius <= -qW/2 || cenY + radius >= qH/2  || cenY - radius <= -qH/2 ){
        return 1;
    }
    for( int i=0; i<Circle::count; i++ ){
        if( distanceBetweenPoints( circle[i].cenX, circle[i].cenY, cenX, cenY ) <= radius + circle[i].radius ){
            return 1;
        }
    }
    return 0;
}

float distanceBetweenPoints( float a, float b, float c, float d ){
    return( sqrt( pow( (a-c), 2 ) + pow( (b-d), 2 ) ) );
}

void wallCollisionDetector( ){
    for( int i=0; i<Circle::count; i++ ){
        if( circle[i].cenX + circle[i].radius >= bW/2 || circle[i].cenX - circle[i].radius <= -bW/2 ){
            circle[i].theta = (int)( 180 - circle[i].theta ) % 360;
            circle[i].m = tan( circle[i].theta * PI / 180.0 );
            circle[i].c = circle[i].cenY - circle[i].m * circle[i].cenX;
            circle[i].translateCircle_Bonus( );
        }
        if( circle[i].cenY + circle[i].radius >= bH/2 || circle[i].cenY - circle[i].radius <= -bH/2 ){
            circle[i].theta = (int)( 360 - circle[i].theta ) % 360;
            circle[i].m = tan( circle[i].theta * PI / 180.0 );
            circle[i].c = circle[i].cenY - circle[i].m * circle[i].cenX;
            circle[i].translateCircle_Bonus( );
        }
    }
    return;
}

float distanceBetweenCentres( int a, int b ){
    return( sqrt( pow( circle[a].cenX - circle[b].cenX, 2 ) + pow( circle[a].cenY - circle[b].cenY, 2 ) ) );
}

float getCollisionAngleFerBallA( int a, int b ){
    float thetaInRadians = atan( ( circle[b].cenY - circle[a].cenY ) / ( circle[b].cenX - circle[a].cenX ) );
    return( (float)( (int)( thetaInRadians * 180.0 / 3.14159265358 ) % 360 ) );
}

#define MIN_INTER_BALL_GAP 3
void ballCollisionDetector( ){
    for( int i=0; i < Circle::count; i++ ){
        for( int j=i+1; j < Circle::count; j++ ){
            if( i!=j && distanceBetweenCentres( i, j ) <= ( circle[i].radius + circle[j].radius + MIN_INTER_BALL_GAP ) ){
                float cThetaA;
                cThetaA = getCollisionAngleFerBallA( i, j );
                float cThetaB = (int)( cThetaA + 180 ) % 360;

                circle[i].theta = (int)( circle[i].theta + 180 + 2 * ( cThetaA - circle[i].theta ) ) % 360;
                circle[j].theta = (int)( circle[j].theta + 180 + 2 * ( cThetaB - circle[j].theta ) ) % 360;

                circle[i].m = tan( circle[i].theta * PI / 180 );
                circle[j].m = tan( circle[j].theta * PI / 180 );

                circle[i].c = circle[i].cenY - circle[i].m * circle[i].cenX;
                circle[j].c = circle[j].cenY - circle[j].m * circle[j].cenX;

                circle[i].translateCircle_Bonus( );
                circle[j].translateCircle_Bonus( );

            }
        }
    }


    return;
}

void setupRC( ){
    glClearColor( 0.10250, 0.10250, 0.10250, 0.0 );
    glClear( GL_COLOR_BUFFER_BIT );
    glLineWidth( 2.4 );
    return;
}

void resizeHandler( GLsizei w, GLsizei h ){
    if( h==0 ) h++;

    GLfloat aspecRatio = (GLfloat)w / (GLfloat)h;

    glViewport( 0, 0, w, h );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );

    if( w <= h ){
        glOrtho( -W/2, W/2, -H/2/aspecRatio, H/2/aspecRatio, -4, +4 );
        qH = (GLfloat) H/aspecRatio;
    }
    else{
        glOrtho( -W/2*aspecRatio, W/2*aspecRatio, -H/2, H/2, -4, +4 );
        qW = (GLfloat) W*aspecRatio;
    }

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity( );

    return;
}

void drawBoundingBox( ){
    glColor3f( 0.0, 0.0, 0.0 );
    glBegin( GL_POLYGON );
        glVertex2f( -bW/2, -bH/2 );
        glVertex2f( -bW/2, bH/2 );
        glVertex2f( bW/2, bH/2 );
        glVertex2f( bW/2, -bH/2 );
    glEnd( );
    return;
}

void drawCrossOnMyBall( ){
    glColor3f( 0.680, 0.560, 0.0 );
    glBegin( GL_LINES );
        glVertex2f( myBall.cenX-8, myBall.cenY+8 );
        glVertex2f( myBall.cenX+8, myBall.cenY-8 );
        glVertex2f( myBall.cenX-8, myBall.cenY-8 );
        glVertex2f( myBall.cenX+8, myBall.cenY+8 );
    glEnd( );

    return;
}


void renderScene( ){
    glClear( GL_COLOR_BUFFER_BIT );

    drawBoundingBox( );

    for( int i=0; i<Circle::count; i++ ){
        circle[i].drawCircle( );
    }
    myBall.drawCircle( );
    drawCrossOnMyBall( );
    char *s = new char[100];
    sprintf( s, "Time Runnin : %.3f", timeRunnin/10 );
    glutPrint( 0.0, -(qH/2-10), glutFonts[0], s, 1.0f, 1.0f, 1.0f, 2.0f);

    if( stopGame == 1 ){
        glutPrint(-80.0f, 0.0f, glutFonts[1], "Click To Start New Game.", 1.0f, 0.0f, 0.0f, 0.5f);
    }

    glFlush( );
    return;
}

void timerFunction( int value ){
    if( gameRunnin == 1 && stopGame == 0 ){
        timeRunnin += 1;
        if( (int)timeRunnin % 40 == 0 ){
            gearFactor+=0.36;
        }
    }

    wallCollisionDetector( );
    ballCollisionDetector( );

    if( stopGame == 0 && gameRunnin == 1 ){
        circle[0].translateCircle( );
        circle[1].translateCircle( );
        circle[2].translateCircle( );
        circle[3].translateCircle( );
        circle[4].translateCircle( );
    }
    renderScene( );
    if( myBall.collisionDetector( ) ){
        stopGame = 1;
        draggin = 0;
    }
    glutTimerFunc( 24, timerFunction, 1 );
    return;
}


void initBalls( ){
    Circle::count = 0;
    circle[0].initialize( -80.0, 20.0, 24.0, 0.80, 0.830, 0.020, 45, 0.0 );
    circle[1].initialize( 120.0, 20.0, 24.0, 0.773985, 0.682, 0.631, 180+45, 0.0 );
    circle[2].initialize( 40.0, -90.0, 24.0, 0.215, 0.64145, 0.611, 60, 78.0 );
    circle[3].initialize( -140.0, -100.0, 24.0, 0.9479199, 0.481, 0.481, 183, 5.2 );
    circle[4].initialize( 0.0, 100.0, 24.0, 0.495, 0.3495, 0.9481, -29, -23.5 );
    myBall.initialize( -120.0, 120.0, 30.0, 1.0, 1.0, 1.0, 0.0, 0.0 );  Circle::count--;
    draggin = 0;
    stopGame = 0;
    gameRunnin = 0;
    timeRunnin = 0;
    gearFactor = 0;

    return;
}

void resetGame( int );
void mouseMotionHandler( int x, int y ){
    x = (int)( x - qW/2 );
    y = -(int)( y - qH/2 );

    if( draggin == 1 ){
        myBall.cenX = x;
        myBall.cenY = y;
    }

    return;
}

void mouseClickHandler( int button, int state, int x, int y ){
    if( stopGame == 1 && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN ){
        glutTimerFunc( 100, resetGame, 1 );
    }
    if( gameRunnin == 0 && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN ){
        gameRunnin = 1;
    }
    if( button == GLUT_LEFT_BUTTON && state == GLUT_UP ){
        draggin = 0;
    }
    else if( button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && myBall.clickedWithin( x, y ) && stopGame == 0 ){
        draggin = 1;
    }
    return;
}

void resetGame( int value ){
    initBalls( );
    glutTimerFunc( 100, timerFunction, 1 );
    return;
}

int main( int argc, char * argv[] ){
    initBalls( );

    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_SINGLE | GLUT_RGB );
    W=400; H=400;   qW = W; qH = H;
    bW = W - 20; bH = H - 20;
    glutInitWindowSize( W, H );
    glutInitWindowPosition( 100, 100 );
    glutCreateWindow( "ballE" );

    glutDisplayFunc( renderScene );
    glutReshapeFunc( resizeHandler );
    glutTimerFunc( 1, timerFunction, 1 );
    glutMouseFunc( mouseClickHandler );
    glutMotionFunc( mouseMotionHandler );

    setupRC( );

    glutMainLoop( );

    return 0;
}
