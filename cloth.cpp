// blaws
// November 7, 2014
// Cloth

#include "Angel.h"
#include <vector>
#include <fstream>

#define TWODIM(x,y) ( (x) + (y) * meshSize )

//////////////////////////////////////////////////////////////////////////////

// window size and framerate
int windowW( 750 );
int windowH( 750 );
const float fps( 30.0 );
const float spf( 1.0 / fps );
const float mspf( 1000.0 * spf );

// options
const float meshSeparation( 0.1 );
int meshSize( 40 );
vec3 grav( 0.0, 0.0, -2.0 );
int numConstraintReps( 10 );
bool wireFrame( false );
bool resetHorizontal( true );

// object data
std::vector<vec3> vert;
std::vector<vec3> prevVert;
std::vector<vec2> texcoord;
std::vector<vec3> normal;
std::vector<GLushort> indices;
GLuint clothTexture;

// view
vec3 lookAt( meshSize * meshSeparation * 0.5, meshSize * meshSeparation * 0.5, 0.0 );
vec3 camera;
const vec3 up( 0.0, 0.0, 1.0 );
float cameraR( 2.5 * meshSize * meshSeparation );
float thetaXY( 1.5 * M_PI );
float thetaZ( 0.25 * M_PI );
int mouseX;
int mouseY;

// matrices
mat4 projMatrix;
mat4 mvMatrix;

// shader handles
GLuint objectBuffer;
GLuint indexBuffer;
GLuint mvMatrixLoc;
GLuint vertLoc;
GLuint texCoordLoc;
GLuint normalLoc;
GLuint textureLoc;

//////////////////////////////////////////////////////////////////////////////

void init()
{
    srand( time( NULL ) );
    glClearColor( 1.0, 1.0, 1.0, 1.0 );
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_TEXTURE_2D );
    glPixelStorei( GL_PACK_ALIGNMENT, 1 );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
}

void setView()
{
    camera.x = lookAt.x + cameraR * cos( thetaXY ) * cos( thetaZ );
    camera.y = lookAt.y + cameraR * sin( thetaXY ) * cos( thetaZ );
    camera.z = lookAt.z + cameraR * sin( thetaZ );
    
    projMatrix = Perspective( 45.0, 1.0, 0.1, 2.0 * std::fmax( cameraR, 1.0 ) );
    mvMatrix = LookAt( camera, lookAt, up );
}

void createMeshNormals()
{
    normal.clear();
    
    for( int y = 0; y < meshSize; ++y )
    {
        for( int x = 0; x < meshSize; ++x )
        {
            vec3 self = vert[ TWODIM( x, y ) ];
            vec3 west(self), east(self), south(self), north(self);
            if( x > 0 )
            {
                west = vert[ TWODIM( x - 1, y ) ];
            }
            if( x < meshSize - 1 )
            {
                east = vert[ TWODIM( x + 1, y ) ];
            }
            if( y > 0 )
            {
                south = vert[ TWODIM( x, y - 1 ) ];
            }
            if( y < meshSize - 1 )
            {
                north = vert[ TWODIM( x, y + 1 ) ];
            }
            
            vec3 xDiff( east.x - west.x, east.y - west.y, east.z - west.z );
            vec3 yDiff( north.x - south.x, north.y - south.y, north.z - south.z );
            
            normal.push_back( normalize( cross( xDiff, yDiff ) ) );
        }
    }
}

void rebuildMesh()
{
    // reset
    vert.clear();
    texcoord.clear();
    indices.clear();
    
    // create mesh of squares
    for( int j = 0; j < meshSize; ++j )
    {
        for( int i = 0; i < meshSize; ++i )
        {
            vert.push_back( vec3( resetHorizontal * meshSeparation * i,
                                  meshSeparation * j,
                                  meshSize * meshSeparation * 0.5 - (!resetHorizontal) * meshSeparation * i ) );
            
            texcoord.push_back( vec2( i / (float)meshSize, j / (float)meshSize ) );

            if( i && j )
            {
                indices.push_back( TWODIM( i - 1, j - 1 ) );
                indices.push_back( TWODIM( i, j - 1 ) );
                indices.push_back( TWODIM( i, j ) );

                indices.push_back( TWODIM( i, j ) );
                indices.push_back( TWODIM( i - 1, j ) );
                indices.push_back( TWODIM( i - 1, j - 1 ) );
            }
        }
    }
    
    prevVert = vert;
    createMeshNormals();
}

void sendData()
{
    // vertices
    glBufferData( GL_ARRAY_BUFFER, ( vert.size() + normal.size() ) * sizeof( vec3 ) + texcoord.size() * sizeof( vec2 ), NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, vert.size() * sizeof( vec3 ), vert[0] );
    glEnableVertexAttribArray( vertLoc );
    glVertexAttribPointer( vertLoc, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( 0 ) );
    
    // indices
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof( GLushort ), &indices[0], GL_STATIC_DRAW );
    
    // texture coordinates
    glBufferSubData( GL_ARRAY_BUFFER, vert.size() * sizeof( vec3 ), texcoord.size() * sizeof( vec2 ), texcoord[0] );
    glEnableVertexAttribArray( texCoordLoc );
    glVertexAttribPointer( texCoordLoc, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( vert.size() * sizeof( vec3 ) ) );
     
    // normals
    glBufferSubData( GL_ARRAY_BUFFER, vert.size() * sizeof( vec3 ) + texcoord.size() * sizeof( vec2 ), normal.size() * sizeof( vec3 ), normal[0] );
    glEnableVertexAttribArray( normalLoc );
    glVertexAttribPointer( normalLoc, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET( vert.size() * sizeof( vec3 ) + texcoord.size() * sizeof( vec2 ) ) );
}

void setupData()
{
    // build mesh
    rebuildMesh();

    // matrices
    setView();
    
    // create shader program
    GLuint sProgram = InitShader( "vert.glsl", "frag.glsl" );
    
    // object buffer
    glGenBuffers( 1, &objectBuffer );
    glBindBuffer( GL_ARRAY_BUFFER, objectBuffer );
    
    // vertices
    vertLoc = glGetAttribLocation( sProgram, "a_vPosition" );

    // indices
    glGenBuffers( 1, &indexBuffer );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indexBuffer );

    // texture and coordinates
    textureLoc = glGetUniformLocation( sProgram, "u_sTexture" );
    glUniform1i( textureLoc, 0 );
    texCoordLoc = glGetAttribLocation( sProgram, "a_vTexCoord" );
    
    // normals
    normalLoc = glGetAttribLocation( sProgram, "a_vNormal" );
    
    // buffer to GPU
    sendData();
}

void display()
{
    // clear
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    // draw tree
    glUniformMatrix4fv( mvMatrixLoc, 1, GL_TRUE, projMatrix * mvMatrix );
    glDrawElements( GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, BUFFER_OFFSET( 0 ) );

    // flush to screen
    glutSwapBuffers();
}

void resize( int w, int h )
{
    windowW = w;
    windowH = h;
    glViewport( 0, 0, w, h );
}

void mouse( int button, int state, int x, int y )
{
    if( button == GLUT_LEFT_BUTTON && state == GLUT_DOWN )
    {
        mouseX = x;
        mouseY = y;
    }
    
    else if( button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN )
    {
        int meshY = x * meshSize / windowW;
        int meshX = y * meshSize / windowH;
        
        prevVert[ TWODIM( meshX, meshY ) ].x += 20.0 * meshSeparation;
    }
}

void mouseMove( int x, int y )
{
    int dx = mouseX - x;
    int dy = y - mouseY;
    
    thetaXY += dx / static_cast<float>( windowW / 2 );
    thetaZ += dy / static_cast<float>( windowH / 2 );

    if( thetaXY >= 2.0 * M_PI )
    {
        thetaXY -= 2.0 * M_PI;
    }
    else if( thetaXY < 0.0 )
    {
        thetaXY += 2.0 * M_PI;
    }
    if( thetaZ < -0.5 * M_PI + 0.01 )
    {
        thetaZ = -0.5 * M_PI + 0.01;
    }
    else if( thetaZ > 0.5 * M_PI - 0.01 )
    {
        thetaZ = 0.5 * M_PI - 0.01;
    }

    setView();
    
    mouseX = x;
    mouseY = y;
}

void keyboard( unsigned char key, int x, int y )
{
    switch( key )
    {
        case 'i':
        case 'I':
            cameraR -= 0.5 * meshSize * meshSeparation;
            if( cameraR < 0.5 )
            {
                cameraR = 0.5 * meshSize * meshSeparation;
            }
            break;
        case 'o':
        case 'O':
            cameraR += 0.5 * meshSize * meshSeparation;
            break;
            
        case 'W':
        case 'w':
            lookAt.z += meshSize * meshSeparation * 0.1;
            break;
        case 's':
        case 'S':
            lookAt.z -= meshSize * meshSeparation * 0.1;
            break;
        case 'A':
        case 'a':
            lookAt.x -= meshSize * meshSeparation * 0.1;
            break;
        case 'D':
        case 'd':
            lookAt.x += meshSize * meshSeparation * 0.1;
            break;
            
        case 'F':
        case 'f':
            --numConstraintReps;
            if( numConstraintReps < 1 )
            {
                numConstraintReps = 1;
            }
            break;
        case 'V':
        case 'v':
            ++numConstraintReps;
            break;
            
        case '=':
        case '+':
            grav *= 2.0;
            break;
        case '-':
        case '_':
            grav *= 0.5;
            break;

        case ']':
        case '}':
            meshSize *= 2.0;
            rebuildMesh();
            break;
        case '[':
        case '{':
            meshSize *= 0.5;
            rebuildMesh();
            break;
            
        case '\'':
            resetHorizontal = !resetHorizontal;
            rebuildMesh();
            break;

        case 'r':
        case 'R':
            rebuildMesh();
            break;
            
        case 13:   // ENTER
            wireFrame = !wireFrame;
            glPolygonMode( GL_FRONT_AND_BACK, wireFrame ? GL_LINE : GL_FILL );
            break;
            
        case 033:  // ESC
        case 'q':
        case 'Q':
            exit( EXIT_SUCCESS );
    }
    
    setView();
}

void stickConstraint( vec3& x1, vec3& x2, float d )
{
    vec3 delta = x2 - x1;
    float deltaLength = sqrt( dot( delta, delta ) );
    vec3 diff = ( deltaLength - d ) / deltaLength;
    x1 += delta * 0.5 * diff;
    x2 -= delta * 0.5 * diff;
}

void timeStep( int s )
{
    // save state
    std::vector<vec3> tmpVert( vert );

    // Verlet integration: velocity and acceleration (gravity)
    vec3 accel = grav * spf * spf;
    for( int j = 0; j < meshSize; ++j )
    {
        for( int i = 0; i < meshSize; ++i )
        {
            vec3 tmpPos = vert[ TWODIM( i, j ) ];
            vert[ TWODIM( i, j ) ] = 2.0 * vert[ TWODIM( i, j ) ] - prevVert[ TWODIM( i, j ) ] + accel;
            prevVert[ TWODIM( i, j ) ] = tmpPos;
        }
    }
    
    // satisfy constraints: repeating this section multiple times makes the constraints more rigid
    for( int rep = 0; rep < numConstraintReps; ++rep )
    {
        // constraint 1: hold mesh together
        for( int j = meshSize-1; j > -1; --j )  // system seems more stable when run in negative direction
        {
            for( int i = 0; i < meshSize; ++i )
            {
                if( i > 0 )
                {
                    stickConstraint( vert[ TWODIM( i, j ) ], vert[ TWODIM( i - 1, j ) ], meshSeparation );
                }
                if( j > 0 )
                {
                    stickConstraint( vert[ TWODIM( i, j ) ], vert[ TWODIM( i, j - 1 ) ], meshSeparation );
                }
                if( i < meshSize - 1 )
                {
                    stickConstraint( vert[ TWODIM( i, j ) ], vert[ TWODIM( i + 1, j ) ], meshSeparation );
                }
                if( j < meshSize - 1 )
                {
                    stickConstraint( vert[ TWODIM( i, j ) ], vert[ TWODIM( i, j + 1 ) ], meshSeparation );
                }
            }
        }
        
        // constraint 2: fix left edge at z = 0.0
        for( int j = 0; j < meshSize; ++j )
        {
            vert[ TWODIM( 0, j ) ] = vec3( 0.0, 0.1*j, meshSize * meshSeparation * 0.5 );
        }
    }
    
    // calculate new normals
    createMeshNormals();
    
    // save previous state
    prevVert = tmpVert;
    
    // update GPU
    sendData();

    glutPostRedisplay();
    glutTimerFunc( mspf, timeStep, s );
}

bool createTextureFromBMP( const char* path, GLuint& texture, GLuint texSlot = 0 )
{
    // open texture file
    std::ifstream inFile( path );
    if( !inFile )
    {
        std::cerr << "Cannot open " << path << ": " << strerror( errno ) << std::endl;
        return false;
    }
    
    // get size of image and discard header
    int width, height;
    inFile.ignore( 18 );
    inFile.read( (char*)&width, 4 );
    inFile.read( (char*)&height, 4 );
    inFile.ignore( 25 );
    
    // flip data if necessary
    int yStart = 0;
    int yInc = 1;
    if( height < 0 )
    {
        height *= -1;
        yStart = height - 1;
        yInc = -1;
    }
    
    // read data (BMP stores pixels in BGR format)
    GLubyte* image = new GLubyte[ width * height * 3 ];
    GLubyte* ptr = image + 2;
    for( int j = yStart; j > -1 && j < height; j += yInc )
    {
        int i;
        for( i = 0; i < width; ++i )
        {
            for( int k = 0; k < 3; ++k )
            {
                inFile.read( (char*)ptr, 1 );
                --ptr;
            }
            ptr += 6;
        }

        // remove padding bytes
        if( i % 4 )
        {
            inFile.ignore( 4 - ( i % 4 ) );
        }
    }
    
    // create texture
    glGenTextures( 1, &texture );
    glBindTexture( GL_TEXTURE_2D, texture );
    glActiveTexture( GL_TEXTURE0 + texSlot );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image );
    
    return true;
}

void createDefaultTexture( GLuint& texture, GLuint texSlot = 0 )
{
    GLubyte image[] = { 255, 255, 255 };
    
    glGenTextures( 1, &texture );
    glBindTexture( GL_TEXTURE_2D, texture );
    glActiveTexture( GL_TEXTURE0 );
    
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, image );
}


//////////////////////////////////////////////////////////////////////////////

int main( int argc, char** argv )
{
    // set up GLUT
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
    glutInitWindowSize( windowW, windowH );
    glutCreateWindow( "Cloth" );
    
    // change default settings
    init();

    // get texture if specified
    if( argc > 1 )
    {
        createTextureFromBMP( argv[1], clothTexture );
    }
    else
    {
        std::cout << "To add a bitmap texture, specify it on the command line:" << std::endl << argv[0] << " <BMP texture>" << std::endl;
    }

    if( !clothTexture )
    {
        createDefaultTexture( clothTexture );
    }
    
    // create view
    setupData();
    
    // set callbacks
    glutDisplayFunc( display );
    glutReshapeFunc( resize );
    glutMouseFunc( mouse );
    glutMotionFunc( mouseMove );
    glutKeyboardFunc( keyboard );
    glutTimerFunc( mspf, timeStep, 0 );
    
    // run
    glutMainLoop();
    return EXIT_SUCCESS;
}
