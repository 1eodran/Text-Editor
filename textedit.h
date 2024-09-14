typedef struct Text {
	int key;
	struct Text* prev;
	struct Text* next;
}Text;

typedef struct Head {
	Text* hp; //text����ü�� head�� ����Ŵ
	int x, y; //���� Ŀ�� x,y
	int rows, cols; //ȭ�� �ִ� ��,��
	int lines; //�Էµ� ���� ��
	int Sline; //��µ� ������ ����
	char* filename; //�����̸�
}Head;

typedef struct Point {
	int x, y;
	int s; //���۶���
	int len; //�ܾ� ����
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

