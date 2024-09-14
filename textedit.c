#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h> // malloc
#include <string.h> // strcpy
#include "textedit.h"

#define ESC 27
#define Ctrl_f 6
#define Ctrl_s 19
#define Ctrl_q 17

#ifdef _WIN32 // Windows 플랫폼
    #include <conio.h> // _getch
    #include <windows.h> //터미널
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
#else // Linux 및 macOS 플랫폼
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
    head->rows = rows - 3; //하단바 제외
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

        // 현재 터미널 설정 저장
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;

        // 입력 모드 수정 (버퍼 없이, 문자 모드)
        newt.c_lflag &= ~(ICANON | ECHO);

        // 터미널 설정 적용
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);

        // 키 입력 받기
        ch = getchar();

        // 이전 터미널 설정으로 복구
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
        if ((head->y == 0 && head->Sline > 0 && head->Sline == 1)|| head->y == 1) //윗줄이 첫줄 일때
        {
            //윗줄을 cnt
            p = linehead->prev;
            while (p != NULL)
            {
                cnt += 1;
                p = p->prev;
            }

            if (head->x > cnt) //아랫줄이 긴 경우
            {
                head->x = cnt;
                if (head->y == 0 && head->Sline > 0 && head->Sline == 1)
                {
                    head->Sline -= 1;
                    initWindow(head);
                }
                else head->y -= 1;
            }
            else //같거나 아랫줄이 짧은 경우
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
            //윗줄을 cnt
            while (p->prev->key != ENTER)
            {
                cnt += 1;
                p = p->prev;
            }

            if (head->x > cnt) //아랫줄이 긴 경우
            {
                head->x = cnt;
                if (head->y == 0 && head->Sline > 0 && head->Sline > 1)
                {
                    head->Sline -= 1;
                    initWindow(head);
                }
                else head->y -= 1;
            }
            else //같거나 아랫줄이 짧은 경우
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
        //두 문장이상일때
        if (head->y == 0)
        {
            if (p != NULL) //head - NULL 일 때
            {
                while (p->next != NULL)
                {
                    if (p->key == ENTER) break; //두줄이고 두번째 줄 널인 경우 X, 두줄이상
                    p = p->next;
                }
            }

            if (p != NULL && p->key == ENTER)
            {
                //아랫줄을 cnt
                p = p->next;
                while (p != NULL)
                {
                    if (p->key == ENTER) break; //두줄이고 두번째 줄 널인 경우 X, 두줄이상
                    cnt += 1;
                    p = p->next;
                }

                if (head->x > cnt) //윗줄이 긴 경우
                {
                    head->x = cnt;
                    head->y += 1;
                }
                else //윗줄이 짧거나 같을 경우
                {
                    head->y += 1;
                }
            }
        }
        else if (head->y > 0) //y>0일 때
        {
            //다음줄 해드로 p이동
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
                //아랫줄을 cnt
                p = p->next;
                while (p != NULL)
                {
                    if (p->key == ENTER) break; //두줄이고 두번째 줄 널인 경우 X, 두줄이상
                    cnt += 1;
                    p = p->next;
                }

                if (head->x > cnt) //윗줄이 긴 경우
                {
                    head->x = cnt;
                    if (head->y == head->rows) //마지막 줄 일 때 그리고 아래에 줄이 있을 때
                    {
                        head->Sline += 1;
                        initWindow(head);
                    }
                    else if (head->y < head->rows) //마지막 줄이 아닐 때
                    {
                        head->y += 1;
                    }
                }
                else //윗줄이 짧거나 같을 경우
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
        //커서 윗문장으로
        if ((head->x == 0 && head->y == 0 && head->Sline == 1) || (head->x == 0 && head->y == 1 && head->Sline == 0)) //x=0이고 윗줄이 head줄일 때
        {
            //윗줄을 cnt
            while (p->prev != NULL)
            {
                cnt += 1;
                p = p->prev;
            }
            //윗줄의 마지막 커서로 이동
            head->x = cnt; 
            if (head->y == 0 && head->Sline == 1)
            {
                head->Sline -= 1; //첫줄에 0,0이고 윗줄이 sline == 0일 때
                initWindow(head);
            }
            else if(head->Sline == 0) head->y -= 1;
        }
        else if ((head->x == 0 && head->y > 0 && head->Sline > 0) || (head->x == 0 && head->y > 1)) //x=0이고 y>1보다 클 때
        {
            //윗줄을 cnt
            p = linehead->prev;
            while (p->key != ENTER)
            {
                cnt += 1;
                p = p->prev;
            }
            //윗줄의 마지막 커서로 이동
            head->x = cnt;
            head->y -= 1;
        }
        else
        {
            if (head->x == 0 && head->y == 0)//첫줄에 0,0이고 윗줄이 sline > 1일 떄
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
                if (p->next == NULL) break; //한문장일 때
                p = p->next;
            }

            if (p->next != NULL) //두 문장이상, y==0
            {
                if (head->x == cnt) //마지막 단어 일 때
                {
                    head->x = 0;
                    head->y += 1;
                }
                else if (head->x < cnt)
                {
                    head->x += 1;
                }
            }
            else //한문장일때, y==0
            {
                p = linehead;
                //현재문장의 끝 글자가 아니면
                if (head->x < cnt)
                {
                    head->x += 1;
                }
            }
        }
        else if (head->y > 0 && p->next != NULL)
        {
            //현재줄의 글자를 countcur
            while (p->next->key != ENTER)
            {
                cnt += 1;
                p = p->next;
                if (p->next == NULL) break;
            }

            if (p->next == NULL) //두문장일 때
            {
                if (head->x < cnt) //현재문장의 끝 글자가 아니면
                {
                    head->x += 1;
                }
            }//밑에는 세문장 이상일 때
            else if (head->x < cnt) //현재문장의 끝 글자가 아니면
            {
                head->x += 1;
            }
            else if (head->x == cnt && p->next->key == ENTER) //두 문장이상, 끝 글자 일 때
            {
                head->x = 0;
                if (head->y == head->rows) //마지막 줄 마지막 단어일 때
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
    return tmp; //phead가 find함수에 전역으로 되어있어서 return 필요
}

void find(Head* head)
{
    Point* phead = NULL;
    Point* ptmp;
    Text* tmp = head->hp;
    //탐색전 좌표
    int fx = head->x;
    int fy = head->y;
    int fs = head->Sline;
    //탐색시 tmp 좌표
    int x = 0;
    int y = 0;
    int s = 0;
    int cnt = 0;
    //탐색시 point 좌표
    int px = 0;
    int py = 0;
    int ps = 0;
    char ch;
    int word[200] = { 0 };
    int len = 0;
    int key;

    //단어 입력
    Toolbar(head);
    gotoxy(0, head->rows + 2); 
    printf("\033[7mFIND\033[0m : ");
    while (1) {
        ch = getchar();
        if (ch == '\n') break;
        word[len] = ch;
        len += 1;
    }
    //enter로 내려가는거 보정
    initWindow(head);
    /*Toolbar(head);
    gotoxy(0, head->rows + 2);
    printf("\033[7mFIND\033[0m : ");
    for (int i = 0; i < len-1; i++)
    {
        printf("%c", word[i]);
    }*/

    
    //단어 탐색, 좌표 저장
    if (tmp != NULL)
    {
        while (tmp != NULL)
        {
            if (word[0] == tmp->key)
            {
                ////첫 글자 처리 x==0
                if (x == 0) if (tmp->key != ENTER) x += 1;
                //좌표 초기화
                px = x;
                py = y;
                ps = s;
                cnt += 1;
                if (len == cnt) //단어 찾으면 px,py,ps 정보 저장
                {
                    cnt = 0; //cnt 초기화
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
                    //같은단어 만나고 다음단어 부터 비교
                    for (int i = 1; i <= len; i++)
                    {
                        if (word[i] == tmp->key) cnt += 1;
                        else
                        {
                            cnt = 0;
                            break;
                        }
                        if (len == cnt) //단어 찾으면 px,py,ps 정보 저장
                        {
                            cnt = 0; //cnt 초기화
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
            else if(word[0] != tmp->key)//시작 단어 안만났을 때
            {
                cnt = 0; // 유사한 단어 없을 때
                //첫 글자 처리 x==0
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

    //탐색한 단어 색반전 표시
    if (phead != NULL)
    {
        //찾은 첫 단어는 미리 색반전
        ptmp = phead->prev; //첫 좌표로
        if (ptmp != NULL)
        {
            //색반전 끝난 후 커서를 위해
            head->Sline = ptmp->s;
            head->x = ptmp->x + len;
            head->y = ptmp->y;
            findWindow(head, ptmp);
        }

        while (1)
        {
            key = custom_getch();
            if (key == 224 || key == 0 || key == ESC) //커서
            {
                key = custom_getch();
                if (key == '[' || key == '0')
                {
                    key = custom_getch();
                }
            }

            if (key == LEFT || key == UP) //이전단어
            {
                ptmp = ptmp->next;
                head->Sline = ptmp->s;
                head->x = ptmp->x + len;
                head->y = ptmp->y;
                findWindow(head, ptmp);
            }
            else if (key == RIGHT || key == DOWN) //다음단어
            {
                ptmp = ptmp->prev;
                head->Sline = ptmp->s;
                head->x = ptmp->x + len;
                head->y = ptmp->y;
                findWindow(head, ptmp);
            }
            else if (key == ESC) //원래 위치로 나가기
            {
                head->x = fx;
                head->y = fy;
                head->Sline = fs;
                initWindow(head);
                break;
            }
            else if (key == ENTER) //탐색단어로 위치로 나가기
            {
                head->x = ptmp->x + len - 1; //탐색단어의 마지막으로 
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
    int cnt = 1; //첫줄 카운트
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
        if (p != NULL) tmp = p->next; //tmp를 Sline의 해드로
    }

    while (tmp != NULL)
    {
        if (tmp->key == ENTER)
        {
            cnt2 += 1;
            if (cnt2 == head->rows + 2) break; //cnt가 1로 시작해서 1이 아니라 2++
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
    int cnt = 1; //첫줄 카운트
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
        if(p != NULL) tmp = p->next; //tmp를 Sline의 해드로
    }

    while (tmp != NULL)
    {
        if (tmp->key == ENTER)
        {
            cnt2 += 1;
            if (cnt2 == head->rows + 2) break; //cnt가 1로 시작해서 1이 아니라 2++
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
            if (cnt == head->rows + 2) break; //cnt가 1로 시작해서 1이 아니라 2++
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
    //lines 세기
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
    //lines 세기
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

    //아래줄 제거
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
            return tmp; //head는 '\n'
        }
        tmp = tmp->next; //마지막 \n이 linehead 값
    }

    return head->hp; //countline == -1
}

int Add(Head* head, int key)
{
    Text* tmp = createText(key);

    //head가 null일 때 첫연결
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
    
    if (linehead->key == ENTER && linehead->next == NULL) //첫head null일 때는 위에서 처리돼서 중복X
    {
        tmp->prev = linehead;
        linehead->next = tmp;
        head->x = 0; //새로운 라인이므로 x = 0 으로 초기화
        if (key == ENTER)
        {
            if (head->y == head->rows) //마지막줄
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

    if (head->x == 0) //index가 0일 때
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
                if (tmp->next != NULL)//중간삽입
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
                if (tmp->next != NULL)//중간삽입
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
                if (head->y == head->rows) //마지막줄
                {
                    head->x = 0;
                    head->Sline += 1;
                    initWindow(head);
                }
                else if (head->y < head->rows)
                {
                    head->x = 0;
                    head->y += 1;
                    if (tmp->next != NULL)//중간삽입
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
                if (tmp->next != NULL)//중간삽입
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
    else if (head->x > 0) //index가 1이상일 때
    {
        if (head->y > 0)
        {
            p = linehead->next; //y>0일 때는 p의 위치가 다르기 떄문에
        }
        
        for (int i = 0; i < head->x - 1; i++)
        {
            p = p->next;
            if (p == NULL) return 1;
        }
        //연결시작
        if (p->next != NULL) //중간삽입 일 때
        {
            p->next->prev = tmp;
        }
        tmp->prev = p;
        tmp->next = p->next;
        p->next = tmp;
        
        if (key == ENTER)
        {
            if (head->y == head->rows) //마지막줄
            {
                head->x = 0;
                head->Sline += 1;
                initWindow(head);
            }
            else if (head->y < head->rows)
            {
                head->x = 0;
                head->y += 1;
                if (tmp->next != NULL)//중간삽입
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
            if (tmp->next != NULL)//중간삽입
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
            //현재 p 위치로
            while (head->x != cnt)
            {
                cnt += 1;
                if (head->x == cnt)  break;
                p = p->next;
            }
            //delete
            if (p->next == NULL)
            {
                if (p->prev == NULL) //첫글자 삭제시 (재시작)
                {
                    head->hp = p->next; //head 변경
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
                if (p->prev == NULL) //첫글자 삭제시
                {
                    head->hp = p->next; //head 변경
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
            //현재 p 위치로
            while (head->x != cnt)
            {
                cnt += 1;
                if (head->x == cnt)  break;
                p = p->next;
            }
            //delete
            if (p->next == NULL)
            {
                if (p->prev == NULL) //첫글자 삭제시 (재시작)
                {
                    head->hp = p->next; //head 변경
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
                if (p->prev == NULL) //첫글자 삭제시
                {
                    head->hp = p->next; //head 변경
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
    else if ((head->x==0 && head->y == 0 && head->Sline > 0 ) || (head->x == 0 && head->y > 0)) //두 줄 이상일떄, 윗글자 enter제거
    {
        //해당 줄의 윗줄 cnt2
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

        //위에 줄과 연결, 윗줄 enter delete
        p = linehead;
        if (p->prev != NULL && p->next != NULL)
        {
            p->next->prev = p->prev;
            p->prev->next = p->next;
            free(p); //enter삭제
            head->x = cnt;
            if (head->y == 0 && head->Sline > 0) head->Sline -= 1;
            else head->y -= 1;
        }
        else if (p->prev == NULL && p->next != NULL)
        {
            head->hp = p->next; //head 변경
            p->next->prev = p->prev;
            free(p); //enter삭제
            head->x = cnt;
            if (head->y == 0 && head->Sline > 0) head->Sline -= 1;
            else head->y -= 1;
        }
        else if (p->prev == NULL && p->next == NULL) //첫글자 enter 삭제됨 (재시작)
        {
            head->hp = p->next; //head 변경
            free(p); //enter삭제
            head->x = cnt;
            if (head->y == 0 && head->Sline > 0) head->Sline -= 1;
            else head->y -= 1;
        }
        else if (p->prev != NULL && p->next == NULL) //첫글자 enter 삭제됨
        {
            p->prev->next = p->next;
            free(p); //enter삭제
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
    //파일 저장
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
    //저장완료 출력
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

        if (key == 224 || key == 0 || key == ESC) //home, end, pageup down, 커서
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
                if(key == 1) custom_getch(); //~제거
                head->x = 0;
            }
            else if (key == END)
            {
                if (key == 4) custom_getch(); //~제거
                end(head);
            }
            else if (key == PAGE_UP)
            {
                if(key == 53) custom_getch(); //~제거

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
                if (key == 54) custom_getch(); //~제거

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
            else if(key == UP || key == DOWN || key == RIGHT || key == LEFT) //커서
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
            //window는 위에서 새롭게 초기화
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
                //enter로 내려가는거 보정
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
            if (key == Ctrl_q) //저장 없이 나가기
            {
                break;
            }
            else if (key == ENTER) //저장 후 나가기
            {
                initWindow(head);
                initToolbar(head);
                if (saved == 0) //q 한번이고 saved = 0 : 저장기능
                {
                    if (head->filename == NULL)
                    {
                        Toolbar(head);
                        gotoxy(0, head->rows + 2);
                        printf("\033[7mNAME\033[0m : ");
                        gets(filename);
                        head->filename = filename;
                        //enter로 내려가는거 보정
                        initWindow(head);
                        Toolbar(head);
                        gotoxy(0, head->rows + 2);
                        printf("\033[7mNAME\033[0m : ");
                    }
                    save(head);
                    saved = 1;
                    continue;
                }
                else if (saved == 1) //마지막으로 저장한것 까지만 저장하고 나가기
                {
                    break;
                }
                
            }
        }
        else // 영어 입력, 중간삽입 가능하게
        {
            if (head->x == head->cols) //cols에 끝이면
            {
                Add(head, ENTER);
                initWindow(head);
            }
            Add(head, key);
        }

        initToolbar(head);
    }
}
