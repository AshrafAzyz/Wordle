/***************************************************************************************
-Command to compile the project in Linux OS (modify and include additional files if exist):
gcc wordle.c gfx.c -o wordle.o -lX11 -lm
***********************************************************************************/
#include <windows.h>
#include <stdio.h>
#include "gfx.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void mainMenu();
void howToPlay();
int option();
void exitGame();
void getWordList(char** wordList);
char *pickWord(char** wordList);
int validCheck(char* g, char** wordList);
char *guessCheck(char *g, char* w);
void changeColor(char* g, char* c, int numGuess);
void changeKeyboard(char* g, char* c);
void endPage(int result, char *w);
void keyboardDisplay();
void clearGuess(int numGuess);
void *getGuess(char *guess, int numGuess);
void wordleboxes(int numGuess);
void playGame();

int main()
{
	int input;	
		
	gfx_open(1920,1080,"Wordle Project");
	
	do{
	mainMenu();
	input = option();
	
	switch(input){
	case 1:
		playGame();
		break;
	case 2:
		howToPlay(); 
		break;
	case 3:
		exitGame(); 
		break;
	}
	} while (input != 3);
	
	return 0;
}
/*
Displaying graphic for main page only
*/
void mainMenu()
{
	char c;
	
	gfx_clear_color(247,241,227);
	gfx_clear();
	
	
	gfx_color(0,0,0);
	gfx_fillrectangle(0,0,500,1080);
	gfx_fillrectangle(1420,0,500,1080);
	
	//w
	gfx_color(33,140,124);
	gfx_fillrectangle(610,120,20,80);
	gfx_fillrectangle(650,140,20,60);
	gfx_fillrectangle(690,120,20,80);
	gfx_fillrectangle(630,200,60,20);
	
	//o
	gfx_color(217,217,217);
	gfx_fillrectangle(750,120,60,20);
	gfx_fillrectangle(730,140,20,60);
	gfx_fillrectangle(810,140,20,60);
	gfx_fillrectangle(750,200,60,20);
	
	//r
	gfx_color(132,129,122);
	gfx_fillrectangle(850,120,20,100);
	gfx_fillrectangle(870,120,60,20);
	gfx_fillrectangle(870,160,60,20);
	gfx_fillrectangle(930,140,20,20);
	gfx_fillrectangle(910,180,20,20);
	gfx_fillrectangle(930,200,20,20);
	
	//d
	gfx_color(132,129,122);
	gfx_fillrectangle(970,120,20,100);
	gfx_fillrectangle(990,120,60,20);
	gfx_fillrectangle(990,200,60,20);
	gfx_fillrectangle(1050,140,20,60);
	
	//l
	gfx_color(217,217,217);
	gfx_fillrectangle(1090,120,20,100);
	gfx_fillrectangle(1110,200,80,20);
	
	//e
	gfx_color(204,142,53);
	gfx_fillrectangle(1210,120,20,100);
	gfx_fillrectangle(1230,120,80,20);
	gfx_fillrectangle(1230,160,60,20);
	gfx_fillrectangle(1230,200,80,20);
	
	//1st button
	gfx_color(132,129,122);
	gfx_fillrectangle(820,410,300,100);
	gfx_color(209,204,192);
	gfx_fillrectangle(810,395,300,100);
	
	//2nd button
	gfx_color(132,129,122);
	gfx_fillrectangle(820,555,300,100);
	gfx_color(209,204,192);
	gfx_fillrectangle(810,540,300,100);
	
	//3rd button
	gfx_color(132,129,122);
	gfx_fillrectangle(820,700,300,100);
	gfx_color(209,204,192);
	gfx_fillrectangle(810,685,300,100);
	
	gfx_color(0,0,0);
	
	gfx_text("WELCOME TO OUR WORDLE GAME!!!",878,300);
	gfx_text("NEW GAME",934,449);
	gfx_text("HOW TO PLAY",928,594);
	gfx_text("EXIT",950,739);
}
/*
Displaying game instructions and back button

*/
void howToPlay()
{
	char c;
	char *wordle = "WORDLE";
	char *text[5] = {"HOW     TO     PLAY",
			"GUESS     THE     WORD     IN     6     TRIES.",
			"EACH GUESS MUST BE A 5-LETTER WORD",
			"THE COLOR OF THE TILES WILL CHANGE TO SHOW HOW CLOSE YOUR GUESS WAS TO THE WORD",
			"EXAMPLES: "};
	char *text2[3] = {"W IS IN THE WORD AND IN THE CORRECT SPOT",
			 "I IS IN THE WORD BUT IN THE WRONG SPOT",
		    	 "G IS NOT IN THE WORD IN ANY SPOT"};
	char *firstExample[5]={"W","E","S","R","Y"};
	char *secondExample[5]={"P","I","L","L","S"};
	char *thirdExample[5]={"V","A","G","U","E"};
	
	int x, y,i,j;
	
	//background color: white
	gfx_clear_color(209,204,192);
	gfx_clear();

	gfx_color(0,0,0);
	gfx_fillrectangle(0,0,500,1080);
	gfx_fillrectangle(1420,0,500,1080);
		
	
	gfx_color(0,0,0);
	gfx_text(wordle,910,56);
	
	for(i=0;i<5;i++){
		gfx_color(0,0,0);
		gfx_text(text[i],801,145+(i*35));
	}
	
	for(i=0;i<3;i++){
		gfx_color(0,0,0);
		gfx_text(text2[i],801,440+(i*113));
	}
	
	//green box and text
	for ( i = 0; i < 5; i++){
			gfx_color(217,217,217);
			for(j=0;j<3;j++){
			gfx_fillrectangle(803+(i*66),364+(j*113),50,50);
			}
	}
	
	gfx_color(33,140,124);
	gfx_fillrectangle(803,364,50,50);
	
	for(i=0;i<5;i++){
		gfx_color(0,0,0);
		gfx_text(firstExample[i],823+(i*66),393);
	}
	
	//yellow box and text
	gfx_color(204,142,53);
	gfx_fillrectangle(869,477,50,50);
	
	for(i=0;i<5;i++){
		gfx_color(0,0,0);
		gfx_text(secondExample[i],823+(i*66),506);
	}
	
	//grey box and text
	gfx_color(132,129,122);
	gfx_fillrectangle(935,590,50,50);
	
	for(i=0;i<5;i++){
		gfx_color(0,0,0);
		gfx_text(thirdExample[i],823+(i*66),619);
	}
	
	//back button
	gfx_color(217,217,217);
	gfx_fillrectangle(810,771,300,100);
	gfx_color(0,0,0);
	gfx_text("BACK",948,824);

	do
	{
		c = gfx_wait();
		x = gfx_xpos();
		y = gfx_ypos();
	
		if (c == 0x01 && x >= 810 && x <= 1110 && y >= 771 && y <= 871)
		{
			break;
			
		}
	} while (1);
}
/*
Waiting for user input (from mouse) to choose 1.New Game 2.Info 3.Exit and returning value 1,2,3

*/
int option()
{
	char c;
	int xMouse, yMouse;
	
	do{
	c = gfx_wait();
	xMouse = gfx_xpos();
	yMouse = gfx_ypos();
	
	if (c == 0x01 && xMouse >= 810 && xMouse <= 1110 && yMouse >= 395 && yMouse <= 495)
	{
		return 1;
	}
	else if (c == 0x01 && xMouse >= 810 && xMouse <= 1110 && yMouse >= 540 && yMouse <= 640)
	{
		return 2;
	}
	else if (c == 0x01 && xMouse >= 810 && xMouse <= 1110 && yMouse >= 685 && yMouse <= 785)
	{
		return 3;
	}
	}while (1);
}
/*
Graphic for game exit and exit button 

*/
void exitGame()
{
	int xsize = 1920;
	int ysize = 1080;
	
	char c;
	int x, y;
	
	//gfx_open(xsize,ysize,"exit");
	gfx_clear_color(247,241,227);
	gfx_clear();
	
	gfx_color(0,0,0);
	gfx_fillrectangle(0,0,500,1080);
	gfx_fillrectangle(1420,0,500,1080);
	
	
	gfx_text("THANK YOU FOR PLAYING WORDLE",880,280);
	gfx_text("PLAY AGAIN SOON",920,400);
	
	//button
	gfx_color(132,129,122);
	gfx_fillrectangle(815,497,300,100);
	gfx_color(209,204,192);
	gfx_fillrectangle(802,482,300,100);
	
	gfx_color(0,0,0);
	gfx_text("EXIT",950,540);
	
	do
	{
		c = gfx_wait();
		x = gfx_xpos();
		y = gfx_ypos();
	
		if (c == 0x01 && x >= 860 && x <= 1060 && y >= 515 && y <= 565)
		{
			break;
			
		}
	} while (1);
}

/*
Reading dictionary file 

*/
void getWordList(char** wordList)
{
	FILE *wordsfile = fopen("dict.txt","r");
	char *word = malloc(6 * sizeof(char));
	int i = 0;
	
	while (fscanf(wordsfile,"%s", word) != EOF && i < 200){
		wordList[i] = word;
		i++;
		word = malloc(6 * sizeof(char));
	}
	fclose(wordsfile);
}
/*
Choosing random word using rand() by referring current time seeds
*/

char *pickWord(char** wordList)
{
	srand(time(NULL));
	return wordList[rand()%200];
}
/*
Only returning valid word
*/

int validCheck(char* g, char** wordList)
{
	int i = 0;
	char tempGuess[5];
	
	strncpy(tempGuess,g,5);
	
	while (i < 200){
		if (strcmp(tempGuess, wordList[i]) == 0){
			return 1;
			break;
		}
		else{
			i++;
		}
	}
	return 0;
}
/*check wether the guesses are the correct answer with the random picked word*/

char *guessCheck(char *g, char* w)
{
	char *c = calloc(6,sizeof(char));
	
	strcpy(c,"-----");
	for (int m = 0; m < 5; m++){
		for (int n = 0; n < 5; n++){
			if (g[m] == w[n]){
				if (m==n){
					c[m] = '*';
				}
				else{
					c[m] = '#';
				}
				break;
			}
		}
	}
	c[6] = '\0';
	printf("%s\n%s\n%s\n",w,g,c);
	return c;
}
/*wordle color changes*/

void changeColor(char* g, char* c, int numGuess)
{
	char *tempGuess = malloc(2*sizeof(char));
	
	for (int i = 0; i < 5; i++){
		Sleep(250000);
		memset(tempGuess,0,sizeof(tempGuess));
		if (c[i] == '*'){
			gfx_color(83,141,78);
		}
		else if (c[i] == '#'){
			gfx_color(181,159,59);
		}
		else {
			gfx_color(128,128,128);
		}
		gfx_fillrectangle(740+(i*85),(150+numGuess*85),75,75);
		gfx_color(0,0,0);
		gfx_rectangle(740+(i*85),(150+numGuess*85),75,75);
		*tempGuess = g[i];
		gfx_color(0,0,0);
		gfx_text(tempGuess,775+(i*85),190+(numGuess*85));
		gfx_flush();
	}
	free(tempGuess);
}


void changeKeyboard(char* g, char* c)
{
	char *tempGuess2 = malloc(2*sizeof(char));
	int xPos,yPos;
	
	
	for (int i = 0; i < 5; i++){
		memset(tempGuess2,0,sizeof(tempGuess2));
		if (c[i] == '*'){
			gfx_color(83,141,78);
		}
		else if (c[i] == '#'){
			gfx_color(181,159,59);
		}
		else {
			gfx_color(58,58,60);
		}
		*tempGuess2 = g[i];
		
		if (g[i] > 77){
			yPos = 1;
		}
		else{
			yPos = 0;
		}
		xPos = g[i] - 65;
		gfx_fillrectangle(660+(xPos%13)*45,680+yPos*60,40,55);
		gfx_color(255,255,255);
		gfx_text(tempGuess2,678+(xPos%13)*45,710+yPos*60);
	}
	free(tempGuess2);
}

/*function and graphic for end page */
void endPage(int result, char *w)
{
	char e;
	int xMouse3,yMouse3;

	
	gfx_clear_color(26,26,27);//change background color
	gfx_clear();
		
	gfx_color(33,140,124);
	gfx_fillrectangle(0,0,500,1080);
	gfx_color(204,142,53);
	gfx_fillrectangle(1420,0,500,1080);
	
	sleep(0.5);
	
	
		
	gfx_text("The correct word was: ",850,390);
	gfx_text(w,1000,390);

	
	
	gfx_color(132,129,122);
	gfx_fillrectangle(815,497,300,100);
	gfx_color(209,204,192);
	gfx_fillrectangle(802,482,300,100);
	gfx_color(0,0,0);
	gfx_text("Back to Main Menu",900,535);
		
	
	
	do{
		e = gfx_wait();
		xMouse3 = gfx_xpos();
		yMouse3 = gfx_ypos();
		
		if (e == 0x01 && xMouse3 >= 802 && xMouse3 <= 1102 && yMouse3 >= 482 && yMouse3 <= 582)
		{
			break;
		}
		
	}while (e != 0x01 || xMouse3 < 280 || xMouse3 > 430 || yMouse3 < 350 || yMouse3 > 390);

}
/*function to create keyboard GUI*/
void keyboardDisplay()
{
	int i,j,k;
	char *alphabet[26]={"A","B","C","D","E","F","G","H","I","J","K","L","M",
			    "N","O","P","Q","R","S","T","U","V","W","X","Y","Z"};
	
	//background
	gfx_clear_color(209,204,192);
	gfx_clear();	
	//wordle
	//w
	gfx_color(33,140,124);
	gfx_fillrectangle(610,40,20,80);
	gfx_fillrectangle(650,60,20,60);
	gfx_fillrectangle(690,40,20,80);
	gfx_fillrectangle(630,120,60,20);
	
	//o
	gfx_color(217,217,217);
	gfx_fillrectangle(750,40,60,20);
	gfx_fillrectangle(730,60,20,60);
	gfx_fillrectangle(810,60,20,60);
	gfx_fillrectangle(750,120,60,20);
	
	//r
	gfx_color(132,129,122);
	gfx_fillrectangle(850,40,20,100);
	gfx_fillrectangle(870,40,60,20);
	gfx_fillrectangle(870,80,60,20);
	gfx_fillrectangle(930,60,20,20);
	gfx_fillrectangle(910,100,20,20);
	gfx_fillrectangle(930,120,20,20);
	
	//d
	gfx_color(132,129,122);
	gfx_fillrectangle(970,40,20,100);
	gfx_fillrectangle(990,40,60,20);
	gfx_fillrectangle(990,120,60,20);
	gfx_fillrectangle(1050,60,20,60);
	
	//l
	gfx_color(217,217,217);
	gfx_fillrectangle(1090,40,20,100);
	gfx_fillrectangle(1110,120,80,20);
	
	//e
	gfx_color(204,142,53);
	gfx_fillrectangle(1210,40,20,100);
	gfx_fillrectangle(1230,40,80,20);
	gfx_fillrectangle(1230,80,60,20);
	gfx_fillrectangle(1230,120,80,20);
	
	gfx_color(0,0,0);
	gfx_fillrectangle(0,0,500,1080);
	gfx_fillrectangle(1420,0,500,1080);
	
	
	//keyboard boxes line 1,2,3
	for (int row = 0; row < 2; row++){
		for (int col = 0; col < 13; col++){
			gfx_color(0,0,0);
			gfx_rectangle(660+col*45,680+row*60,40,55);
		}	
	}
	
	//keyboard character line 1,2
	for (int row = 0; row < 13; row++){
		for (int col = 0; col < 13; col++){
			gfx_text(alphabet[row],678+(row*45),710);
		}	
	}
	for(i=0,k=13;k<26;k++,i++){
		gfx_text(alphabet[k],678+(i*45),770);	
	}	
	
	//enter & backspace boxes
	gfx_rectangle(700,810,65,55);
	gfx_rectangle(1125,810,65,55);
	gfx_text("ENTER",720,839);
	gfx_text("CLEAR",1145,839);
	

	
}
/*function to create graphic button for clear guesses*/
void clearGuess(int numGuess)
{
	int i;
	for (i = 0; i < 5; i++){
			gfx_color(209,204,192);//change color to background
			gfx_fillrectangle(740+(i*85),150+(numGuess*85),75,75);
			gfx_color(0,0,0);//black border
			gfx_rectangle(740+(i*85),150+(numGuess*85),75,75);
		}
}
/*function for all alphabets' button and enter button */
void *getGuess(char *guess, int numGuess)
{
		int k = 0;
		
		while (k<6){
		
		gfx_color(0,0,0);
		char c = gfx_wait();
		int  x = gfx_xpos();
		int  y = gfx_ypos();
			
		if (k != 5){		
		
			if (c = 0x01 && x >= 660 && x <= 700 && y >= 680 && y <= 735){
				guess[k] = 'A';
				gfx_text("A",775+(k*85),190+(numGuess*85));
				k++;
			}
			else if (c = 0x01 && x >= 705 && x <= 745 && y >= 680 && y <=  735){
				guess[k] = 'B';
				gfx_text("B",775+(k*85),190+(numGuess*85));
				k++;
			}
			else if (c = 0x01 && x >= 750 && x <= 790 && y >= 680 && y <= 735){
				guess[k] = 'C';
				gfx_text("C",775+(k*85),190+(numGuess*85));
				k++;
			}
			else if (c = 0x01 && x >= 795 && x <= 835 && y >= 680 && y <= 735){
				guess[k] = 'D';
				gfx_text("D",775+(k*85),190+(numGuess*85));
				k++;
			}
			else if (c = 0x01 && x >= 840 && x <= 880 && y >= 680 && y <= 735){
				guess[k] = 'E';
				gfx_text("E",775+(k*85),190+(numGuess*85));
				k++;
			}
			else if (c = 0x01 && x >= 885 && x <= 925 && y >= 680 && y <= 735){
				guess[k] = 'F';
				gfx_text("F",775+(k*85),190+(numGuess*85));
				k++;
			}
			else if (c = 0x01 && x >= 930 && x <= 970 && y >= 680 && y <= 735){
				guess[k] = 'G';
				gfx_text("G",775+(k*85),190+(numGuess*85));
				k++;
			}
			else if (c = 0x01 && x >= 975 && x <= 1015 && y >= 680 && y <= 735){
				guess[k] = 'H';
				gfx_text("H",775+(k*85),190+(numGuess*85));
				k++;
			}
			else if (c = 0x01 && x >= 1020 && x <= 1060 && y >= 680 && y <= 735){
				guess[k] = 'I';
				gfx_text("I",775+(k*85),190+(numGuess*85));
				k++;
			}
			else if (c = 0x01 && x >= 1065 && x <= 1105 && y >= 680 && y <= 735){
				guess[k] = 'J';
				gfx_text("J",775+(k*85),190+(numGuess*85));
				k++;
			}
			else if (c = 0x01 && x >= 1110 && x <= 1150 && y >= 680 && y <= 735){
				guess[k] = 'K';
				gfx_text("K",775+(k*85),190+(numGuess*85));
				k++;
			}
			else if (c = 0x01 && x >= 1155 && x <= 1195 && y >= 680 && y <= 735){
				guess[k] = 'L';
				gfx_text("L",775+(k*85),190+(numGuess*85));
				k++;
			}
			else if (c = 0x01 && x >= 1200 && x <= 1240 && y >= 680 && y <= 735){
				guess[k] = 'M';
				gfx_text("M",775+(k*85),190+(numGuess*85));
				k++;
			}
			else if (c = 0x01 && x >= 660 && x <= 700 && y >= 740 && y <= 795){
				guess[k] = 'N';
				gfx_text("N",775+(k*85),190+(numGuess*85));
				k++;
			}
			else if (c = 0x01 && x >= 705 && x <= 745 && y >= 740 && y <= 795){
				guess[k] = 'O';
				gfx_text("O",775+(k*85),190+(numGuess*85));
				k++;
			}
			else if (c = 0x01 && x >= 750 && x <= 790 && y >= 740 && y <= 795){
				guess[k] = 'P';
				gfx_text("P",775+(k*85),190+(numGuess*85));
				k++;
			}
			else if (c = 0x01 && x >= 795 && x <= 835 && y >= 740 && y <= 795){
				guess[k] = 'Q';
				gfx_text("Q",775+(k*85),190+(numGuess*85));
				k++;
			}
			else if (c = 0x01 && x >= 840 && x <= 880 && y >= 740 && y <= 795){
				guess[k] = 'R';
				gfx_text("R",775+(k*85),190+(numGuess*85));
				k++;
			}
			else if (c = 0x01 && x >= 885 && x <= 925 && y >= 740 && y <= 795){
				guess[k] = 'S';
				gfx_text("S",775+(k*85),190+(numGuess*85));
				k++;
			}
			else if (c = 0x01 && x >= 930 && x <= 970 && y >= 740 && y <= 795){
				guess[k] = 'T';
				gfx_text("T",775+(k*85),190+(numGuess*85));
				k++;
			}
			else if (c = 0x01 && x >= 975 && x <= 1015 && y >= 740 && y <= 795){
				guess[k] = 'U';
				gfx_text("U",775+(k*85),190+(numGuess*85));
				k++;
			}
			else if (c = 0x01 && x >= 1020 && x <= 1060 && y >= 740 && y <= 795){
				guess[k] = 'V';
				gfx_text("V",775+(k*85),190+(numGuess*85));
				k++;
			}
			else if (c = 0x01 && x >= 1065 && x <= 1105 && y >= 740 && y <= 795){
				guess[k] = 'W';
				gfx_text("W",775+(k*85),190+(numGuess*85));
				k++;
			}
			else if (c = 0x01 && x >= 1110 && x <= 1150 && y >= 740 && y <= 795){
				guess[k] = 'X';
				gfx_text("X",775+(k*85),190+(numGuess*85));
				k++;
			}
			else if (c = 0x01 && x >= 1155 && x <= 1195 && y >= 740 && y <= 795){
				guess[k] = 'Y';
				gfx_text("Y",775+(k*85),190+(numGuess*85));
				k++;
			}
			else if (c = 0x01 && x >= 1200 && x <= 1240 && y >= 740 && y <= 795){
				guess[k] = 'Z';
				gfx_text("Z",775+(k*85),190+(numGuess*85));
				k++;
			}
			
			else if (c= 0x01 && x >=1125 && x <= 1190 && y >= 810 && y <= 865){
				clearGuess(numGuess);
				memset(guess-2,0,sizeof(guess));
				k = 0;
			}
			}
		else if (k ==5){
			if (c= 0x01 && x >=1125 && x <= 1190 && y >= 810 && y <= 865){
				clearGuess(numGuess);
				memset(guess-2,0,sizeof(guess));
				k = 0;
			}
			else if (c= 0x01 && x >=700 && x <= 765 && y >= 810 && y <= 865){
				k++;
			}
			}
	}
}
/*graphic for wordle boxes*/
void wordle(int numGuess)
{
	int i, j;

	for ( i = numGuess; i < 5; i++){
			gfx_color(0,0,0);
			for(j=0;j<6;j++){
			gfx_rectangle(740+(i*85),150+(j*85),75,75);
			}	
	}
}

/*function used to call all other functions in order to start new game*/
void playGame()
{
	int j;
	char guess[6];
	int guessNum;
	char** wordlist =  calloc(200, sizeof(char*));
	char word[6];
	char *current;
	
	memset(word,0,sizeof(word));
	getWordList(wordlist);//call word from the text file
	strcpy(word,pickWord(wordlist));
	printf("%s",word);
	fflush(stdout);

	gfx_clear_color(26,26,27);//change background color
	gfx_clear();

	keyboardDisplay();
	for (guessNum = 0; guessNum < 6; guessNum++){
		memset(guess,0,sizeof(guess));
		wordle(guessNum);
		getGuess(guess, guessNum);
		if (validCheck(guess,wordlist) == 0){
			gfx_color(255,0,0);
			gfx_fillrectangle(850,77,180,40);
			gfx_color(0,0,0);
			gfx_text("I N V A L I D  I N P U T",870,100);
			gfx_rectangle(850,77,180,40);
			gfx_flush();
			sleep(1.5);
			clearGuess(guessNum);
			gfx_color(0,0,0);
			gfx_fillrectangle(850,77,180,40);
			gfx_color(255,0,0);
			gfx_text("I N V A L I D  I N P U T",870,100);
			gfx_rectangle(850,77,180,40);
			guessNum--;
			continue;
		}
		current = guessCheck(guess,word);
		changeColor(guess,current,guessNum);
		changeKeyboard(guess,current);
		if (strcmp(current,"*****") == 0){
			break;
		}
		else if (guessNum == 5){
			break;
		}
		printf("\n");
		free(current);
	}	
	Sleep(700000);
	if (strcmp(current,"*****") == 0){
		endPage(1,&word[0]);
		gfx_flush();
	}
	else{
		endPage(0,&word[0]);
		gfx_flush();
	}
}

