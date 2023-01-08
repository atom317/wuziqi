#include <stdio.h>
#include <iostream>
using namespace std;
#include <windows.h>
#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0)  //������ⰴ���ĵ���¼�

#define ROW 20				//��������
#define COLUMN 20			//��������

//����λ�õ�״̬
#define EMPTY 0
#define BLACK 1
#define WHITE 2

//���̵İ˸�����
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
}SString;			//���Ķ���˳�򴢴�ṹ
POINT p;
HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
HWND h = GetForegroundWindow();
CONSOLE_FONT_INFO consoleCurrentFont;
SString sequence[53] = { {0,0},									//��һ�����ò���
					 {"011110",6},{"022220",6},					//1����   2
					 {"211110",6},{"011112",6},{"11101",5},{"11011",5},			//3-6����
					 {"122220",6},{"022221",6},{"22202",5},{"22022",5},			//7-10
					 {"01110",5},{"010110",6},{"011010",6},						//11-13����
					 {"02220",5},{"020220",6},{"022020",6},						//14-16
					 {"001112",6},{"211100",6},{"010112",6},{"211010",6},{"10011",5},{"11001",5},{"10101",5},	//17-23����
					 {"002221",6},{"122200",6},{"020221",6},{"122020",6},{"20022",5},{"22002",5},{"20202",5},	//24-30
					 {"001100",6},{"01010",5},{"010010",6},			//31-33 ���
					 {"002200",6},{"02020",5},{"020020",6},			//34-36
					 {"000112",6},{"211000",6},{"001012",6},{"210100",6},{"010012",6},{"210010",6},{"10001",5},	//37-43�߶�
					 {"000221",6},{"122000",6},{"002021",6},{"120200",6},{"020021",6},{"120020",6},{"20002",5}, //44-50
					 {"11111",5},{"22222",5}};						//51-53����
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
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);		//��ȡ����̨���
	SetConsoleTextAttribute(handle, uFore + uBack * 0x10);
}

void gotoxy(int x, int y)				//�ƶ���꺯��
{
	COORD pos;							//������λ�õĽṹ�����
	pos.X = x;
	pos.Y = y;
	HANDLE hout = GetStdHandle(STD_OUTPUT_HANDLE);//��ȡ����̨���
	SetConsoleCursorPosition(hout, pos);			//���ù��λ��
}

void HideCursor()						//���ع�꺯��
{
	CONSOLE_CURSOR_INFO curInfo; //��������Ϣ�Ľṹ�����
	curInfo.dwSize = 1; //���û��ֵ�Ļ������������Ч
	curInfo.bVisible = FALSE; //���������Ϊ���ɼ�
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE); //��ȡ����̨���
	SetConsoleCursorInfo(handle, &curInfo); //���ù����Ϣ
}

int X(int x, int direction, int distance)			////��ȡ��j,i��direction�����Ͼ���Ϊdistanceλ�õĺ�����
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

int Y(int y, int direction, int distance)			////��ȡ��j,i��direction�����Ͼ���Ϊdistanceλ�õ�������
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

Chess GetChess(int x, int y, int direction, int distance)	//��ȡ��x,y��direction�����Ͼ���Ϊdistanceλ�õ���Ϣ
{
	x = X(x, direction, distance);
	y = Y(y, direction, distance);
	if (x < 0 || y < 0 || x > COLUMN || y > ROW)	//Խ�緵��-1
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
					++i;					//�����ȽϺ����ַ�
					++j;
				}
				else
				{
					j = j - i + 1;			//ָ�����,��������ϲ�һ��,��Ϊi�Ǵ��㿪ʼ
					i = 0;
				}
			}
			if (i >= sequence[k].length)					//������ҳɹ������ض�Ӧ�±�
				return k;
		}
		if (sequence[k].length == 5)
		{
			j = 1;
			while (i < sequence[k].length && j < 10)
			{
				if (sequence[k].ch[i] == c[j])
				{
					++i;					//�����ȽϺ����ַ�
					++j;
				}
				else
				{
					j = j - i + 1;			//ָ�����
					i = 0;
				}
			}
			if (i >= sequence[k].length)					//������ҳɹ����������Ӧ����
				return k;
		}
	}
	return 0;			//����ʧ�ܣ�����0 
}

void AssessScore(int x, int y)
{
	int count[4] = {0};					//0,1,2,3�ֱ��¼C41,C42,H31,H32
	int k;
	char situation[11];			//��һ�������¼4�����򸽽�11��λ�õ����
	score[y][x] = 0;
	board[y][x] = BLACK;			//�����λ��Ϊ�����ǻ�������������
	for (int t = EAST; t <= NORTHWEST; t++)
	{
		for (int n = -5; n < 6; n++)
		{
			situation[n + 5] = GetChess(x, y, t, n) + '0';		//�����Ӧλ�ü�¼
		}
		k = Index_BF(situation);
		score[y][x] += seqscore[k];
		if (k <= 6 && k >= 3)			//����C41
			count[0]++;
		else if (k <= 10 && k >= 7)		//����C42
			count[1]++;
		else if (k <= 13 && k >= 11)	//����H31
			count[2]++;
		else if (k <= 16 && k >= 14)	//����H31
			count[3]++;
	}
	board[y][x] = WHITE;			//�����λ��Ϊ�����ǻ�������������
	for (int t = EAST; t <= NORTHWEST; t++)
	{
		for (int n = -5; n < 6; n++)
		{
			situation[n + 5] = GetChess(x, y, t, n) + '0';		//�����Ӧλ�ü�¼
		}
		k = Index_BF(situation);
		score[y][x] += seqscore[k];
		if (k <= 6 && k >= 3)			//����C41
			count[0]++;
		else if (k <= 10 && k >= 7)		//����C42
			count[1]++;
		else if (k <= 13 && k >= 11)	//����H31
			count[2]++;
		else if (k <= 16 && k >= 14)	//����H31
			count[3]++;
	}
	if (count[0] > 1)					//�����������
		score[y][x] += 40000;
	else if (count[1] > 1)				//�����������
		score[y][x] += 4000;
	else if (count[2] > 1 || count[0] + count[2] > 1)	//�����������
		score[y][x] += 3000;
	else if (count[3] > 1 || count[1] + count[3] > 1)	//�����������
		score[y][x] += 2560;
	board[y][x] = EMPTY;			//��ָֻ�ԭ״
}

void Upgrade(int x, int y)			//�����������Ӹ���������Χ����λ�÷���
{
	for (int m = EAST; m <= SOUTHEAST; m++)
	{
		for (int n = 1; n < 6; n++)					//����λ��ֻ����Ӱ�쵽����6�����ڵ���Ԫ��
		{
			if (GetChess(x, y, m, n) == EMPTY)		//�Կ������ӵ�λ��ĳһ�ض�����ķ���������������
			{
				AssessScore(X(x, m, n), Y(y, m, n));
			}
		}
	}
}	

void ComputerMove(int &x, int &y)		//�������壬ͨ��i,j���������λ��
{
	int max = score[0][0];
	for (int m = 0; m < COLUMN; m++)
	{
		for (int n = 0; n < ROW; n++)
		{
			if (max < score[n][m] && board[n][m] == EMPTY)		//���˴�Ϊ��ʱ
			{
				max = score[n][m];
				x = m;
				y = n;
			}
		}
	}
	gotoxy(4*x, 2*y);
	printf("��");
	score[y][x] = 0;
	board[y][x] = BLACK;
}

void PlayerMove(int& x, int& y)			//�������,ͨ��i,j���������λ��
{
	int flag = 1;
	//----------�Ƴ����ٱ༭ģʽ(����win10�û�)----------
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode;
	GetConsoleMode(hStdin, &mode);
	mode &= ~ENABLE_QUICK_EDIT_MODE;
	SetConsoleMode(hStdin, mode);

	//----------ѭ�����----------
	while (flag) {                      			//ѭ�����
		HideCursor();
		if (KEY_DOWN(VK_LBUTTON)) {  			//����������
			POINT p;
			GetCursorPos(&p);
			ScreenToClient(h, &p);               //��ȡ����ڴ����ϵ�λ��
			GetCurrentConsoleFont(hOutput, FALSE, &consoleCurrentFont); //��ȡ������Ϣ
			x = p.x / (4 * consoleCurrentFont.dwFontSize.X);
			y = p.y / (2 * consoleCurrentFont.dwFontSize.Y);
			if ((x > (p.x - 2 * consoleCurrentFont.dwFontSize.X) / (4 * consoleCurrentFont.dwFontSize.X) || (p.x > 0 && p.x < 2 * consoleCurrentFont.dwFontSize.X)) //�ж�xλ�ںϷ�����
				&& (y > (p.y - consoleCurrentFont.dwFontSize.Y) / (2 * consoleCurrentFont.dwFontSize.Y) || (p.y > 0 && p.y < consoleCurrentFont.dwFontSize.Y)))
			{
				if (x >= 0 && x < COLUMN && y >= 0 && y < ROW && board[y][x] == EMPTY)		//�������������Ҹ�λ��Ϊ��ʱ
				{
					flag = 0;
					gotoxy(4 * x, 2 * y);
					printf("��");
					board[y][x] = WHITE;
				}
			}
		}
		Sleep(100);                  			//�ȴ�100���룬����CPUռ��
	}
}

Status isFull()			//�Ƿ�ƽ�֣��������Ƿ�����
{
	for (int i = 0; i < ROW; i++)
	{
		for (int j = 0; j < COLUMN; j++)
		{
			if (board[i][j] == EMPTY)			//���п�λ
				return FALSE;
		}
	}
	return TRUE;
}

Chess JudgeWin(int x, int y)		//�������������ж��Ƿ������ʤ��
{
	int a ,b;
	for (a = 1; board[y][x + a] == board[y][x] && x + a < COLUMN; a++);			//�������ͬɫ����������
	for (b = 1; board[y][x - b] == board[y][x] && x - b > -1; b++);				//�������ͬɫ����������
	if (a + b > 5)		//�������ͬɫ�������Ѿ��������
		return board[y][x];				//���ش˴�������ɫ������ʤ������ɫ
	for (a = 1; board[y + a][x] == board[y][x] && y + a < ROW; a++);		//��������ͬɫ����������
	for (b = 1; board[y - b][x] == board[y][x] && y - b > -1; b++);			//��������ͬɫ����������
	if (a + b > 5)		//�������ͬɫ�������Ѿ��������
		return board[y][x];				//���ش˴�������ɫ������ʤ������ɫ
	for (a = 1; board[y - a][x + a] == board[y][x] && y - a > -1 && x + a < COLUMN; a++);		//����б��ͬɫ����������
	for (b = 1; board[y + b][x - b] == board[y][x] && y + b < ROW && x - b > -1; b++);		//����б��ͬɫ����������
	if (a + b > 5)		//���б��ͬɫ�������Ѿ��������
		return board[y][x];				//���ش˴�������ɫ������ʤ������ɫ
	for (a = 1; board[y + a][x + a] == board[y][x] && y + a < ROW && x + a < COLUMN; a++);		//����б��ͬɫ����������
	for (b = 1; board[y - b][x - b] == board[y][x] && y - b > -1 && x - b > -1; b++);		//����б��ͬɫ����������
	if (a + b > 5)		//���б��ͬɫ�������Ѿ��������
		return board[y][x];				//���ش˴�������ɫ������ʤ������ɫ
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
	SetColor(0, 14);			//���ñ���ɫΪ����ɫ
	printf("�� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� \n");
	printf("��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   �� \n");
	printf("�� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� \n");
	printf("��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   �� \n");
	printf("�� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� \n");
	printf("��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   �� \n");
	printf("�� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� \n");
	printf("��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   �� \n");
	printf("�� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� \n");
	printf("��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   �� \n");
	printf("�� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� \n");
	printf("��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   �� \n");
	printf("�� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� \n");
	printf("��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   �� \n");
	printf("�� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� \n");
	printf("��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   �� \n");
	printf("�� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� \n");
	printf("��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   �� \n");
	printf("�� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� \n");
	printf("��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   �� \n");
	printf("�� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� \n");
	printf("��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   �� \n");
	printf("�� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� \n");
	printf("��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   �� \n");
	printf("�� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� \n");
	printf("��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   �� \n");
	printf("�� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� \n");
	printf("��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   �� \n");
	printf("�� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� \n");
	printf("��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   �� \n");
	printf("�� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� \n");
	printf("��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   �� \n");
	printf("�� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� \n");
	printf("��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   �� \n");
	printf("�� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� \n");
	printf("��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   �� \n");
	printf("�� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� \n");
	printf("��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   ��   �� \n");
	printf("�� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� �� \n");
}

int cover()
{
	int n;
	char c;
	system("cls");
	system("color 07");
	gotoxy(20, 10);
	printf("������������������           ����������������               ��        ��      ��  ");
	gotoxy(20, 11);
	printf("        ��                              ��             ����������  ��������������");
	gotoxy(20, 12);
	printf("        ��                          ��                    ������      ��      ��  ");
	gotoxy(20, 13);
	printf("        ��                          ��                   �� �� ��     ����������");
	gotoxy(20, 14);
	printf("  ��������������        ������������������������        ��  ��  ��    ��      ��");
	gotoxy(20, 15);
	printf("        ��      ��                  ��                 ��   ��   ��   ����������");
	gotoxy(20, 16);
	printf("        ��      ��                  ��               ��     ��        ��      ��");
	gotoxy(20, 17);
	printf("        ��      ��                  ��             ��       ��    ������������������    ");
	gotoxy(20, 18);
	printf("        ��      ��               �� ��                      ��       ��        ��");
	gotoxy(20, 19);
	printf("������������������������            ��                      ��     ��            ��");
	gotoxy(30, 30);
	printf("����������ѡ���Ⱥ���:1(����)/2(����)\n");
	gotoxy(30, 31);
	cin >> n;
	while (n != 1 && n != 2)
	{
		gotoxy(30, 32);
		printf("������Ϸ�����ѡ���Ⱥ���");
		gotoxy(30, 33);
		printf("      ");
		gotoxy(30, 33);
		cin >> n;
	}
	c = getchar();			//����\n�����Ӱ����������
	return n;
}

Status Game()				//��Ϸ���庯��
{
	int x = COLUMN / 2, y = ROW / 2, n;
	n = cover();			//��Ϸ����
	if (n == 1)				//���������
	{
		Initboard();				//���̳�ʼ��
		while (1)
		{
			Chess result;
			PlayerMove(x, y);
			Upgrade(x, y);									//���·���
			result = JudgeWin(x, y);
			if (result != EMPTY)	return result;			//����Ѿ����˽�����򷵻�ʤ����ɫ
			if (isFull() == TRUE)	return DRAW;					//��������������򷵻�ƽ��
			ComputerMove(x, y);
			Upgrade(x, y);
			result = JudgeWin(x, y);
			if (result != EMPTY)	return result;
			if (isFull())	return DRAW;
		}
	}
	else if (n == 2)				//�������
	{
		Initboard();				//���̳�ʼ��
		while (1)
		{
			Chess result;
			ComputerMove(x, y);
			Upgrade(x, y);									//���·���
			result = JudgeWin(x, y);
			if (result != EMPTY)	return result;			//����Ѿ����˽�����򷵻�ʤ����ɫ
			if (isFull() == TRUE)	return DRAW;					//��������������򷵻�ƽ��
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
		printf("������\n");
		gotoxy(80, 11);
		printf("�밴�»س�������...");
		c = getchar();
		system("cls");
		system("color 04");
		gotoxy(20, 10);
		printf("������������     ������������  ������������  ������������        ��      ��������������");
		gotoxy(20, 11);
		printf("��         ��    ��            ��            ��                 ����           ��");
		gotoxy(20, 12);
		printf("��          ��   ��            ��            ��                ��  ��          ��");
		gotoxy(20, 13);
		printf("��           ��  ������������  ������������  ����������       ��������         ��");
		gotoxy(20, 14);
		printf("��          ��   ��            ��            ��              ��      ��        ��");
		gotoxy(20, 15);
		printf("��         ��    ��            ��            ��             ��        ��       ��");
		gotoxy(20, 16);
		printf("������������     ������������  ��            ������������  ��          ��      ��\n");
	}
	else if (result == WHITE) {
		gotoxy(80, 10);
		printf("��Ӯ��\n");
		gotoxy(80, 11);
		printf("�밴�»س�������...");
		c = getchar();
		system("cls");
		system("color 09");
		gotoxy(8, 10);
		printf("��            ��  ����������   ������������ �������������� ��������������  �� �� ��    ��              ��\n");
		gotoxy(8, 11);
		printf(" ��          ��       ��       ��                 ��       ��          ��  ��     ��     ��          �� \n");
		gotoxy(8, 12);
		printf("  ��        ��        ��       ��                 ��       ��          ��  ��      ��      ��      ��\n");
		gotoxy(8, 13);
		printf("   ��      ��         ��       ��                 ��       ��          ��  ��     ��         ��  ��\n");
		gotoxy(8, 14);
		printf("    ��    ��          ��       ��                 ��       ��          ��  �� �� ��            ��\n");
		gotoxy(8, 15);
		printf("     ��  ��           ��       ��                 ��       ��          ��  ��  ��              ��\n");
		gotoxy(8, 16);
		printf("      ����            ��       ��                 ��       ��          ��  ��    ��            ��\n");
		gotoxy(8, 17);
		printf("       ��         ����������   ������������       ��       ��������������  ��      ��          ��\n");
	}
	else if (result == DRAW) {
		system("cls");
		printf("��������������Ϸƽ�֣�");
	}
	printf("\n\n\n\t\t\t\t\t�Ƿ�����һ��\n\t\t\t\t\tyes:������1/no:���������������ַ�\n");
	int n;
	gotoxy(50, 25);
	cin >> n;
	return n;
}

int main()
{
	int flag = 1, result;
	system("title ������"); 		//����cmd���ڵ�����
	system("mode con cols=120 lines=40"); 	//����cmd���ڵĴ�С
	do
	{
		result = Game();
		flag = ResultCover(result);
	} while (flag == 1);
}
