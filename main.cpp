#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <windows.h>
#include "winbgi.h"

#define RED COLOR(255,0,0)
#define WHITE COLOR(255,255,255)
#define windowheight 600
#define windowwidth 600
#define refreshtime 200

bool isbuttonclicked(bool mouseclick, int mouse_x, int mouse_y, struct button button); //Checks if button is clicked, if yes returns 1; Draws red rectangle around the button
void drawbutton(struct button button, int color); //Draws button
void drawapple(int apple_x, int apple_y, bool graphics3d);	//Draws apple
void drawsnake(int* x, int* y, int count, bool graphics3d);	//Draws snake
void drawtext(bool newhighscore, int count, int highscore, char* countstr, char* highscorestr);	//Draws text 
void applegenerate(int& apple_x, int& apple_y, int* x, int* y, int count, bool& isapple);	//Generates apple
void applecollect(int& apple_x, int& apple_y, int* x, int* y, int &count, bool& isapple);	//Snake eats apple
void collision(int *x, int *y, int count, bool iswallcollision, bool& dead);	//Checks if there is collision between snake and its body or snake and walls
void keyboard(char key, int &velx, int &vely);	//Handles keyboard input
void movesnake(int* x, int* y, int velx, int vely, int count);	//Moves snake's head and body
void highscorechange(FILE* score, int& highscore);	//Updates highscore
void getmousestate(bool& mouseclick, int& mouse_x, int& mouse_y);	//Gets mouse position and left button state
void playagainmenu(bool& done, bool& mouseclick, int& mouse_x, int& mouse_y);	//Menu after game over
void mainmenu(FILE* settings, FILE* score, bool& mouseclick, bool& iswallcollision,bool &graphics3d, int& mouse_x, int& mouse_y, int& speed, int& highscore);	//Main menu loop

struct button
{
	double x1;
	double y1;
	double x2;
	double y2;
	int textsize;
	char text[20];
};

button start{ 200, 260, 400, 310, 4, "START" };
button exitbutton{ 220, 380, 380, 430,4,"EXIT" };
button options{ 170, 320, 430, 370,4,"OPTIONS" };
button resethighscore{ 40, 50, 560, 110,4,"RESET HIGHSCORE" };
button easy{ 240, 270, 360, 310,3,"EASY" };
button medium{ 220, 310, 380, 350,3,"MEDIUM" };
button hard{ 240, 350, 360, 390 ,3,"HARD" };
button on{ 200, 450, 270, 490 ,3,"ON" };
button off{ 330, 450, 420, 490 ,3,"OFF" };
button back{ 220, 500, 380, 550 ,4,"BACK" };
button yes{ 190, 340, 270, 380 ,3,"YES" };
button no{ 330, 340, 390, 380 ,3,"NO" };
button g2d{ 200,170,260,210,3,"2D" };
button g3d{ 340,170,400,210,3,"3D" };

void main()
{
	FILE* score, * settings;
	bool done;
	bool dead;
	bool isapple;
	bool mouseclick;
	bool iswallcollision;
	bool newhighscore;
	bool graphics3d;
	int highscore;
	int apple_x, apple_y;
	int mouse_x, mouse_y;
	int count;
	int speed;
	int velx, vely;
	int* x;
	int* y;
	char key;
	char countstr[3], highscorestr[3];

	/*
	if (PlaySound("song.wav", NULL, SND_ASYNC | SND_LOOP))
		printf("File song.wav loaded successfully\n");
	else printf("ERROR: File song.wav couldn't be loaded\n");
	*/
	srand(time(NULL));

	score = fopen("score.txt", "r");
	if (score == NULL)
	{
		printf("ERROR: Couldn't open the score.txt file\n");
		score = fopen("score.txt", "w");
		printf("File score.txt successfully created\n");
		fprintf(score, "0");
		fclose(score);
		score = fopen("score.txt", "r");
	}
	fscanf(score, "%d", &highscore);
	fclose(score);

	settings = fopen("settings.txt", "r");
	if (settings == NULL)
	{
		printf("ERROR: Couldn't open the settings.txt file\n");
		settings = fopen("settings.txt", "w");
		printf("File settings.txt successfully created\n");
		fprintf(settings, "100\n0\n0");
		fclose(settings);
		settings = fopen("settings.txt", "r");
	}
	fscanf(settings, "%d%d%d", &speed, &iswallcollision,&graphics3d);
	fclose(settings);

	initwindow(windowwidth, windowheight, "SNAKE");

	while (1)
	{
		mainmenu(settings, score, mouseclick, iswallcollision, graphics3d,mouse_x, mouse_y, speed, highscore);
		done = 0;
		while (!done)
		{
			x = (int*)malloc(sizeof(int));
			y = (int*)malloc(sizeof(int));
			x[0] = windowwidth/2;
			y[0] = windowheight/2;
			count = 0;
			velx = 0, vely = 0;
			isapple = 0;
			dead = 0;
			newhighscore = 0;
			while (kbhit())
			{
				getch();
			}
			key = 0;
			while (!dead)
			{
				keyboard(key, velx, vely);
				movesnake(x, y, velx, vely, count);
				collision(x, y, count, iswallcollision, dead);
				if (dead)
					break;
				applegenerate(apple_x, apple_y, x, y, count, isapple);
				applecollect(apple_x, apple_y, x, y, count, isapple);
				x = (int*)realloc(x, sizeof(int) * (count + 1));
				y = (int*)realloc(y, sizeof(int) * (count + 1));
				if (count == highscore + 1)
					newhighscore = 1;
				highscore = max(highscore, count);

				cleardevice();
				drawtext(newhighscore, count, highscore, countstr, highscorestr);
				drawapple(apple_x, apple_y, graphics3d);
				drawsnake(x, y, count,graphics3d);
				swapbuffers();
				delay(speed);
			}
			delay(1000);
			free(x);
			free(y);
			highscorechange(score, highscore);
			playagainmenu(done, mouseclick, mouse_x, mouse_y);
		}
	}
}

bool isbuttonclicked(bool mouseclick,int mouse_x,int mouse_y, struct button button)
{
	if (mouse_x >= button.x1 && mouse_x <= button.x2 && mouse_y >= button.y1 && mouse_y <= button.y2)
	{
		setcolor(RED);
		rectangle(button.x1-10, button.y1-10, button.x2+10, button.y2-10);
		setcolor(WHITE);
		if (mouseclick == 1)
			return 1;
		else return 0;
	}
}
void drawbutton(struct button button, int color)
{
	double x = button.x2 - ((button.x2 - button.x1) / 2);
	double y = button.y2 - ((button.y2 - button.y1) / 2);
	char *text = button.text;
	settextjustify(CENTER_TEXT, CENTER_TEXT);
	settextstyle(0, HORIZ_DIR, button.textsize);
	setcolor(color);
	outtextxy(x, y, text);
	setcolor(WHITE);
}
void drawapple(int apple_x, int apple_y, bool graphics3d)
{
	setcolor(RED);
	setfillstyle(SOLID_FILL, RED);
	if (graphics3d)
		bar3d(apple_x + 1, apple_y + 2, apple_x + 16, apple_y + 19, 4, 1);
	else bar(apple_x + 1, apple_y + 1, apple_x + 19, apple_y + 19);
	setcolor(WHITE);
}
void drawsnake(int* x, int* y, int count, bool graphics3d)
{
	setcolor(WHITE);
	setfillstyle(SOLID_FILL, WHITE);
	for (int i = 0; i <= count; i++)
	{
		if (graphics3d)
			bar3d(x[i] + 1, y[i] + 2, x[i] + 16, y[i] + 19, 4, 1);
		else bar(x[i] + 1, y[i] + 1, x[i] + 19, y[i] + 19);
	}
}
void drawtext(bool newhighscore, int count, int highscore, char *countstr, char *highscorestr)
{
	itoa(count, countstr, 10);
	itoa(highscore, highscorestr, 10);
	rectangle(100, 100, 500, 500);
	settextstyle(0, HORIZ_DIR, 6);
	settextjustify(CENTER_TEXT, CENTER_TEXT);
	outtextxy(260, 60, "SCORE:");
	settextstyle(0, HORIZ_DIR, 3);
	if (newhighscore)
		outtextxy(300, 530, "NEW HIGHSCORE!");
	settextstyle(0, HORIZ_DIR, 2);
	outtextxy(280, 90, "HIGHSCORE:");
	outtextxy(300, 550, "USE ARROWS TO MOVE");
	outtextxy(300, 570, "PRESS SPACE TO PAUSE GAME");
	settextjustify(LEFT_TEXT, CENTER_TEXT);
	settextstyle(0, HORIZ_DIR, 6);
	outtextxy(400, 60, countstr);
	settextstyle(0, HORIZ_DIR, 2);
	outtextxy(360, 90, highscorestr);
}
void applegenerate(int& apple_x, int& apple_y, int* x, int* y, int count, bool& isapple)
{
	while (!isapple)
	{
		apple_x = rand() % 19 * 20 + 100;
		apple_y = rand() % 19 * 20 + 100;
		isapple = !isapple;
		for (int i = 0; i <= count; i++)
		{
			if (x[i] == apple_x && y[i] == apple_y)
			{
				isapple = !isapple;
				break;
			}
		}
	}
}
void applecollect(int& apple_x, int& apple_y, int* x, int* y, int &count, bool& isapple)
{
	if (x[0] == apple_x && y[0] == apple_y)
	{
		isapple = !isapple;
		count++;
		x[count] = apple_x;
		y[count] = apple_y;
	}
}

void collision(int *x, int *y, int count, bool iswallcollision, bool& dead)
{
	for (int i = 1; i <= count; i++)
	{
		if (x[0] == x[i] && y[0] == y[i])
			dead = !dead;
	}
	if (!iswallcollision)
	{
		if (x[0] > 480)
			x[0] = 100;
		if (x[0] < 100)
			x[0] = 480;
		if (y[0] > 480)
			y[0] = 100;
		if (y[0] < 100)
			y[0] = 480;
	}
	else if (iswallcollision)
	{
		if (x[0] > 480 || x[0] < 100 || y[0] > 480 || y[0] < 100)
			dead = !dead;
	}
}
void keyboard(char key, int &velx, int &vely)
{
	if (kbhit())
	{
		key = getch();
		if (key == 0)
			key = getch();
	}
	if (key == 32)
	{
		key = 0;
		while (1)
		{
			if (kbhit())
				key = getch();
			if (key == 32)
				break;
			settextstyle(0, HORIZ_DIR, 4);
			settextjustify(CENTER_TEXT, CENTER_TEXT);
			swapbuffers();
			outtextxy(300, 300, "PAUSED");
			swapbuffers();
			delay(refreshtime);
		}
		key = 0;
	}
	switch (key)
	{
	case 72:
		if (vely != 20)
		{
			velx = 0;
			vely = -20;
		}
		break;
	case 80:
		if (vely != -20)
		{
			velx = 0;
			vely = 20;
		}
		break;
	case 75:	
		if (velx != 20)
		{
			velx = -20;
			vely = 0;
		}
		break;
	case 77:
		if (velx != -20)
		{
			velx = 20;
			vely = 0;
		}
		break;
	}
}
void movesnake(int* x, int* y, int velx, int vely, int count)
{
	for (int i = count; i > 0; i--)
	{
		x[i] = x[i - 1];
		y[i] = y[i - 1];
	}
	x[0] += velx;
	y[0] += vely;
}
void highscorechange(FILE*score, int& highscore)
{
	score = fopen("score.txt", "w");
	fprintf(score, "%d", highscore);
	fclose(score);
}
void getmousestate(bool& mouseclick, int& mouse_x, int& mouse_y)
{
	mouseclick = 0;
	mouse_x = mousex();
	mouse_y = mousey();
	mouseclick = ismouseclick(WM_LBUTTONDOWN);
	clearmouseclick(WM_LBUTTONDOWN);
}
void playagainmenu(bool &done, bool &mouseclick, int &mouse_x, int &mouse_y)
{
	while (1)
	{
		cleardevice();
		settextstyle(0, HORIZ_DIR, 6);
		settextjustify(CENTER_TEXT, CENTER_TEXT);
		outtextxy(300, 260, "GAME OVER!");
		settextstyle(0, HORIZ_DIR, 4);
		outtextxy(300, 310, "RETRY?");
		settextstyle(0, HORIZ_DIR, 3);
		drawbutton(yes, WHITE);
		outtextxy(300, 360, "/");
		drawbutton(no, WHITE);
		getmousestate(mouseclick, mouse_x, mouse_y);
		if (isbuttonclicked(mouseclick, mouse_x, mouse_y, yes)==1)
		{
			drawbutton(yes, RED);
			swapbuffers();
			delay(refreshtime);
			break;
		}
		if (isbuttonclicked(mouseclick, mouse_x, mouse_y, no)==1)
		{
			drawbutton(no, RED);
			swapbuffers();
			delay(refreshtime);
			done = !done;
			break;
		}
		swapbuffers();
		delay(refreshtime);
	}
}
void mainmenu(FILE* settings, FILE* score, bool& mouseclick, bool& iswallcollision, bool& graphics3d, int& mouse_x, int& mouse_y, int& speed, int& highscore)
{
	while (1)
	{
		cleardevice();
		settextjustify(CENTER_TEXT, CENTER_TEXT);
		settextstyle(0, HORIZ_DIR, 10);
		outtextxy(300, 200, "SNAKE");
		drawbutton(start, WHITE);
		drawbutton(options, WHITE);
		drawbutton(exitbutton, WHITE);
		getmousestate(mouseclick, mouse_x, mouse_y);
		if (isbuttonclicked(mouseclick, mouse_x, mouse_y, start)==1)
		{
			drawbutton(start, RED);
			swapbuffers();
			delay(refreshtime);
			break;
		}
		if (isbuttonclicked(mouseclick, mouse_x, mouse_y, exitbutton)==1)
		{
			drawbutton(exitbutton, RED);
			swapbuffers();
			delay(refreshtime);
			exit(0);
		}
		if (isbuttonclicked(mouseclick, mouse_x, mouse_y, options)==1)
		{
			drawbutton(options, RED);
			swapbuffers();
			delay(refreshtime);
			while (1)
			{
				cleardevice();
				settextstyle(0, HORIZ_DIR, 4);
				drawbutton(resethighscore, WHITE);
				outtextxy(300, 250, "DIFFICULTY:");
				outtextxy(300, 420, "WALL COLLISION:");
				outtextxy(300, 470, "/");
				outtextxy(300, 140, "GRAPHICS:");
				outtextxy(300, 190, "/");
				drawbutton(back, WHITE);
				settextstyle(0, HORIZ_DIR, 3);
				if (graphics3d)
				{
					drawbutton(g2d, WHITE);
					drawbutton(g3d, RED);
				}
				else
				{
					drawbutton(g2d, RED);
					drawbutton(g3d, WHITE);
				}
				if (speed == 200)
					drawbutton(easy, RED);
				else drawbutton(easy, WHITE);
				if (speed == 100)
					drawbutton(medium, RED);
				else drawbutton(medium, WHITE);
				if (speed == 50)
					drawbutton(hard, RED);
				else drawbutton(hard, WHITE);
				if (iswallcollision == 1)
					drawbutton(on, RED);
				else drawbutton(on, WHITE);
				if (iswallcollision == 0)
					drawbutton(off, RED);
				else drawbutton(off, WHITE);
				getmousestate(mouseclick, mouse_x, mouse_y);
				if (isbuttonclicked(mouseclick, mouse_x, mouse_y, resethighscore)==1)
				{
					drawbutton(resethighscore, RED);
					score = fopen("score.txt", "w");
					fprintf(score, "0");
					highscore = 0;
					fclose(score);
				}
				if (isbuttonclicked(mouseclick, mouse_x, mouse_y, g2d)==1)
				{
					graphics3d = 0;
					settings = fopen("settings.txt", "w");
					fprintf(settings, "%d\n%d\n0", speed,iswallcollision);
					fclose(settings);
				}
				if (isbuttonclicked(mouseclick, mouse_x, mouse_y, g3d)==1)
				{
					graphics3d = 1;
					settings = fopen("settings.txt", "w");
					fprintf(settings, "%d\n%d\n1", speed, iswallcollision);
					fclose(settings);
				}
				if (isbuttonclicked(mouseclick, mouse_x, mouse_y,easy)==1)
				{
					speed = 200;
					settings = fopen("settings.txt", "w");
					fprintf(settings, "200\n%d\n%d", iswallcollision, graphics3d);
					fclose(settings);
				}
				if (isbuttonclicked(mouseclick, mouse_x, mouse_y, medium)==1)
				{
					speed = 100;
					settings = fopen("settings.txt", "w");
					fprintf(settings, "100\n%d\n%d", iswallcollision, graphics3d);
					fclose(settings);
				}
				if (isbuttonclicked(mouseclick, mouse_x, mouse_y, hard)==1)
				{
					speed = 50;
					settings = fopen("settings.txt", "w");
					fprintf(settings, "50\n%d\n%d", iswallcollision, graphics3d);
					fclose(settings);
				}
				if (isbuttonclicked(mouseclick, mouse_x, mouse_y, on)==1)
				{
					iswallcollision = 1;
					settings = fopen("settings.txt", "w");
					fprintf(settings, "%d\n1\n%d", speed, graphics3d);
					fclose(settings);
				}
				if (isbuttonclicked(mouseclick, mouse_x, mouse_y, off)==1)
				{
					iswallcollision = 0;
					settings = fopen("settings.txt", "w");
					fprintf(settings, "%d\n0\n%d", speed, graphics3d);
					fclose(settings);
				}
				if (isbuttonclicked(mouseclick, mouse_x, mouse_y, back)==1)
				{
					drawbutton(back, RED);
					delay(refreshtime);
					break;
				}
				swapbuffers();
				delay(refreshtime);
			}
		}
		swapbuffers();
		delay(refreshtime);
	}
}