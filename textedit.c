#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h> // malloc
#include <string.h> // strcpy
#include "textedit.h"

#define ESC 27
#define Ctrl_f 6
#define Ctrl_s 19
#define Ctrl_q 17

#ifdef _WIN32 // Windows �÷���
    #include <conio.h> // _getch
    #include <windows.h> //�͹̳�
    #define UP 72
    #define DOWN 80
    #define RIGHT 77
    #define LEFT 75
    #define BACKSPACE 8
    #define HOME 71
    #define END 79
    #define PAGE_UP 73
    #define PAGE_DOWN 81
    #define ENTER 13
    #define CLEAR_SCREEN "cls"
#else // Linux �� macOS �÷���
    #include <sys/ioctl.h>
    #include <termios.h>
    #include <unistd.h>
    #define UP 65
    #define DOWN 66
    #define RIGHT 67
    #define LEFT 68
    #define BACKSPACE 127
    #define HOME 1
    #define END 4
    #define PAGE_UP 53
    #define PAGE_DOWN 54
    #define ENTER 10
    #define CLEAR_SCREEN "clear"
#endif

Head* createHead(Head* head,int rows, int cols)
{
    head->hp = NULL;
    head->x = 0;
    head->y = 0;
    head->rows = rows - 3; //�ϴܹ� ����
    head->cols = cols;
    head->lines = 0;
    head->Sline = 0;
    head->filename = NULL;
    return head;
}

Text* createText(int key)
{
    Text* tmp = (Text*)malloc(sizeof(Text));
    tmp->key = key;
    tmp->prev = NULL;
    tmp->next = NULL;
    return tmp;
}

Point* createPoint(int x, int y, int s, int len)
{
    Point* tmp = (Point*)malloc(sizeof(Point));
    tmp->x = x;
    tmp->y = y;
    tmp->s = s;
    tmp->len = len;
    tmp->prev = NULL;
    tmp->next = NULL;
    return tmp;
}

int custom_getch()
{
    #ifdef _WIN32
        return _getch();
    #else
        struct termios oldt, newt;
        int ch;

        // ���� �͹̳� ���� ����
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;

        // �Է� ��� ���� (���� ����, ���� ���)
        newt.c_lflag &= ~(ICANON | ECHO);

        // �͹̳� ���� ����
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);

        // Ű �Է� �ޱ�
        ch = getchar();

        // ���� �͹̳� �������� ����
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

        return ch;
    #endif  
}

void gotoxy(int x, int y)
{ 
    printf("\x1b[%d;%dH", y + 1, x + 1);
}

void cursor(Head* head, int key)
{
    Text* linehead = searchLineHead(head);
    Text* p = linehead;
    int cnt = 0;

    if (key == UP)
    {
        if ((head->y == 0 && head->Sline > 0 && head->Sline == 1)|| head->y == 1) //������ ù�� �϶�
        {
            //������ cnt
            p = linehead->prev;
            while (p != NULL)
            {
                cnt += 1;
                p = p->prev;
            }

            if (head->x > cnt) //�Ʒ����� �� ���
            {
                head->x = cnt;
                if (head->y == 0 && head->Sline > 0 && head->Sline == 1)
                {
                    head->Sline -= 1;
                    initWindow(head);
                }
                else head->y -= 1;
            }
            else //���ų� �Ʒ����� ª�� ���
            {
                if (head->y == 0 && head->Sline > 0 && head->Sline == 1)
                {
                    head->Sline -= 1;
                    initWindow(head);
                }
                else head->y -= 1;
            }
        }
        else if ((head->y == 0 && head->Sline > 0 && head->Sline > 1)|| head->y > 1)
        {
            //������ cnt
            while (p->prev->key != ENTER)
            {
                cnt += 1;
                p = p->prev;
            }

            if (head->x > cnt) //�Ʒ����� �� ���
            {
                head->x = cnt;
                if (head->y == 0 && head->Sline > 0 && head->Sline > 1)
                {
                    head->Sline -= 1;
                    initWindow(head);
                }
                else head->y -= 1;
            }
            else //���ų� �Ʒ����� ª�� ���
            {
                if (head->y == 0 && head->Sline > 0 && head->Sline > 1)
                {
                    head->Sline -= 1;
                    initWindow(head);
                }
                else head->y -= 1;
            }
        }
    }
    else if (key == DOWN)
    {
        //�� �����̻��϶�
        if (head->y == 0)
        {
            if (p != NULL) //head - NULL �� ��
            {
                while (p->next != NULL)
                {
                    if (p->key == ENTER) break; //�����̰� �ι�° �� ���� ��� X, �����̻�
                    p = p->next;
                }
            }

            if (p != NULL && p->key == ENTER)
            {
                //�Ʒ����� cnt
                p = p->next;
                while (p != NULL)
                {
                    if (p->key == ENTER) break; //�����̰� �ι�° �� ���� ��� X, �����̻�
                    cnt += 1;
                    p = p->next;
                }

                if (head->x > cnt) //������ �� ���
                {
                    head->x = cnt;
                    head->y += 1;
                }
                else //������ ª�ų� ���� ���
                {
                    head->y += 1;
                }
            }
        }
        else if (head->y > 0) //y>0�� ��
        {
            //������ �ص�� p�̵�
            p = p->next; 
            if (p != NULL)
            {
                while (p->key != ENTER)
                {
                    p = p->next;
                    if (p == NULL) break;
                }
            }

            if (p != NULL && p->key == ENTER)
            {
                //�Ʒ����� cnt
                p = p->next;
                while (p != NULL)
                {
                    if (p->key == ENTER) break; //�����̰� �ι�° �� ���� ��� X, �����̻�
                    cnt += 1;
                    p = p->next;
                }

                if (head->x > cnt) //������ �� ���
                {
                    head->x = cnt;
                    if (head->y == head->rows) //������ �� �� �� �׸��� �Ʒ��� ���� ���� ��
                    {
                        head->Sline += 1;
                        initWindow(head);
                    }
                    else if (head->y < head->rows) //������ ���� �ƴ� ��
                    {
                        head->y += 1;
                    }
                }
                else //������ ª�ų� ���� ���
                {
                    if (head->y == head->rows) 
                    {
                        head->Sline += 1;
                        initWindow(head);
                    }
                    else if (head->y < head->rows)
                    {
                        head->y += 1;
                    }
                }
            }
        }
    }
    else if (key == LEFT)
    {
        //Ŀ�� ����������
        if ((head->x == 0 && head->y == 0 && head->Sline == 1) || (head->x == 0 && head->y == 1 && head->Sline == 0)) //x=0�̰� ������ head���� ��
        {
            //������ cnt
            while (p->prev != NULL)
            {
                cnt += 1;
                p = p->prev;
            }
            //������ ������ Ŀ���� �̵�
            head->x = cnt; 
            if (head->y == 0 && head->Sline == 1)
            {
                head->Sline -= 1; //ù�ٿ� 0,0�̰� ������ sline == 0�� ��
                initWindow(head);
            }
            else if(head->Sline == 0) head->y -= 1;
        }
        else if ((head->x == 0 && head->y > 0 && head->Sline > 0) || (head->x == 0 && head->y > 1)) //x=0�̰� y>1���� Ŭ ��
        {
            //������ cnt
            p = linehead->prev;
            while (p->key != ENTER)
            {
                cnt += 1;
                p = p->prev;
            }
            //������ ������ Ŀ���� �̵�
            head->x = cnt;
            head->y -= 1;
        }
        else
        {
            if (head->x == 0 && head->y == 0)//ù�ٿ� 0,0�̰� ������ sline > 1�� ��
            {
                if(head->Sline > 1) head->Sline -= 1;
                initWindow(head);
            }
            else head->x -= 1;
        }
    }
    else if (key == RIGHT)
    {
        if (head->y == 0 && head->hp != NULL)
        {
            //cnt
            while (p->key != ENTER)
            {
                cnt += 1;
                if (p->next == NULL) break; //�ѹ����� ��
                p = p->next;
            }

            if (p->next != NULL) //�� �����̻�, y==0
            {
                if (head->x == cnt) //������ �ܾ� �� ��
                {
                    head->x = 0;
                    head->y += 1;
                }
                else if (head->x < cnt)
                {
                    head->x += 1;
                }
            }
            else //�ѹ����϶�, y==0
            {
                p = linehead;
                //���繮���� �� ���ڰ� �ƴϸ�
                if (head->x < cnt)
                {
                    head->x += 1;
                }
            }
        }
        else if (head->y > 0 && p->next != NULL)
        {
            //�������� ���ڸ� countcur
            while (p->next->key != ENTER)
            {
                cnt += 1;
                p = p->next;
                if (p->next == NULL) break;
            }

            if (p->next == NULL) //�ι����� ��
            {
                if (head->x < cnt) //���繮���� �� ���ڰ� �ƴϸ�
                {
                    head->x += 1;
                }
            }//�ؿ��� ������ �̻��� ��
            else if (head->x < cnt) //���繮���� �� ���ڰ� �ƴϸ�
            {
                head->x += 1;
            }
            else if (head->x == cnt && p->next->key == ENTER) //�� �����̻�, �� ���� �� ��
            {
                head->x = 0;
                if (head->y == head->rows) //������ �� ������ �ܾ��� ��
                {
                    head->Sline += 1;
                    initWindow(head);
                }
                else if (head->y < head->rows)
                {
                    head->y += 1;
                }
            }
        }
    }
    gotoxy(head->x, head->y);
}

void end(Head* head)
{
    Text* p = searchLineHead(head);
    int cnt = 0;
    if (head->y == 0)
    {
        if (p != NULL)
        {
            while (p->key != ENTER)
            {
                cnt += 1;
                p = p->next;
                if (p == NULL) break;
            }
        }
    }
    else if (head->y > 0)
    {
        p = p->next;
        if (p != NULL)
        {
            while (p->key != ENTER)
            {
                cnt += 1;
                p = p->next;
                if (p == NULL) break;
            }
        }
    }
    head->x = cnt;
    gotoxy(head->x, head->y);
}

Point* insertPoint(Point* phead, int x, int y, int s, int len)
{
    Point* tmp = createPoint(x, y, s, len);
    if (phead == NULL)
    {
        phead = tmp;
        tmp->prev = phead;
        tmp->next = phead;
    }
    else
    {
        phead->prev->next = tmp;
        tmp->prev = phead->prev;
        phead->prev = tmp;
        tmp->next = phead;
    }
    return tmp; //phead�� find�Լ��� �������� �Ǿ��־ return �ʿ�
}

void find(Head* head)
{
    Point* phead = NULL;
    Point* ptmp;
    Text* tmp = head->hp;
    //Ž���� ��ǥ
    int fx = head->x;
    int fy = head->y;
    int fs = head->Sline;
    //Ž���� tmp ��ǥ
    int x = 0;
    int y = 0;
    int s = 0;
    int cnt = 0;
    //Ž���� point ��ǥ
    int px = 0;
    int py = 0;
    int ps = 0;
    char ch;
    int word[200] = { 0 };
    int len = 0;
    int key;

    //�ܾ� �Է�
    Toolbar(head);
    gotoxy(0, head->rows + 2); 
    printf("\033[7mFIND\033[0m : ");
    while (1) {
        ch = getchar();
        if (ch == '\n') break;
        word[len] = ch;
        len += 1;
    }
    //enter�� �������°� ����
    initWindow(head);
    /*Toolbar(head);
    gotoxy(0, head->rows + 2);
    printf("\033[7mFIND\033[0m : ");
    for (int i = 0; i < len-1; i++)
    {
        printf("%c", word[i]);
    }*/

    
    //�ܾ� Ž��, ��ǥ ����
    if (tmp != NULL)
    {
        while (tmp != NULL)
        {
            if (word[0] == tmp->key)
            {
                ////ù ���� ó�� x==0
                if (x == 0) if (tmp->key != ENTER) x += 1;
                //��ǥ �ʱ�ȭ
                px = x;
                py = y;
                ps = s;
                cnt += 1;
                if (len == cnt) //�ܾ� ã���� px,py,ps ���� ����
                {
                    cnt = 0; //cnt �ʱ�ȭ
                    phead = insertPoint(phead, px, py, ps, len);
                }

                tmp = tmp->next;
                if (tmp == NULL) break;
                if (tmp->key != ENTER) x += 1;
                if (tmp->key == ENTER)
                {
                    tmp = tmp->next;
                    x = 0;
                    y += 1;
                }
                if (y == head->rows + 1)
                {
                    s += head->rows + 1;
                    y = 0;
                }

                if (cnt == 1)
                {
                    //�����ܾ� ������ �����ܾ� ���� ��
                    for (int i = 1; i <= len; i++)
                    {
                        if (word[i] == tmp->key) cnt += 1;
                        else
                        {
                            cnt = 0;
                            break;
                        }
                        if (len == cnt) //�ܾ� ã���� px,py,ps ���� ����
                        {
                            cnt = 0; //cnt �ʱ�ȭ
                            phead = insertPoint(phead, px, py, ps, len);
                        }
                        if (i != len)
                        {
                            tmp = tmp->next;
                            if (tmp == NULL) break;
                            if (tmp->key != ENTER) x += 1;
                            if (tmp->key == ENTER)
                            {
                                tmp = tmp->next;
                                x = 0;
                                y += 1;
                            }
                            if (y == head->rows + 1)
                            {
                                s += head->rows + 1;
                                y = 0;
                            }
                        }
                    }
                }
            }
            else if(word[0] != tmp->key)//���� �ܾ� �ȸ����� ��
            {
                cnt = 0; // ������ �ܾ� ���� ��
                //ù ���� ó�� x==0
                if (x == 0)
                {
                    if (tmp->key != ENTER) x += 1;
                    if (tmp->key == ENTER)
                    {
                        tmp = tmp->next;
                        x = 0;
                        y += 1;
                    }
                    if (y == head->rows + 1)
                    {
                        s += head->rows + 1;
                        y = 0;
                    }
                    continue;
                }
                tmp = tmp->next;
                if (tmp == NULL) break;
                if (tmp->key != ENTER) x += 1;
                if (tmp->key == ENTER)
                {
                    tmp = tmp->next;
                    x = 0;
                    y += 1;
                }
                if (y == head->rows + 1)
                {
                    s += head->rows + 1;
                    y = 0;
                }
            }
        }
    }

    //Ž���� �ܾ� ������ ǥ��
    if (phead != NULL)
    {
        //ã�� ù �ܾ�� �̸� ������
        ptmp = phead->prev; //ù ��ǥ��
        if (ptmp != NULL)
        {
            //������ ���� �� Ŀ���� ����
            head->Sline = ptmp->s;
            head->x = ptmp->x + len;
            head->y = ptmp->y;
            findWindow(head, ptmp);
        }

        while (1)
        {
            key = custom_getch();
            if (key == 224 || key == 0 || key == ESC) //Ŀ��
            {
                key = custom_getch();
                if (key == '[' || key == '0')
                {
                    key = custom_getch();
                }
            }

            if (key == LEFT || key == UP) //�����ܾ�
            {
                ptmp = ptmp->next;
                head->Sline = ptmp->s;
                head->x = ptmp->x + len;
                head->y = ptmp->y;
                findWindow(head, ptmp);
            }
            else if (key == RIGHT || key == DOWN) //�����ܾ�
            {
                ptmp = ptmp->prev;
                head->Sline = ptmp->s;
                head->x = ptmp->x + len;
                head->y = ptmp->y;
                findWindow(head, ptmp);
            }
            else if (key == ESC) //���� ��ġ�� ������
            {
                head->x = fx;
                head->y = fy;
                head->Sline = fs;
                initWindow(head);
                break;
            }
            else if (key == ENTER) //Ž���ܾ�� ��ġ�� ������
            {
                head->x = ptmp->x + len - 1; //Ž���ܾ��� ���������� 
                head->y = ptmp->y;
                head->Sline = ptmp->s;
                initWindow(head);
                break;
            }
        }
    }
}

void findWindow(Head* head, Point* ptmp)
{
    Text* p = head->hp;
    Text* tmp = head->hp;
    int cnt = 1; //ù�� ī��Ʈ
    int cnt2 = 1;
    int xcnt = 1;
    int ycnt = 0;
    system(CLEAR_SCREEN);

    if (head->Sline > 0)
    {
        while (p != NULL)
        {
            if (p->key == ENTER) cnt += 1;
            if (head->Sline + 1 == cnt) break;
            p = p->next;
        }
        if (p != NULL) tmp = p->next; //tmp�� Sline�� �ص��
    }

    while (tmp != NULL)
    {
        if (tmp->key == ENTER)
        {
            cnt2 += 1;
            if (cnt2 == head->rows + 2) break; //cnt�� 1�� �����ؼ� 1�� �ƴ϶� 2++
            printf("%c", '\n');
            ycnt += 1;
            xcnt = 1;
            tmp = tmp->next;
        }
        else
        {
            if (ptmp->x == xcnt && ptmp->y == ycnt)
            {
                for (int i = 0; i <= ptmp->len - 1; i++)
                {
                    printf("\033[7m%c\033[0m", tmp->key);
                    xcnt += 1;
                    tmp = tmp->next;
                }
            }
            else
            {
                printf("%c", tmp->key);
                xcnt += 1;
                tmp = tmp->next;
            }
        }
    }
    gotoxy(ptmp->x + (ptmp->len - 1), ptmp->y);
}

void initWindow(Head* head)
{
    Text* p = head->hp;
    Text* tmp = head->hp;
    int cnt = 1; //ù�� ī��Ʈ
    int cnt2 = 1;
    system(CLEAR_SCREEN);
   
    if (head->Sline > 0) 
    {
        while (p != NULL)
        {
            if (p->key == ENTER) cnt += 1;
            if (head->Sline + 1 == cnt) break;
            p = p->next;
        }
        if(p != NULL) tmp = p->next; //tmp�� Sline�� �ص��
    }

    while (tmp != NULL)
    {
        if (tmp->key == ENTER)
        {
            cnt2 += 1;
            if (cnt2 == head->rows + 2) break; //cnt�� 1�� �����ؼ� 1�� �ƴ϶� 2++
            printf("%c", '\n');
        }
        else
        {
            printf("%c", tmp->key);
        }
        tmp = tmp->next;
    }
}

void fileWindow(Head* head)
{
    Text* tmp = head->hp;
    int cnt = 1;
    system(CLEAR_SCREEN);

    while (tmp != NULL)
    {
        if (tmp->key == ENTER)
        {
            cnt += 1;
            if (cnt == head->rows + 2) break; //cnt�� 1�� �����ؼ� 1�� �ƴ϶� 2++
            printf("%c", '\n');
        }
        else
        {
            printf("%c", tmp->key);
        }
        tmp = tmp->next;
    }
}

void createToolbar(Head* head)
{
    for (int i = 0; i <= head->rows; i++)
    {
        if (i == head->rows / 3)
        {
            printf("~");
            for (int i = 2; i <= head->cols / 4; i++)
            {
                printf(" ");
            }
            printf("Visual Text editor -- version 0.0.1");
        }
        else printf("~");
        if (i != head->rows) printf("\n");
    }
    gotoxy(0, head->rows + 1);
    for (int i = 1; i <= head->cols; i++)
    {
        printf("\033[7m \033[0m");
    }
    gotoxy(0, head->rows + 1);
    printf("\033[7m[No Name] - %d lines", head->lines);
    gotoxy(head->cols-20, head->rows + 1);
    printf("no ft | %d/%d \033[0m\n", head->x, head->y);
    printf("HELP:Ctrl-s=save | Ctrl-q=quit | Ctrl-f=find");
    gotoxy(head->x, head->y);
}

void initToolbar(Head* head)
{
    Text* tmp = head->hp;
    int cnt = 1;
    //lines ����
    if (tmp == NULL) head->lines = 0;
    else if (tmp != NULL)
    {
        while (tmp != NULL)
        {
            if (tmp->key == ENTER)
            {
                cnt += 1;
            }
            tmp = tmp->next;
        }
        if (cnt == 0) head->lines = 1;
        else if (cnt > 0) head->lines = cnt;
    }

    gotoxy(0, head->rows + 1);
    if (head->filename == NULL)
    {
        gotoxy(0, head->rows + 1);
        for (int i = 1; i <= head->cols; i++)
        {
            printf("\033[7m \033[0m");
        }
        gotoxy(0, head->rows + 1);
        printf("\033[7m[No Name] - %d lines", head->lines);
        gotoxy(head->cols - 20, head->rows + 1);
        printf("no ft | %d/%d \033[0m\n", head->x, head->y);
    }
    else
    {
        gotoxy(0, head->rows + 1);
        for (int i = 1; i <= head->cols-1; i++)
        {
            printf("\033[7m \033[0m");
        }
        gotoxy(0, head->rows + 1);
        printf("\033[7m[%s] - %d lines", head->filename, head->lines);
        gotoxy(head->cols - 20, head->rows + 1);
        printf("no ft | %d/%d \033[0m\n", head->x, head->y);
    }
    printf("HELP:Ctrl-s=save | Ctrl-q=quit | Ctrl-f=find");
    gotoxy(head->x, head->y);
}

void fileToolbar(Head* head)
{
    gotoxy(0, head->rows + 1);
    if (head->filename == NULL)
    {
        gotoxy(0, head->rows + 1);
        for (int i = 1; i <= head->cols; i++)
        {
            printf("\033[7m \033[0m");
        }
        gotoxy(0, head->rows + 1);
        printf("\033[7m[No Name] - %d lines", head->lines);
        gotoxy(head->cols - 20, head->rows + 1);
        printf("no ft | %d/%d \033[0m\n", head->x, head->y);
    }
    else
    {
        gotoxy(0, head->rows + 1);
        for (int i = 1; i <= head->cols - 1; i++)
        {
            printf("\033[7m \033[0m");
        }
        gotoxy(0, head->rows + 1);
        printf("\033[7m[%s] - %d lines", head->filename, head->lines);
        gotoxy(head->cols - 20, head->rows + 1);
        printf("no ft | %d/%d \033[0m\n", head->x, head->y);
    }
    printf("HELP:Ctrl-s=save | Ctrl-q=quit | Ctrl-f=find");
    gotoxy(head->x, head->y);
}

void Toolbar(Head* head)
{
    Text* tmp = head->hp;
    int cnt = 1;
    //lines ����
    if (tmp == NULL) head->lines = 0;
    else if (tmp != NULL)
    {
        while (tmp != NULL)
        {
            if (tmp->key == ENTER)
            {
                cnt += 1;
            }
            tmp = tmp->next;
        }
        if (cnt == 0) head->lines = 1;
        else if (cnt > 0) head->lines = cnt;
    }
    if (head->filename == NULL)
    {
        gotoxy(0, head->rows + 1);
        for (int i = 1; i <= head->cols; i++)
        {
            printf("\033[7m \033[0m");
        }
        gotoxy(0, head->rows + 1);
        printf("\033[7m[No Name] - %d lines", head->lines);
        gotoxy(head->cols - 20, head->rows + 1);
        printf("no ft | %d/%d \033[0m\n", head->x, head->y);
    }
    else
    {
        gotoxy(0, head->rows + 1);
        for (int i = 1; i <= head->cols; i++)
        {
            printf("\033[7m \033[0m");
        }
        gotoxy(0, head->rows + 1);
        printf("\033[7m[%s] - %d lines", head->filename, head->lines);
        gotoxy(head->cols - 20, head->rows + 1);
        printf("no ft | %d/%d \033[0m\n", head->x, head->y);
    }

    //�Ʒ��� ����
    for (int i = 1; i <= head->cols; i++)
    {
        printf(" ");
    }
}

Text* searchLineHead(Head* head)
{
    Text* tmp = head->hp;
    int countline = -1;

    while (tmp != NULL)
    {
        if (tmp->key == ENTER) countline += 1;
        if ((head->y + head->Sline) - 1 == countline)
        {
            return tmp; //head�� '\n'
        }
        tmp = tmp->next; //������ \n�� linehead ��
    }

    return head->hp; //countline == -1
}

int Add(Head* head, int key)
{
    Text* tmp = createText(key);

    //head�� null�� �� ù����
    if (head->hp == NULL && head->y == 0)
    {
        head->hp = tmp;
        if (key == ENTER)
        {
            head->x = 0;
            head->y += 1;
        }
        else
        {
            head->x += 1;
        }
        initWindow(head);
        return 1;
    }
    
    Text* linehead = searchLineHead(head);
    Text* p = linehead;
    
    if (linehead->key == ENTER && linehead->next == NULL) //ùhead null�� ���� ������ ó���ż� �ߺ�X
    {
        tmp->prev = linehead;
        linehead->next = tmp;
        head->x = 0; //���ο� �����̹Ƿ� x = 0 ���� �ʱ�ȭ
        if (key == ENTER)
        {
            if (head->y == head->rows) //��������
            {
                head->x = 0;
                head->Sline += 1;
                initWindow(head);
            }
            else if (head->y < head->rows)
            {
                head->x = 0;
                head->y += 1;
                printf("%c", key);
            }
        }
        else 
        {
            head->x += 1;
            printf("%c", key);
        }
        return 1;
    }

    if (head->x == 0) //index�� 0�� ��
    {
        if (head->y == 0)
        {
            tmp->next = head->hp;
            head->hp->prev = tmp;
            head->hp = tmp;
            if (key == ENTER)
            {
                head->x = 0;
                head->y += 1;
                if (tmp->next != NULL)//�߰�����
                {
                    initWindow(head);
                }
                else
                {
                    printf("%c", key);
                }
            }
            else
            {
                head->x += 1;
                if (tmp->next != NULL)//�߰�����
                {
                    initWindow(head);
                }
                else
                {
                    printf("%c", key);
                }
            }
        }
        else if (head->y > 0)
        {
            tmp->prev = linehead;
            tmp->next = linehead->next;
            linehead->next->prev = tmp;
            linehead->next = tmp;
            if (key == ENTER)
            {
                if (head->y == head->rows) //��������
                {
                    head->x = 0;
                    head->Sline += 1;
                    initWindow(head);
                }
                else if (head->y < head->rows)
                {
                    head->x = 0;
                    head->y += 1;
                    if (tmp->next != NULL)//�߰�����
                    {
                        initWindow(head);
                    }
                    else
                    {
                        printf("%c", key);
                    }
                }
            }
            else
            {
                head->x += 1;
                if (tmp->next != NULL)//�߰�����
                {
                    initWindow(head);
                }
                else
                {
                    printf("%c", key);
                }
            }
        }
    }
    else if (head->x > 0) //index�� 1�̻��� ��
    {
        if (head->y > 0)
        {
            p = linehead->next; //y>0�� ���� p�� ��ġ�� �ٸ��� ������
        }
        
        for (int i = 0; i < head->x - 1; i++)
        {
            p = p->next;
            if (p == NULL) return 1;
        }
        //�������
        if (p->next != NULL) //�߰����� �� ��
        {
            p->next->prev = tmp;
        }
        tmp->prev = p;
        tmp->next = p->next;
        p->next = tmp;
        
        if (key == ENTER)
        {
            if (head->y == head->rows) //��������
            {
                head->x = 0;
                head->Sline += 1;
                initWindow(head);
            }
            else if (head->y < head->rows)
            {
                head->x = 0;
                head->y += 1;
                if (tmp->next != NULL)//�߰�����
                {
                    initWindow(head);
                }
                else
                {
                    printf("%c", key);
                }
            }
        }
        else
        {
            head->x += 1;
            if (tmp->next != NULL)//�߰�����
            {
                initWindow(head);
            }
            else
            {
                printf("%c", key);
            }
        }
    }
    return 1;
}

void Delete(Head* head)
{
    Text* linehead = searchLineHead(head);
    Text* p = linehead;
    int cnt = 0;
    if (head->x > 0)
    {
        if (head->y == 0 && head->Sline == 0)
        {
            //���� p ��ġ��
            while (head->x != cnt)
            {
                cnt += 1;
                if (head->x == cnt)  break;
                p = p->next;
            }
            //delete
            if (p->next == NULL)
            {
                if (p->prev == NULL) //ù���� ������ (�����)
                {
                    head->hp = p->next; //head ����
                    free(p);
                    head->x -= 1;
                }
                else
                {
                    p->prev->next = NULL;
                    free(p);
                    head->x -= 1;
                }
            }
            else if(p->next != NULL)
            {
                if (p->prev == NULL) //ù���� ������
                {
                    head->hp = p->next; //head ����
                    p->next->prev = NULL;
                    free(p);
                    head->x -= 1;
                }
                else
                {
                    p->prev->next = p->next;
                    p->next->prev = p->prev;
                    free(p);
                    head->x -= 1;
                }
            }
        }
        else if (head->Sline > 0 || head->y > 0)
        {
            p = p->next;
            //���� p ��ġ��
            while (head->x != cnt)
            {
                cnt += 1;
                if (head->x == cnt)  break;
                p = p->next;
            }
            //delete
            if (p->next == NULL)
            {
                if (p->prev == NULL) //ù���� ������ (�����)
                {
                    head->hp = p->next; //head ����
                    free(p);
                    head->x -= 1;
                }
                else
                {
                    p->prev->next = NULL;
                    free(p);
                    head->x -= 1;
                }
            }
            else
            {
                if (p->prev == NULL) //ù���� ������
                {
                    head->hp = p->next; //head ����
                    p->next->prev = NULL;
                    free(p);
                    head->x -= 1;
                }
                else
                {
                    p->prev->next = p->next;
                    p->next->prev = p->prev;
                    free(p);
                    head->x -= 1;
                }
            }
        }
    }
    else if ((head->x==0 && head->y == 0 && head->Sline > 0 ) || (head->x == 0 && head->y > 0)) //�� �� �̻��ϋ�, ������ enter����
    {
        //�ش� ���� ���� cnt2
        p = p->prev;
        if (p != NULL)
        {
            while (p->key != ENTER)
            {
                cnt += 1;
                p = p->prev;
                if (p == NULL) break; 
            }
        }

        //���� �ٰ� ����, ���� enter delete
        p = linehead;
        if (p->prev != NULL && p->next != NULL)
        {
            p->next->prev = p->prev;
            p->prev->next = p->next;
            free(p); //enter����
            head->x = cnt;
            if (head->y == 0 && head->Sline > 0) head->Sline -= 1;
            else head->y -= 1;
        }
        else if (p->prev == NULL && p->next != NULL)
        {
            head->hp = p->next; //head ����
            p->next->prev = p->prev;
            free(p); //enter����
            head->x = cnt;
            if (head->y == 0 && head->Sline > 0) head->Sline -= 1;
            else head->y -= 1;
        }
        else if (p->prev == NULL && p->next == NULL) //ù���� enter ������ (�����)
        {
            head->hp = p->next; //head ����
            free(p); //enter����
            head->x = cnt;
            if (head->y == 0 && head->Sline > 0) head->Sline -= 1;
            else head->y -= 1;
        }
        else if (p->prev != NULL && p->next == NULL) //ù���� enter ������
        {
            p->prev->next = p->next;
            free(p); //enter����
            head->x = cnt;
            if (head->y == 0 && head->Sline > 0) head->Sline -= 1;
            else head->y -= 1;
        }
    }
}

void save(Head* head)
{
    Text* tmp = head->hp;
    FILE* file;
    //���� ����
    if (head->filename != NULL)
    {
        file = fopen(head->filename, "wt");
        while (tmp != NULL)
        {
            fputc(tmp->key, file);
            tmp = tmp->next;
        }
        fclose(file);
    }
    //����Ϸ� ���
    Toolbar(head);
    gotoxy(0, head->rows + 2);
    printf("\033[7mSAVED!\033[0m");
    gotoxy(head->x, head->y);
}

void editText(Head* head)
{
    int key;
    char filename[100];
    int saved = 0;
    
    while (1)
    {
        key = custom_getch();

        if (key == 224 || key == 0 || key == ESC) //home, end, pageup down, Ŀ��
        {
            key = custom_getch();
            if (key == '[' || key == '0')
            {
                key = custom_getch();
                if (key == 'H') // home 
                {
                    head->x = 0;
                }
                else if (key == 'F')//end
                {
                    end(head);
                    initWindow(head);
                }
            }

            if (key == HOME)
            {
                if(key == 1) custom_getch(); //~����
                head->x = 0;
            }
            else if (key == END)
            {
                if (key == 4) custom_getch(); //~����
                end(head);
            }
            else if (key == PAGE_UP)
            {
                if(key == 53) custom_getch(); //~����

                if (head->Sline >= head->rows + 1)
                {
                    head->Sline -= head->rows + 1;
                    head->x = 0;
                    head->y = 0;
                }
                else if (head->Sline <= head->rows)
                {
                    head->Sline = 0;
                    head->x = 0;
                    head->y = 0;
                }
                initWindow(head);
            }
            else if (key == PAGE_DOWN)
            {
                if (key == 54) custom_getch(); //~����

                if (head->lines > head->Sline + head->rows + 1)
                {
                    head->Sline += head->rows + 1;
                    head->x = 0;
                    head->y = 0;
                }
                else if (head->lines <= head->Sline + head->rows + 1)
                {
                    head->x = 0;
                    head->y = head->lines - head->Sline + 1 - 2;
                }
                initWindow(head);
            }
            else if(key == UP || key == DOWN || key == RIGHT || key == LEFT) //Ŀ��
            {
                cursor(head, key);
            }
        }
        else if (key == ENTER) 
        {
            Add(head, ENTER);
        }
        else if (key == BACKSPACE)
        {
            Delete(head);
            initWindow(head);
        }
        else if (key == Ctrl_f)
        {
            find(head);
            //window�� ������ ���Ӱ� �ʱ�ȭ
            initToolbar(head);
            continue;
        }
        else if (key == Ctrl_s)
        {
            if (head->filename == NULL)
            {
                Toolbar(head);
                gotoxy(0, head->rows + 2);
                printf("\033[7mNAME\033[0m : ");
                gets(filename);
                head->filename = filename;
                //enter�� �������°� ����
                initWindow(head);
                Toolbar(head);
                gotoxy(0, head->rows + 2);
                printf("\033[7mNAME\033[0m : ");
            }
            save(head);
            saved = 1;
            continue;
        }
        else if (key == Ctrl_q)
        {
            Toolbar(head);
            gotoxy(0, head->rows + 2);
            printf("\033[7mPress 'ENTER' or 'Ctrl_q'\033[0m");
            key = custom_getch();
            if (key == Ctrl_q) //���� ���� ������
            {
                break;
            }
            else if (key == ENTER) //���� �� ������
            {
                initWindow(head);
                initToolbar(head);
                if (saved == 0) //q �ѹ��̰� saved = 0 : ������
                {
                    if (head->filename == NULL)
                    {
                        Toolbar(head);
                        gotoxy(0, head->rows + 2);
                        printf("\033[7mNAME\033[0m : ");
                        gets(filename);
                        head->filename = filename;
                        //enter�� �������°� ����
                        initWindow(head);
                        Toolbar(head);
                        gotoxy(0, head->rows + 2);
                        printf("\033[7mNAME\033[0m : ");
                    }
                    save(head);
                    saved = 1;
                    continue;
                }
                else if (saved == 1) //���������� �����Ѱ� ������ �����ϰ� ������
                {
                    break;
                }
                
            }
        }
        else // ���� �Է�, �߰����� �����ϰ�
        {
            if (head->x == head->cols) //cols�� ���̸�
            {
                Add(head, ENTER);
                initWindow(head);
            }
            Add(head, key);
        }

        initToolbar(head);
    }
}
