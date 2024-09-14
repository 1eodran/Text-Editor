typedef struct Text {
	int key;
	struct Text* prev;
	struct Text* next;
}Text;

typedef struct Head {
	Text* hp; //text구조체의 head를 가리킴
	int x, y; //현재 커서 x,y
	int rows, cols; //화면 최대 행,열
	int lines; //입력된 라인 수
	int Sline; //출력될 라인의 시작
	char* filename; //파일이름
}Head;

typedef struct Point {
	int x, y;
	int s; //시작라인
	int len; //단어 길이
	struct Point* prev;
	struct Point* next;
}Point;

Head* createHead(Head* head, int rows, int cols);
Text* createText(int key);
Point* createPoint(int x, int y, int s, int len);
int custom_getch();
void gotoxy(int x, int y);
void cursor(Head* head, int key);
void end(Head* head);
Point* insertPoint(Point* phead, int x, int y, int s, int len);
void find(Head* head);
void findWindow(Head* head, Point* ptmp);
void initWindow(Head* head);
void fileWindow(Head* head);
void createToolbar(Head* head);
void initToolbar(Head* head);
void fileToolbar(Head* head);
void Toolbar(Head* head);
Text* searchLineHead(Head* head);
int Add(Head* head, int key);
void Delete(Head* head);
void save(Head* head);
void editText(Head* head);

