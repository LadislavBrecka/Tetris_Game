#include "olcConsoleGameEngine.h"
#include <string>

// Enum class for direction of block
enum class eDir {
	LEFT, RIGHT, DOWN
};

// Matrix structure, with operator overloaded for better aproach
struct mat4x4
{
	int m[4][4] = { 0 };

	mat4x4& operator=(const mat4x4& right)
	{
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				m[i][j] = right.m[i][j];

		return *this;
	}
};

// Enum class for shapes
enum eShape
{
	O, L, T, J, I, S, Z
};

// One block of world space is represented by its collor + info about occupancy 
struct sBlock
{
	bool bFree = true;
	unsigned uCol;
};

// Main class of the game derivated from engine with drawing stuff
class Tetris : public olcConsoleGameEngine
{
public:
	Tetris()
	{
		m_sAppName = L"Tetris Game";
	}

	// Drawing functions for all shapes of Tetris game
	void DrawShape(int x, int y, short col, mat4x4 mat)
	{
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				if (mat.m[i][j] == 1)
				{
					Fill(x + j, y + i, x + j + 1, y + i + 1, PIXEL_SOLID, col);
				}
	}

	// Rotation of matrix, for rotating shapes in world - each block is represented
	// as matrix
	void RotationMatrix(mat4x4 &mat)
	{
		int N = 4;
		// Traverse each cycle 
		for (int i = 0; i < N / 2; i++)
		{
			for (int j = i; j < N - i - 1; j++)
			{
				// Swap elements of each cycle 
				// in clockwise direction 
				int temp = mat.m[i][j];
				mat.m[i][j] = mat.m[N - 1 - j][i];
				mat.m[N - 1 - j][i] = mat.m[N - 1 - i][N - 1 - j];
				mat.m[N - 1 - i][N - 1 - j] = mat.m[j][N - 1 - i];
				mat.m[j][N - 1 - i] = temp;
			}
		}
	}

	// Function which is provading matrixes for requaired block
	mat4x4 GetMatrix(int shape)
	{
		mat4x4 m = { 0 };

		switch (shape)
		{
		case eShape::O:
			m.m[0][0] = 1;
			m.m[0][1] = 1;
			m.m[1][0] = 1;
			m.m[1][1] = 1;
			break;

		case eShape::L:
			m.m[0][0] = 1;
			m.m[0][1] = 1;
			m.m[0][2] = 1;
			m.m[1][0] = 1;
			break;

		case eShape::T:
			m.m[0][0] = 1;
			m.m[0][1] = 1;
			m.m[0][2] = 1;
			m.m[1][1] = 1;
			break;

		case eShape::J:
			m.m[0][0] = 1;
			m.m[0][1] = 1;
			m.m[0][2] = 1;
			m.m[1][2] = 1;
			break;

		case eShape::I:
			m.m[0][0] = 1;
			m.m[0][1] = 1;
			m.m[0][2] = 1;
			m.m[0][3] = 1;
			break;

		case eShape::S:
			m.m[1][0] = 1;
			m.m[1][1] = 1;
			m.m[0][1] = 1;
			m.m[0][2] = 1;
			break;

		case eShape::Z:
			m.m[0][0] = 1;
			m.m[0][1] = 1;
			m.m[1][1] = 1;
			m.m[1][2] = 1;
			break;
		}

		return m;
	}

	void ResetShape()
	{
		srand((int)(time(NULL)));

		py = 0.0f;				px = (float)ScreenWidth() / 2.0f;
		nShape = rand() % 7;	uColour = rand() % 6 + 1;

		mTetMatrix = GetMatrix(nShape);
	}

	void ResetGame()
	{
		for (int i = 0; i < (ScreenHeight() - dOffsetTop); i++)
		{
			for (int j = 0; j < ScreenWidth(); j++)
			{
				auto& w = world[i * ScreenWidth() + j];
				w.bFree = true;
				w.uCol = 0;
			}
		}
		ResetShape();
		uScore = 0;
	}

	void CollideDetection(int r)
	{
		bool bCollision = false;

		if (py + r + 1 >= (float)(ScreenHeight() - dOffsetTop))	bCollision = true;
		
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				if (mTetMatrix.m[i][j] == 1 &&
					!world[(int)(py + i) * ScreenWidth() + (int)(px + j)].bFree)
				{
					bCollision = true;
					switch (eDirection)
					{
					case eDir::DOWN:	py -= 1; break;
					case eDir::LEFT:	px += 1; bCollision = false; break;
					case eDir::RIGHT:	px -= 1; bCollision = false; break;
					}
				}
			}
		}
		
		if (bCollision)
		{
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					auto &w = world[(int)(py + i) * ScreenWidth() + (int)(px + j)];
					if (mTetMatrix.m[i][j] == 1)
					{
						w.bFree = false;
						w.uCol = uColour;
					}
				}
			}
			ResetShape();
		}
	}

	void EreaseRow(int nRow)
	{
		for (int k = nRow; k > 0; k--)
		{
			for (int l = 0; l < ScreenWidth(); l++)
			{
				auto& w = world[k * ScreenWidth() + l];
				auto& q = world[(k - 1) * ScreenWidth() + l];
				w.bFree = q.bFree;
				w.uCol  = q.uCol;
			}
		}
	}

	void CheckRow()
	{
		int nRowsCleared = 0;
		for (int i = 0; i < (ScreenHeight() - dOffsetTop); i++)
		{
			for (int j = 0; j < ScreenWidth(); j++)
			{
				if (world[i * ScreenWidth() + j].bFree)	break;
				else if (j == (ScreenWidth() - 1))
				{
					EreaseRow(i);
					nRowsCleared++;
				}
			}
		}
		uScore += ScoreLevels[nRowsCleared];
	}

private:
	float px, py;
	float fSpeed = 5.0f;
	unsigned uColour = 5;
	int nShape;
	mat4x4 mTetMatrix = { 0 };

	eDir eDirection;
	sBlock* world = (sBlock*) new sBlock[ScreenWidth() * (ScreenHeight() - dOffsetTop)];
	
	unsigned uScore = 0;
	const int ScoreLevels[5] = { 0, 40, 100, 300, 1200 };
	int dOffsetTop = 4;

	bool bGameOver = false;
	bool bPause = false;

public:
	bool OnUserCreate()
	{
		ResetGame();
		return true;
	}

	bool OnUserDestroy()
	{
		delete[] world;

		return true;
	}

	bool OnUserUpdate(float fElapsedTime)
	{
		srand((int)(time(NULL)));

		if (bGameOver && GetKey(L'R').bPressed)
		{
			ResetGame();
			bGameOver = false;
		}

		if (GetKey(L'P').bPressed)
		{
			if (bPause) bPause = false;
			else        bPause = true;
		}

		if (!bGameOver && !bPause)
		{
			//Clear the frame
			Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);

			//Movement down
			py += fSpeed * fElapsedTime;
			if (GetKey(L'S').bHeld)	py += fSpeed * 5.0f * fElapsedTime;
			eDirection = eDir::DOWN;
			
			//Left and right move
			if (GetKey(L'A').bHeld)
			{
				px -= fSpeed * 2.0f * fElapsedTime;
				eDirection = eDir::LEFT;
			}
			if (GetKey(L'D').bHeld)
			{
				px += fSpeed * 2.0f * fElapsedTime;
				eDirection = eDir::RIGHT;
			}

			//Rotation
			if (GetKey(VK_SPACE).bPressed && nShape != eShape::O) RotationMatrix(mTetMatrix);

			//Finding the most left/right columnt and the lowest row of matrix of shape
			int leftcolumn;
			int rightcolumn;
			int row;
			bool f = true;

			// Left and right column
			for (int i = 0; i < 4; i++)
				for (int j = 0; j < 4; j++)
					if (mTetMatrix.m[j][i] == 1)
					{
						if (f)
						{
							leftcolumn = i;
							f = false;
						}
						rightcolumn = i;
					}
			// The lowest row
			for (int i = 0; i < 4; i++)
				for (int j = 0; j < 4; j++)
					if (mTetMatrix.m[i][j] == 1)
						row = i;

			//Left border of game space
			if ((px + leftcolumn) < 0.0f)
			{
				px = (float)(0.0f - leftcolumn);
				eDirection = eDir::DOWN;
			}

			//Right border of game space
			if (px + rightcolumn + 1 > (float)ScreenWidth())
			{
				px = (float)(ScreenWidth() - rightcolumn - 1);
				eDirection = eDir::DOWN;
			}

			//Call function to check for collide
			CollideDetection(row);

			//Call function to check for full row and erease blocks if needed
			CheckRow();

			//Draw actual block
			DrawShape((int)px, (int)py, uColour, mTetMatrix);

			//Drawing all blocks in world
			for (int i = 0; i < (ScreenHeight() - dOffsetTop); i++)
			{
				for (int j = 0; j < ScreenWidth(); j++)
				{
					auto& w = world[i * ScreenWidth() + j];
					if (!w.bFree)	Draw(j, i, PIXEL_SOLID, w.uCol);
				}
			}

			// Printing score
			DrawLine(0, 30, ScreenWidth(), 30, PIXEL_SOLID, FG_GREY);
			DrawString(0, 31, L"Score:" + std::to_wstring(uScore), FG_CYAN);

			// Checking if player can still play, if not GameOver happened
			for (int i = 0; i < ScreenWidth(); i++)
			{
				if (world[0 * ScreenWidth() + i].bFree == false)
				{
					bGameOver = true;
					DrawString(3, 3, L"Game Over.", FG_BLUE);
				}
			}
		}
		return true;
	}
};

int main()
{
	Tetris game;
	if(game.ConstructConsole(16, 34, 20, 20))
		game.Start();

	return 0;
}