#include <stdio.h>
#include <iostream>
using namespace std;
#include <windows.h>
#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0)  //用来检测按键的点击事件

#define ROW 20				//棋盘行数
#define COLUMN 20			//棋盘列数

//棋盘位置的状态
#define EMPTY 0
#define BLACK 1
#define WHITE 2

//棋盘的八个方向
#define EAST 1		
#define NORTHEAST 2	
#define NORTH 3		
#define NORTHWEST 4	
#define WEST 5
#define SOUTHWEST 6
#define SOUTH 7
#define SOUTHEAST 8

#define TRUE 1
#define FALSE 0 
#define DRAW 3
typedef int Status;
typedef int Chess;
typedef int direction;
typedef struct 
{
	char ch[7];
	int length;
}SString;			//串的定长顺序储存结构
POINT p;
HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
HWND h = GetForegroundWindow();
CONSOLE_FONT_INFO consoleCurrentFont;
SString sequence[53] = { {0,0},									//第一项闲置不用
					 {"011110",6},{"022220",6},					//1活四   2
					 {"211110",6},{"011112",6},{"11101",5},{"11011",5},			//3-6冲四
					 {"122220",6},{"022221",6},{"22202",5},{"22022",5},			//7-10
					 {"01110",5},{"010110",6},{"011010",6},						//11-13活三
					 {"02220",5},{"020220",6},{"022020",6},						//14-16
					 {"001112",6},{"211100",6},{"010112",6},{"211010",6},{"10011",5},{"11001",5},{"10101",5},	//17-23眠三
					 {"002221",6},{"122200",6},{"020221",6},{"122020",6},{"20022",5},{"22002",5},{"20202",5},	//24-30
					 {"001100",6},{"01010",5},{"010010",6},			//31-33 活二
					 {"002200",6},{"02020",5},{"020020",6},			//34-36
					 {"000112",6},{"211000",6},{"001012",6},{"210100",6},{"010012",6},{"210010",6},{"10001",5},	//37-43眠二
					 {"000221",6},{"122000",6},{"002021",6},{"120200",6},{"020021",6},{"120020",6},{"20002",5}, //44-50
					 {"11111",5},{"22222",5}};						//51-53连五
int seqscore[53] = {0,50000,5000,
					256,256,256,256,
					128,128,128,128,
					64,64,64,
					32,32,32,
					16,16,16,16,16,16,16,
					8,8,8,8,8,8,8,
					4,4,4,
					2,2,2,
					1,1,1,1,1,1,1,
					1,1,1,1,1,1,1,
					5000000,500000};
Chess board[ROW][COLUMN];
int score[ROW][COLUMN];
void SetColor(UINT uFore, UINT uBack) {
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);		//获取控制台句柄
	SetConsoleTextAttribute(handle, uFore + uBack * 0x10);
}

void gotoxy(int x, int y)				//移动光标函数
{
	COORD pos;							//定义光标位置的结构体变量
	pos.X = x;
	pos.Y = y;
	HANDLE hout = GetStdHandle(STD_OUTPUT_HANDLE);//获取控制台句柄
	SetConsoleCursorPosition(hout, pos);			//设置光标位置
}

void HideCursor()						//隐藏光标函数
{
	CONSOLE_CURSOR_INFO curInfo; //定义光标信息的结构体变量
	curInfo.dwSize = 1; //如果没赋值的话，光标隐藏无效
	curInfo.bVisible = FALSE; //将光标设置为不可见
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE); //获取控制台句柄
	SetConsoleCursorInfo(handle, &curInfo); //设置光标信息
}

int X(int x, int direction, int distance)			////获取（j,i）direction方向上距离为distance位置的横坐标
{
	switch (direction)
	{
		case EAST:
		case NORTHEAST:
		case SOUTHEAST:
				x += distance;
				break;
		case NORTHWEST:
		case WEST:
		case SOUTHWEST:
				x -= distance;
				break;
			default:break;
	}
	return x;
}

int Y(int y, int direction, int distance)			////获取（j,i）direction方向上距离为distance位置的纵坐标
{
	switch (direction)
	{
		case NORTH:
		case NORTHEAST:
		case NORTHWEST:
			y -= distance;
			break;
		case SOUTH:
		case SOUTHEAST:
		case SOUTHWEST:
			y += distance;
			break;
		default:break;
	}
	return y;
}

Chess GetChess(int x, int y, int direction, int distance)	//获取（x,y）direction方向上距离为distance位置的信息
{
	x = X(x, direction, distance);
	y = Y(y, direction, distance);
	if (x < 0 || y < 0 || x > COLUMN || y > ROW)	//越界返回-1
		return -1;
	else return board[y][x];
}

int Index_BF(char* c)
{
	int i, j;
	for (int k = 1; k < 53; k++)
	{
		i = 0;
		if (sequence[k].length == 6)
		{
			j = 0;
			while (i < sequence[k].length && j < 11)
			{
				if (sequence[k].ch[i] == c[j])
				{
					++i;					//继续比较后续字符
					++j;
				}
				else
				{
					j = j - i + 1;			//指针回溯,这里和书上不一样,因为i是从零开始
					i = 0;
				}
			}
			if (i >= sequence[k].length)					//如果查找成功，返回对应下标
				return k;
		}
		if (sequence[k].length == 5)
		{
			j = 1;
			while (i < sequence[k].length && j < 10)
			{
				if (sequence[k].ch[i] == c[j])
				{
					++i;					//继续比较后续字符
					++j;
				}
				else
				{
					j = j - i + 1;			//指针回溯
					i = 0;
				}
			}
			if (i >= sequence[k].length)					//如果查找成功，则加上相应分数
				return k;
		}
	}
	return 0;			//查找失败，返回0 
}

void AssessScore(int x, int y)
{
	int count[4] = {0};					//0,1,2,3分别记录C41,C42,H31,H32
	int k;
	char situation[11];			//用一个数组记录4个方向附近11个位置的棋局
	score[y][x] = 0;
	board[y][x] = BLACK;			//假设此位置为黑棋是会产生怎样的棋局
	for (int t = EAST; t <= NORTHWEST; t++)
	{
		for (int n = -5; n < 6; n++)
		{
			situation[n + 5] = GetChess(x, y, t, n) + '0';		//数组对应位置记录
		}
		k = Index_BF(situation);
		score[y][x] += seqscore[k];
		if (k <= 6 && k >= 3)			//出现C41
			count[0]++;
		else if (k <= 10 && k >= 7)		//出现C42
			count[1]++;
		else if (k <= 13 && k >= 11)	//出现H31
			count[2]++;
		else if (k <= 16 && k >= 14)	//出现H31
			count[3]++;
	}
	board[y][x] = WHITE;			//假设此位置为白棋是会产生怎样的棋局
	for (int t = EAST; t <= NORTHWEST; t++)
	{
		for (int n = -5; n < 6; n++)
		{
			situation[n + 5] = GetChess(x, y, t, n) + '0';		//数组对应位置记录
		}
		k = Index_BF(situation);
		score[y][x] += seqscore[k];
		if (k <= 6 && k >= 3)			//出现C41
			count[0]++;
		else if (k <= 10 && k >= 7)		//出现C42
			count[1]++;
		else if (k <= 13 && k >= 11)	//出现H31
			count[2]++;
		else if (k <= 16 && k >= 14)	//出现H31
			count[3]++;
	}
	if (count[0] > 1)					//二级进攻棋局
		score[y][x] += 40000;
	else if (count[1] > 1)				//二级防守棋局
		score[y][x] += 4000;
	else if (count[2] > 1 || count[0] + count[2] > 1)	//三级进攻棋局
		score[y][x] += 3000;
	else if (count[3] > 1 || count[1] + count[3] > 1)	//三级防守棋局
		score[y][x] += 2560;
	board[y][x] = EMPTY;			//棋局恢复原状
}

void Upgrade(int x, int y)			//根据最新棋子更新棋子周围可下位置分数
{
	for (int m = EAST; m <= SOUTHEAST; m++)
	{
		for (int n = 1; n < 6; n++)					//最新位置只可能影响到附近6格以内的六元组
		{
			if (GetChess(x, y, m, n) == EMPTY)		//对可能落子的位置某一特定方向的分数进行重新评分
			{
				AssessScore(X(x, m, n), Y(y, m, n));
			}
		}
	}
}	

void ComputerMove(int &x, int &y)		//电脑下棋，通过i,j传出下棋的位置
{
	int max = score[0][0];
	for (int m = 0; m < COLUMN; m++)
	{
		for (int n = 0; n < ROW; n++)
		{
			if (max < score[n][m] && board[n][m] == EMPTY)		//当此处为空时
			{
				max = score[n][m];
				x = m;
				y = n;
			}
		}
	}
	gotoxy(4*x, 2*y);
	printf("●");
	score[y][x] = 0;
	board[y][x] = BLACK;
}

void PlayerMove(int& x, int& y)			//玩家下棋,通过i,j传出下棋的位置
{
	int flag = 1;
	//----------移除快速编辑模式(对于win10用户)----------
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode;
	GetConsoleMode(hStdin, &mode);
	mode &= ~ENABLE_QUICK_EDIT_MODE;
	SetConsoleMode(hStdin, mode);

	//----------循环检测----------
	while (flag) {                      			//循环检测
		HideCursor();
		if (KEY_DOWN(VK_LBUTTON)) {  			//鼠标左键按下
			POINT p;
			GetCursorPos(&p);
			ScreenToClient(h, &p);               //获取鼠标在窗口上的位置
			GetCurrentConsoleFont(hOutput, FALSE, &consoleCurrentFont); //获取字体信息
			x = p.x / (4 * consoleCurrentFont.dwFontSize.X);
			y = p.y / (2 * consoleCurrentFont.dwFontSize.Y);
			if ((x > (p.x - 2 * consoleCurrentFont.dwFontSize.X) / (4 * consoleCurrentFont.dwFontSize.X) || (p.x > 0 && p.x < 2 * consoleCurrentFont.dwFontSize.X)) //判定x位于合法区域
				&& (y > (p.y - consoleCurrentFont.dwFontSize.Y) / (2 * consoleCurrentFont.dwFontSize.Y) || (p.y > 0 && p.y < consoleCurrentFont.dwFontSize.Y)))
			{
				if (x >= 0 && x < COLUMN && y >= 0 && y < ROW && board[y][x] == EMPTY)		//当坐标棋盘内且该位置为空时
				{
					flag = 0;
					gotoxy(4 * x, 2 * y);
					printf("○");
					board[y][x] = WHITE;
				}
			}
		}
		Sleep(100);                  			//等待100毫秒，减少CPU占用
	}
}

Status isFull()			//是否平局，即棋盘是否已满
{
	for (int i = 0; i < ROW; i++)
	{
		for (int j = 0; j < COLUMN; j++)
		{
			if (board[i][j] == EMPTY)			//若有空位
				return FALSE;
		}
	}
	return TRUE;
}

Chess JudgeWin(int x, int y)		//根据最新棋子判断是否有玩家胜利
{
	int a ,b;
	for (a = 1; board[y][x + a] == board[y][x] && x + a < COLUMN; a++);			//计算横向同色相连棋子数
	for (b = 1; board[y][x - b] == board[y][x] && x - b > -1; b++);				//计算横向同色相连棋子数
	if (a + b > 5)		//如果横向同色棋子数已经超过五个
		return board[y][x];				//返回此处棋子颜色，即获胜棋子颜色
	for (a = 1; board[y + a][x] == board[y][x] && y + a < ROW; a++);		//计算纵向同色相连棋子数
	for (b = 1; board[y - b][x] == board[y][x] && y - b > -1; b++);			//计算纵向同色相连棋子数
	if (a + b > 5)		//如果纵向同色棋子数已经超过五个
		return board[y][x];				//返回此处棋子颜色，即获胜棋子颜色
	for (a = 1; board[y - a][x + a] == board[y][x] && y - a > -1 && x + a < COLUMN; a++);		//计算斜上同色相连棋子数
	for (b = 1; board[y + b][x - b] == board[y][x] && y + b < ROW && x - b > -1; b++);		//计算斜上同色相连棋子数
	if (a + b > 5)		//如果斜上同色棋子数已经超过五个
		return board[y][x];				//返回此处棋子颜色，即获胜棋子颜色
	for (a = 1; board[y + a][x + a] == board[y][x] && y + a < ROW && x + a < COLUMN; a++);		//计算斜上同色相连棋子数
	for (b = 1; board[y - b][x - b] == board[y][x] && y - b > -1 && x - b > -1; b++);		//计算斜上同色相连棋子数
	if (a + b > 5)		//如果斜上同色棋子数已经超过五个
		return board[y][x];				//返回此处棋子颜色，即获胜棋子颜色
	return EMPTY;
}

void Initboard()
{
	system("cls");
	int i, j;
	for (i = 0; i < ROW; i++)
	{
		for (j = 0; j < COLUMN; j++)
		{
			score[i][j] = 0;
			board[i][j] = 0;
		}
	}
	SetColor(0, 14);			//设置背景色为淡黄色
	printf("┏ ━ ┳ ━ ┳ ━ ┳ ━ ┳ ━ ┳ ━ ┳ ━ ┳ ━ ┳ ━ ┳ ━ ┳ ━ ┳ ━ ┳ ━ ┳ ━ ┳ ━ ┳ ━ ┳ ━ ┳ ━ ┳ ━ ┓ \n");
	printf("┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃ \n");
	printf("┣ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ┫ \n");
	printf("┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃ \n");
	printf("┣ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ┫ \n");
	printf("┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃ \n");
	printf("┣ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ┫ \n");
	printf("┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃ \n");
	printf("┣ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ┫ \n");
	printf("┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃ \n");
	printf("┣ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ┫ \n");
	printf("┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃ \n");
	printf("┣ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ┫ \n");
	printf("┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃ \n");
	printf("┣ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ┫ \n");
	printf("┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃ \n");
	printf("┣ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ┫ \n");
	printf("┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃ \n");
	printf("┣ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ┫ \n");
	printf("┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃ \n");
	printf("┣ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ┫ \n");
	printf("┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃ \n");
	printf("┣ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ┫ \n");
	printf("┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃ \n");
	printf("┣ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ┫ \n");
	printf("┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃ \n");
	printf("┣ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ┫ \n");
	printf("┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃ \n");
	printf("┣ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ┫ \n");
	printf("┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃ \n");
	printf("┣ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ┫ \n");
	printf("┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃ \n");
	printf("┣ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ┫ \n");
	printf("┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃ \n");
	printf("┣ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ┫ \n");
	printf("┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃ \n");
	printf("┣ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ╋ ━ ┫ \n");
	printf("┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃   ┃ \n");
	printf("┗ ━ ┻ ━ ┻ ━ ┻ ━ ┻ ━ ┻ ━ ┻ ━ ┻ ━ ┻ ━ ┻ ━ ┻ ━ ┻ ━ ┻ ━ ┻ ━ ┻ ━ ┻ ━ ┻ ━ ┻ ━ ┻ ━ ┛ \n");
}

int cover()
{
	int n;
	char c;
	system("cls");
	system("color 07");
	gotoxy(20, 10);
	printf("■■■■■■■■■           ■■■■■■■■               ■        ■      ■  ");
	gotoxy(20, 11);
	printf("        ■                              ■             ■■■■■  ■■■■■■■");
	gotoxy(20, 12);
	printf("        ■                          ■                    ■■■      ■      ■  ");
	gotoxy(20, 13);
	printf("        ■                          ■                   ■ ■ ■     ■■■■■");
	gotoxy(20, 14);
	printf("  ■■■■■■■        ■■■■■■■■■■■■        ■  ■  ■    ■      ■");
	gotoxy(20, 15);
	printf("        ■      ■                  ■                 ■   ■   ■   ■■■■■");
	gotoxy(20, 16);
	printf("        ■      ■                  ■               ■     ■        ■      ■");
	gotoxy(20, 17);
	printf("        ■      ■                  ■             ■       ■    ■■■■■■■■■    ");
	gotoxy(20, 18);
	printf("        ■      ■               ■ ■                      ■       ■        ■");
	gotoxy(20, 19);
	printf("■■■■■■■■■■■■            ■                      ■     ■            ■");
	gotoxy(30, 30);
	printf("请输入数字选择先后手:1(先手)/2(后手)\n");
	gotoxy(30, 31);
	cin >> n;
	while (n != 1 && n != 2)
	{
		gotoxy(30, 32);
		printf("请输入合法数字选择先后手");
		gotoxy(30, 33);
		printf("      ");
		gotoxy(30, 33);
		cin >> n;
	}
	c = getchar();			//读掉\n否则会影响后面的输入
	return n;
}

Status Game()				//游戏主体函数
{
	int x = COLUMN / 2, y = ROW / 2, n;
	n = cover();			//游戏封面
	if (n == 1)				//计算机先手
	{
		Initboard();				//棋盘初始化
		while (1)
		{
			Chess result;
			PlayerMove(x, y);
			Upgrade(x, y);									//更新分数
			result = JudgeWin(x, y);
			if (result != EMPTY)	return result;			//如果已经有了结果，则返回胜者颜色
			if (isFull() == TRUE)	return DRAW;					//如果棋盘已满，则返回平局
			ComputerMove(x, y);
			Upgrade(x, y);
			result = JudgeWin(x, y);
			if (result != EMPTY)	return result;
			if (isFull())	return DRAW;
		}
	}
	else if (n == 2)				//玩家先手
	{
		Initboard();				//棋盘初始化
		while (1)
		{
			Chess result;
			ComputerMove(x, y);
			Upgrade(x, y);									//更新分数
			result = JudgeWin(x, y);
			if (result != EMPTY)	return result;			//如果已经有了结果，则返回胜者颜色
			if (isFull() == TRUE)	return DRAW;					//如果棋盘已满，则返回平局
			PlayerMove(x, y);
			Upgrade(x, y);
			result = JudgeWin(x, y);
			if (result != EMPTY)	return result;
			if (isFull())	return DRAW;
		}
	}
}

int ResultCover(int result)
{
	char c;
	SetColor(15, 0);
	if (result == BLACK) {
		gotoxy(80, 10);
		printf("你输了\n");
		gotoxy(80, 11);
		printf("请按下回车键继续...");
		c = getchar();
		system("cls");
		system("color 04");
		gotoxy(20, 10);
		printf("■■■■■■     ■■■■■■  ■■■■■■  ■■■■■■        ■      ■■■■■■■");
		gotoxy(20, 11);
		printf("■         ■    ■            ■            ■                 ■■           ■");
		gotoxy(20, 12);
		printf("■          ■   ■            ■            ■                ■  ■          ■");
		gotoxy(20, 13);
		printf("■           ■  ■■■■■■  ■■■■■■  ■■■■■       ■■■■         ■");
		gotoxy(20, 14);
		printf("■          ■   ■            ■            ■              ■      ■        ■");
		gotoxy(20, 15);
		printf("■         ■    ■            ■            ■             ■        ■       ■");
		gotoxy(20, 16);
		printf("■■■■■■     ■■■■■■  ■            ■■■■■■  ■          ■      ■\n");
	}
	else if (result == WHITE) {
		gotoxy(80, 10);
		printf("你赢了\n");
		gotoxy(80, 11);
		printf("请按下回车键继续...");
		c = getchar();
		system("cls");
		system("color 09");
		gotoxy(8, 10);
		printf("■            ■  ■■■■■   ■■■■■■ ■■■■■■■ ■■■■■■■  ■ ■ ■    ■              ■\n");
		gotoxy(8, 11);
		printf(" ■          ■       ■       ■                 ■       ■          ■  ■     ■     ■          ■ \n");
		gotoxy(8, 12);
		printf("  ■        ■        ■       ■                 ■       ■          ■  ■      ■      ■      ■\n");
		gotoxy(8, 13);
		printf("   ■      ■         ■       ■                 ■       ■          ■  ■     ■         ■  ■\n");
		gotoxy(8, 14);
		printf("    ■    ■          ■       ■                 ■       ■          ■  ■ ■ ■            ■\n");
		gotoxy(8, 15);
		printf("     ■  ■           ■       ■                 ■       ■          ■  ■  ■              ■\n");
		gotoxy(8, 16);
		printf("      ■■            ■       ■                 ■       ■          ■  ■    ■            ■\n");
		gotoxy(8, 17);
		printf("       ■         ■■■■■   ■■■■■■       ■       ■■■■■■■  ■      ■          ■\n");
	}
	else if (result == DRAW) {
		system("cls");
		printf("棋盘已下满，游戏平局！");
	}
	printf("\n\n\n\t\t\t\t\t是否再来一局\n\t\t\t\t\tyes:请输入1/no:请输入其它任意字符\n");
	int n;
	gotoxy(50, 25);
	cin >> n;
	return n;
}

int main()
{
	int flag = 1, result;
	system("title 五子棋"); 		//设置cmd窗口的名字
	system("mode con cols=120 lines=40"); 	//设置cmd窗口的大小
	do
	{
		result = Game();
		flag = ResultCover(result);
	} while (flag == 1);
}
