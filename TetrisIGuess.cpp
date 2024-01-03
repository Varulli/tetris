#include <iostream>
#include <vector>
#include <Windows.h>
#include <stdio.h>
#include <ctime>


std::string tetrominos[7];
std::vector<int> remainingTetrominos;
std::vector<int> lines;
int lineClears = 0;

int score = 0;

bool keys[5];

int fieldWidth = 12;
int fieldHeight = 18;
unsigned char* field = nullptr;

int screenWidth = 80;
int screenHeight = 30;

int currTetromino = 0;
int heldTetromino = -1;
char heldChar = ' ';
int currPosX = 4;
int currPosY = 0;
int currRotation = 0;
bool rotationLocked = false;
bool swapLocked = false;

int tickCount = 0;
int tickLimit = 20;

char buffer[10];

int getRotatedIndex(int x, int y, int r)
{
	switch (r % 4)
	{
	case 0:
		return y * 4 + x;
	case 1:
		return 12 + y - x * 4;
	case 2:
		return 15 - y * 4 - x;
	case 3:
		return 3 - y + x * 4;
	default:
		return 0;
	}
}

bool pieceFits(int tetromino, int rotation, int xPos, int yPos)
{
	for (int x = 0; x < 4; x++)
		for (int y = 0; y < 4; y++)
		{
			int iPiece = getRotatedIndex(x, y, rotation);
			int iField = (yPos + y) * fieldWidth + (xPos + x);

			if (xPos + x >= 0 && xPos + x < fieldWidth && yPos + y > 0 && yPos + y < fieldHeight)
			{
				if (tetrominos[tetromino][iPiece] == 'X' && field[iField] != 0)
					return false;
			}
		}

	return true;
}

void addLine(int y)
{
	bool lineExists = true;
	for (int x = 1; x < fieldWidth - 1; x++)
		if (field[y * fieldWidth + x] == 0) lineExists = false;

	if (lineExists)
	{
		for (int x = 1; x < fieldWidth - 1; x++)
			field[y * fieldWidth + x] = 8;
		lines.push_back(y);
	}
}

int main()
{
	// Tetromino assets
	tetrominos[0].append("  X ");
	tetrominos[0].append("  X ");
	tetrominos[0].append("  X ");
	tetrominos[0].append("  X ");

	tetrominos[1].append("  X ");
	tetrominos[1].append(" XX ");
	tetrominos[1].append(" X  ");
	tetrominos[1].append("    ");

	tetrominos[2].append(" X  ");
	tetrominos[2].append(" XX ");
	tetrominos[2].append("  X ");
	tetrominos[2].append("    ");

	tetrominos[3].append(" XX ");
	tetrominos[3].append("  X ");
	tetrominos[3].append("  X ");
	tetrominos[3].append("    ");

	tetrominos[4].append(" XX ");
	tetrominos[4].append(" X  ");
	tetrominos[4].append(" X  ");
	tetrominos[4].append("    ");

	tetrominos[5].append("    ");
	tetrominos[5].append(" XX ");
	tetrominos[5].append(" XX ");
	tetrominos[5].append("    ");

	tetrominos[6].append("  X ");
	tetrominos[6].append(" XX ");
	tetrominos[6].append("  X ");
	tetrominos[6].append("    ");

	// Allocate game field
	field = new unsigned char[fieldWidth * fieldHeight];
	for (int x = 0; x < fieldWidth; x++)
		for (int y = 0; y < fieldHeight; y++)
			field[y * fieldWidth + x] = (x == 0 || x == fieldWidth - 1 || y == fieldHeight - 1) ? 9 : 0;

	// Allocate screen
	char* screen = new char[screenWidth * screenHeight];
	for (int i = 0; i < screenWidth * screenHeight; i++) screen[i] = ' ';

	HANDLE console = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(console);
	DWORD bytesWritten = 0;

	for (int i = 0; i < 4; i++) screen[2 * screenWidth + (fieldWidth + 5 + i)] = "HELD"[i];
	for (int i = 0; i < 5; i++) screen[11 * screenWidth + (fieldWidth + 5 + i)] = "SCORE"[i];
	for (int i = 0; i < 5; i++) screen[14 * screenWidth + (fieldWidth + 5 + i)] = "LEVEL"[i];

	// Seed rng
	srand((int)time(nullptr));

	for (int i = 0; i < 7; i++)
		remainingTetrominos.push_back(i);
	auto it = remainingTetrominos.begin() + rand() % 7;
	currTetromino = *it;
	remainingTetrominos.erase(it);

	bool done = false;

	while (!done)
	{
		// Game timing
		Sleep(60);

		if (++tickCount == tickLimit)
		{
			// Increase score
			if (!lines.empty())
			{
				score += ("01358"[lines.size()] - '0') * 100 * (21 - tickLimit);
				lineClears += lines.size();
			}

			// Update level
			if (lineClears >= (21 - tickLimit) * 5 && tickLimit > 1)
			{
				lineClears %= (21 - tickLimit) * 10;
				tickLimit--;
			}

			// Clear lines
			while (!lines.empty())
			{
				int y = lines.front();
				lines.erase(lines.begin());
				for (y; y > 0; y--)
					for (int x = 1; x < fieldWidth - 1; x++)
						field[y * fieldWidth + x] = field[(y - 1) * fieldWidth + x];
			}

			if (pieceFits(currTetromino, currRotation, currPosX, currPosY + 1))
				currPosY++;
			else
			{
				// Fix tetromino into field
				for (int y = 0; y < 4; y++)
				{
					for (int x = 0; x < 4; x++)
					{
						if (tetrominos[currTetromino][getRotatedIndex(x, y, currRotation)] == 'X')
						{
							field[(currPosY + y) * fieldWidth + (currPosX + x)] = currTetromino + 1;
							field[(fieldHeight - 1) * fieldWidth + (currPosX + x)] = 9;
						}
					}
					if (currPosY + y < fieldHeight - 1) addLine(currPosY + y);
				}
				
				// Generate new tetromino at starting position
				if (remainingTetrominos.empty())
					for (int i = 0; i < 7; i++)
						remainingTetrominos.push_back(i);

				auto it = remainingTetrominos.begin() + rand() % remainingTetrominos.size();
				currTetromino = *it;
				remainingTetrominos.erase(it);
				currPosX = 4;
				currPosY = 0;
				currRotation = 0;
				
				// End game if tetromino at starting position doesn't fit
				if (!pieceFits(currTetromino, currRotation, currPosX, currPosY)) done = true;
			}
			tickCount = 0;
		}


		// Input
		for (int k = 0; k < 5; k++)
			keys[k] = (0x8000 & GetAsyncKeyState((unsigned char)"\x41\x44\x53\x57\x52"[k])) != 0;


		// Game logic
		if (keys[0]) // A/Left
		{
			if (pieceFits(currTetromino, currRotation, currPosX - 1, currPosY))
				currPosX --;
		}
		if (keys[1]) // D/Right
		{
			if (pieceFits(currTetromino, currRotation, currPosX + 1, currPosY))
				currPosX ++;
		}
		if (keys[2]) // S/Down
		{
			if (pieceFits(currTetromino, currRotation, currPosX, currPosY + 1))
				currPosY++;
		}
		if (keys[3]) // W/Rotate
		{
			if (!rotationLocked && pieceFits(currTetromino, currRotation + 1, currPosX, currPosY))
			{
				currRotation++;
				rotationLocked = true;
			}
		}
		else rotationLocked = false;

		if (keys[4]) // R/Swap
		{
			if (!swapLocked)
			{
				if (heldTetromino >= 0 && pieceFits(heldTetromino, 0, currPosX, currPosY))
				{
					int temp = heldTetromino;
					heldTetromino = currTetromino;
					currTetromino = temp;

					currRotation = 0;
				}
				else
				{
					heldTetromino = currTetromino;

					// Generate new tetromino at starting position
					if (remainingTetrominos.empty())
						for (int i = 0; i < 7; i++)
							remainingTetrominos.push_back(i);

					auto it = remainingTetrominos.begin() + rand() % remainingTetrominos.size();
					currTetromino = *it;
					remainingTetrominos.erase(it);
					currPosX = 4;
					currPosY = 0;
					currRotation = 0;

					// End game if tetromino at starting position doesn't fit
					if (!pieceFits(currTetromino, currRotation, currPosX, currPosY)) done = true;
				}
			}
			swapLocked = true;
		}
		else swapLocked = false;


		// Render output
		for (int x = 0; x < 4; x++)
			for (int y = 0; y < 4; y++)
				if (tetrominos[currTetromino][getRotatedIndex(x, y, currRotation)] == 'X')
				{
					field[(currPosY + y) * fieldWidth + (currPosX + x)] = currTetromino + 1;
					field[(fieldHeight - 1) * fieldWidth + (currPosX + x)] = 10;
				}


		// Draw field to screen
		for (int x = 0; x < fieldWidth; x++)
			for (int y = 0; y < fieldHeight; y++)
				screen[(y + 2) * screenWidth + (x + 4)] = " ABCDEFG=#^"[field[y * fieldWidth + x]];

		for (int x = 0; x < 4; x++)
			for (int y = 0; y < 4; y++)
				if (tetrominos[currTetromino][getRotatedIndex(x, y, currRotation)] == 'X')
				{
					field[(currPosY + y) * fieldWidth + (currPosX + x)] = 0;
					field[(fieldHeight - 1) * fieldWidth + (currPosX + x)] = 9;
				}

		// Draw held piece
		if (heldTetromino >= 0)
		{
			heldChar = "ABCDEFG"[heldTetromino];
			for (int x = 0; x < 4; x++)
				for (int y = 0; y < 4; y++)
					screen[(4 + y) * screenWidth + (fieldWidth + 5 + x)] = tetrominos[heldTetromino][getRotatedIndex(x, y, 0)] == 'X' ? heldChar : ' ';
		}


		// Print score and level
		sprintf_s(buffer, "%5d", score);
		std::string scoreStr(buffer);
		for (int i = 0; i < scoreStr.length(); i++) screen[12 * screenWidth + (fieldWidth + 5 + i)] = scoreStr[i];
		sprintf_s(buffer, "%5d", 21 - tickLimit);
		std::string levelStr(buffer);
		for (int i = 0; i < levelStr.length(); i++) screen[15 * screenWidth + (fieldWidth + 5 + i)] = levelStr[i];


		// Display screen
		WriteConsoleOutputCharacterA(console, screen, screenWidth * screenHeight, { 0, 0 }, &bytesWritten);
	}

	// Game over screen
	for (int i = 0; i < 9; i++) screen[2 * screenWidth + (fieldWidth + 5 + i)] = ' ';
	for (int i = 0; i < 9; i++) screen[4 * screenWidth + (fieldWidth + 5 + i)] = "GAME OVER"[i];
	for (int i = 0; i < 9; i++) screen[5 * screenWidth + (fieldWidth + 5 + i)] = ' ';
	for (int i = 0; i < 11; i++) screen[6 * screenWidth + (fieldWidth + 5 + i)] = "PRESS ENTER"[i];
	for (int i = 0; i < 7; i++) screen[7 * screenWidth + (fieldWidth + 5 + i)] = "TO EXIT"[i];
	WriteConsoleOutputCharacterA(console, screen, screenWidth * screenHeight, { 0, 0 }, &bytesWritten);
	
	while (GetAsyncKeyState(VK_RETURN) == 0) {};
	
	return 0;
}