#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h> // malloc
#include <string.h> // strcpy
#include "textedit.h"

#define ESC 27
#define Ctrl_f 6
#define Ctrl_s 19
#define Ctrl_q 17

#ifdef _WIN32 // Windows ÇÃ·§Æûar
#include <conio.h> // _getch
#include <windows.h> 
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
#else // Linux ¹× macOS ÇÃ·§Æû
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

int main(int argc, char* argv[]) {
    system(CLEAR_SCREEN);
    #ifdef _WIN32 // Windows ÇÃ·§Æû
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        int rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        int cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    #else // Linux ¹× macOS ÇÃ·§Æû
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        int rows = w.ws_row;
        int cols = w.ws_col;

        struct termios term;
        tcgetattr(STDIN_FILENO, &term);
        term.c_iflag &= ~(IXON | IXOFF); // Ctrl+S ¹× Ctrl+Q ºñÈ°¼ºÈ­
        tcsetattr(STDIN_FILENO, TCSANOW, &term); // ÅÍ¹Ì³Î ¸ðµå Àû¿ë
    #endif

    Head hp; //ÁÖ¼Ú°ª
    Head* head;  //ÁÖ¼Ú°ªÀ» °¡¸®Å°´Â Æ÷ÀÎÅÍ
    head = createHead(&hp, rows, cols);
    int key;

    if (argc == 2)
    {
        head->filename = argv[1];
        FILE* file = fopen(head->filename, "rt");
        Text* p = NULL;
        int col = -1;
        int cntline = 0;
        if (file != NULL)
        {
            while ((key = fgetc(file)) != EOF)
            {
                Text* tmp = createText(key);
                if (key == ENTER) cntline += 1;
                //head°¡ nullÀÏ ¶§ Ã¹¿¬°á
                if (head->hp == NULL)
                {
                    head->hp = tmp;
                    p = tmp;
                    col += 1;
                    continue;
                }
                if (p != NULL && p->next == NULL)
                {
                    p->next = tmp;
                    tmp->prev = p;
                    p = tmp;
                    col += 1;
                    if (col == head->cols) //cols¿¡ ³¡ÀÌ¸é
                    {
                        Text* ent = createText(ENTER);
                        p->next = ent;
                        ent->prev = p;
                        p = ent;
                        cntline += 1;
                    }
                    continue;
                }
            }
            fclose(file);
            head->lines = cntline;
            fileWindow(head);
            fileToolbar(head);
            gotoxy(0, 0);
        }
    }
    else
    {
        createToolbar(head);
    }
    
    editText(head);

    return 0;
}
