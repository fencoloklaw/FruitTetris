/*
CMPT 361 Assignment 1 - FruitTetris implementation Sample Skeleton Code

- This is ONLY a skeleton code showing:
How to use multiple buffers to store different objects
An efficient scheme to represent the grids and blocks

- Compile and Run:
Type make in terminal, then type ./FruitTetris

This code is extracted from Connor MacLeod's (crmacleo@sfu.ca) assignment submission
by Rui Ma (ruim@sfu.ca) on 2014-03-04. 

Modified in Sep 2014 by Honghua Li (honghual@sfu.ca).
*/

#include "include/Angel.h"
#include <cstdlib>
#include <iostream>
#include <vector>
#include <array>

using namespace std;

bool stopwatch = true;
bool speedup = false;
// xsize and ysize represent the window size - updated if window is reshaped to prevent stretching of the game
int xsize = 400; 
int ysize = 720;

// current tile
vec2 tile[4]; // An array of 4 2d vectors representing displacement from a 'center' piece of the tile, on the grid
vec2 tilepos = vec2(5, 19); // The position of the current tile using grid coordinates ((0,0) is the bottom left corner)

// An array storing all possible orientations of all possible tiles
// The 'tile' array will always be some element [i][j] of this array (an array of vec2)
/**vec2 allRotationsLshape[4][4] = 
	{{vec2(0, 0), vec2(-1,0), vec2(1, 0), vec2(-1,-1)},
	{vec2(0, 1), vec2(0, 0), vec2(0,-1), vec2(1, -1)},     
	{vec2(1, 1), vec2(-1,0), vec2(0, 0), vec2(1,  0)},  
	{vec2(-1,1), vec2(0, 1), vec2(0, 0), vec2(0, -1)}};**/
vec2 Sshape[4] = {vec2(-1, -1), vec2(0,-1), vec2(0, 0), vec2(1,0)};
vec2 Lshape[4] = {vec2(-1, -1), vec2(-1,0), vec2(0, 0), vec2(1,0)};
vec2 Ishape[4] = {vec2(2,0), vec2(-1,0), vec2(0,0), vec2(1,0)};
// colors
vec4 orange = vec4(1.0, 0.5, 0.0, 1.0); 
vec4 white  = vec4(1.0, 1.0, 1.0, 1.0);
vec4 black  = vec4(0.0, 0.0, 0.0, 1.0); 
vec4 red = vec4(1.0,0.0,0.0,1.0);
vec4 yellow = vec4(1.0,1.0,0.0,1.0);
vec4 purple = vec4(1.0,0.0,1.0,1.0);
vec4 green = vec4(0.0,1.0,0.0,1.0);
vector<vec4> cvector;


//board[x][y] represents whether the cell (x,y) is occupied
bool board[10][20]; 

//An array containing the colour of each of the 10*20*2*3 vertices that make up the board
//Initially, all will be set to black. As tiles are placed, sets of 6 vertices (2 triangles; 1 square)
//will be set to the appropriate colour in this array before updating the corresponding VBO
vec4 boardcolours[1200];

// location of vertex attributes in the shader program
GLuint vPosition;
GLuint vColor;

// locations of uniform variables in shader program
GLuint locxsize;
GLuint locysize;

// VAO and VBO
GLuint vaoIDs[3]; // One VAO for each object: the grid, the board, the current piece
GLuint vboIDs[6]; // Two Vertex Buffer Objects for each VAO (specifying vertex positions and colours, respectively)
vec4 newcolours[24];
bool collision(vec2 direction);
bool rotatecollision(vec2 temptile[]);
void rotate();
void restart();
//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
bool gameover(vec2 direction)
{
	bool col = false;// no collision
	int fx;
	int fy;
	for(int i = 0; i<4; i++)
	{
		fx = direction.x + tile[i].x;
		fy = direction.y + tile[i].y;
		if((board[fx][fy])&&(fx >= 0)&&(fy >= 0)&&(fx <= 9)&&(fy<=19))
		{
			col = true;

			//collision with board
		}
	}
	return col;
}
// When the current tile is moved or rotated (or created), update the VBO containing its vertex position data
void updatetile()
{
	// Bind the VBO containing current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]); 

	// For each of the 4 'cells' of the tile,
	for (int i = 0; i < 4; i++) 
	{
		// Calculate the grid coordinates of the cell
		GLfloat x = tilepos.x + tile[i].x; 
		GLfloat y = tilepos.y + tile[i].y;

		// Create the 4 corners of the square - these vertices are using location in pixels
		// These vertices are later converted by the vertex shader
		vec4 p1 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1); 
		vec4 p2 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);
		vec4 p3 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1);
		vec4 p4 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);

		// Two points are used by two triangles each
		vec4 newpoints[6] = {p1, p2, p3, p2, p3, p4}; 

		// Put new data in the VBO
		glBufferSubData(GL_ARRAY_BUFFER, i*6*sizeof(vec4), 6*sizeof(vec4), newpoints); 
	}

	glBindVertexArray(0);
}

//-------------------------------------------------------------------------------------------------------------------
// Called at the start of play and every time a tile is placed
void newtile()
{	
	// Update the geometry VBO of current tile
	int tilerand = rand() % 3;
	for (int i = 0; i < 4; i++)
	{
		if(tilerand == 0)
			tile[i] = Ishape[i];
		else if(tilerand == 1)
			tile[i] = Sshape[i];
		else
			tile[i] = Lshape[i];
	}
	bool boo = true;
	bool foo = true;
	int rot = -1;
	int xloc = 0;
	int yloc = 0;
	vec2 direction;
	while (foo){
		while(boo){
			//random location for the block
			xloc = rand() % 10;
			yloc = rand() % 3;
			direction = vec2(xloc, 19-yloc);
			if(gameover(direction))
			{
				//restart();
				stopwatch = false;
				return;
			}
			if(!collision(direction))
			{
				boo = false;
				tilepos = direction;
			}

		}
		//random rotation for the block
		rot = rand()%4;
		for(int k = 0; k<rot; k++){
			rotate();
		}
		if(!rotatecollision(tile))
		{
			foo = false;
		}
	}
	updatetile();
	//random colours
	int cvalue = -1;
	int oldcvalue = -1;
	// Update the color VBO of current tile
	//vec4 newcolours[24];
	//randomize colours
	for(int i = 0; i<4; i++)
	{
		int j = i*6;
		int jmax = j+6;
		while(cvalue == oldcvalue)
		{
			cvalue = rand() % cvector.size();
		}

		oldcvalue = cvalue;
		for(j; j<jmax; j++)
		{
			newcolours[j] = cvector.at(cvalue);
		}
	}
		// You should randomlize the color
	
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}
//-------------------------------------------------------------------------------------------------------------------

void initGrid()
{
	// ***Generate geometry data
	vec4 gridpoints[64]; // Array containing the 64 points of the 32 total lines to be later put in the VBO
	vec4 gridcolours[64]; // One colour per vertex
	// Vertical lines 
	for (int i = 0; i < 11; i++){
		gridpoints[2*i] = vec4((33.0 + (33.0 * i)), 33.0, 0, 1);
		gridpoints[2*i + 1] = vec4((33.0 + (33.0 * i)), 693.0, 0, 1);
		
	}
	// Horizontal lines
	for (int i = 0; i < 21; i++){
		gridpoints[22 + 2*i] = vec4(33.0, (33.0 + (33.0 * i)), 0, 1);
		gridpoints[22 + 2*i + 1] = vec4(363.0, (33.0 + (33.0 * i)), 0, 1);
	}
	// Make all grid lines white
	for (int i = 0; i < 64; i++)
		gridcolours[i] = white;


	// *** set up buffer objects
	// Set up first VAO (representing grid lines)
	glBindVertexArray(vaoIDs[0]); // Bind the first VAO
	glGenBuffers(2, vboIDs); // Create two Vertex Buffer Objects for this VAO (positions, colours)

	// Grid vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]); // Bind the first grid VBO (vertex positions)
	glBufferData(GL_ARRAY_BUFFER, 64*sizeof(vec4), gridpoints, GL_STATIC_DRAW); // Put the grid points in the VBO
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0); 
	glEnableVertexAttribArray(vPosition); // Enable the attribute
	
	// Grid vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]); // Bind the second grid VBO (vertex colours)
	glBufferData(GL_ARRAY_BUFFER, 64*sizeof(vec4), gridcolours, GL_STATIC_DRAW); // Put the grid colours in the VBO
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor); // Enable the attribute
}


void initBoard()
{
	// *** Generate the geometric data
	vec4 boardpoints[1200];
	for (int i = 0; i < 1200; i++)
		boardcolours[i] = black; // Let the empty cells on the board be black
	// Each cell is a square (2 triangles with 6 vertices)
	for (int i = 0; i < 20; i++){
		for (int j = 0; j < 10; j++)
		{		
			vec4 p1 = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), .5, 1);
			vec4 p2 = vec4(33.0 + (j * 33.0), 66.0 + (i * 33.0), .5, 1);
			vec4 p3 = vec4(66.0 + (j * 33.0), 33.0 + (i * 33.0), .5, 1);
			vec4 p4 = vec4(66.0 + (j * 33.0), 66.0 + (i * 33.0), .5, 1);
			
			// Two points are reused
			boardpoints[6*(10*i + j)    ] = p1;
			boardpoints[6*(10*i + j) + 1] = p2;
			boardpoints[6*(10*i + j) + 2] = p3;
			boardpoints[6*(10*i + j) + 3] = p2;
			boardpoints[6*(10*i + j) + 4] = p3;
			boardpoints[6*(10*i + j) + 5] = p4;
		}
	}

	// Initially no cell is occupied
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 20; j++)
			board[i][j] = false; 


	// *** set up buffer objects
	glBindVertexArray(vaoIDs[1]);
	glGenBuffers(2, &vboIDs[2]);

	// Grid cell vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[2]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardpoints, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Grid cell vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

// No geometry for current tile initially
void initCurrentTile()
{
	glBindVertexArray(vaoIDs[2]);
	glGenBuffers(2, &vboIDs[4]);

	// Current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]);
	glBufferData(GL_ARRAY_BUFFER, 24*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Current tile vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
	glBufferData(GL_ARRAY_BUFFER, 24*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

void init()
{
	// Load shaders and use the shader program
	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	// Get the location of the attributes (for glVertexAttribPointer() calls)
	vPosition = glGetAttribLocation(program, "vPosition");
	vColor = glGetAttribLocation(program, "vColor");

	// Create 3 Vertex Array Objects, each representing one 'object'. Store the names in array vaoIDs
	glGenVertexArrays(3, &vaoIDs[0]);

	// Initialize the grid, the board, and the current tile
	initGrid();
	initBoard();
	initCurrentTile();

	// The location of the uniform variables in the shader program
	locxsize = glGetUniformLocation(program, "xsize"); 
	locysize = glGetUniformLocation(program, "ysize");

	// Game initialization
	newtile(); // create new next tile

	// set to default
	glBindVertexArray(0);
	glClearColor(0, 0, 0, 0);
}

//-------------------------------------------------------------------------------------------------------------------

// Rotates the current tile, if there is room
void rotate()
{   
	int fx;
	int fy;
	vec2 tshape[4];
	for(int i = 0; i<4 ;i++){
		fx = tile[i].x;
		fy = tile[i].y;
		tshape[i] = vec2(-1*fy, fx);
	}
	if(!rotatecollision(tshape)){
		//no rotation collision
		for(int i = 0; i<4; i++)
		{
			tile[i] = tshape[i];
			//cout << "tilepos " << tile[i] << " " << endl;
		}
		updatetile();
		glutPostRedisplay();
	}
}

//-------------------------------------------------------------------------------------------------------------------
// shuffle fruits
void shuffle()
{      
	//get colour array
	//shuffle fruits
	vec4 temp;
	temp = newcolours[0];
	for(int i = 0; i<18; i++)
	{
		newcolours[i] = newcolours[i+6]; 
	}
	for(int i = 18; i<24; i++)
	{
		newcolours[i] = temp;
	}
	//redraw
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//repost
	glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------

// Checks if the specified row (0 is the bottom 19 the top) is full
// If every cell in the row is occupied, it will clear that cell and everything above it will shift down one row
void checkfullrow(int row)
{
	bool full = true; 
	int bcp = 0;
	int j = 0;
	int jmax = 0;
	int above = 0;
	for(int i = 0; i<10; i++){
		if(board[i][row] == false)
		{
			full = false;
		}
	}
	if(full == true)
	{
		cout << "full" << endl;
		//clear row
		if(row == 19)
		{
			//clear top row
			//clear board to empty
			for(int i = 0; i<10; i++)
			{
				board[i][row] = false;
				bcp = (row+i)*6;
				j = i*6;
				jmax = j+6;
				for(j; j<jmax; j++){
					boardcolours[bcp] = black;
					bcp++;
				}
			}
		}
		else if(row < 19)
		{
			//clear row
			//shift down
			int temprow = row;

			for(temprow; temprow<19; temprow++)
			{
				for(int i = 0; i<10; i++)
				{
					board[i][temprow] = board[i][temprow+1];
					board[i][temprow+1] = false;
					bcp = (temprow*10+i)*6;
					above = ((temprow+1)*10+i)*6;
					//cout << "bcp " << bcp << " above " << above << endl;
					j = i*6;
					jmax = j+6;
					for(j; j<jmax; j++){
						boardcolours[bcp] = boardcolours[above];
						bcp++;
						above++;
					}
				}
			}
		}
		//redraw board
		glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
		glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
		//glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(vColor);
		glutPostRedisplay();
	}


}

//-------------------------------------------------------------------------------------------------------------------

// Places the current tile - update the board vertex colour VBO and the array maintaining occupied cells
void settile()
{
	int fx;
	int fy;
	int bcp = 0;
	int j = 0;
	int jmax = 0;
	for(int i = 0; i<4; i++)
	{
		fx = tilepos.x + tile[i].x;
		fy = tilepos.y + tile[i].y;
		board[fx][fy] = true;
		bcp = (fx+fy*10)*6;
		j = i*6;
		jmax = j+6;
		for(j; j<jmax; j++){
			boardcolours[bcp] = newcolours[j];
			bcp++;
		}
	}

	//update colours of board
	// Grid cell vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	//glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
	glutPostRedisplay();

}

//-------------------------------------------------------------------------------------------------------------------

// Given (x,y), tries to move the tile x squares to the right and y squares down
// Returns true if the tile was successfully moved, or false if there was some issue
bool movetile(vec2 direction)
{	
	//cout << "hello" << endl;
	//cout << direction.y << endl;
	//collision detection right here pls;
	if(collision(direction)){
		return false;
	}
	tilepos = direction;
	updatetile();
	//glutPostRedisplay();
	//cout << "tile position " << tilepos << endl;
	return true;
}
bool rotatecollision(vec2 temptile[]){
	bool col = false;
	int fx;
	int fy;
	for(int i = 0; i<4; i++){
		fx = tilepos.x + temptile[i].x;
		fy = tilepos.y + temptile[i].y;
		if((board[fx][fy])||(fx < 0)||(fy < 0)||(fx > 9)||(fy>19))
		{
			col = true;
			//cout << "fx fy " << fx << " " << fy << endl;
			//collision with board
		}
	}
	return col;
}
bool collision(vec2 direction)
{
	bool col = false;// no collision
	int fx;
	int fy;
	for(int i = 0; i<4; i++)
	{
		fx = direction.x + tile[i].x;
		fy = direction.y + tile[i].y;
		if((board[fx][fy])||(fx < 0)||(fy < 0)||(fx > 9)||(fy>19))
		{
			col = true;
			//cout << "fx fy " << fx << " " << fy << endl;
			//collision with board
		}
	}
	return col;

}
bool comparevec(vec4 original, vec4 compare)
{
	return (original.x == compare.x)&&(original.y == compare.y)&&(original.z == compare.z);

}
bool checkrow()
{
	int savetile = 0;
	int counter = 0;
	bool boo = false;
	for(int j = 0; j<20; j++)
	{
		savetile = 0;
		counter = 0;
		for(int i = 0; i<10; i++)
		{
			if((comparevec(boardcolours[(j*10+i)*6],boardcolours[(j*10+savetile)*6])) && (!comparevec(boardcolours[(j*10+savetile)*6],black)))
			{
				counter++;
				if((counter >= 3)&&(i==9))
				{
					boo = true;
					for(int k = savetile; k<(counter+savetile); k++)
					{
						//clear row
						board[k][j] = false;
						//board[k][j] = board[k][j+1];
						//change colour to black
						for(int m = 0; m<6 ; m++){
							boardcolours[(j*10+k)*6+m] = black;
						}

						//drop row above all the way to the top
						for(int p = j; p<19; p++){
							board[k][p] = board[k][p+1];
							for(int m = 0; m<6 ; m++){
								boardcolours[(p*10+k)*6+m] = boardcolours[((p+1)*10+k)*6+m];
							}
						}

					}
				}
			}
			else
			{
				if(counter >= 3)
				{
					boo = true;
					for(int k = savetile; k<(counter+savetile); k++)
					{
						//clear row
						board[k][j] = false;
						//board[k][j] = board[k][j+1];
						//change colour to black
						for(int m = 0; m<6 ; m++){
							boardcolours[(j*10+k)*6+m] = black;
						}

						//drop row above all the way to the top
						for(int p = j; p<19; p++){
							board[k][p] = board[k][p+1];
							for(int m = 0; m<6 ; m++){
								boardcolours[(p*10+k)*6+m] = boardcolours[((p+1)*10+k)*6+m];
							}
						}

					}
				}
				counter = 1;
				savetile = i;
			}
		}
	}	

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	//glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
	glutPostRedisplay();
	return boo;

}

bool checkcolumn()
{
	int savetile = 0;
	int counter = 0;
	bool boo = false;

	for(int i = 0; i<10; i++)
	{
		savetile = 0;
		counter = 0;
		for(int j = 0; j<20; j++)
		{
			if((!comparevec(boardcolours[(savetile*10+i)*6],black))&&(comparevec(boardcolours[(j*10+i)*6],boardcolours[(savetile*10+i)*6])))
			{
				counter++;
			}
			else
			{
				if(counter >= 3)
				{
					cout << "clear" << counter << "i " << i << endl;
					boo = true;
					for(int k = savetile; k<(counter+savetile); k++)
					{
						//clear row
						board[i][k] = false;
						//change colour to black
						for(int m = 0; m<6 ; m++){
							boardcolours[(k*10+i)*6+m] = black;
						}
					}
					//death from above
					for(int p = savetile+counter; p<19; p++){
						board[i][p-counter] = board[i][p];
						for(int m = 0; m<6; m++)
						{
							boardcolours[((p-counter)*10+i)*6+m] = boardcolours[(p*10+i)*6+m];
						}
					}
					for(int p = 19; p>19-counter;p--)
					{
						board[i][p] = false;
					}
					return boo;
				}
				counter = 1;
				savetile = j;
			}
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	//glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
	glutPostRedisplay();
	return boo;
}
void runTimer(int n)
{
	vec2 direction;
	direction.x = tilepos.x;
	direction.y = tilepos.y-1;
	if(!stopwatch)
	{
		restart();
		speedup = false;
		newtile();

	}
	else if(!movetile(direction))
	{
		//collision detected
		//set old tile
		settile();
		//check for clears
		//clear rows
		//clear triples
		for(int i = 0; i<20; i++)
		{
			checkfullrow(19-i);
			glutPostRedisplay();
		}

		//bool boo = true;
		while(checkrow() || checkcolumn())
		{
		}

		//make new tile
		speedup = false;
		newtile();
		//loop again
		//glutTimerFunc(500,runTimer,0);
		//until gameover
	}
	if(speedup)
	{
		glutTimerFunc(50, runTimer, 0);
		glutPostRedisplay();
		return;
	}
	glutTimerFunc(500,runTimer,0);
	glutPostRedisplay();


}
//-------------------------------------------------------------------------------------------------------------------

// Starts the game over - empties the board, creates new tiles, resets line counters
void restart()
{	
	//set the tile for clearing
	//stopwatch = false;
	//settile();
	//clear board logic
	for(int i = 0; i<10; i++){
		for(int j = 0; j<20; j++){
			board[i][j] = false;
		}
	}
	//clear colours
	for(int k = 0; k<1200; k++){
		boardcolours[k] = black;
	}

	for(int i = 0; i<24; i++){
		newcolours[i] = black;
	}

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	//glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
	glutPostRedisplay();

	/**
	vec2 tile[4]; // An array of 4 2d vectors representing displacement from a 'center' piece of the tile, on the grid
	vec2 tilepos = vec2(5, 19); // The position of the current tile using grid coordinates ((0,0) is the bottom left corner)
	**/
	//clear tile
	//newtile();
	//restart tile
	stopwatch = true;

}
//-------------------------------------------------------------------------------------------------------------------

// Draws the game
void display()
{

	glClear(GL_COLOR_BUFFER_BIT);

	glUniform1i(locxsize, xsize); // x and y sizes are passed to the shader program to maintain shape of the vertices on screen
	glUniform1i(locysize, ysize);

	glBindVertexArray(vaoIDs[1]); // Bind the VAO representing the grid cells (to be drawn first)
	glDrawArrays(GL_TRIANGLES, 0, 1200); // Draw the board (10*20*2 = 400 triangles)

	glBindVertexArray(vaoIDs[2]); // Bind the VAO representing the current tile (to be drawn on top of the board)
	glDrawArrays(GL_TRIANGLES, 0, 24); // Draw the current tile (8 triangles)

	glBindVertexArray(vaoIDs[0]); // Bind the VAO representing the grid lines (to be drawn on top of everything else)
	glDrawArrays(GL_LINES, 0, 64); // Draw the grid lines (21+11 = 32 lines)


	glutSwapBuffers();
}

//-------------------------------------------------------------------------------------------------------------------

// Reshape callback will simply change xsize and ysize variables, which are passed to the vertex shader
// to keep the game the same from stretching if the window is stretched
void reshape(GLsizei w, GLsizei h)
{
	xsize = w;
	ysize = h;
	glViewport(0, 0, w, h);
}

//-------------------------------------------------------------------------------------------------------------------

// Handle arrow key keypresses
void special(int key, int x, int y)
{
	vec2 direction;
	switch(key)
	{
		case GLUT_KEY_UP:
			rotate();
			//rotate
		break;	
		case GLUT_KEY_DOWN:
			/**direction.x = tilepos.x;
			direction.y =tilepos.y -1;
			movetile(direction);**/
			//speed up
			speedup = true;
		break;
		case GLUT_KEY_LEFT:
			//translate left
			direction.x = tilepos.x -1;
			direction.y = tilepos.y;
			movetile(direction);
		break;
		case GLUT_KEY_RIGHT:
			//translate right
			direction.x = tilepos.x + 1;
			direction.y = tilepos.y;
			movetile(direction);
		break;
	}
}

//-------------------------------------------------------------------------------------------------------------------

// Handles standard keypresses
void keyboard(unsigned char key, int x, int y)
{
	switch(key) 
	{
		case 033: // Both escape key and 'q' cause the game to exit
		    exit(EXIT_SUCCESS);
		    break;
		case 'q':
			exit (EXIT_SUCCESS);
			break;
		case 'r': // 'r' key restarts the game
			stopwatch = false;
			break;
		case 32: //space bar to shuffle fruits
			shuffle();
			break;
	}
	glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------

void idle(void)
{
	glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	cvector.push_back(orange);
	cvector.push_back(red);
	cvector.push_back(yellow);
	cvector.push_back(purple);
	cvector.push_back(green);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(xsize, ysize);
	glutInitWindowPosition(680, 178); // Center the game window (well, on a 1920x1080 display)
	glutCreateWindow("Fruit Tetris");
	glewInit();
	init();

	// Callback functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(special);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);
	glutTimerFunc(500,runTimer,0);
	glutMainLoop(); // Start main loop
	return 0;
}
