//
//  main.cpp
//  Sudoku
//
//  Created by Bing Yu Li on 2023/12/4.
//

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include <string>
#include <ctime>
#include <unistd.h> //sleep
#include <thread> //for multi thread
#include <mutex> //exclusive access for multi thread
#include <locale.h> //library for using unicode
#include <ncurses.h>
using namespace std;


bool game_thread_end = false;
bool finishgame = false;
bool multi_off = true;
int mat, sqrmat, N, dif;
int key;
int icursey = 0, icursex = 0, cursey = 0, cursex = 0, yMax, xMax;
long int inigametime;
char base[16][16], display[16][16], ans[16][16];
string gametime = "00 : 00 : 00";
mutex m;

//tamplate generte
//==================================================================================================
bool row_check(int rowc, char value) {
    for(int j = 0; j < mat; j++) {
        if(base[rowc][j] == value)
            return false;
    }
    return true;
}

bool column_check(int columnc, char value) {
    for(int i = 0; i < mat; i++) {
        if(base[i][columnc] == value)
            return false;
    }
    return true;
}

bool box_check(int rowbc, int columnbc, char value) {
    for(int i = 0; i < sqrmat; i++) {
        for(int j = 0; j < sqrmat; j++) {
            if(base[rowbc+i][columnbc+j] == value)
                return false;
        }
    }
    return true;
}

void dia_box_fill() {
    int ran;
    char randvalue;
    srand((unsigned)time(0)); //generate rand (put it in while takes large amount of time ('bout 15 sec)
    for(int m = 0; m < mat;) {
        for(int i = 0; i < sqrmat; i++) {
            for(int j = 0; j < sqrmat; j++) {
                do {
                    ran = rand()%mat+1;
                    if(ran < 10) //int to char
                        randvalue = ran + 48;
                    else
                        randvalue = ran + 55;
                } while(! box_check(m, m, randvalue));
                base[m+i][m+j] = randvalue;
            }
        }
        m += sqrmat;
    }
}

bool remain_fill(int i, int j) {
    if (i < mat-1 && j >= mat) { //the right but not the last row
        i++; //down a row
        j = 0; //set to the left row
    }
    if (i >= mat && j >= mat) //the end
        return true;
    
    if (i < sqrmat) { //top box
        if (j < sqrmat) //top left box
            j = sqrmat; //set to left row of mid box
    }
    else if (mat != 4 && i < mat - sqrmat) { //mid box row
        if (j == int(i / sqrmat) * sqrmat) //left row of mid box
            j += sqrmat; //set to left row of mid box
    }
    else { //bottom box
        if (j == mat - sqrmat) { //bottom right box
            i++; //down a row
            j = 0; //set to the left row
            if (i >= mat) //the bottom
                return true;
        }
    }
    for (int k = 1; k <= mat; k++) {
        char value;
        if(k < 10) //int to char
            value = k+48;
        else
            value = k+55;
        if (row_check(i, value) && column_check(j, value) && box_check(i - i%sqrmat, j - j%sqrmat, value)) { //check value
            base[i][j] = value;
            if (remain_fill(i, j+1))
                return true; //recursion
            base[i][j] = '0';
        }
    }
    return false;
}

void value_remove()
   {
    int count = N;
    srand((unsigned)time(0));
    while (count != 0) {
        int i = rand()%mat;
        int j = rand()%mat;
        if (display[i][j] != ' ') {
            count--;
            display[i][j] = ' ';
        }
    }
}

bool empty_line(int line, bool givenx) {
    if(givenx) {
        for(int i = 0; i < mat; i++) {
            if(display[i][line] != ' ')
                return false;
        }
    } //column
    else {
        for(int i = 0; i < mat; i++) {
            if(display[line][i] != ' ')
                return false;
        }
    } //row
    return true;
} //check if the line is empty

bool empty_line_check() {
    for(int k = 0; k < sqrmat; k++) {
        for(int i = 0; i < sqrmat-1; i++) {
            for(int j = sqrmat-1; j > i; j--) {
                if(!(empty_line(sqrmat*k+i, true) && empty_line(sqrmat*k+j, true))) {
                    return true;
                } //empty column check
                if(!(empty_line(sqrmat*k+i, false) && empty_line(sqrmat*k+j, false))) {
                    return true;
                } //empty row check
            }
        }
    }
    return false;
} //check if any two lines are empty in a line of box

bool array_check(int pair1[], int pair2[]) {
    for(int i = 0; i < sqrmat; i++) {
        if(pair1[i] != pair2[i]) {
            return false;
        }
    }
    return true;
} //check if all elements are the same

bool empty_pair_check() {
    for(int k = 0; k < sqrmat; k++) {
        bool check[mat];
        for(int i = 0; i < sqrmat-1; i++) {
            for(int j = sqrmat-1; j > i; j--) {
                for(int y = 0; y < mat; y++) {
                    if(display[y][sqrmat*k+i] == ' ' && display[y][sqrmat*k+j] == ' ')
                        check[y] = true;
                    else
                        check[y] = false;
                } //find empty pair
                for(int m = 0; m < mat-1; m++) {
                    for(int n = mat-1; n > m; n--) {
                        if(check[m] && check[n]) {
                            if(base[m][sqrmat*k+i] == base[n][sqrmat*k+j] && base[m][sqrmat*k+j] == base[n][sqrmat*k+i])
                                return false;
                        }
                    }
                } //check paired pair
            }
        } //two elements
        if(mat == 9) {
            int pair1[sqrmat], pair2[sqrmat];
            for(int y = 0; y < mat; y++) {
                if(display[y][sqrmat*k] == ' ' && display[y][sqrmat*k+1] == ' ' && display[y][sqrmat*k+2] == ' ')
                    check[y] = true;
                else
                    check[y] = false;
            } //find empty three
            for(int m = 0; m < mat-1; m++) {
                for(int n = mat-1; n > m; n--) {
                    if(check[m] && check[n]) {
                        for(int i = 0; i < 3; i++) {
                            pair1[i] = base[m][sqrmat*k+i];
                        }
                        for(int i = 0; i < 3; i++) {
                            pair2[i] = base[n][sqrmat*k+i];
                        }
                        pair1[4] = 0;
                        pair2[4] = 0;
                        sort(pair1, pair1+sqrmat);
                        sort(pair2, pair2+sqrmat);
                        if(array_check(pair1, pair2))
                            return false;
                    }
                }
            } //check paired three's
        }
        if(mat == 16) {
            int pair1[sqrmat], pair2[sqrmat];
            for(int y = 0; y < mat; y++) {
                if(display[y][sqrmat*k] == ' ' && display[y][sqrmat*k+1] == ' ' && display[y][sqrmat*k+2] == ' ')
                    check[y] = true;
                else
                    check[y] = false;
            } //find empty 123X's
            for(int m = 0; m < mat-1; m++) {
                for(int n = mat-1; n > m; n--) {
                    if(check[m] && check[n]) {
                        pair1[0] = base[m][sqrmat*k];
                        pair1[1] = base[m][sqrmat*k+1];
                        pair1[2] = base[m][sqrmat*k+2];
                        pair1[3] = 0;
                        pair2[0] = base[n][sqrmat*k];
                        pair2[1] = base[n][sqrmat*k+1];
                        pair2[2] = base[n][sqrmat*k+2];
                        pair2[3] = 0;
                        sort(pair1, pair1+4);
                        sort(pair2, pair2+4);
                        if(array_check(pair1, pair2))
                            return false;
                    }
                }
            } //check paired 123X'
            for(int y = 0; y < mat; y++) {
                if(display[y][sqrmat*k] == ' ' && display[y][sqrmat*k+1] == ' ' && display[y][sqrmat*k+3] == ' ')
                    check[y] = true;
                else
                    check[y] = false;
            } //find empty 12X4's
            for(int m = 0; m < mat-1; m++) {
                for(int n = mat-1; n > m; n--) {
                    if(check[m] && check[n]) {
                        pair1[0] = base[m][sqrmat*k];
                        pair1[1] = base[m][sqrmat*k+1];
                        pair1[2] = base[m][sqrmat*k+3];
                        pair1[3] = 0;
                        pair2[0] = base[n][sqrmat*k];
                        pair2[1] = base[n][sqrmat*k+1];
                        pair2[2] = base[n][sqrmat*k+3];
                        pair2[3] = 0;
                        sort(pair1, pair1+4);
                        sort(pair2, pair2+4);
                        if(array_check(pair1, pair2))
                            return false;
                    }
                }
            } //check paired 12X4'
            for(int y = 0; y < mat; y++) {
                if(display[y][sqrmat*k] == ' ' && display[y][sqrmat*k+2] == ' ' && display[y][sqrmat*k+3] == ' ')
                    check[y] = true;
                else
                    check[y] = false;
            } //find empty 1X34's
            for(int m = 0; m < mat-1; m++) {
                for(int n = mat-1; n > m; n--) {
                    if(check[m] && check[n]) {
                        pair1[0] = base[m][sqrmat*k];
                        pair1[1] = base[m][sqrmat*k+2];
                        pair1[2] = base[m][sqrmat*k+3];
                        pair1[3] = 0;
                        pair2[0] = base[n][sqrmat*k];
                        pair2[1] = base[n][sqrmat*k+2];
                        pair2[2] = base[n][sqrmat*k+3];
                        pair2[3] = 0;
                        sort(pair1, pair1+4);
                        sort(pair2, pair2+4);
                        if(array_check(pair1, pair2))
                            return false;
                    }
                }
            } //check paired 1X34'
            for(int y = 0; y < mat; y++) {
                if(display[y][sqrmat*k+1] == ' ' && display[y][sqrmat*k+2] == ' ' && display[y][sqrmat*k+3] == ' ')
                    check[y] = true;
                else
                    check[y] = false;
            } //find empty X234's
            for(int m = 0; m < mat-1; m++) {
                for(int n = mat-1; n > m; n--) {
                    if(check[m] && check[n]) {
                        pair1[0] = base[m][sqrmat*k+1];
                        pair1[1] = base[m][sqrmat*k+2];
                        pair1[2] = base[m][sqrmat*k+3];
                        pair1[3] = 0;
                        pair2[0] = base[n][sqrmat*k+1];
                        pair2[1] = base[n][sqrmat*k+2];
                        pair2[2] = base[n][sqrmat*k+3];
                        pair2[3] = 0;
                        sort(pair1, pair1+4);
                        sort(pair2, pair2+4);
                        if(array_check(pair1, pair2))
                            return false;
                    }
                }
            } //check paired X234'
            for(int y = 0; y < mat; y++) {
                if(display[y][sqrmat*k] == ' ' && display[y][sqrmat*k+1] == ' ' && display[y][sqrmat*k+2] == ' ' && display[y][sqrmat*k+3] == ' ')
                    check[y] = true;
                else
                    check[y] = false;
            } //find empty four
            for(int m = 0; m < mat-1; m++) {
                for(int n = mat-1; n > m; n--) {
                    if(check[m] && check[n]) {
                        for(int i = 0; i < 4; i++) {
                            pair1[i] = base[m][sqrmat*k+i];
                        }
                        for(int i = 0; i < 4; i++) {
                            pair2[i] = base[n][sqrmat*k+i];
                        }
                        sort(pair1, pair1+sqrmat);
                        sort(pair2, pair2+sqrmat);
                        if(array_check(pair1, pair2))
                            return false;
                    }
                }
            } //check paired four's
        }
            
    } //empty column check
    
    for(int k = 0; k < sqrmat; k++) {
        bool check[mat];
        for(int i = 0; i < sqrmat-1; i++) {
            for(int j = sqrmat-1; j > i; j--) {
                for(int x = 0; x < mat; x++) {
                    if(display[sqrmat*k+i][x] == ' ' && display[sqrmat*k+j][x] == ' ')
                        check[x] = true;
                    else
                        check[x] = false;
                } //find empty pair
                for(int m = 0; m < mat-1; m++) {
                    for(int n = mat-1; n > m; n--) {
                        if(check[m] && check[n]) {
                            if(base[sqrmat*k+i][m] == base[sqrmat*k+j][n] && base[sqrmat*k+j][m] == base[sqrmat*k+i][n])
                                return false;
                        }
                    }
                } //check paired pair
            }
        } //two elements
        if(mat == 9) {
            int pair1[sqrmat], pair2[sqrmat];
            for(int x = 0; x < mat; x++) {
                if(display[sqrmat*k][x] == ' ' && display[sqrmat*k+1][x] == ' ' && display[sqrmat*k+2][x] == ' ')
                    check[x] = true;
                else
                    check[x] = false;
            } //find empty three
            for(int m = 0; m < mat-1; m++) {
                for(int n = mat-1; n > m; n--) {
                    if(check[m] && check[n]) {
                        for(int i = 0; i < 3; i++) {
                            pair1[i] = base[sqrmat*k+i][m];
                        }
                        for(int i = 0; i < 3; i++) {
                            pair2[i] = base[sqrmat*k+i][n];
                        }
                        pair1[4] = 0;
                        pair2[4] = 0;
                        sort(pair1, pair1+sqrmat);
                        sort(pair2, pair2+sqrmat);
                        if(array_check(pair1, pair2))
                            return false;
                    }
                }
            } //check paired three's
        }
        if(mat == 16) {
            int pair1[sqrmat], pair2[sqrmat];
            for(int x = 0; x < mat; x++) {
                if(display[sqrmat*k][x] == ' ' && display[sqrmat*k+1][x] == ' ' && display[sqrmat*k+2][x] == ' ')
                    check[x] = true;
                else
                    check[x] = false;
            } //find empty 123X's
            for(int m = 0; m < mat-1; m++) {
                for(int n = mat-1; n > m; n--) {
                    if(check[m] && check[n]) {
                        pair1[0] = base[sqrmat*k][m];
                        pair1[1] = base[sqrmat*k+1][m];
                        pair1[2] = base[sqrmat*k+2][m];
                        pair1[3] = 0;
                        pair2[0] = base[sqrmat*k][n];
                        pair2[1] = base[sqrmat*k+1][n];
                        pair2[2] = base[sqrmat*k+2][n];
                        pair2[3] = 0;
                        sort(pair1, pair1+4);
                        sort(pair2, pair2+4);
                        if(array_check(pair1, pair2))
                            return false;
                    }
                }
            } //check paired 123X'
            for(int x = 0; x < mat; x++) {
                if(display[sqrmat*k][x] == ' ' && display[sqrmat*k+1][x] == ' ' && display[sqrmat*k+3][x] == ' ')
                    check[x] = true;
                else
                    check[x] = false;
            } //find empty 12X4's
            for(int m = 0; m < mat-1; m++) {
                for(int n = mat-1; n > m; n--) {
                    if(check[m] && check[n]) {
                        pair1[0] = base[sqrmat*k][m];
                        pair1[1] = base[sqrmat*k+1][m];
                        pair1[2] = base[sqrmat*k+3][m];
                        pair1[3] = 0;
                        pair2[0] = base[sqrmat*k][n];
                        pair2[1] = base[sqrmat*k+1][n];
                        pair2[2] = base[sqrmat*k+3][n];
                        pair2[3] = 0;
                        sort(pair1, pair1+4);
                        sort(pair2, pair2+4);
                        if(array_check(pair1, pair2))
                            return false;
                    }
                }
            } //check paired 12X4'
            for(int x = 0; x < mat; x++) {
                if(display[sqrmat*k][x] == ' ' && display[sqrmat*k+2][x] == ' ' && display[sqrmat*k+3][x] == ' ')
                    check[x] = true;
                else
                    check[x] = false;
            } //find empty 1X34's
            for(int m = 0; m < mat-1; m++) {
                for(int n = mat-1; n > m; n--) {
                    if(check[m] && check[n]) {
                        pair1[0] = base[sqrmat*k][m];
                        pair1[1] = base[sqrmat*k+2][m];
                        pair1[2] = base[sqrmat*k+3][m];
                        pair1[3] = 0;
                        pair2[0] = base[sqrmat*k][n];
                        pair2[1] = base[sqrmat*k+2][n];
                        pair2[2] = base[sqrmat*k+3][n];
                        pair2[3] = 0;
                        sort(pair1, pair1+4);
                        sort(pair2, pair2+4);
                        if(array_check(pair1, pair2))
                            return false;
                    }
                }
            } //check paired 1X34'
            for(int x = 0; x < mat; x++) {
                if(display[sqrmat*k+1][x] == ' ' && display[sqrmat*k+2][x] == ' ' && display[sqrmat*k+3][x] == ' ')
                    check[x] = true;
                else
                    check[x] = false;
            } //find empty X234's
            for(int m = 0; m < mat-1; m++) {
                for(int n = mat-1; n > m; n--) {
                    if(check[m] && check[n]) {
                        pair1[0] = base[sqrmat*k+1][m];
                        pair1[1] = base[sqrmat*k+2][m];
                        pair1[2] = base[sqrmat*k+3][m];
                        pair1[3] = 0;
                        pair2[0] = base[sqrmat*k+1][n];
                        pair2[1] = base[sqrmat*k+2][n];
                        pair2[2] = base[sqrmat*k+3][n];
                        pair2[3] = 0;
                        sort(pair1, pair1+4);
                        sort(pair2, pair2+4);
                        if(array_check(pair1, pair2))
                            return false;
                    }
                }
            } //check paired X234'
            for(int x = 0; x < mat; x++) {
                if(display[sqrmat*k][x] == ' ' && display[sqrmat*k+1][x] == ' ' && display[sqrmat*k+2][x] == ' ' && display[sqrmat*k+3][x] == ' ')
                    check[x] = true;
                else
                    check[x] = false;
            } //find empty four
            for(int m = 0; m < mat-1; m++) {
                for(int n = mat-1; n > m; n--) {
                    if(check[m] && check[n]) {
                        for(int i = 0; i < 4; i++) {
                            pair1[i] = base[sqrmat*k+i][m];
                        }
                        for(int i = 0; i < 4; i++) {
                            pair2[i] = base[sqrmat*k+i][n];
                        }
                        sort(pair1, pair1+sqrmat);
                        sort(pair2, pair2+sqrmat);
                        if(array_check(pair1, pair2))
                            return false;
                    }
                }
            } //check paired four's
        }
    } //empty row check
    return true;
} //check mirrored pair

void get_matrix() {
    switch(dif) {
        case 0:
            N = int(mat*mat*0.5);
            break;
        case 1:
            N = int(mat*mat*0.6);
            break;
        case 2:
            N = int(mat*mat*0.75);
            break;
        default:
            cout << "mode value error!!\n\n";
            exit(1);
    }
    sqrmat = sqrt(mat);
    do {
        do {
            for(int i = 0; i < mat; i++) {
                for(int j = 0; j < mat; j++) {
                    base[i][j] = '0';
                    display[i][j] = '0';
                    ans[i][j] = '0';
                }
            } //reset to '0'
            dia_box_fill();
            remain_fill(0, sqrmat);
        } while(base[0][sqrmat] == '0'); //avoid 4 by 4 error, when filling remaining, 2+2=4, diabox fill may creat remaining that can't be filled
        for(int i = 0; i < mat; i++) {
            for(int j = 0; j < mat; j++) {
                display[i][j] = base[i][j];
            }
        } //assign display with base
        value_remove();
        if(multi_off || (empty_line_check() && empty_pair_check())){
            break;
        }
    } while(1);
    for(int i = 0; i < mat; i++) {
        for(int j = 0; j < mat; j++) {
            ans[i][j] = display[i][j];
        }
    } //assign answer with display
}
//==================================================================================================

//Grid reference
//==================================================================================================
void _4by4_grid(WINDOW * win, int y, int x) {
    mvwprintw(win, y, x, "╔═══╤═══╦═══╤═══╗");
    mvwprintw(win, y+1, x, "║   │   ║   │   ║");
    mvwprintw(win, y+2, x, "╟───┼───╫───┼───╢");
    mvwprintw(win, y+3, x, "║   │   ║   │   ║");
    mvwprintw(win, y+4, x, "╠═══╪═══╬═══╪═══╣");
    mvwprintw(win, y+5, x, "║   │   ║   │   ║");
    mvwprintw(win, y+6, x, "╟───┼───╫───┼───╢");
    mvwprintw(win, y+7, x, "║   │   ║   │   ║");
    mvwprintw(win, y+8, x, "╚═══╧═══╩═══╧═══╝");
}
void _9by9_grid(WINDOW * win, int y, int x) {
    mvwprintw(win, y, x, "╔═══╤═══╤═══╦═══╤═══╤═══╦═══╤═══╤═══╗");
    mvwprintw(win, y+1, x, "║   │   │   ║   │   │   ║   │   │   ║");
    mvwprintw(win, y+2, x, "╟───┼───┼───╫───┼───┼───╫───┼───┼───╢");
    mvwprintw(win, y+3, x, "║   │   │   ║   │   │   ║   │   │   ║");
    mvwprintw(win, y+4, x, "╟───┼───┼───╫───┼───┼───╫───┼───┼───╢");
    mvwprintw(win, y+5, x, "║   │   │   ║   │   │   ║   │   │   ║");
    mvwprintw(win, y+6, x, "╠═══╪═══╪═══╬═══╪═══╪═══╬═══╪═══╪═══╣");
    mvwprintw(win, y+7, x, "║   │   │   ║   │   │   ║   │   │   ║");
    mvwprintw(win, y+8, x, "╟───┼───┼───╫───┼───┼───╫───┼───┼───╢");
    mvwprintw(win, y+9, x, "║   │   │   ║   │   │   ║   │   │   ║");
    mvwprintw(win, y+10, x, "╟───┼───┼───╫───┼───┼───╫───┼───┼───╢");
    mvwprintw(win, y+11, x, "║   │   │   ║   │   │   ║   │   │   ║");
    mvwprintw(win, y+12, x, "╠═══╪═══╪═══╬═══╪═══╪═══╬═══╪═══╪═══╣");
    mvwprintw(win, y+13, x, "║   │   │   ║   │   │   ║   │   │   ║");
    mvwprintw(win, y+14, x, "╟───┼───┼───╫───┼───┼───╫───┼───┼───╢");
    mvwprintw(win, y+15, x, "║   │   │   ║   │   │   ║   │   │   ║");
    mvwprintw(win, y+16, x, "╟───┼───┼───╫───┼───┼───╫───┼───┼───╢");
    mvwprintw(win, y+17, x, "║   │   │   ║   │   │   ║   │   │   ║");
    mvwprintw(win, y+18, x, "╚═══╧═══╧═══╩═══╧═══╧═══╩═══╧═══╧═══╝");
}
void _16by16_grid(WINDOW * win, int y, int x) {
    mvwprintw(win, y, x,"╔═══╤═══╤═══╤═══╦═══╤═══╤═══╤═══╦═══╤═══╤═══╤═══╦═══╤═══╤═══╤═══╗\n");
    mvwprintw(win, y+1, x, "║   │   │   │   ║   │   │   │   ║   │   │   │   ║   │   │   │   ║\n");
    mvwprintw(win, y+2, x, "╟───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╢\n");
    mvwprintw(win, y+3, x, "║   │   │   │   ║   │   │   │   ║   │   │   │   ║   │   │   │   ║\n");
    mvwprintw(win, y+4, x, "╟───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╢\n");
    mvwprintw(win, y+5, x, "║   │   │   │   ║   │   │   │   ║   │   │   │   ║   │   │   │   ║\n");
    mvwprintw(win, y+6, x, "╟───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╢\n");
    mvwprintw(win, y+7, x, "║   │   │   │   ║   │   │   │   ║   │   │   │   ║   │   │   │   ║\n");
    mvwprintw(win, y+8, x, "╠═══╪═══╪═══╪═══╬═══╪═══╪═══╪═══╬═══╪═══╪═══╪═══╬═══╪═══╪═══╪═══╣\n");
    mvwprintw(win, y+9, x, "║   │   │   │   ║   │   │   │   ║   │   │   │   ║   │   │   │   ║\n");
    mvwprintw(win, y+10, x, "╟───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╢\n");
    mvwprintw(win, y+11, x, "║   │   │   │   ║   │   │   │   ║   │   │   │   ║   │   │   │   ║\n");
    mvwprintw(win, y+12, x, "╟───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╢\n");
    mvwprintw(win, y+13, x, "║   │   │   │   ║   │   │   │   ║   │   │   │   ║   │   │   │   ║\n");
    mvwprintw(win, y+14, x, "╟───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╢\n");
    mvwprintw(win, y+15, x, "║   │   │   │   ║   │   │   │   ║   │   │   │   ║   │   │   │   ║\n");
    mvwprintw(win, y+16, x, "╠═══╪═══╪═══╪═══╬═══╪═══╪═══╪═══╬═══╪═══╪═══╪═══╬═══╪═══╪═══╪═══╣\n");
    mvwprintw(win, y+17, x, "║   │   │   │   ║   │   │   │   ║   │   │   │   ║   │   │   │   ║\n");
    mvwprintw(win, y+18, x, "╟───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╢\n");
    mvwprintw(win, y+19, x, "║   │   │   │   ║   │   │   │   ║   │   │   │   ║   │   │   │   ║\n");
    mvwprintw(win, y+20, x, "╟───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╢\n");
    mvwprintw(win, y+21, x, "║   │   │   │   ║   │   │   │   ║   │   │   │   ║   │   │   │   ║\n");
    mvwprintw(win, y+22, x, "╟───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╢\n");
    mvwprintw(win, y+23, x, "║   │   │   │   ║   │   │   │   ║   │   │   │   ║   │   │   │   ║\n");
    mvwprintw(win, y+24, x, "╠═══╪═══╪═══╪═══╬═══╪═══╪═══╪═══╬═══╪═══╪═══╪═══╬═══╪═══╪═══╪═══╣\n");
    mvwprintw(win, y+25, x, "║   │   │   │   ║   │   │   │   ║   │   │   │   ║   │   │   │   ║\n");
    mvwprintw(win, y+26, x, "╟───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╢\n");
    mvwprintw(win, y+27, x, "║   │   │   │   ║   │   │   │   ║   │   │   │   ║   │   │   │   ║\n");
    mvwprintw(win, y+28, x, "╟───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╢\n");
    mvwprintw(win, y+29, x, "║   │   │   │   ║   │   │   │   ║   │   │   │   ║   │   │   │   ║\n");
    mvwprintw(win, y+30, x, "╟───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╢\n");
    mvwprintw(win, y+31, x, "║   │   │   │   ║   │   │   │   ║   │   │   │   ║   │   │   │   ║\n");
    mvwprintw(win, y+32, x, "╚═══╧═══╧═══╧═══╩═══╧═══╧═══╧═══╩═══╧═══╧═══╧═══╩═══╧═══╧═══╧═══╝\n");
}
//==================================================================================================

//Mutex
//==================================================================================================
bool game_function_end(bool gavein, bool returnback) {
    m.lock();
    if(returnback)
    {
        m.unlock();
        return game_thread_end;
    }
    else
    {
        game_thread_end = gavein;
        m.unlock();
        return true;
    }
} //mutex function
//==================================================================================================

//Timer thread
//==================================================================================================
void timer_thread() {
    int min, sec, hour, systemtime, inisystemtime = 0;
    long int start = time(0);
    WINDOW * timewin = newwin(3, 20, (yMax-35)/2+11, (xMax-129)/2+87);
    mvwprintw(timewin, 0, 0, "┌──────────────────┐");
    mvwprintw(timewin, 1, 0, "│   00 : 00 : 00   │");
    mvwprintw(timewin, 2, 0, "└──────────────────┘");
    refresh();
    wrefresh(timewin);
    
    while(!game_function_end(true, true)) {
        systemtime = int(time(0) - start);
        if(inisystemtime != systemtime) {
            hour = systemtime/3600;
            min = (systemtime%3600)/60;
            sec = (systemtime%3600)%60;
            if(hour < 10)
                gametime = "0" + to_string(hour) + " : ";
            else if (hour >= 100) {
                gametime = "99 : 59 : 59";
                break;
            }
            else
                gametime = to_string(hour) + " : ";
            if(min < 10)
                gametime = gametime + "0" + to_string(min) + " : ";
            else
                gametime = gametime + to_string(min) + " : ";
            if(sec < 10)
                gametime = gametime + "0" + to_string(sec);
            else
                gametime = gametime + to_string(sec);
            
            mvwprintw(timewin, 1, 4, "%s", gametime.c_str());
            wrefresh(timewin);
        }
        inisystemtime = systemtime;
        usleep(60000);
    }
    delwin(timewin);
    refresh();
}
//==================================================================================================

//Gmae thread
//==================================================================================================
void game_thread(WINDOW * gamewin, int mode) {
    keypad(gamewin, true); //start keypad
    start_color(); //start color attribute
    use_default_colors();
    init_pair(1, COLOR_GREEN, -1);
    init_pair(2, COLOR_RED, -1);
    init_pair(3, -1, COLOR_GREEN);
    init_pair(4, -1, COLOR_RED);
    bool fill = false, exitgame = false;
    finishgame = false;
    switch(mode){
        //4 by 4
        //------------------------------------------------------------------------------
        case 0:
            while(1) {
                key = wgetch(gamewin);
                fill = false;
                exitgame = false;
                cursey = icursey;
                cursex = icursex;
                bool inichanged = false;
                switch(key) {
                    case KEY_UP:
                        cursey--;
                        if(cursey == -1)
                            cursey = 0;
                        else {
                            if(display[cursey][cursex] != ' ') {
                                bool end0, end;
                                for(int m = 0; m < 4; m++) {
                                    bool breakm = false;
                                    end0 = false;
                                    end = false;
                                    for(int n = 1; n <= 4; n++) {
                                        if(end0 == false) {
                                            cursex = icursex-n;
                                            if(icursex-n == -1) {
                                                cursex = 0;
                                                end0 = true; //break after finish this n loop execution
                                            } //border restriction
                                        } //serch for left
                                        if(display[cursey][cursex] != ' ') {
                                            if(end == false) {
                                                cursex = icursex+n;
                                                if(icursex+n == 4) {
                                                    cursex = 3;
                                                    end = true; //break after finish this n loop execution
                                                }
                                                else {
                                                    if(display[cursey][cursex] == ' ') {
                                                        if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                                            wattron(gamewin, COLOR_PAIR(3));
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                            wattroff(gamewin,COLOR_PAIR(3));
                                                        }
                                                        else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                                            wattron(gamewin, COLOR_PAIR(4));
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                            wattroff(gamewin,COLOR_PAIR(4));
                                                        }
                                                        else {
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                        }
                                                        wattroff(gamewin, A_REVERSE);
                                                        inichanged = true;
                                                        breakm = true;
                                                        break;
                                                    } //if right suit
                                                }
                                            } //serch for right
                                        } //if left fail
                                        else {
                                            if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                                wattron(gamewin, COLOR_PAIR(3));
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                wattroff(gamewin,COLOR_PAIR(3));
                                            }
                                            else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                                wattron(gamewin, COLOR_PAIR(4));
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                wattroff(gamewin,COLOR_PAIR(4));
                                            }
                                            else {
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                            }
                                            wattroff(gamewin, A_REVERSE);
                                            inichanged = true;
                                            breakm = true;
                                            break;
                                        } //if leftt suit
                                    } //serch for row
                                    if(breakm)
                                        break;
                                    cursey--; //no suit element in the row
                                    if(cursey == -1) {
                                        cursey = 0;
                                        break;
                                    } //end restriction
                                }
                            }
                            else {
                                if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                    wattron(gamewin, COLOR_PAIR(3));
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    wattroff(gamewin,COLOR_PAIR(3));
                                }
                                else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                    wattron(gamewin, COLOR_PAIR(4));
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    wattroff(gamewin,COLOR_PAIR(4));
                                }
                                else {
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                }
                                wattroff(gamewin, A_REVERSE);
                                inichanged = true;
                            }
                        }
                        break;
                    case KEY_DOWN:
                        cursey++;
                        if(cursey == 4)
                            cursey = 3;
                        else {
                            if(display[cursey][cursex] != ' ') {
                                bool end0, end;
                                for(int m = 0; m < 4; m++) {
                                    bool breakm = false;
                                    end0 = false;
                                    end = false;
                                    for(int n = 1; n <= 4; n++) {
                                        if(end0 == false) {
                                            cursex = icursex-n;
                                            if(icursex-n == -1) {
                                                cursex = 0;
                                                end0 = true; //break after finish this n loop execution
                                            } //border restriction
                                        } //serch for left
                                        if(display[cursey][cursex] != ' ') {
                                            if(end == false) {
                                                cursex = icursex+n;
                                                if(icursex+n == 4) {
                                                    cursex = 3;
                                                    end = true; //break after finish this n loop execution
                                                }
                                                else {
                                                    if(display[cursey][cursex] == ' ') {
                                                        if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                                            wattron(gamewin, COLOR_PAIR(3));
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                            wattroff(gamewin,COLOR_PAIR(3));
                                                        }
                                                        else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                                            wattron(gamewin, COLOR_PAIR(4));
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                            wattroff(gamewin,COLOR_PAIR(4));
                                                        }
                                                        else {
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                        }
                                                        wattroff(gamewin, A_REVERSE);
                                                        inichanged = true;
                                                        breakm = true;
                                                        break;
                                                    } //if right suit
                                                }
                                            } //serch for right
                                        } //if left fail
                                        else {
                                            if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                                wattron(gamewin, COLOR_PAIR(3));
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                wattroff(gamewin,COLOR_PAIR(3));
                                            }
                                            else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                                wattron(gamewin, COLOR_PAIR(4));
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                wattroff(gamewin,COLOR_PAIR(4));
                                            }
                                            else {
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                            }
                                            wattroff(gamewin, A_REVERSE);
                                            inichanged = true;
                                            breakm = true;
                                            break;
                                        } //if right suit
                                    } //serch for row
                                    if(breakm)
                                        break;
                                    cursey++; //no suit element in the row
                                    if(cursey == 4) {
                                        cursey = 3;
                                        break;
                                    } //end restriction
                                }
                            }
                            else {
                                if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                    wattron(gamewin, COLOR_PAIR(3));
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    wattroff(gamewin,COLOR_PAIR(3));
                                }
                                else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                    wattron(gamewin, COLOR_PAIR(4));
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    wattroff(gamewin,COLOR_PAIR(4));
                                }
                                else {
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                }
                                wattroff(gamewin, A_REVERSE);
                                inichanged = true;
                            }
                        }
                        break;
                    case KEY_LEFT:
                        while(1) {
                            cursex--;
                            if(cursex == -1) {
                                cursex = 0;
                                break;
                            }
                            else {
                                if(display[cursey][cursex] == ' ') {
                                    if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                        wattron(gamewin, COLOR_PAIR(3));
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                        wattroff(gamewin,COLOR_PAIR(3));
                                    }
                                    else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                        wattron(gamewin, COLOR_PAIR(4));
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                        wattroff(gamewin,COLOR_PAIR(4));
                                    }
                                    else {
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    }
                                    wattroff(gamewin, A_REVERSE);
                                    inichanged = true;
                                    break;
                                }
                            }
                        }
                        break;
                    case KEY_RIGHT:
                        while(1) {
                            cursex++;
                            if(cursex == 4) {
                                cursex = 3;
                                break;
                            }
                            else {
                                if(display[cursey][cursex] == ' ') {
                                    if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                        wattron(gamewin, COLOR_PAIR(3));
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                        wattroff(gamewin,COLOR_PAIR(3));
                                    }
                                    else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                        wattron(gamewin, COLOR_PAIR(4));
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                        wattroff(gamewin,COLOR_PAIR(4));
                                    }
                                    else {
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    }
                                    wattroff(gamewin, A_REVERSE);
                                    inichanged = true;
                                    break;
                                }
                            }
                        }
                        break;
                    case 'w':
                        cursey--;
                        if(cursey == -1)
                            cursey = 0;
                        else {
                            if(display[cursey][cursex] != ' ') {
                                bool end0, end;
                                for(int m = 0; m < 4; m++) {
                                    bool breakm = false;
                                    end0 = false;
                                    end = false;
                                    for(int n = 1; n <= 4; n++) {
                                        if(end0 == false) {
                                            cursex = icursex-n;
                                            if(icursex-n == -1) {
                                                cursex = 0;
                                                end0 = true; //break after finish this n loop execution
                                            } //border restriction
                                        } //serch for left
                                        if(display[cursey][cursex] != ' ') {
                                            if(end == false) {
                                                cursex = icursex+n;
                                                if(icursex+n == 4) {
                                                    cursex = 3;
                                                    end = true; //break after finish this n loop execution
                                                }
                                                else {
                                                    if(display[cursey][cursex] == ' ') {
                                                        if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                                            wattron(gamewin, COLOR_PAIR(3));
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                            wattroff(gamewin,COLOR_PAIR(3));
                                                        }
                                                        else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                                            wattron(gamewin, COLOR_PAIR(4));
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                            wattroff(gamewin,COLOR_PAIR(4));
                                                        }
                                                        else {
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                        }
                                                        wattroff(gamewin, A_REVERSE);
                                                        inichanged = true;
                                                        breakm = true;
                                                        break;
                                                    } //if right suit
                                                }
                                            } //serch for right
                                        } //if left fail
                                        else {
                                            if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                                wattron(gamewin, COLOR_PAIR(3));
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                wattroff(gamewin,COLOR_PAIR(3));
                                            }
                                            else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                                wattron(gamewin, COLOR_PAIR(4));
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                wattroff(gamewin,COLOR_PAIR(4));
                                            }
                                            else {
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                            }
                                            wattroff(gamewin, A_REVERSE);
                                            inichanged = true;
                                            breakm = true;
                                            break;
                                        } //if leftt suit
                                    } //serch for row
                                    if(breakm)
                                        break;
                                    cursey--; //no suit element in the row
                                    if(cursey == -1) {
                                        cursey = 0;
                                        break;
                                    } //end restriction
                                }
                            }
                            else {
                                if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                    wattron(gamewin, COLOR_PAIR(3));
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    wattroff(gamewin,COLOR_PAIR(3));
                                }
                                else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                    wattron(gamewin, COLOR_PAIR(4));
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    wattroff(gamewin,COLOR_PAIR(4));
                                }
                                else {
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                }
                                wattroff(gamewin, A_REVERSE);
                                inichanged = true;
                            }
                        }
                        break;
                    case 's':
                        cursey++;
                        if(cursey == 4)
                            cursey = 3;
                        else {
                            if(display[cursey][cursex] != ' ') {
                                bool end0, end;
                                for(int m = 0; m < 4; m++) {
                                    bool breakm = false;
                                    end0 = false;
                                    end = false;
                                    for(int n = 1; n <= 4; n++) {
                                        if(end0 == false) {
                                            cursex = icursex-n;
                                            if(icursex-n == -1) {
                                                cursex = 0;
                                                end0 = true; //break after finish this n loop execution
                                            } //border restriction
                                        } //serch for left
                                        if(display[cursey][cursex] != ' ') {
                                            if(end == false) {
                                                cursex = icursex+n;
                                                if(icursex+n == 4) {
                                                    cursex = 3;
                                                    end = true; //break after finish this n loop execution
                                                }
                                                else {
                                                    if(display[cursey][cursex] == ' ') {
                                                        if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                                            wattron(gamewin, COLOR_PAIR(3));
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                            wattroff(gamewin,COLOR_PAIR(3));
                                                        }
                                                        else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                                            wattron(gamewin, COLOR_PAIR(4));
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                            wattroff(gamewin,COLOR_PAIR(4));
                                                        }
                                                        else {
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                        }
                                                        wattroff(gamewin, A_REVERSE);
                                                        inichanged = true;
                                                        breakm = true;
                                                        break;
                                                    } //if right suit
                                                }
                                            } //serch for right
                                        } //if left fail
                                        else {
                                            if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                                wattron(gamewin, COLOR_PAIR(3));
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                wattroff(gamewin,COLOR_PAIR(3));
                                            }
                                            else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                                wattron(gamewin, COLOR_PAIR(4));
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                wattroff(gamewin,COLOR_PAIR(4));
                                            }
                                            else {
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                            }
                                            wattroff(gamewin, A_REVERSE);
                                            inichanged = true;
                                            breakm = true;
                                            break;
                                        } //if right suit
                                    } //serch for row
                                    if(breakm)
                                        break;
                                    cursey++; //no suit element in the row
                                    if(cursey == 4) {
                                        cursey = 3;
                                        break;
                                    } //end restriction
                                }
                            }
                            else {
                                if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                    wattron(gamewin, COLOR_PAIR(3));
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    wattroff(gamewin,COLOR_PAIR(3));
                                }
                                else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                    wattron(gamewin, COLOR_PAIR(4));
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    wattroff(gamewin,COLOR_PAIR(4));
                                }
                                else {
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                }
                                wattroff(gamewin, A_REVERSE);
                                inichanged = true;
                            }
                        }
                        break;
                    case 'a':
                        while(1) {
                            cursex--;
                            if(cursex == -1) {
                                cursex = 0;
                                break;
                            }
                            else {
                                if(display[cursey][cursex] == ' ') {
                                    if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                        wattron(gamewin, COLOR_PAIR(3));
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                        wattroff(gamewin,COLOR_PAIR(3));
                                    }
                                    else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                        wattron(gamewin, COLOR_PAIR(4));
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                        wattroff(gamewin,COLOR_PAIR(4));
                                    }
                                    else {
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    }
                                    wattroff(gamewin, A_REVERSE);
                                    inichanged = true;
                                    break;
                                }
                            }
                        }
                        break;
                    case 'd':
                        while(1) {
                            cursex++;
                            if(cursex == 4) {
                                cursex = 3;
                                break;
                            }
                            else {
                                if(display[cursey][cursex] == ' ') {
                                    if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                        wattron(gamewin, COLOR_PAIR(3));
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                        wattroff(gamewin,COLOR_PAIR(3));
                                    }
                                    else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                        wattron(gamewin, COLOR_PAIR(4));
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                        wattroff(gamewin,COLOR_PAIR(4));
                                    }
                                    else {
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    }
                                    wattroff(gamewin, A_REVERSE);
                                    inichanged = true;
                                    break;
                                }
                            }
                        }
                        break;
                    case '1':
                        ans[icursey][icursex] = '1';
                        wattron(gamewin, A_REVERSE);
                        mvwprintw(gamewin, icursey*2+1, icursex*4+2, "1");
                        wattroff(gamewin, A_REVERSE);
                        fill = true;
                        break;
                    case '2':
                        ans[icursey][icursex] = '2';
                        wattron(gamewin, A_REVERSE);
                        mvwprintw(gamewin, icursey*2+1, icursex*4+2, "2");
                        wattroff(gamewin, A_REVERSE);
                        fill = true;
                        break;
                    case '3':
                        ans[icursey][icursex] = '3';
                        wattron(gamewin, A_REVERSE);
                        mvwprintw(gamewin, icursey*2+1, icursex*4+2, "3");
                        wattroff(gamewin, A_REVERSE);
                        fill = true;
                        break;
                    case '4':
                        ans[icursey][icursex] = '4';
                        wattron(gamewin, A_REVERSE);
                        mvwprintw(gamewin, icursey*2+1, icursex*4+2, "4");
                        wattroff(gamewin, A_REVERSE);
                        fill = true;
                        break;
                    case ' ':
                        ans[icursey][icursex] = ' ';
                        wattron(gamewin, A_REVERSE);
                        mvwprintw(gamewin, icursey*2+1, icursex*4+2, " ");
                        wattroff(gamewin, A_REVERSE);
                        break;
                    case 27:
                        exitgame = true;
                        break;
                    default:
                        break;
                }
                if(inichanged) {
                    if(ans[icursey][icursex] != ' ' && ans[icursey][icursex] == base[icursey][icursex]) {
                        wattron(gamewin, COLOR_PAIR(1));
                        mvwprintw(gamewin, icursey*2+1, icursex*4+1, " %c ", ans[icursey][icursex]);
                        wattroff(gamewin,COLOR_PAIR(1));
                    }
                    else if(ans[icursey][icursex] != ' ' && ans[icursey][icursex] != base[icursey][icursex]) {
                        wattron(gamewin,COLOR_PAIR(2));
                        mvwprintw(gamewin, icursey*2+1, icursex*4+1, " %c ", ans[icursey][icursex]);
                        wattroff(gamewin,COLOR_PAIR(2));
                    }
                    else {
                        mvwprintw(gamewin, icursey*2+1, icursex*4+1, " %c ", ans[icursey][icursex]);
                    }
                    icursey = cursey;
                    icursex = cursex;
                } //recover initial element
                if(exitgame) {
                    WINDOW * confirmwin = newwin(9, 41, (yMax-35)/2+20, (xMax-129)/2+77);
                    refresh();
                    box(confirmwin, 0, 0);
                    wattron(confirmwin, A_REVERSE);
                    mvwprintw(confirmwin, 0, 0, "                                         ");
                    wattroff(confirmwin, A_REVERSE);
                    mvwprintw(confirmwin, 3, 8, "DO YOU REALY WANT TO QUIT");
                    mvwprintw(confirmwin, 5, 3, "esc to Cancel     return to Confirm");
                    wrefresh(confirmwin);
                    bool confirm = false;
                    while(1) {
                        key = wgetch(confirmwin);
                        if(key == 27)
                            break;
                        else if(key == 10) {
                            confirm = true;
                            break;
                        }
                    }
                    if(confirm) {
                        game_function_end(true, false);
                        confirm = false;
                        break;
                    }
                    else {
                        wclear(confirmwin);
                        wrefresh(confirmwin);
                        delwin(confirmwin);
                        refresh();
                    }
                }
                if(fill) {
                    finishgame = true;
                    for(int i = 0; i < 4; i++) {
                        for(int j = 0; j < 4; j++) {
                            if(ans[i][j] != base[i][j]) {
                                finishgame = false;
                                break;
                            }
                        }
                    }
                    if(finishgame) {
                        game_function_end(true, false);
                        break;
                    }
                }
            }
            break;
            
        //9 by 9
        //------------------------------------------------------------------------------
        case 1:
            while(1) {
                key = wgetch(gamewin);
                fill = false;
                exitgame = false;
                cursey = icursey;
                cursex = icursex;
                bool inichanged = false;
                switch(key) {
                    case KEY_UP:
                        cursey--;
                        if(cursey == -1)
                            cursey = 0;
                        else {
                            if(display[cursey][cursex] != ' ') {
                                bool end0, end;
                                for(int m = 0; m < 9; m++) {
                                    bool breakm = false;
                                    end0 = false;
                                    end = false;
                                    for(int n = 1; n <= 9; n++) {
                                        if(end0 == false) {
                                            cursex = icursex-n;
                                            if(icursex-n == -1) {
                                                cursex = 0;
                                                end0 = true; //break after finish this n loop execution
                                            } //border restriction
                                        } //serch for left
                                        if(display[cursey][cursex] != ' ') {
                                            if(end == false) {
                                                cursex = icursex+n;
                                                if(icursex+n == 9) {
                                                    cursex = 8;
                                                    end = true; //break after finish this n loop execution
                                                }
                                                else {
                                                    if(display[cursey][cursex] == ' ') {
                                                        if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                                            wattron(gamewin, COLOR_PAIR(3));
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                            wattroff(gamewin,COLOR_PAIR(3));
                                                        }
                                                        else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                                            wattron(gamewin, COLOR_PAIR(4));
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                            wattroff(gamewin,COLOR_PAIR(4));
                                                        }
                                                        else {
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                        }
                                                        wattroff(gamewin, A_REVERSE);
                                                        inichanged = true;
                                                        breakm = true;
                                                        break;
                                                    } //if right suit
                                                }
                                            } //serch for right
                                        } //if left fail
                                        else {
                                            if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                                wattron(gamewin, COLOR_PAIR(3));
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                wattroff(gamewin,COLOR_PAIR(3));
                                            }
                                            else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                                wattron(gamewin, COLOR_PAIR(4));
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                wattroff(gamewin,COLOR_PAIR(4));
                                            }
                                            else {
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                            }
                                            wattroff(gamewin, A_REVERSE);
                                            inichanged = true;
                                            breakm = true;
                                            break;
                                        } //if leftt suit
                                    } //serch for row
                                    if(breakm)
                                        break;
                                    cursey--; //no suit element in the row
                                    if(cursey == -1) {
                                        cursey = 0;
                                        break;
                                    } //end restriction
                                }
                            }
                            else {
                                if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                    wattron(gamewin, COLOR_PAIR(3));
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    wattroff(gamewin,COLOR_PAIR(3));
                                }
                                else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                    wattron(gamewin, COLOR_PAIR(4));
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    wattroff(gamewin,COLOR_PAIR(4));
                                }
                                else {
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                }
                                wattroff(gamewin, A_REVERSE);
                                inichanged = true;
                            }
                        }
                        break;
                    case KEY_DOWN:
                        cursey++;
                        if(cursey == 9)
                            cursey = 8;
                        else {
                            if(display[cursey][cursex] != ' ') {
                                bool end0, end;
                                for(int m = 0; m < 9; m++) {
                                    bool breakm = false;
                                    end0 = false;
                                    end = false;
                                    for(int n = 1; n <= 8; n++) {
                                        if(end0 == false) {
                                            cursex = icursex-n;
                                            if(icursex-n == -1) {
                                                cursex = 0;
                                                end0 = true; //break after finish this n loop execution
                                            } //border restriction
                                        } //serch for left
                                        if(display[cursey][cursex] != ' ') {
                                            if(end == false) {
                                                cursex = icursex+n;
                                                if(icursex+n == 9) {
                                                    cursex = 8;
                                                    end = true; //break after finish this n loop execution
                                                }
                                                else {
                                                    if(display[cursey][cursex] == ' ') {
                                                        if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                                            wattron(gamewin, COLOR_PAIR(3));
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                            wattroff(gamewin,COLOR_PAIR(3));
                                                        }
                                                        else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                                            wattron(gamewin, COLOR_PAIR(4));
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                            wattroff(gamewin,COLOR_PAIR(4));
                                                        }
                                                        else {
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                        }
                                                        wattroff(gamewin, A_REVERSE);
                                                        inichanged = true;
                                                        breakm = true;
                                                        break;
                                                    } //if right suit
                                                }
                                            } //serch for right
                                        } //if left fail
                                        else {
                                            if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                                wattron(gamewin, COLOR_PAIR(3));
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                wattroff(gamewin,COLOR_PAIR(3));
                                            }
                                            else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                                wattron(gamewin, COLOR_PAIR(4));
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                wattroff(gamewin,COLOR_PAIR(4));
                                            }
                                            else {
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                            }
                                            wattroff(gamewin, A_REVERSE);
                                            inichanged = true;
                                            breakm = true;
                                            break;
                                        } //if right suit
                                    } //serch for row
                                    if(breakm)
                                        break;
                                    cursey++; //no suit element in the row
                                    if(cursey == 9) {
                                        cursey = 8;
                                        break;
                                    } //end restriction
                                }
                            }
                            else {
                                if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                    wattron(gamewin, COLOR_PAIR(3));
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    wattroff(gamewin,COLOR_PAIR(3));
                                }
                                else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                    wattron(gamewin, COLOR_PAIR(4));
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    wattroff(gamewin,COLOR_PAIR(4));
                                }
                                else {
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                }
                                wattroff(gamewin, A_REVERSE);
                                inichanged = true;
                            }
                        }
                        break;
                    case KEY_LEFT:
                        while(1) {
                            cursex--;
                            if(cursex == -1) {
                                cursex = 0;
                                break;
                            }
                            else {
                                if(display[cursey][cursex] == ' ') {
                                    if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                        wattron(gamewin, COLOR_PAIR(3));
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                        wattroff(gamewin,COLOR_PAIR(3));
                                    }
                                    else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                        wattron(gamewin, COLOR_PAIR(4));
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                        wattroff(gamewin,COLOR_PAIR(4));
                                    }
                                    else {
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    }
                                    wattroff(gamewin, A_REVERSE);
                                    inichanged = true;
                                    break;
                                }
                            }
                        }
                        break;
                    case KEY_RIGHT:
                        while(1) {
                            cursex++;
                            if(cursex == 9) {
                                cursex = 8;
                                break;
                            }
                            else {
                                if(display[cursey][cursex] == ' ') {
                                    if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                        wattron(gamewin, COLOR_PAIR(3));
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                        wattroff(gamewin,COLOR_PAIR(3));
                                    }
                                    else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                        wattron(gamewin, COLOR_PAIR(4));
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                        wattroff(gamewin,COLOR_PAIR(4));
                                    }
                                    else {
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    }
                                    wattroff(gamewin, A_REVERSE);
                                    inichanged = true;
                                    break;
                                }
                            }
                        }
                        break;
                    case 'w':
                        cursey--;
                        if(cursey == -1)
                            cursey = 0;
                        else {
                            if(display[cursey][cursex] != ' ') {
                                bool end0, end;
                                for(int m = 0; m < 9; m++) {
                                    bool breakm = false;
                                    end0 = false;
                                    end = false;
                                    for(int n = 1; n <= 9; n++) {
                                        if(end0 == false) {
                                            cursex = icursex-n;
                                            if(icursex-n == -1) {
                                                cursex = 0;
                                                end0 = true; //break after finish this n loop execution
                                            } //border restriction
                                        } //serch for left
                                        if(display[cursey][cursex] != ' ') {
                                            if(end == false) {
                                                cursex = icursex+n;
                                                if(icursex+n == 9) {
                                                    cursex = 8;
                                                    end = true; //break after finish this n loop execution
                                                }
                                                else {
                                                    if(display[cursey][cursex] == ' ') {
                                                        if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                                            wattron(gamewin, COLOR_PAIR(3));
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                            wattroff(gamewin,COLOR_PAIR(3));
                                                        }
                                                        else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                                            wattron(gamewin, COLOR_PAIR(4));
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                            wattroff(gamewin,COLOR_PAIR(4));
                                                        }
                                                        else {
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                        }
                                                        wattroff(gamewin, A_REVERSE);
                                                        inichanged = true;
                                                        breakm = true;
                                                        break;
                                                    } //if right suit
                                                }
                                            } //serch for right
                                        } //if left fail
                                        else {
                                            if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                                wattron(gamewin, COLOR_PAIR(3));
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                wattroff(gamewin,COLOR_PAIR(3));
                                            }
                                            else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                                wattron(gamewin, COLOR_PAIR(4));
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                wattroff(gamewin,COLOR_PAIR(4));
                                            }
                                            else {
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                            }
                                            wattroff(gamewin, A_REVERSE);
                                            inichanged = true;
                                            breakm = true;
                                            break;
                                        } //if leftt suit
                                    } //serch for row
                                    if(breakm)
                                        break;
                                    cursey--; //no suit element in the row
                                    if(cursey == -1) {
                                        cursey = 0;
                                        break;
                                    } //end restriction
                                }
                            }
                            else {
                                if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                    wattron(gamewin, COLOR_PAIR(3));
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    wattroff(gamewin,COLOR_PAIR(3));
                                }
                                else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                    wattron(gamewin, COLOR_PAIR(4));
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    wattroff(gamewin,COLOR_PAIR(4));
                                }
                                else {
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                }
                                wattroff(gamewin, A_REVERSE);
                                inichanged = true;
                            }
                        }
                        break;
                    case 's':
                        cursey++;
                        if(cursey == 9)
                            cursey = 8;
                        else {
                            if(display[cursey][cursex] != ' ') {
                                bool end0, end;
                                for(int m = 0; m < 9; m++) {
                                    bool breakm = false;
                                    end0 = false;
                                    end = false;
                                    for(int n = 1; n <= 8; n++) {
                                        if(end0 == false) {
                                            cursex = icursex-n;
                                            if(icursex-n == -1) {
                                                cursex = 0;
                                                end0 = true; //break after finish this n loop execution
                                            } //border restriction
                                        } //serch for left
                                        if(display[cursey][cursex] != ' ') {
                                            if(end == false) {
                                                cursex = icursex+n;
                                                if(icursex+n == 9) {
                                                    cursex = 8;
                                                    end = true; //break after finish this n loop execution
                                                }
                                                else {
                                                    if(display[cursey][cursex] == ' ') {
                                                        if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                                            wattron(gamewin, COLOR_PAIR(3));
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                            wattroff(gamewin,COLOR_PAIR(3));
                                                        }
                                                        else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                                            wattron(gamewin, COLOR_PAIR(4));
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                            wattroff(gamewin,COLOR_PAIR(4));
                                                        }
                                                        else {
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                        }
                                                        wattroff(gamewin, A_REVERSE);
                                                        inichanged = true;
                                                        breakm = true;
                                                        break;
                                                    } //if right suit
                                                }
                                            } //serch for right
                                        } //if left fail
                                        else {
                                            if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                                wattron(gamewin, COLOR_PAIR(3));
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                wattroff(gamewin,COLOR_PAIR(3));
                                            }
                                            else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                                wattron(gamewin, COLOR_PAIR(4));
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                wattroff(gamewin,COLOR_PAIR(4));
                                            }
                                            else {
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                            }
                                            wattroff(gamewin, A_REVERSE);
                                            inichanged = true;
                                            breakm = true;
                                            break;
                                        } //if right suit
                                    } //serch for row
                                    if(breakm)
                                        break;
                                    cursey++; //no suit element in the row
                                    if(cursey == 9) {
                                        cursey = 8;
                                        break;
                                    } //end restriction
                                }
                            }
                            else {
                                if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                    wattron(gamewin, COLOR_PAIR(3));
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    wattroff(gamewin,COLOR_PAIR(3));
                                }
                                else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                    wattron(gamewin, COLOR_PAIR(4));
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    wattroff(gamewin,COLOR_PAIR(4));
                                }
                                else {
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                }
                                wattroff(gamewin, A_REVERSE);
                                inichanged = true;
                            }
                        }
                        break;
                    case 'a':
                        while(1) {
                            cursex--;
                            if(cursex == -1) {
                                cursex = 0;
                                break;
                            }
                            else {
                                if(display[cursey][cursex] == ' ') {
                                    if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                        wattron(gamewin, COLOR_PAIR(3));
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                        wattroff(gamewin,COLOR_PAIR(3));
                                    }
                                    else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                        wattron(gamewin, COLOR_PAIR(4));
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                        wattroff(gamewin,COLOR_PAIR(4));
                                    }
                                    else {
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    }
                                    wattroff(gamewin, A_REVERSE);
                                    inichanged = true;
                                    break;
                                }
                            }
                        }
                        break;
                    case 'd':
                        while(1) {
                            cursex++;
                            if(cursex == 9) {
                                cursex = 8;
                                break;
                            }
                            else {
                                if(display[cursey][cursex] == ' ') {
                                    if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                        wattron(gamewin, COLOR_PAIR(3));
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                        wattroff(gamewin,COLOR_PAIR(3));
                                    }
                                    else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                        wattron(gamewin, COLOR_PAIR(4));
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                        wattroff(gamewin,COLOR_PAIR(4));
                                    }
                                    else {
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    }
                                    wattroff(gamewin, A_REVERSE);
                                    inichanged = true;
                                    break;
                                }
                            }
                        }
                        break;
                    case '1':
                        ans[icursey][icursex] = '1';
                        wattron(gamewin, A_REVERSE);
                        mvwprintw(gamewin, icursey*2+1, icursex*4+2, "1");
                        wattroff(gamewin, A_REVERSE);
                        fill = true;
                        break;
                    case '2':
                        ans[icursey][icursex] = '2';
                        wattron(gamewin, A_REVERSE);
                        mvwprintw(gamewin, icursey*2+1, icursex*4+2, "2");
                        wattroff(gamewin, A_REVERSE);
                        fill = true;
                        break;
                    case '3':
                        ans[icursey][icursex] = '3';
                        wattron(gamewin, A_REVERSE);
                        mvwprintw(gamewin, icursey*2+1, icursex*4+2, "3");
                        wattroff(gamewin, A_REVERSE);
                        fill = true;
                        break;
                    case '4':
                        ans[icursey][icursex] = '4';
                        wattron(gamewin, A_REVERSE);
                        mvwprintw(gamewin, icursey*2+1, icursex*4+2, "4");
                        wattroff(gamewin, A_REVERSE);
                        fill = true;
                        break;
                    case '5':
                        ans[icursey][icursex] = '5';
                        wattron(gamewin, A_REVERSE);
                        mvwprintw(gamewin, icursey*2+1, icursex*4+2, "5");
                        wattroff(gamewin, A_REVERSE);
                        fill = true;
                        break;
                    case '6':
                        ans[icursey][icursex] = '6';
                        wattron(gamewin, A_REVERSE);
                        mvwprintw(gamewin, icursey*2+1, icursex*4+2, "6");
                        wattroff(gamewin, A_REVERSE);
                        fill = true;
                        break;
                    case '7':
                        ans[icursey][icursex] = '7';
                        wattron(gamewin, A_REVERSE);
                        mvwprintw(gamewin, icursey*2+1, icursex*4+2, "7");
                        wattroff(gamewin, A_REVERSE);
                        fill = true;
                        break;
                    case '8':
                        ans[icursey][icursex] = '8';
                        wattron(gamewin, A_REVERSE);
                        mvwprintw(gamewin, icursey*2+1, icursex*4+2, "8");
                        wattroff(gamewin, A_REVERSE);
                        fill = true;
                        break;
                    case '9':
                        ans[icursey][icursex] = '9';
                        wattron(gamewin, A_REVERSE);
                        mvwprintw(gamewin, icursey*2+1, icursex*4+2, "9");
                        wattroff(gamewin, A_REVERSE);
                        fill = true;
                        break;
                    case ' ':
                        ans[icursey][icursex] = ' ';
                        wattron(gamewin, A_REVERSE);
                        mvwprintw(gamewin, icursey*2+1, icursex*4+2, " ");
                        wattroff(gamewin, A_REVERSE);
                        break;
                    case 27:
                        exitgame = true;
                        break;
                    default:
                        break;
                }
                if(inichanged) {
                    if(ans[icursey][icursex] != ' ' && ans[icursey][icursex] == base[icursey][icursex]) {
                        wattron(gamewin, COLOR_PAIR(1));
                        mvwprintw(gamewin, icursey*2+1, icursex*4+1, " %c ", ans[icursey][icursex]);
                        wattroff(gamewin,COLOR_PAIR(1));
                    }
                    else if(ans[icursey][icursex] != ' ' && ans[icursey][icursex] != base[icursey][icursex]) {
                        wattron(gamewin,COLOR_PAIR(2));
                        mvwprintw(gamewin, icursey*2+1, icursex*4+1, " %c ", ans[icursey][icursex]);
                        wattroff(gamewin,COLOR_PAIR(2));
                    }
                    else {
                        mvwprintw(gamewin, icursey*2+1, icursex*4+1, " %c ", ans[icursey][icursex]);
                    }
                    icursey = cursey;
                    icursex = cursex;
                } //recover initial element
                if(exitgame) {
                    WINDOW * confirmwin = newwin(9, 41, (yMax-35)/2+20, (xMax-129)/2+77);
                    refresh();
                    box(confirmwin, 0, 0);
                    wattron(confirmwin, A_REVERSE);
                    mvwprintw(confirmwin, 0, 0, "                                         ");
                    wattroff(confirmwin, A_REVERSE);
                    mvwprintw(confirmwin, 3, 8, "DO YOU REALY WANT TO QUIT");
                    mvwprintw(confirmwin, 5, 3, "esc to Cancel     return to Confirm");
                    wrefresh(confirmwin);
                    bool confirm = false;
                    while(1) {
                        key = wgetch(confirmwin);
                        if(key == 27)
                            break;
                        else if(key == 10) {
                            confirm = true;
                            break;
                        }
                    }
                    if(confirm) {
                        game_function_end(true, false);
                        confirm = false;
                        break;
                    }
                    else {
                        wclear(confirmwin);
                        wrefresh(confirmwin);
                        delwin(confirmwin);
                        refresh();
                    }
                }
                if(fill) {
                    finishgame = true;
                    for(int i = 0; i < 9; i++) {
                        for(int j = 0; j < 9; j++) {
                            if(ans[i][j] != base[i][j]) {
                                finishgame = false;
                                break;
                            }
                        }
                    }
                    if(finishgame) {
                        game_function_end(true, false);
                        break;
                    }
                }
            }
            break;
            
        //16 by 16
        //------------------------------------------------------------------------------
        case 2:
            while(1) {
                key = wgetch(gamewin);
                fill = false;
                exitgame = false;
                cursey = icursey;
                cursex = icursex;
                bool inichanged = false;
                switch(key) {
                    case KEY_UP:
                        cursey--;
                        if(cursey == -1)
                            cursey = 0;
                        else {
                            if(display[cursey][cursex] != ' ') {
                                bool end0, end;
                                for(int m = 0; m < 16; m++) {
                                    bool breakm = false;
                                    end0 = false;
                                    end = false;
                                    for(int n = 1; n <= 16; n++) {
                                        if(end0 == false) {
                                            cursex = icursex-n;
                                            if(icursex-n == -1) {
                                                cursex = 0;
                                                end0 = true; //break after finish this n loop execution
                                            } //border restriction
                                        } //serch for left
                                        if(display[cursey][cursex] != ' ') {
                                            if(end == false) {
                                                cursex = icursex+n;
                                                if(icursex+n == 16) {
                                                    cursex = 15;
                                                    end = true; //break after finish this n loop execution
                                                }
                                                else {
                                                    if(display[cursey][cursex] == ' ') {
                                                        if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                                            wattron(gamewin, COLOR_PAIR(3));
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                            wattroff(gamewin,COLOR_PAIR(3));
                                                        }
                                                        else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                                            wattron(gamewin, COLOR_PAIR(4));
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                            wattroff(gamewin,COLOR_PAIR(4));
                                                        }
                                                        else {
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                        }
                                                        wattroff(gamewin, A_REVERSE);
                                                        inichanged = true;
                                                        breakm = true;
                                                        break;
                                                    } //if right suit
                                                }
                                            } //serch for right
                                        } //if left fail
                                        else {
                                            if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                                wattron(gamewin, COLOR_PAIR(3));
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                wattroff(gamewin,COLOR_PAIR(3));
                                            }
                                            else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                                wattron(gamewin, COLOR_PAIR(4));
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                wattroff(gamewin,COLOR_PAIR(4));
                                            }
                                            else {
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                            }
                                            wattroff(gamewin, A_REVERSE);
                                            inichanged = true;
                                            breakm = true;
                                            break;
                                        } //if leftt suit
                                    } //serch for row
                                    if(breakm)
                                        break;
                                    cursey--; //no suit element in the row
                                    if(cursey == -1) {
                                        cursey = 0;
                                        break;
                                    } //end restriction
                                }
                            }
                            else {
                                if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                    wattron(gamewin, COLOR_PAIR(3));
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    wattroff(gamewin,COLOR_PAIR(3));
                                }
                                else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                    wattron(gamewin, COLOR_PAIR(4));
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    wattroff(gamewin,COLOR_PAIR(4));
                                }
                                else {
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                }
                                wattroff(gamewin, A_REVERSE);
                                inichanged = true;
                            }
                        }
                        break;
                    case KEY_DOWN:
                        cursey++;
                        if(cursey == 16)
                            cursey = 15;
                        else {
                            if(display[cursey][cursex] != ' ') {
                                bool end0, end;
                                for(int m = 0; m < 16; m++) {
                                    bool breakm = false;
                                    end0 = false;
                                    end = false;
                                    for(int n = 1; n <= 16; n++) {
                                        if(end0 == false) {
                                            cursex = icursex-n;
                                            if(icursex-n == -1) {
                                                cursex = 0;
                                                end0 = true; //break after finish this n loop execution
                                            } //border restriction
                                        } //serch for left
                                        if(display[cursey][cursex] != ' ') {
                                            if(end == false) {
                                                cursex = icursex+n;
                                                if(icursex+n == 16) {
                                                    cursex = 15;
                                                    end = true; //break after finish this n loop execution
                                                }
                                                else {
                                                    if(display[cursey][cursex] == ' ') {
                                                        if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                                            wattron(gamewin, COLOR_PAIR(3));
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                            wattroff(gamewin,COLOR_PAIR(3));
                                                        }
                                                        else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                                            wattron(gamewin, COLOR_PAIR(4));
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                            wattroff(gamewin,COLOR_PAIR(4));
                                                        }
                                                        else {
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                        }
                                                        wattroff(gamewin, A_REVERSE);
                                                        inichanged = true;
                                                        breakm = true;
                                                        break;
                                                    } //if right suit
                                                }
                                            } //serch for right
                                        } //if left fail
                                        else {
                                            if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                                wattron(gamewin, COLOR_PAIR(3));
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                wattroff(gamewin,COLOR_PAIR(3));
                                            }
                                            else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                                wattron(gamewin, COLOR_PAIR(4));
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                wattroff(gamewin,COLOR_PAIR(4));
                                            }
                                            else {
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                            }
                                            wattroff(gamewin, A_REVERSE);
                                            inichanged = true;
                                            breakm = true;
                                            break;
                                        } //if right suit
                                    } //serch for row
                                    if(breakm)
                                        break;
                                    cursey++; //no suit element in the row
                                    if(cursey == 16) {
                                        cursey = 15;
                                        break;
                                    } //end restriction
                                }
                            }
                            else {
                                if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                    wattron(gamewin, COLOR_PAIR(3));
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    wattroff(gamewin,COLOR_PAIR(3));
                                }
                                else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                    wattron(gamewin, COLOR_PAIR(4));
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    wattroff(gamewin,COLOR_PAIR(4));
                                }
                                else {
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                }
                                wattroff(gamewin, A_REVERSE);
                                inichanged = true;
                            }
                        }
                        break;
                    case KEY_LEFT:
                        while(1) {
                            cursex--;
                            if(cursex == -1) {
                                cursex = 0;
                                break;
                            }
                            else {
                                if(display[cursey][cursex] == ' ') {
                                    if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                        wattron(gamewin, COLOR_PAIR(3));
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                        wattroff(gamewin,COLOR_PAIR(3));
                                    }
                                    else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                        wattron(gamewin, COLOR_PAIR(4));
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                        wattroff(gamewin,COLOR_PAIR(4));
                                    }
                                    else {
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    }
                                    wattroff(gamewin, A_REVERSE);
                                    inichanged = true;
                                    break;
                                }
                            }
                        }
                        break;
                    case KEY_RIGHT:
                        while(1) {
                            cursex++;
                            if(cursex == 16) {
                                cursex = 15;
                                break;
                            }
                            else {
                                if(display[cursey][cursex] == ' ') {
                                    if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                        wattron(gamewin, COLOR_PAIR(3));
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                        wattroff(gamewin,COLOR_PAIR(3));
                                    }
                                    else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                        wattron(gamewin, COLOR_PAIR(4));
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                        wattroff(gamewin,COLOR_PAIR(4));
                                    }
                                    else {
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    }
                                    wattroff(gamewin, A_REVERSE);
                                    inichanged = true;
                                    break;
                                }
                            }
                        }
                        break;
                    case 'w':
                        cursey--;
                        if(cursey == -1)
                            cursey = 0;
                        else {
                            if(display[cursey][cursex] != ' ') {
                                bool end0, end;
                                for(int m = 0; m < 16; m++) {
                                    bool breakm = false;
                                    end0 = false;
                                    end = false;
                                    for(int n = 1; n <= 16; n++) {
                                        if(end0 == false) {
                                            cursex = icursex-n;
                                            if(icursex-n == -1) {
                                                cursex = 0;
                                                end0 = true; //break after finish this n loop execution
                                            } //border restriction
                                        } //serch for left
                                        if(display[cursey][cursex] != ' ') {
                                            if(end == false) {
                                                cursex = icursex+n;
                                                if(icursex+n == 16) {
                                                    cursex = 15;
                                                    end = true; //break after finish this n loop execution
                                                }
                                                else {
                                                    if(display[cursey][cursex] == ' ') {
                                                        if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                                            wattron(gamewin, COLOR_PAIR(3));
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                            wattroff(gamewin,COLOR_PAIR(3));
                                                        }
                                                        else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                                            wattron(gamewin, COLOR_PAIR(4));
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                            wattroff(gamewin,COLOR_PAIR(4));
                                                        }
                                                        else {
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                        }
                                                        wattroff(gamewin, A_REVERSE);
                                                        inichanged = true;
                                                        breakm = true;
                                                        break;
                                                    } //if right suit
                                                }
                                            } //serch for right
                                        } //if left fail
                                        else {
                                            if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                                wattron(gamewin, COLOR_PAIR(3));
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                wattroff(gamewin,COLOR_PAIR(3));
                                            }
                                            else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                                wattron(gamewin, COLOR_PAIR(4));
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                wattroff(gamewin,COLOR_PAIR(4));
                                            }
                                            else {
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                            }
                                            wattroff(gamewin, A_REVERSE);
                                            inichanged = true;
                                            breakm = true;
                                            break;
                                        } //if leftt suit
                                    } //serch for row
                                    if(breakm)
                                        break;
                                    cursey--; //no suit element in the row
                                    if(cursey == -1) {
                                        cursey = 0;
                                        break;
                                    } //end restriction
                                }
                            }
                            else {
                                if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                    wattron(gamewin, COLOR_PAIR(3));
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    wattroff(gamewin,COLOR_PAIR(3));
                                }
                                else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                    wattron(gamewin, COLOR_PAIR(4));
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    wattroff(gamewin,COLOR_PAIR(4));
                                }
                                else {
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                }
                                wattroff(gamewin, A_REVERSE);
                                inichanged = true;
                            }
                        }
                        break;
                    case 's':
                        cursey++;
                        if(cursey == 16)
                            cursey = 15;
                        else {
                            if(display[cursey][cursex] != ' ') {
                                bool end0, end;
                                for(int m = 0; m < 16; m++) {
                                    bool breakm = false;
                                    end0 = false;
                                    end = false;
                                    for(int n = 1; n <= 16; n++) {
                                        if(end0 == false) {
                                            cursex = icursex-n;
                                            if(icursex-n == -1) {
                                                cursex = 0;
                                                end0 = true; //break after finish this n loop execution
                                            } //border restriction
                                        } //serch for left
                                        if(display[cursey][cursex] != ' ') {
                                            if(end == false) {
                                                cursex = icursex+n;
                                                if(icursex+n == 16) {
                                                    cursex = 15;
                                                    end = true; //break after finish this n loop execution
                                                }
                                                else {
                                                    if(display[cursey][cursex] == ' ') {
                                                        if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                                            wattron(gamewin, COLOR_PAIR(3));
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                            wattroff(gamewin,COLOR_PAIR(3));
                                                        }
                                                        else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                                            wattron(gamewin, COLOR_PAIR(4));
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                            wattroff(gamewin,COLOR_PAIR(4));
                                                        }
                                                        else {
                                                            wattron(gamewin, A_REVERSE);
                                                            mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                        }
                                                        wattroff(gamewin, A_REVERSE);
                                                        inichanged = true;
                                                        breakm = true;
                                                        break;
                                                    } //if right suit
                                                }
                                            } //serch for right
                                        } //if left fail
                                        else {
                                            if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                                wattron(gamewin, COLOR_PAIR(3));
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                wattroff(gamewin,COLOR_PAIR(3));
                                            }
                                            else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                                wattron(gamewin, COLOR_PAIR(4));
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                                wattroff(gamewin,COLOR_PAIR(4));
                                            }
                                            else {
                                                wattron(gamewin, A_REVERSE);
                                                mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                            }
                                            wattroff(gamewin, A_REVERSE);
                                            inichanged = true;
                                            breakm = true;
                                            break;
                                        } //if right suit
                                    } //serch for row
                                    if(breakm)
                                        break;
                                    cursey++; //no suit element in the row
                                    if(cursey == 16) {
                                        cursey = 15;
                                        break;
                                    } //end restriction
                                }
                            }
                            else {
                                if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                    wattron(gamewin, COLOR_PAIR(3));
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    wattroff(gamewin,COLOR_PAIR(3));
                                }
                                else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                    wattron(gamewin, COLOR_PAIR(4));
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    wattroff(gamewin,COLOR_PAIR(4));
                                }
                                else {
                                    wattron(gamewin, A_REVERSE);
                                    mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                }
                                wattroff(gamewin, A_REVERSE);
                                inichanged = true;
                            }
                        }
                        break;
                    case 'a':
                        while(1) {
                            cursex--;
                            if(cursex == -1) {
                                cursex = 0;
                                break;
                            }
                            else {
                                if(display[cursey][cursex] == ' ') {
                                    if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                        wattron(gamewin, COLOR_PAIR(3));
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                        wattroff(gamewin,COLOR_PAIR(3));
                                    }
                                    else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                        wattron(gamewin, COLOR_PAIR(4));
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                        wattroff(gamewin,COLOR_PAIR(4));
                                    }
                                    else {
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    }
                                    wattroff(gamewin, A_REVERSE);
                                    inichanged = true;
                                    break;
                                }
                            }
                        }
                        break;
                    case 'd':
                        while(1) {
                            cursex++;
                            if(cursex == 16) {
                                cursex = 15;
                                break;
                            }
                            else {
                                if(display[cursey][cursex] == ' ') {
                                    if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] == base[cursey][cursex]) {
                                        wattron(gamewin, COLOR_PAIR(3));
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                        wattroff(gamewin,COLOR_PAIR(3));
                                    }
                                    else if(ans[cursey][cursex] != ' ' && ans[cursey][cursex] != base[cursey][cursex]) {
                                        wattron(gamewin, COLOR_PAIR(4));
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                        wattroff(gamewin,COLOR_PAIR(4));
                                    }
                                    else {
                                        wattron(gamewin, A_REVERSE);
                                        mvwprintw(gamewin, cursey*2+1, cursex*4+1, " %c ", ans[cursey][cursex]);
                                    }
                                    wattroff(gamewin, A_REVERSE);
                                    inichanged = true;
                                    break;
                                }
                            }
                        }
                        break;
                    case '1':
                        ans[icursey][icursex] = '1';
                        wattron(gamewin, A_REVERSE);
                        mvwprintw(gamewin, icursey*2+1, icursex*4+2, "1");
                        wattroff(gamewin, A_REVERSE);
                        fill = true;
                        break;
                    case '2':
                        ans[icursey][icursex] = '2';
                        wattron(gamewin, A_REVERSE);
                        mvwprintw(gamewin, icursey*2+1, icursex*4+2, "2");
                        wattroff(gamewin, A_REVERSE);
                        fill = true;
                        break;
                    case '3':
                        ans[icursey][icursex] = '3';
                        wattron(gamewin, A_REVERSE);
                        mvwprintw(gamewin, icursey*2+1, icursex*4+2, "3");
                        wattroff(gamewin, A_REVERSE);
                        fill = true;
                        break;
                    case '4':
                        ans[icursey][icursex] = '4';
                        wattron(gamewin, A_REVERSE);
                        mvwprintw(gamewin, icursey*2+1, icursex*4+2, "4");
                        wattroff(gamewin, A_REVERSE);
                        fill = true;
                        break;
                    case '5':
                        ans[icursey][icursex] = '5';
                        wattron(gamewin, A_REVERSE);
                        mvwprintw(gamewin, icursey*2+1, icursex*4+2, "5");
                        wattroff(gamewin, A_REVERSE);
                        fill = true;
                        break;
                    case '6':
                        ans[icursey][icursex] = '6';
                        wattron(gamewin, A_REVERSE);
                        mvwprintw(gamewin, icursey*2+1, icursex*4+2, "6");
                        wattroff(gamewin, A_REVERSE);
                        fill = true;
                        break;
                    case '7':
                        ans[icursey][icursex] = '7';
                        wattron(gamewin, A_REVERSE);
                        mvwprintw(gamewin, icursey*2+1, icursex*4+2, "7");
                        wattroff(gamewin, A_REVERSE);
                        fill = true;
                        break;
                    case '8':
                        ans[icursey][icursex] = '8';
                        wattron(gamewin, A_REVERSE);
                        mvwprintw(gamewin, icursey*2+1, icursex*4+2, "8");
                        wattroff(gamewin, A_REVERSE);
                        fill = true;
                        break;
                    case '9':
                        ans[icursey][icursex] = '9';
                        wattron(gamewin, A_REVERSE);
                        mvwprintw(gamewin, icursey*2+1, icursex*4+2, "9");
                        wattroff(gamewin, A_REVERSE);
                        fill = true;
                        break;
                    case 'A':
                        ans[icursey][icursex] = 'A';
                        wattron(gamewin, A_REVERSE);
                        mvwprintw(gamewin, icursey*2+1, icursex*4+2, "A");
                        wattroff(gamewin, A_REVERSE);
                        fill = true;
                        break;
                    case 'B':
                        ans[icursey][icursex] = 'B';
                        wattron(gamewin, A_REVERSE);
                        mvwprintw(gamewin, icursey*2+1, icursex*4+2, "B");
                        wattroff(gamewin, A_REVERSE);
                        fill = true;
                        break;
                    case 'C':
                        ans[icursey][icursex] = 'C';
                        wattron(gamewin, A_REVERSE);
                        mvwprintw(gamewin, icursey*2+1, icursex*4+2, "C");
                        wattroff(gamewin, A_REVERSE);
                        fill = true;
                        break;
                    case 'D':
                        ans[icursey][icursex] = 'D';
                        wattron(gamewin, A_REVERSE);
                        mvwprintw(gamewin, icursey*2+1, icursex*4+2, "D");
                        wattroff(gamewin, A_REVERSE);
                        fill = true;
                        break;
                    case 'E':
                        ans[icursey][icursex] = 'E';
                        wattron(gamewin, A_REVERSE);
                        mvwprintw(gamewin, icursey*2+1, icursex*4+2, "E");
                        wattroff(gamewin, A_REVERSE);
                        fill = true;
                        break;
                    case 'F':
                        ans[icursey][icursex] = 'F';
                        wattron(gamewin, A_REVERSE);
                        mvwprintw(gamewin, icursey*2+1, icursex*4+2, "F");
                        wattroff(gamewin, A_REVERSE);
                        fill = true;
                        break;
                    case 'G':
                        ans[icursey][icursex] = 'G';
                        wattron(gamewin, A_REVERSE);
                        mvwprintw(gamewin, icursey*2+1, icursex*4+2, "G");
                        wattroff(gamewin, A_REVERSE);
                        fill = true;
                        break;
                    case ' ':
                        ans[icursey][icursex] = ' ';
                        wattron(gamewin, A_REVERSE);
                        mvwprintw(gamewin, icursey*2+1, icursex*4+2, " ");
                        wattroff(gamewin, A_REVERSE);
                        break;
                    case 27:
                        exitgame = true;
                        break;
                    default:
                        break;
                }
                if(inichanged) {
                    if(ans[icursey][icursex] != ' ' && ans[icursey][icursex] == base[icursey][icursex]) {
                        wattron(gamewin, COLOR_PAIR(1));
                        mvwprintw(gamewin, icursey*2+1, icursex*4+1, " %c ", ans[icursey][icursex]);
                        wattroff(gamewin,COLOR_PAIR(1));
                    }
                    else if(ans[icursey][icursex] != ' ' && ans[icursey][icursex] != base[icursey][icursex]) {
                        wattron(gamewin,COLOR_PAIR(2));
                        mvwprintw(gamewin, icursey*2+1, icursex*4+1, " %c ", ans[icursey][icursex]);
                        wattroff(gamewin,COLOR_PAIR(2));
                    }
                    else {
                        mvwprintw(gamewin, icursey*2+1, icursex*4+1, " %c ", ans[icursey][icursex]);
                    }
                    icursey = cursey;
                    icursex = cursex;
                } //recover initial element
                if(exitgame) {
                    WINDOW * confirmwin = newwin(9, 41, (yMax-35)/2+20, (xMax-129)/2+77);
                    refresh();
                    box(confirmwin, 0, 0);
                    wattron(confirmwin, A_REVERSE);
                    mvwprintw(confirmwin, 0, 0, "                                         ");
                    wattroff(confirmwin, A_REVERSE);
                    mvwprintw(confirmwin, 3, 8, "DO YOU REALY WANT TO QUIT");
                    mvwprintw(confirmwin, 5, 3, "esc to Cancel     return to Confirm");
                    wrefresh(confirmwin);
                    bool confirm = false;
                    while(1) {
                        key = wgetch(confirmwin);
                        if(key == 27)
                            break;
                        else if(key == 10) {
                            confirm = true;
                            break;
                        }
                    }
                    if(confirm) {
                        game_function_end(true, false);
                        confirm = false;
                        break;
                    }
                    else {
                        wclear(confirmwin);
                        wrefresh(confirmwin);
                        delwin(confirmwin);
                        refresh();
                    }
                }
                if(fill) {
                    finishgame = true;
                    for(int i = 0; i < 16; i++) {
                        for(int j = 0; j < 16; j++) {
                            if(ans[i][j] != base[i][j]) {
                                finishgame = false;
                                break;
                            }
                        }
                    }
                    if(finishgame) {
                        game_function_end(true, false);
                        break;
                    }
                }
            }
            break;
        default:
            cout << "mode selection error!!\n\n";
            exit(1);
    }
    wattron(gamewin,COLOR_PAIR(1));
    mvwprintw(gamewin, icursey*2+1, icursex*4+1, " %c ", ans[icursey][icursex]);
    wattroff(gamewin,COLOR_PAIR(1));
    wrefresh(gamewin);
}
//==================================================================================================


int main() {
    setlocale(LC_ALL, ""); //function for unicode
    
    initscr(); //initialize screen
    cbreak(); //disable line buffering
    curs_set(0); //hide curse
    noecho(); //not printing pressed key
    
    bool readjust = false, startgame = false;
    while(1) { //allow readjust windows
    //Adjusting interface
    //==============================================================================================
        WINDOW * adjwin = newwin(36, 130, 0, 0);
        refresh(); //screen refresh
        bool jump = false;
        int c;
        while(1) {
            mvwprintw(adjwin, 0, 0, "Please adjust the window to fit the grid, zoom adjustion is allowed");
            mvwprintw(adjwin, 1, 0, "press \"r\" to refresh");
            mvwprintw(adjwin, 2, 0, "press \"c\" to complete setup (making sure you've press \"r\")");
            _16by16_grid(adjwin, 3, 0);
            _16by16_grid(adjwin, 3, 65);
            wrefresh(adjwin); //win refresh
            while(1) {
                c = wgetch(adjwin);
                if (c == int('r') || c == int('R'))
                    break;
                else if (c == int('c') || c == int('C')) {
                    jump = true;
                    break;
                }
            } //r & c key detection
            clear();
            if(jump)
                break;
        }
        delwin(adjwin);
        
    //Border window
    //==============================================================================================
        WINDOW * borderwin;
        getmaxyx(stdscr, yMax, xMax);
        borderwin = newwin(35, 129, (yMax-35)/2, (xMax-129)/2);
        box(borderwin, 0, 0);
        refresh();
        wrefresh(borderwin);
        
    //Starting display
    //==============================================================================================
        if(!readjust) {
            int delay;
            double root = 1.1;
            mvwprintw(borderwin, 15, 32, "                                      _");
            wrefresh(borderwin);
            delay = 700000;
            usleep(delay);
            mvwprintw(borderwin, 15, 32, "                                      _");
            mvwprintw(borderwin, 17, 32, "\\");
            wrefresh(borderwin);
            delay /= root;
            usleep(delay);
            mvwprintw(borderwin, 15, 32, "                                      _                      _");
            mvwprintw(borderwin, 17, 32, "\\");
            wrefresh(borderwin);
            delay /= root;
            usleep(delay);
            mvwprintw(borderwin, 15, 32, "                                      _                      _");
            mvwprintw(borderwin, 17, 32, "\\");
            mvwprintw(borderwin, 19, 32, "                  _");
            wrefresh(borderwin);
            delay /= root;
            usleep(delay);
            mvwprintw(borderwin, 15, 32, "  _                                   _                      _");
            mvwprintw(borderwin, 17, 32, "\\");
            mvwprintw(borderwin, 19, 32, "                  _");
            wrefresh(borderwin);
            delay /= root;
            usleep(delay);
            mvwprintw(borderwin, 15, 32, "  _                                   _                      _");
            mvwprintw(borderwin, 17, 32, "\\");
            mvwprintw(borderwin, 18, 32, "                                  \\");
            mvwprintw(borderwin, 19, 32, "                  _");
            wrefresh(borderwin);
            delay /= root;
            usleep(delay);
            mvwprintw(borderwin, 15, 32, "  _             _                     _                      _");
            mvwprintw(borderwin, 17, 32, "\\");
            mvwprintw(borderwin, 18, 32, "                                  \\             _");
            mvwprintw(borderwin, 19, 32, "                  _                     _");
            wrefresh(borderwin);
            delay /= root;
            usleep(delay);
            mvwprintw(borderwin, 15, 32, "  _             _                     _                      _");
            mvwprintw(borderwin, 17, 32, "\\");
            mvwprintw(borderwin, 18, 32, "            \\                 -   \\             _");
            mvwprintw(borderwin, 19, 32, "                  _                     _                     _");
            wrefresh(borderwin);
            delay /= root;
            usleep(delay);
            mvwprintw(borderwin, 15, 32, "  _             _                     _      _               _");
            mvwprintw(borderwin, 16, 32, "                       \\");
            mvwprintw(borderwin, 17, 32, "\\                                                      \\");
            mvwprintw(borderwin, 18, 32, "        _   \\                 -   \\             _");
            mvwprintw(borderwin, 19, 32, "                  _          _          _                     _");
            wrefresh(borderwin);
            delay /= root;
            usleep(delay);
            mvwprintw(borderwin, 15, 32, "  _             _          _          _      _               _");
            mvwprintw(borderwin, 16, 32, "     _                 \\");
            mvwprintw(borderwin, 17, 32, "\\               _                                      \\");
            mvwprintw(borderwin, 18, 32, "        _   \\                 -   \\             _");
            mvwprintw(borderwin, 19, 32, "                  _          _          _     \\               _");
            wrefresh(borderwin);
            root = 1.07;
            delay /= root;
            usleep(delay);
            mvwprintw(borderwin, 15, 32, "  _             _          _          _      _               _");
            mvwprintw(borderwin, 16, 32, "     _            \\    \\");
            mvwprintw(borderwin, 17, 32, "\\               _                           \\          \\");
            mvwprintw(borderwin, 18, 32, "        _   \\                 -   \\             _");
            mvwprintw(borderwin, 19, 32, "        _         _     \\    _          _     \\            _  _");
            wrefresh(borderwin);
            delay /= root;
            usleep(delay);
            mvwprintw(borderwin, 15, 32, "  _             _        _ _          _      _               _");
            mvwprintw(borderwin, 16, 32, "     _            \\    \\");
            mvwprintw(borderwin, 17, 32, "\\               _                           \\          \\");
            mvwprintw(borderwin, 18, 32, "        _   \\                 -   \\             _   _");
            mvwprintw(borderwin, 19, 32, "        _         _     \\    _       _  _     \\            _  _");
            wrefresh(borderwin);
            delay /= root;
            usleep(delay);
            mvwprintw(borderwin, 15, 32, "  _             _        _ _          _      _          _    _");
            mvwprintw(borderwin, 16, 32, "     _     /      \\    \\");
            mvwprintw(borderwin, 17, 32, "\\               _                        \\  \\          \\");
            mvwprintw(borderwin, 18, 32, "  /     _   \\                 -   \\             _   _");
            mvwprintw(borderwin, 19, 32, "        _         _     \\    _       _  _     \\            _  _");
            wrefresh(borderwin);
            delay /= root;
            usleep(delay);
            mvwprintw(borderwin, 15, 32, "  _             _        _ _      _   _      _          _    _");
            mvwprintw(borderwin, 16, 32, "     _     /      \\    \\");
            mvwprintw(borderwin, 17, 32, "\\               _                        \\  \\          \\       \\");
            mvwprintw(borderwin, 18, 32, "  /     _   \\                 -   \\    _        _   _");
            mvwprintw(borderwin, 19, 32, "        _       _ _     \\    _       _  _     \\            _  _");
            wrefresh(borderwin);
            delay /= root;
            usleep(delay);
            mvwprintw(borderwin, 15, 32, "  _         _   _        _ _      _   _      _          _    _");
            mvwprintw(borderwin, 16, 32, "     _     /      \\    \\                          /");
            mvwprintw(borderwin, 17, 32, "\\               _             \\          \\  \\          \\       \\");
            mvwprintw(borderwin, 18, 32, "  /     _   \\                 -   \\    _        _   _");
            mvwprintw(borderwin, 19, 32, "     _  _       _ _     \\  _ _       _  _     \\            _  _");
            wrefresh(borderwin);
            delay /= root;
            usleep(delay);
            mvwprintw(borderwin, 15, 32, "  _         _   _        _ _      _   _      _          _    _");
            mvwprintw(borderwin, 16, 32, "     _     /      \\    \\                        / /");
            mvwprintw(borderwin, 17, 32, "\\    _          _     \\       \\          \\  \\          \\       \\");
            mvwprintw(borderwin, 18, 32, "  /     _   \\    _            -   \\    _        _   _");
            mvwprintw(borderwin, 19, 32, "     _  _       _ _     \\  _ _       _  _     \\            _  _");
            wrefresh(borderwin);
            delay /= root;
            usleep(delay);
            mvwprintw(borderwin, 15, 32, "  _         _   _        _ _      _   _      _          __   _");
            mvwprintw(borderwin, 16, 32, "     _ \\   /      \\    \\    -                   / /");
            mvwprintw(borderwin, 17, 32, "\\    _          _     \\       \\          \\  \\          \\       \\");
            mvwprintw(borderwin, 18, 32, "  / _   _   \\    _            -   \\    _        _   _      _");
            mvwprintw(borderwin, 19, 32, "     _  _       _ _     \\  _ _       _  _ /   \\            _  _");
            wrefresh(borderwin);
            delay /= root;
            usleep(delay);
            mvwprintw(borderwin, 15, 32, "  _         _   _        _ _      _   _      _          __   _");
            mvwprintw(borderwin, 16, 32, "     _ \\   /      \\    \\    -     \\             / /");
            mvwprintw(borderwin, 17, 32, "\\    _          _     \\       \\          \\  \\    _     \\       \\");
            mvwprintw(borderwin, 18, 32, "  / _   _   \\    _       \\    -   \\    _        _   _      _");
            mvwprintw(borderwin, 19, 32, "     _  _     / _ _     \\  _ _       _  _ /   \\          \\ _  _");
            wrefresh(borderwin);
            delay /= root;
            usleep(delay);
            mvwprintw(borderwin, 15, 32, "  _  _      _   __       _ _      _   _      _          __   _");
            mvwprintw(borderwin, 16, 32, "     _ \\   /      \\    \\   _-     \\             / /");
            mvwprintw(borderwin, 17, 32, "\\    _          _     \\       \\          \\  \\    _     \\       \\");
            mvwprintw(borderwin, 18, 32, "  / _   _   \\    __      \\    -   \\    _        _   _      _");
            mvwprintw(borderwin, 19, 32, "     _  _     / _ _     \\  _ _      /_  _ /   \\          \\ _  _");
            wrefresh(borderwin);
            delay /= root;
            usleep(delay);
            mvwprintw(borderwin, 15, 32, "  _  _      _   __       _ _      _   _      _          __   _");
            mvwprintw(borderwin, 16, 32, "     _ \\   /      \\    \\   _-     \\   _         / /");
            mvwprintw(borderwin, 17, 32, "\\  _ _          _     \\       \\          \\  \\    _     \\       \\");
            mvwprintw(borderwin, 18, 32, "  / _   _   \\    __      \\    -   \\    _        _   _      _");
            mvwprintw(borderwin, 19, 32, "     _  _     /__ _     \\  _ _      /_  _ /   \\      /   \\ _  _");
            wrefresh(borderwin);
            delay /= root;
            usleep(delay);
            mvwprintw(borderwin, 15, 32, "  _  _      _   __       _ _      _ _ _      _          __   _");
            mvwprintw(borderwin, 16, 32, "     _ \\   /      \\    \\   _-     \\   _         / /");
            mvwprintw(borderwin, 17, 32, "\\  _ _          _  \\  \\       \\          \\  \\    _     \\    _  \\");
            mvwprintw(borderwin, 18, 32, "  / _   _   \\    __      \\    -   \\    _        _   _      _");
            mvwprintw(borderwin, 19, 32, "     _  _     /__ _     \\  _ _      /_  _ /   \\   \\  /   \\ _  _");
            wrefresh(borderwin);
            delay /= root;
            usleep(delay);
            mvwprintw(borderwin, 15, 32, "  _  _      _   __       _ _      _ _ __     _          __   _");
            mvwprintw(borderwin, 16, 32, "     _ \\   /      \\    \\   _-.    \\   _     /   / /");
            mvwprintw(borderwin, 17, 32, "\\  _ _          _  \\  \\       \\          \\  \\    _     \\    _  \\");
            mvwprintw(borderwin, 18, 32, "  / _   _   \\    __      \\   _-   \\    __       _   _      _");
            mvwprintw(borderwin, 19, 32, "     _  _/    /__ _     \\  _ _      /_  _ /   \\   \\  /   \\ _  _");
            wrefresh(borderwin);
            delay /= root;
            usleep(delay);
            mvwprintw(borderwin, 15, 32, "  _  _      _   __     _ _ _      _ _ __     _          __   _");
            mvwprintw(borderwin, 16, 32, "     __\\   /      \\    \\   _-.    \\   _     /   / /");
            mvwprintw(borderwin, 17, 32, "\\  _ _          _  \\  \\       \\      \\   \\  \\    _ -   \\    _  \\");
            mvwprintw(borderwin, 18, 32, "  / _   _   \\    __      \\   _-   \\    __       _   _      _");
            mvwprintw(borderwin, 19, 32, "     _  _/    /__ __    \\  _ _      /_  _ /   \\   \\  /   \\ _  _");
            wrefresh(borderwin);
            delay /= root;
            usleep(delay);
            mvwprintw(borderwin, 15, 32, "  _  _      _   __     _ _ _      _ _ __     _   _      __   _");
            mvwprintw(borderwin, 16, 32, "     __\\   /\\     \\    \\   _-.   /\\   _     /   / /");
            mvwprintw(borderwin, 17, 32, "\\  _ _          _  \\  \\       \\      \\   \\  \\    _ -   \\    _  \\");
            mvwprintw(borderwin, 18, 32, "  / __  _   \\    __      \\   _-   \\    __       _   _      _");
            mvwprintw(borderwin, 19, 32, "     _  _/    /__ __    \\ __ _      /_  _ /   \\   \\  /   \\/_  _");
            wrefresh(borderwin);
            delay /= root;
            usleep(delay);
            mvwprintw(borderwin, 15, 32, "  _  _      _   __     _ _ _      _ _ __     _   _      __   _");
            mvwprintw(borderwin, 16, 32, "     __\\   /\\     \\    \\  __-.   /\\   _     /   / /");
            mvwprintw(borderwin, 17, 32, "\\  _ _          _  \\  \\     \\ \\      \\   \\  \\    _ -.  \\    _  \\");
            mvwprintw(borderwin, 18, 32, "  / __  _   \\    __      \\   _-   \\ \\  __       _   _      _");
            mvwprintw(borderwin, 19, 32, "     __ _/    /__ __    \\ __ _      /_  _ /   \\   \\  /   \\/_  _");
            wrefresh(borderwin);
            delay /= root;
            usleep(delay);
            mvwprintw(borderwin, 15, 32, "  _  _      _   __     _ _ _      _ _ __     _   _      __   _");
            mvwprintw(borderwin, 16, 32, "     __\\   /\\     \\   /\\  __-.   /\\   _     /   / /       \\");
            mvwprintw(borderwin, 17, 32, "\\ \\_ _          _  \\  \\    /\\ \\      \\ \\ \\  \\    _ -.  \\    _  \\");
            mvwprintw(borderwin, 18, 32, "  / __  _   \\ \\  __      \\   _-   \\ \\  __       _   _      _");
            mvwprintw(borderwin, 19, 32, "     __ _/    /__ __    \\ __ _      /_  _ /   \\   \\ _/   \\/_  _");
            wrefresh(borderwin);
            delay /= root;
            usleep(delay);
            mvwprintw(borderwin, 15, 32, "  _  _      _   __     _ ___      ___ __     _   _      __   _");
            mvwprintw(borderwin, 16, 32, "     __\\   /\\     \\   /\\  __-.   /\\   _     /   / /       \\");
            mvwprintw(borderwin, 17, 32, "\\ \\_ _     \\    _  \\  \\    /\\ \\      \\ \\ \\  \\    _ -.  \\    _  \\");
            mvwprintw(borderwin, 18, 32, "  / __  _   \\ \\  __      \\   _-   \\ \\  __      \\_   _   \\  _  _");
            mvwprintw(borderwin, 19, 32, "   / __ _/    /__ __    \\ __ _      /_  __/   \\   \\/_/   \\/_  _ /");
            wrefresh(borderwin);
            delay /= root;
            usleep(delay);
            mvwprintw(borderwin, 15, 32, "  _  _      _   __     _ ___      ___ __     _   _      __   _");
            mvwprintw(borderwin, 16, 32, "     __\\   /\\   \\ \\   /\\  __-.   /\\   _     /   / /       \\ \\ \\");
            mvwprintw(borderwin, 17, 32, "\\ \\_ _     \\    _  \\  \\    /\\ \\      \\ \\ \\  \\    _ -.  \\ \\  _  \\");
            mvwprintw(borderwin, 18, 32, "  / __ __   \\ \\  __      \\  __-   \\ \\  __      \\_   _   \\  _  _");
            mvwprintw(borderwin, 19, 32, "   / __ _/    /__ __/   \\ __ _      /_  __/   \\  /\\/_/   \\/_  _ /");
            wrefresh(borderwin);
            delay /= root;
            usleep(delay);
            mvwprintw(borderwin, 15, 32, "  _  _      _   __     _ ___      ___ __     _   _      __  __");
            mvwprintw(borderwin, 16, 32, "     __\\   /\\ \\ \\ \\   /\\  __-.   /\\   _     /   / /       \\ \\ \\");
            mvwprintw(borderwin, 17, 32, "\\ \\_ _     \\    _  \\  \\    /\\ \\  \\   \\ \\ \\  \\    _ -.  \\ \\  _  \\");
            mvwprintw(borderwin, 18, 32, "  /\\__ __   \\ \\  __ \\    \\  __-   \\ \\  __      \\_   _\\  \\  _  _");
            mvwprintw(borderwin, 19, 32, "   / __ _/   \\/__ __/   \\ __ _/     /_  __/   \\  /\\/_/   \\/__ _ /");
            wrefresh(borderwin);
            delay /= root;
            usleep(delay);
            mvwprintw(borderwin, 15, 32, "  _  _      _   __     _ ___      ______     _   _      __  __");
            mvwprintw(borderwin, 16, 32, " \\   __\\   /\\ \\ \\ \\   /\\  __-.   /\\   _     /  \\/ /       \\ \\ \\");
            mvwprintw(borderwin, 17, 32, "\\ \\___     \\   \\_  \\  \\    /\\ \\  \\   \\ \\ \\  \\    _\"-.  \\ \\  _  \\");
            mvwprintw(borderwin, 18, 32, "  /\\__ __   \\ \\  __ \\    \\  __-   \\ \\ ___ \\    \\_\\  _\\  \\  _  __");
            mvwprintw(borderwin, 19, 32, "   / __ _/   \\/__ __/   \\/____/     /__ __/   \\  /\\/_/   \\/__ _ /");
            wrefresh(borderwin);
            delay /= root;
            usleep(delay);
            mvwprintw(borderwin, 15, 32, "  ____      _   __     _ ___      ______     __  _      __  __");
            mvwprintw(borderwin, 16, 32, " \\  ___\\   /\\ \\ \\ \\   /\\  __-.   /\\   _     /  \\/ /    /  \\ \\ \\");
            mvwprintw(borderwin, 17, 32, "\\ \\___     \\   \\_\\ \\  \\ \\  /\\ \\  \\   \\ \\ \\  \\    _\"-.  \\ \\  _\\ \\");
            mvwprintw(borderwin, 18, 32, "  /\\_____   \\ \\  __ \\    \\  __-   \\ \\ ____\\  \\ \\_\\  _\\  \\  _  __");
            mvwprintw(borderwin, 19, 32, "   / __ _/   \\/__ __/   \\/____/    \\/__ __/   \\  /\\/_/   \\/__ _ /");
            wrefresh(borderwin);
            delay /= root;
            usleep(delay);
            mvwprintw(borderwin, 15, 32, "  _____     _   __     _____      ______     __  __     __  __");
            mvwprintw(borderwin, 16, 32, "/\\  ___\\   /\\ \\ \\ \\   /\\  __-.   /\\  __     /\\ \\/ /    /  \\ \\ \\");
            mvwprintw(borderwin, 17, 32, "\\ \\___     \\   \\_\\ \\  \\ \\  /\\ \\  \\   \\/\\ \\  \\    _\"-.  \\ \\ \\_\\ \\");
            mvwprintw(borderwin, 18, 32, "  /\\_____   \\ \\ ____\\  \\ \\  __-   \\ \\ ____\\  \\ \\_\\  _\\  \\  _  __");
            mvwprintw(borderwin, 19, 32, "  \\/ __ _/   \\/__ __/   \\/____/    \\/_____/   \\  /\\/_/   \\/____ /");
            wrefresh(borderwin);
            delay /= root;
            usleep(delay);
            mvwprintw(borderwin, 15, 32, " ______     __  __     _____      ______     __  __     __  __");
            mvwprintw(borderwin, 16, 32, "/\\  ___\\   /\\ \\/\\ \\   /\\  __-.   /\\  __ \\   /\\ \\/ /    /\\ \\/\\ \\");
            mvwprintw(borderwin, 17, 32, "\\ \\___  \\  \\ \\ \\_\\ \\  \\ \\ \\/\\ \\  \\ \\ \\/\\ \\  \\ \\  _\"-.  \\ \\ \\_\\ \\");
            mvwprintw(borderwin, 18, 32, " \\/\\_____\\  \\ \\_____\\  \\ \\____-   \\ \\_____\\  \\ \\_\\ \\_\\  \\ \\_____\\");
            mvwprintw(borderwin, 19, 32, "  \\/_____/   \\/_____/   \\/____/    \\/_____/   \\/_/\\/_/   \\/_____/");
            wrefresh(borderwin);
            usleep(3000000);
            wclear(borderwin);
            box(borderwin, 0, 0);
            wrefresh(borderwin);
        }
        while(1) { //home
            
    //Home menu: LOGO, PLAY, SETTINGS(readjust win, clear record, info)
    //==============================================================================================
            mvwprintw(borderwin, 7, 40, "╔══════════════════════════════════════════════╗");
            mvwprintw(borderwin, 8, 40, "║          ___          _     _                ║");
            mvwprintw(borderwin, 9, 40, "║         / __|_  _  __| |___| |___  _         ║");
            mvwprintw(borderwin, 10, 40, "║         \\__ \\ || |/ _` | _ \\ / / || |        ║");
            mvwprintw(borderwin, 11, 40, "║         |___/\\_,_|\\__,_|___/_\\_\\\\_,_|        ║");
            mvwprintw(borderwin, 12, 40, "║                                              ║");
            mvwprintw(borderwin, 13, 40, "╚══════════════════════════════════════════════╝");
            
            startgame = false;
            int homeselec = 0;
            string record[9];
            keypad(borderwin, true);
            while(1) {
                for(int i = 0; i < 3; i++) {
                    if (i == homeselec) //start text atrribute
                        wattron(borderwin, A_REVERSE); //cast text atrribute
                    switch(i) {
                        case 0:
                            mvwprintw(borderwin, 15, 47, " ╔═════════════════════════════╗ ");
                            mvwprintw(borderwin, 16, 47, " ║         PLAY SUDOKU         ║ ");
                            mvwprintw(borderwin, 17, 47, " ╚═════════════════════════════╝ ");
                            break;
                        case 1:
                            mvwprintw(borderwin, 19, 47, " ╔═════════════════════════════╗ ");
                            mvwprintw(borderwin, 20, 47, " ║        GAME SETTINGS        ║ ");
                            mvwprintw(borderwin, 21, 47, " ╚═════════════════════════════╝ ");
                            break;
                        case 2:
                            mvwprintw(borderwin, 23, 47, " ╔═════════════════════════════╗ ");
                            mvwprintw(borderwin, 24, 47, " ║          QUIT GAME          ║ ");
                            mvwprintw(borderwin, 25, 47, " ╚═════════════════════════════╝ ");
                            break;
                    }
                    wattroff(borderwin, A_REVERSE); //end text atrribute
                } //print options
                
                key = wgetch(borderwin); //get movement
                switch(key) {
                    case KEY_UP:
                        homeselec--;
                        if(homeselec == -1) //end restriction
                            homeselec = 0;
                        break;
                    case KEY_DOWN:
                        homeselec++;
                        if(homeselec == 3)
                            homeselec = 2;
                        break;
                    default:
                        break;
                } //movement
                if(key == 10)
                    break; //if enter is presses
            }
            if(homeselec == 0) {
                wclear(borderwin);
                box(borderwin, 0, 0);
                wrefresh(borderwin);
                startgame = true;
            } //start game
            else if(homeselec == 1) {
                bool breakhome = false;
                int settingselect = 0;
                while(1) { //setting
                    
    //Settings
    //==============================================================================================
                    wclear(borderwin);
                    box(borderwin, 0, 0);
                    wrefresh(borderwin);
                    while(1) {
                        for(int i = 0; i < 5; i++) {
                            if (i == settingselect) //start text atrribute
                                wattron(borderwin, A_REVERSE); //cast text atrribute
                            switch(i) {
                                case 0:
                                    mvwprintw(borderwin, 8, 47, " ╔═════════════════════════════╗ ");
                                    mvwprintw(borderwin, 9, 47, " ║       Readjust Window       ║ ");
                                    mvwprintw(borderwin, 10, 47, " ╚═════════════════════════════╝ ");
                                    break;
                                case 1:
                                    mvwprintw(borderwin, 12, 47, " ╔═════════════════════════════╗ ");
                                    if(multi_off)
                                        mvwprintw(borderwin, 13, 47, " ║  Exclude Multiple Ans ║ OFF ║ ");
                                    else
                                        mvwprintw(borderwin, 13, 47, " ║  Exclude Multiple Ans ║ ON  ║ ");
                                    mvwprintw(borderwin, 14, 47, " ╚═════════════════════════════╝ ");
                                    break;
                                case 2:
                                    mvwprintw(borderwin, 16, 47, " ╔═════════════════════════════╗ ");
                                    mvwprintw(borderwin, 17, 47, " ║      Clear Game Record      ║ ");
                                    mvwprintw(borderwin, 18, 47, " ╚═════════════════════════════╝ ");
                                    break;
                                case 3:
                                    mvwprintw(borderwin, 20, 47, " ╔═════════════════════════════╗ ");
                                    mvwprintw(borderwin, 21, 47, " ║            About            ║ ");
                                    mvwprintw(borderwin, 22, 47, " ╚═════════════════════════════╝ ");
                                    break;
                                case 4:
                                    mvwprintw(borderwin, 24, 47, " ╔═════════════════════════════╗ ");
                                    mvwprintw(borderwin, 25, 47, " ║        Exit Settings        ║ ");
                                    mvwprintw(borderwin, 26, 47, " ╚═════════════════════════════╝ ");
                                    break;
                            }
                            wattroff(borderwin, A_REVERSE); //end text atrribute
                        } //print options
                        
                        key = wgetch(borderwin); //get movement
                        switch(key) {
                            case KEY_UP:
                                settingselect--;
                                if(settingselect == -1) //end restriction
                                    settingselect = 0;
                                break;
                            case KEY_DOWN:
                                settingselect++;
                                if(settingselect == 5)
                                    settingselect = 4;
                                break;
                            default:
                                break;
                        } //movement
                        if(key == 10)
                            break; //if enter is presses
                    }
                    if(settingselect == 0) {
                        wclear(borderwin);
                        wrefresh(borderwin);
                        delwin(borderwin);
                        refresh();
                        breakhome = true;
                        readjust = true;
                        break; //break setting while loop
                    }
                    else if(settingselect == 1) {
                        if(multi_off) {
                            WINDOW * confirmwin = newwin(15, 77, (yMax-15)/2, (xMax-77)/2);
                            refresh();
                            box(confirmwin, 0, 0);
                            wattron(confirmwin, A_REVERSE);
                            mvwprintw(confirmwin, 0, 0, "                                                                             ");
                            wattroff(confirmwin, A_REVERSE);
                            mvwprintw(confirmwin, 2, 34, "WARNNING!");
                            mvwprintw(confirmwin, 4, 6, "Turning on this feature will significantly increase loading time,"); //65
                            mvwprintw(confirmwin, 5, 6, "and MAY CAUSE THE GAME TO FREEZE (higher risk under 16 by 16 norm"); //65
                            mvwprintw(confirmwin, 6, 7, "and hard). You would have to force quit the program if the game"); //63
                            mvwprintw(confirmwin, 7, 5, "stuck in Loading windows. Personal computer usually takes 15sec to "); //67
                            mvwprintw(confirmwin, 8, 26, "2min to finish the task."); //24
                            mvwprintw(confirmwin, 10, 16, "Are you sure you want to enable this feature?");
                            mvwprintw(confirmwin, 13, 21, "esc to Cancel     return to Confirm");
                            wrefresh(confirmwin);
                            bool confirm = false;
                            while(1) {
                                key = wgetch(confirmwin);
                                if(key == 27)
                                    break;
                                else if(key == 10)
                                    confirm = true;
                                break;
                            }
                            if(confirm)
                                multi_off = false;
                            else
                                multi_off = true;
                            wclear(confirmwin);
                            wrefresh(confirmwin);
                            delwin(confirmwin);
                            refresh();
                        }
                        else
                            multi_off = true;
                    }
                    else if(settingselect == 2) {
                        //Clear record
                        WINDOW * confirmwin = newwin(9, 41, (yMax-9)/2, (xMax-41)/2);
                        refresh();
                        box(confirmwin, 0, 0);
                        wattron(confirmwin, A_REVERSE);
                        mvwprintw(confirmwin, 0, 0, "                                         ");
                        wattroff(confirmwin, A_REVERSE);
                        mvwprintw(confirmwin, 2, 3, "Are you sure you want to delete all");
                        mvwprintw(confirmwin, 3, 11, "record permanently?");
                        mvwprintw(confirmwin, 4, 7, "You can't undo this action.");
                        mvwprintw(confirmwin, 7, 3, "esc to Cancel     return to Confirm");
                        wrefresh(confirmwin);
                        bool confirm = false;
                        while(1) {
                            key = wgetch(confirmwin);
                            if(key == 27)
                                break;
                            else if(key == 10)
                                confirm = true;
                            break;
                        }
                        if(confirm) {
                            ofstream recordfile_clear;
                            recordfile_clear.open("/Users/bingyuli/Documents/CPP/Sudoku/Sudoku/record.dat");
                            if (recordfile_clear.fail()) {
                                cout << "Failed to open and output record file.";
                                exit(1);
                            }
                            for(int i = 0; i < 9; i++) {
                                record[i] = "00 : 00 : 00";
                                recordfile_clear << "00 : 00 : 00\n";
                            }
                            recordfile_clear.close();
                            confirm = false;
                        }
                        wclear(confirmwin);
                        wrefresh(confirmwin);
                        delwin(confirmwin);
                        refresh();
                    } //clear record
                    else if(settingselect == 3) {
                        wclear(borderwin);
                        box(borderwin, 0, 0);
                        wrefresh(borderwin);
                        mvwprintw(borderwin, 14, 29, "This is the final project of \"Introduction to Computers and Programming\".");
                        mvwprintw(borderwin, 16, 46, "Designed and coded by 游婕 and 李秉諭.");
                        mvwprintw(borderwin, 20, 47, "Copyright 2024, All rights reserved");
                        mvwprintw(borderwin, 28, 53, "Press any key to return");
                        wrefresh(borderwin);
                        getch();
                    } //info
                    else if(settingselect == 4) {
                        wclear(borderwin);
                        box(borderwin, 0, 0);
                        wrefresh(borderwin);
                        break; //break setting while loop
                    }
                    else {
                        cout << "settings selection error!!\n\n";
                        exit(1);
                    }
                } //setting while loop
                if(breakhome) {
                    breakhome = false;
                    break;
                }
            } //settings else if
            else if(homeselec == 2){
                clear();
                refresh();
                endwin();
                return 0;
            } //quit game
            else {
                cout << "home selection error!!\n\n";
                exit(1);
            }
    
    //GAME
    //XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
            if(startgame) { //game
                
    //Get Record
    //==============================================================================================
                ifstream recordfile_in;
                recordfile_in.open("/Users/bingyuli/Documents/CPP/Sudoku/Sudoku/record.dat");
                if (recordfile_in.fail()) {
                    ofstream recordfile_clear;
                    recordfile_clear.open("/Users/bingyuli/Documents/CPP/Sudoku/Sudoku/record.dat");
                    if (recordfile_clear.fail()) {
                        cout << "Failed to create record file.";
                        exit(1);
                    }
                    for(int i = 0; i < 9; i++) {
                        record[i] = "00 : 00 : 00";
                        recordfile_clear << "00 : 00 : 00\n";
                    }
                    recordfile_clear.close();
                    recordfile_in.open("/Users/bingyuli/Documents/CPP/Sudoku/Sudoku/record.dat");
                }
                for(int i = 0; i < 9; i++) {
                    getline(recordfile_in, record[i]);
                }
                recordfile_in.close();
                
    //Mode menu
    //==============================================================================================
                int mode_selected = 0;
                while(1) {
                    for(int i = 0; i < 3; i++) {
                        if (i == mode_selected)
                            wattron(borderwin, A_REVERSE);
                        switch(i) {
                            case 0:
                                mvwprintw(borderwin, 4, 44, " ╔═══════════════════════════════════╗ ");
                                mvwprintw(borderwin, 5, 44, " ║      _ _    _           _ _       ║ ");
                                mvwprintw(borderwin, 6, 44, " ║     | | |  | |__ _  _  | | |      ║ ");
                                mvwprintw(borderwin, 7, 44, " ║     |_  _| | '_ \\ || | |_  _|     ║ ");
                                mvwprintw(borderwin, 8, 44, " ║       |_|  |_.__/\\_, |   |_|      ║ ");
                                mvwprintw(borderwin, 9, 44,  " ║                  |__/             ║ ");
                                mvwprintw(borderwin, 10, 44, " ╚═══════════════════════════════════╝ ");
                                break;
                            case 1:
                                mvwprintw(borderwin, 14, 44, " ╔═══════════════════════════════════╗ ");
                                mvwprintw(borderwin, 15, 44, " ║       ___   _           ___       ║ ");
                                mvwprintw(borderwin, 16, 44, " ║      / _ \\ | |__ _  _  / _ \\      ║ ");
                                mvwprintw(borderwin, 17, 44, " ║      \\_, / | '_ \\ || | \\_, /      ║ ");
                                mvwprintw(borderwin, 18, 44, " ║       /_/  |_.__/\\_, |  /_/       ║ ");
                                mvwprintw(borderwin, 19, 44, " ║                  |__/             ║ ");
                                mvwprintw(borderwin, 20, 44, " ╚═══════════════════════════════════╝ ");
                                break;
                            case 2:
                                mvwprintw(borderwin, 24, 44, " ╔═══════════════════════════════════╗ ");
                                mvwprintw(borderwin, 25, 44, " ║     _  __   _           _  __     ║ ");
                                mvwprintw(borderwin, 26, 44, " ║    / |/ /  | |__ _  _  / |/ /     ║ ");
                                mvwprintw(borderwin, 27, 44, " ║    | / _ \\ | '_ \\ || | | / _ \\    ║ ");
                                mvwprintw(borderwin, 28, 44, " ║    |_\\___/ |_.__/\\_, | |_\\___/    ║ ");
                                mvwprintw(borderwin, 29, 44, " ║                  |__/             ║ ");
                                mvwprintw(borderwin, 30, 44, " ╚═══════════════════════════════════╝ ");
                                break;
                        }
                        wattroff(borderwin, A_REVERSE);
                    }
                    
                    key = wgetch(borderwin);
                    switch(key) {
                        case KEY_UP:
                            mode_selected--;
                            if(mode_selected == -1)
                                mode_selected = 0;
                            break;
                        case KEY_DOWN:
                            mode_selected++;
                            if(mode_selected == 3)
                                mode_selected = 2;
                            break;
                        default:
                            break;
                    }
                    if(key == 10)
                        break;
                }
                
    //Difficulty menu
    //==============================================================================================
                int dif_selected = 0;
                while(1) {
                    for(int i = 0; i < 3; i++) {
                        if (i == dif_selected)
                            wattron(borderwin, A_REVERSE);
                        switch(i) {
                            case 0:
                                mvwprintw(borderwin, 4, 44, " ╔═══════════════════════════════════╗ ");
                                mvwprintw(borderwin, 5, 44, " ║        ___   _   _____   __       ║ ");
                                mvwprintw(borderwin, 6, 44, " ║       | __| /_\\ / __\\ \\ / /       ║ ");
                                mvwprintw(borderwin, 7, 44, " ║       | _| / _ \\\\__ \\\\ V /        ║ ");
                                mvwprintw(borderwin, 8, 44, " ║       |___|_/ \\_\\___/ |_|         ║ ");
                                mvwprintw(borderwin, 9, 44, " ║      ═══════════════════════      ║ ");
                                mvwprintw(borderwin, 10, 44, " ╚═══════════════════════════════════╝ ");
                                break;
                            case 1:
                                mvwprintw(borderwin, 14, 44, " ╔═══════════════════════════════════╗ ");
                                mvwprintw(borderwin, 15, 44, " ║       _  _  ___  ___ __  __       ║ ");
                                mvwprintw(borderwin, 16, 44, " ║      | \\| |/ _ \\| _ \\  \\/  |      ║ ");
                                mvwprintw(borderwin, 17, 44, " ║      | .` | (_) |   / |\\/| |      ║ ");
                                mvwprintw(borderwin, 18, 44, " ║      |_|\\_|\\___/|_|_\\_|  |_|      ║ ");
                                mvwprintw(borderwin, 19, 44, " ║      ═══════════════════════      ║ ");
                                mvwprintw(borderwin, 20, 44, " ╚═══════════════════════════════════╝ ");
                                break;
                            case 2:
                                mvwprintw(borderwin, 24, 44, " ╔═══════════════════════════════════╗ ");
                                mvwprintw(borderwin, 25, 44, " ║        _  _   _   ___  ___        ║ ");
                                mvwprintw(borderwin, 26, 44, " ║       | || | /_\\ | _ \\|   \\       ║ ");
                                mvwprintw(borderwin, 27, 44, " ║       | __ |/ _ \\|   /| |) |      ║ ");
                                mvwprintw(borderwin, 28, 44, " ║       |_||_/_/ \\_\\_|_\\|___/       ║ ");
                                mvwprintw(borderwin, 29, 44, " ║      ═══════════════════════      ║ ");
                                mvwprintw(borderwin, 30, 44, " ╚═══════════════════════════════════╝ ");
                                break;
                        }
                        wattroff(borderwin, A_REVERSE);
                    }
                    
                    key = wgetch(borderwin);
                    switch(key) {
                        case KEY_UP:
                            dif_selected--;
                            if(dif_selected == -1)
                                dif_selected = 0;
                            break;
                        case KEY_DOWN:
                            dif_selected++;
                            if(dif_selected == 3)
                                dif_selected = 2;
                            break;
                        default:
                            break;
                    }
                    if(key == 10)
                        break;
                }
                
    //Loading window & get matrix
    //==============================================================================================
                wclear(borderwin);
                box(borderwin, 0, 0);
                mvwprintw(borderwin, 15, 27, " __        ______    _____     __    __   __    ______");
                mvwprintw(borderwin, 16, 27, "/\\ \\      /\\  __ \\  /\\  __-.  /\\ \\  /\\ \"-.\\ \\  /\\  ___\\");
                mvwprintw(borderwin, 17, 27, "\\ \\ \\____ \\ \\ \\/\\ \\ \\ \\ \\/\\ \\ \\ \\ \\ \\ \\ \\-.  \\ \\ \\ \\__ \\    __    __    __");
                mvwprintw(borderwin, 18, 27, " \\ \\_____\\ \\ \\_____\\ \\ \\____-  \\ \\_\\ \\ \\_\\\\\"\\_\\ \\ \\_____\\  /\\_\\  /\\_\\  /\\_\\");
                mvwprintw(borderwin, 19, 27, "  \\/_____/  \\/_____/  \\/____/   \\/_/  \\/_/ \\/_/  \\/_____/  \\/_/  \\/_/  \\/_/");
                wrefresh(borderwin);
                dif = dif_selected;
                switch(mode_selected){
                    case 0:
                        mat = 4;
                        break;
                    case 1:
                        mat = 9;
                        break;
                    case 2:
                        mat = 16;
                        break;
                    default:
                        cout << "mode selection error!!\n\n";
                        exit(1);
                }
                get_matrix();

    //Game, text, time, record, guide menu initialize
    //==============================================================================================
                WINDOW * gamewin;
                switch(mode_selected){
                    case 0:
                        gamewin = newwin(9, 17, (yMax-35)/2+14, (xMax-129)/2+25);
                        _4by4_grid(gamewin, 0, 0);
                        break;
                    case 1:
                        gamewin = newwin(19, 37, (yMax-35)/2+8, (xMax-129)/2+15);
                        _9by9_grid(gamewin, 0, 0);
                        break;
                    case 2:
                        gamewin = newwin(33, 65, (yMax-35)/2+1, (xMax-129)/2+1);
                        _16by16_grid(gamewin, 0, 0);
                        break;
                    default:
                        cout << "mode selection error!!\n\n";
                        exit(1);
                } //grid position and printing
                string modetext;
                switch(dif_selected) {
                    case 0:
                        modetext = "EASY";
                        break;
                    case 1:
                        modetext = "NORMAL";
                        break;
                    case 2:
                        modetext = "HARD";
                        break;
                    default:
                        cout << "dificulty selection error!!\n\n";
                        exit(1);
                } //dificulty displayed on the right
                int recordline = mode_selected*3+dif_selected;
                WINDOW * textwin = newwin(33, 46, (yMax-35)/2+1, (xMax-129)/2+74);
                mvwprintw(textwin, 0, 0, "╔════════════════════════════════════════════╗");
                mvwprintw(textwin, 1, 0, "║         ___          _     _               ║");
                mvwprintw(textwin, 2, 0, "║        / __|_  _  __| |___| |___  _        ║");
                mvwprintw(textwin, 3, 0, "║        \\__ \\ || |/ _` | _ \\ / / || |       ║");
                mvwprintw(textwin, 4, 0, "║        |___/\\_,_|\\__,_|___/_\\_\\\\_,_|       ║");
                mvwprintw(textwin, 5, 0, "║                                            ║");
                mvwprintw(textwin, 6, 0, "╚════════════════════════════════════════════╝");
                mvwprintw(textwin, 8, 0, "MODE : %s", modetext.c_str());
                mvwprintw(textwin, 14, 20, "RECORD");
                mvwprintw(textwin, 15, 13, "┌──────────────────┐");
                mvwprintw(textwin, 16, 13, "│   %s   │", record[recordline].c_str());
                mvwprintw(textwin, 17, 13, "└──────────────────┘");
                switch(mode_selected){
                    case 0:
                        mvwprintw(textwin, 30, 0, "Type the number 1 to 4 to fill in the cell");
                        break;
                    case 1:
                        mvwprintw(textwin, 30, 0, "Type the number 1 to 9 to fill in the cell");
                        break;
                    case 2:
                        mvwprintw(textwin, 29, 0, "Type the number 1 to 9 and Capital \"A B C D E");
                        mvwprintw(textwin, 30, 0, "F G\" to fill in the cell");
                        break;
                }
                mvwprintw(textwin, 31, 0, "Press \"esc\" to exit the game without saving");
                
                wclear(borderwin);
                box(borderwin, 0, 0);
                refresh();
                wrefresh(borderwin);
                wrefresh(gamewin);
                wrefresh(textwin);
                
    //Gaming elements initialize
    //==============================================================================================
                for(int i = 0; i < mat; i++) {
                    for(int j = 0; j < mat; j++) {
                        mvwprintw(gamewin, i*2+1, j*4+2, "%c", display[i][j]);
                    }
                } //display value
                for(int i = 0; i < mat; i++) {
                    for(int j = 0; j < mat; j++) {
                        if(display[i][j] == ' ') {
                            icursey = i;
                            icursex = j;
                            wattron(gamewin, A_REVERSE);
                            mvwprintw(gamewin, i*2+1, j*4+1, " %c ", ans[i][j]);
                            wattroff(gamewin, A_REVERSE);
                            i = mat;
                            break;
                        }
                    }
                } //hightlight the first empty top left element
                wrefresh(gamewin);
                
    //Game & Time Thread
    //==============================================================================================
                game_thread_end = false;
                thread t1(game_thread, gamewin, mode_selected);
                thread t2(timer_thread);
                t1.join();
                t2.join();
                
    //win
    //==============================================================================================
                if(finishgame) //keep the screen for 2 sec if winnig
                    usleep(2000000);
                switch(mode_selected){
                    case 0:
                        for(int i = 0; i < 12; i++) {
                            mvwprintw(textwin, i, 0, "                                              ");
                            usleep(50000);
                            wrefresh(textwin);
                        }
                        for(int i = 0; i < 9; i++) {
                            mvwprintw(textwin, 12+i, 0, "                                              ");
                            mvwprintw(gamewin, i, 0, "                 ");
                            wrefresh(textwin);
                            wrefresh(gamewin);
                            usleep(50000);
                        }
                        for(int i = 0; i < 12; i++) {
                            mvwprintw(textwin, 21+i, 0, "                                              ");
                            wrefresh(textwin);
                            usleep(50000);
                        }
                        break;
                    case 1:
                        for(int i = 0; i < 7; i++) {
                            mvwprintw(textwin, i, 0, "                                              ");
                            usleep(50000);
                            wrefresh(textwin);
                        }
                        for(int i = 0; i < 19; i++) {
                            mvwprintw(textwin, 7+i, 0, "                                              ");
                            mvwprintw(gamewin, i, 0, "                                     ");
                            wrefresh(textwin);
                            wrefresh(gamewin);
                            usleep(50000);
                        }
                        for(int i = 0; i < 7; i++) {
                            mvwprintw(textwin, 26+i, 0, "                                              ");
                            wrefresh(textwin);
                            usleep(50000);
                        }
                        break;
                    case 2:
                        for(int i = 0; i < 33; i++) {
                            mvwprintw(textwin, i, 0, "                                              ");
                            mvwprintw(gamewin, i, 0, "                                                                 ");
                            wrefresh(textwin);
                            wrefresh(gamewin);
                            usleep(50000);
                        }
                        break;
                    default:
                        cout << "mode selection error!!\n\n";
                        exit(1);
                } //clear motion
                delwin(gamewin);
                delwin(textwin);
                refresh();
                
                bool newrecord = false;
                string oldrecord = record[recordline];
                if(finishgame) {
                    if(oldrecord == "00 : 00 : 00")
                        record[recordline] = gametime;
                    else {
                        if(oldrecord[0] > gametime[0])
                            newrecord = true;
                        else if(oldrecord[0] == gametime[0]) {
                            if(int(oldrecord[1]) > int(gametime[1]))
                                newrecord = true;
                            else if(oldrecord[1] == gametime[1]){
                                if(oldrecord[5] > gametime[5])
                                    newrecord = true;
                                else if(oldrecord[5] == gametime[5]){
                                    if(oldrecord[6] > gametime[6])
                                        newrecord = true;
                                    else if(oldrecord[6] == gametime[6]){
                                        if(oldrecord[10] > gametime[10])
                                            newrecord = true;
                                        else if(oldrecord[10] == gametime[10]){
                                            if(oldrecord[11] > gametime[11])
                                                newrecord = true;
                                            else
                                                newrecord = false;
                                        }
                                    }
                                }
                            }
                        }
                    }
                } //check new record
                
    //New Record Overwrite
    //==============================================================================================
                if(newrecord)
                    record[recordline] = gametime;
                ofstream recordfile_overwrite;
                recordfile_overwrite.open("/Users/bingyuli/Documents/CPP/Sudoku/Sudoku/record.dat");
                if (recordfile_overwrite.fail()) {
                    cout << "Failed to open and overwrite record file.";
                    exit(1);
                }
                for(int i = 0; i < 9; i++) {
                    recordfile_overwrite << record[i]+"\n";
                }
                recordfile_overwrite.close();
                
    //Game Finished Win
    //==============================================================================================
                if(finishgame) {
                    mvwprintw(borderwin, 13, 29, " __  __    ______    __  __      _____    __   _____       __   ______");
                    mvwprintw(borderwin, 14, 29, "/\\ \\_\\ \\  /\\  __ \\  /\\ \\/\\ \\    /\\  __-. /\\ \\ /\\  __-.    /\\ \\ /\\__  _\\");
                    mvwprintw(borderwin, 15, 29, "\\ \\____ \\ \\ \\ \\/\\ \\ \\ \\ \\_\\ \\   \\ \\ \\/\\ \\\\ \\ \\\\ \\ \\/\\ \\   \\ \\ \\\\/_/\\ \\/");
                    mvwprintw(borderwin, 16, 29, " \\/\\_____\\ \\ \\_____\\ \\ \\_____\\   \\ \\____- \\ \\_\\\\ \\____-    \\ \\_\\  \\ \\_\\");
                    mvwprintw(borderwin, 17, 29, "  \\/_____/  \\/_____/  \\/_____/    \\/____/  \\/_/ \\/____/     \\/_/   \\/_/");
                    if(newrecord) {
                        mvwprintw(borderwin, 20, 51, "NEW RECORD!!!  %s", gametime.c_str());
                    }
                    else {
                        mvwprintw(borderwin, 20, 54, "You took %s", gametime.c_str());
                    }
                    mvwprintw(borderwin, 24, 52, "Press any key to continue");
                    wrefresh(borderwin);
                    getch();
                    startgame = false;
                }
                wclear(borderwin);
                box(borderwin, 0, 0);
                wrefresh(borderwin);
            } //game if
        } //home while
    } //allow readjust windows while
}

/*
0
                                      _


 
 
1
                                      _

\


2
                                      _                      _

\


3
                                      _                      _

\

                  _
4
  _                                   _                      _
 
\
                                   
                  _
5
  _                                   _                      _
  
\
                                  \
                  _
6
  _             _                     _                      _
   
\
                                  \             _
                  _                     _
7
  _             _                     _                      _
                       
\                                                      \
            \                 -   \             _
                  _                     _                     _
8
  _             _                     _      _               _
                       \
\                                                      \
        _   \                 -   \             _
                  _          _          _                     _
9
  _             _          _          _      _               _
     _                 \
\               _                                      \
        _   \                 -   \             _
                  _          _          _     \               _
10
  _             _          _          _      _               _
     _            \    \
\               _                           \          \
        _   \                 -   \             _
        _         _     \    _          _     \            _  _
11
  _             _        _ _          _      _               _
     _            \    \
\               _                           \          \
        _   \                 -   \             _   _
        _         _     \    _       _  _     \            _  _
12
  _             _        _ _          _      _          _    _
     _     /      \    \
\               _                        \  \          \
  /     _   \                 -   \             _   _
        _         _     \    _       _  _     \            _  _
13
  _             _        _ _      _   _      _          _    _
     _     /      \    \
\               _                        \  \          \       \
  /     _   \                 -   \    _        _   _
        _       _ _     \    _       _  _     \            _  _
14
  _         _   _        _ _      _   _      _          _    _
     _     /      \    \                          /
\               _             \          \  \          \       \
  /     _   \                 -   \    _        _   _
     _  _       _ _     \  _ _       _  _     \            _  _
15
  _         _   _        _ _      _   _      _          _    _
     _     /      \    \                        / /
\    _          _     \       \          \  \          \       \
  /     _   \    _            -   \    _        _   _
     _  _       _ _     \  _ _       _  _     \            _  _
16
  _         _   _        _ _      _   _      _          __   _
     _ \   /      \    \    -                   / /
\    _          _     \       \          \  \          \       \
  / _   _   \    _            -   \    _        _   _      _
     _  _       _ _     \  _ _       _  _ /   \            _  _
17
  _         _   _        _ _      _   _      _          __   _
     _ \   /      \    \    -     \             / /
\    _          _     \       \          \  \    _     \       \
  / _   _   \    _       \    -   \    _        _   _      _
     _  _     / _ _     \  _ _       _  _ /   \          \ _  _
18
  _  _      _   __       _ _      _   _      _          __   _
     _ \   /      \    \   _-     \             / /
\    _          _     \       \          \  \    _     \       \
  / _   _   \    __      \    -   \    _        _   _      _
     _  _     / _ _     \  _ _      /_  _ /   \          \ _  _
19
  _  _      _   __       _ _      _   _      _          __   _
     _ \   /      \    \   _-     \   _         / /
\  _ _          _     \       \          \  \    _     \       \
  / _   _   \    __      \    -   \    _        _   _      _
     _  _     /__ _     \  _ _      /_  _ /   \      /   \ _  _
20
  _  _      _   __       _ _      _ _ _      _          __   _
     _ \   /      \    \   _-     \   _         / /
\  _ _          _  \  \       \          \  \    _     \    _  \
  / _   _   \    __      \    -   \    _        _   _      _
     _  _     /__ _     \  _ _      /_  _ /   \   \  /   \ _  _
21
  _  _      _   __       _ _      _ _ __     _          __   _
     _ \   /      \    \   _-.    \   _     /   / /
\  _ _          _  \  \       \          \  \    _     \    _  \
  / _   _   \    __      \   _-   \    __       _   _      _
     _  _/    /__ _     \  _ _      /_  _ /   \   \  /   \ _  _
22
  _  _      _   __     _ _ _      _ _ __     _          __   _
     __\   /      \    \   _-.    \   _     /   / /
\  _ _          _  \  \       \      \   \  \    _ -   \    _  \
  / _   _   \    __      \   _-   \    __       _   _      _
     _  _/    /__ __    \  _ _      /_  _ /   \   \  /   \ _  _
23
  _  _      _   __     _ _ _      _ _ __     _   _      __   _
     __\   /\     \    \   _-.   /\   _     /   / /
\  _ _          _  \  \       \      \   \  \    _ -   \    _  \
  / __  _   \    __      \   _-   \    __       _   _      _
     _  _/    /__ __    \ __ _      /_  _ /   \   \  /   \/_  _
24
  _  _      _   __     _ _ _      _ _ __     _   _      __   _
     __\   /\     \    \  __-.   /\   _     /   / /
\  _ _          _  \  \     \ \      \   \  \    _ -.  \    _  \
  / __  _   \    __      \   _-   \ \  __       _   _      _
     __ _/    /__ __    \ __ _      /_  _ /   \   \  /   \/_  _
25
  _  _      _   __     _ _ _      _ _ __     _   _      __   _
     __\   /\     \   /\  __-.   /\   _     /   / /       \
\ \_ _          _  \  \    /\ \      \ \ \  \    _ -.  \    _  \
  / __  _   \ \  __      \   _-   \ \  __       _   _      _
     __ _/    /__ __    \ __ _      /_  _ /   \   \ _/   \/_  _
26
  _  _      _   __     _ ___      ___ __     _   _      __   _
     __\   /\     \   /\  __-.   /\   _     /   / /       \
\ \_ _     \    _  \  \    /\ \      \ \ \  \    _ -.  \    _  \
  / __  _   \ \  __      \   _-   \ \  __      \_   _   \  _  _
   / __ _/    /__ __    \ __ _      /_  __/   \   \/_/   \/_  _ /
27
  _  _      _   __     _ ___      ___ __     _   _      __   _
     __\   /\   \ \   /\  __-.   /\   _     /   / /       \ \ \
\ \_ _     \    _  \  \    /\ \      \ \ \  \    _ -.  \ \  _  \
  / __ __   \ \  __      \  __-   \ \  __      \_   _   \  _  _
   / __ _/    /__ __/   \ __ _      /_  __/   \  /\/_/   \/_  _ /
28
  _  _      _   __     _ ___      ___ __     _   _      __  __
     __\   /\ \ \ \   /\  __-.   /\   _     /   / /       \ \ \
\ \_ _     \    _  \  \    /\ \  \   \ \ \  \    _ -.  \ \  _  \
  /\__ __   \ \  __ \    \  __-   \ \  __      \_   _\  \  _  _
   / __ _/   \/__ __/   \ __ _/     /_  __/   \  /\/_/   \/__ _ /
29
  _  _      _   __     _ ___      ______     _   _      __  __
 \   __\   /\ \ \ \   /\  __-.   /\   _     /  \/ /       \ \ \
\ \___     \   \_  \  \    /\ \  \   \ \ \  \    _"-.  \ \  _  \
  /\__ __   \ \  __ \    \  __-   \ \ ___ \    \_\  _\  \  _  __
   / __ _/   \/__ __/   \/____/     /__ __/   \  /\/_/   \/__ _ /
30
  ____      _   __     _ ___      ______     __  _      __  __
 \  ___\   /\ \ \ \   /\  __-.   /\   _     /  \/ /    /  \ \ \
\ \___     \   \_\ \  \ \  /\ \  \   \ \ \  \    _"-.  \ \  _\ \
  /\_____   \ \  __ \    \  __-   \ \ ____\  \ \_\  _\  \  _  __
   / __ _/   \/__ __/   \/____/    \/__ __/   \  /\/_/   \/__ _ /
31
  _____     _   __     _____      ______     __  __     __  __
/\  ___\   /\ \ \ \   /\  __-.   /\  __     /\ \/ /    /  \ \ \
\ \___     \   \_\ \  \ \  /\ \  \   \/\ \  \    _"-.  \ \ \_\ \
  /\_____   \ \ ____\  \ \  __-   \ \ ____\  \ \_\  _\  \  _  __
  \/ __ _/   \/__ __/   \/____/    \/_____/   \  /\/_/   \/____ /
32
 ______     __  __     _____      ______     __  __     __  __
/\  ___\   /\ \/\ \   /\  __-.   /\  __ \   /\ \/ /    /\ \/\ \
\ \___  \  \ \ \_\ \  \ \ \/\ \  \ \ \/\ \  \ \  _"-.  \ \ \_\ \
 \/\_____\  \ \_____\  \ \____-   \ \_____\  \ \_\ \_\  \ \_____\
  \/_____/   \/_____/   \/____/    \/_____/   \/_/\/_/   \/_____/
 
 ╔═══╤═══╦═══╤═══╗
 ║   │   ║   │   ║
 ╟───┼───╫───┼───╢
 ║   │   ║   │   ║
 ╠═══╪═══╬═══╪═══╣
 ║   │   ║   │   ║
 ╟───┼───╫───┼───╢
 ║   │   ║   │   ║
 ╚═══╧═══╩═══╧═══╝
 
 ╔═══╤═══╤═══╦═══╤═══╤═══╦═══╤═══╤═══╗
 ║   │   │   ║   │   │   ║   │   │   ║
 ╟───┼───┼───╫───┼───┼───╫───┼───┼───╢
 ║   │   │   ║   │   │   ║   │   │   ║
 ╟───┼───┼───╫───┼───┼───╫───┼───┼───╢
 ║   │   │   ║   │   │   ║   │   │   ║
 ╠═══╪═══╪═══╬═══╪═══╪═══╬═══╪═══╪═══╣
 ║   │   │   ║   │   │   ║   │   │   ║
 ╟───┼───┼───╫───┼───┼───╫───┼───┼───╢
 ║   │   │   ║   │   │   ║   │   │   ║
 ╟───┼───┼───╫───┼───┼───╫───┼───┼───╢
 ║   │   │   ║   │   │   ║   │   │   ║
 ╠═══╪═══╪═══╬═══╪═══╪═══╬═══╪═══╪═══╣
 ║   │   │   ║   │   │   ║   │   │   ║
 ╟───┼───┼───╫───┼───┼───╫───┼───┼───╢
 ║   │   │   ║   │   │   ║   │   │   ║
 ╟───┼───┼───╫───┼───┼───╫───┼───┼───╢
 ║   │   │   ║   │   │   ║   │   │   ║
 ╚═══╧═══╧═══╩═══╧═══╧═══╩═══╧═══╧═══╝
 
 ╔═══╤═══╤═══╤═══╦═══╤═══╤═══╤═══╦═══╤═══╤═══╤═══╦═══╤═══╤═══╤═══╗
 ║   │   │   │   ║   │   │   │   ║   │   │   │   ║   │   │   │   ║
 ╟───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╢
 ║   │   │   │   ║   │   │   │   ║   │   │   │   ║   │   │   │   ║
 ╟───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╢
 ║   │   │   │   ║   │   │   │   ║   │   │   │   ║   │   │   │   ║
 ╟───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╢
 ║   │   │   │   ║   │   │   │   ║   │   │   │   ║   │   │   │   ║
 ╠═══╪═══╪═══╪═══╬═══╪═══╪═══╪═══╬═══╪═══╪═══╪═══╬═══╪═══╪═══╪═══╣
 ║   │   │   │   ║   │   │   │   ║   │   │   │   ║   │   │   │   ║
 ╟───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╢
 ║   │   │   │   ║   │   │   │   ║   │   │   │   ║   │   │   │   ║
 ╟───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╢
 ║   │   │   │   ║   │   │   │   ║   │   │   │   ║   │   │   │   ║
 ╟───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╢
 ║   │   │   │   ║   │   │   │   ║   │   │   │   ║   │   │   │   ║
 ╠═══╪═══╪═══╪═══╬═══╪═══╪═══╪═══╬═══╪═══╪═══╪═══╬═══╪═══╪═══╪═══╣
 ║   │   │   │   ║   │   │   │   ║   │   │   │   ║   │   │   │   ║
 ╟───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╢
 ║   │   │   │   ║   │   │   │   ║   │   │   │   ║   │   │   │   ║
 ╟───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╢
 ║   │   │   │   ║   │   │   │   ║   │   │   │   ║   │   │   │   ║
 ╟───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╢
 ║   │   │   │   ║   │   │   │   ║   │   │   │   ║   │   │   │   ║
 ╠═══╪═══╪═══╪═══╬═══╪═══╪═══╪═══╬═══╪═══╪═══╪═══╬═══╪═══╪═══╪═══╣
 ║   │   │   │   ║   │   │   │   ║   │   │   │   ║   │   │   │   ║
 ╟───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╢
 ║   │   │   │   ║   │   │   │   ║   │   │   │   ║   │   │   │   ║
 ╟───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╢
 ║   │   │   │   ║   │   │   │   ║   │   │   │   ║   │   │   │   ║
 ╟───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╫───┼───┼───┼───╢
 ║   │   │   │   ║   │   │   │   ║   │   │   │   ║   │   │   │   ║
 ╚═══╧═══╧═══╧═══╩═══╧═══╧═══╧═══╩═══╧═══╧═══╧═══╩═══╧═══╧═══╧═══╝
 
 ╔═════════════════════════════════════════════╗
 ║         ___          _     _                ║
 ║        / __|_  _  __| |___| |___  _         ║
 ║        \__ \ || |/ _` | _ \ / / || |        ║
 ║        |___/\_,_|\__,_|___/_\_\\_,_|        ║
 ║                                             ║
 ╚═════════════════════════════════════════════╝
         ╔═════════════════════════════╗
         ║         PLAY SUDOKU         ║
         ╚═════════════════════════════╝
         ╔═════════════════════════════╗
         ║        GAME SETTINGS        ║
         ╚═════════════════════════════╝
         ╔═════════════════════════════╗
         ║          QUIT GAME          ║
         ╚═════════════════════════════╝
 
 ╔═════════════════════════════╗
 ║       Readjust Window       ║
 ╚═════════════════════════════╝
 ╔═════════════════════════════╗
 ║  Exclude Multiple Ans ║ OFF ║
 ╚═════════════════════════════╝
 ╔═════════════════════════════╗
 ║  Exclude Multiple Ans ║ ON  ║
 ╚═════════════════════════════╝
 ╔═════════════════════════════╗
 ║      Clear Game Record      ║
 ╚═════════════════════════════╝
 ╔═════════════════════════════╗
 ║            About            ║
 ╚═════════════════════════════╝
 ╔═════════════════════════════╗
 ║        Exit Settings        ║
 ╚═════════════════════════════╝
 
 ╔═══════════════════════════════════╗
 ║      _ _    _           _ _       ║
 ║     | | |  | |__ _  _  | | |      ║
 ║     |_  _| | '_ \ || | |_  _|     ║
 ║       |_|  |_.__/\_, |   |_|      ║
 ║                  |__/             ║
 ╚═══════════════════════════════════╝
 ╔═══════════════════════════════════╗
 ║       ___   _           ___       ║
 ║      / _ \ | |__ _  _  / _ \      ║
 ║      \_, / | '_ \ || | \_, /      ║
 ║       /_/  |_.__/\_, |  /_/       ║
 ║                  |__/             ║
 ╚═══════════════════════════════════╝
 ╔═══════════════════════════════════╗
 ║     _  __   _           _  __     ║
 ║    / |/ /  | |__ _  _  / |/ /     ║
 ║    | / _ \ | '_ \ || | | / _ \    ║
 ║    |_\___/ |_.__/\_, | |_\___/    ║
 ║                  |__/             ║
 ╚═══════════════════════════════════╝
 
 ╔═══════════════════════════════════╗
 ║        ___   _   _____   __       ║
 ║       | __| /_\ / __\ \ / /       ║
 ║       | _| / _ \\__ \\ V /        ║
 ║       |___|_/ \_\___/ |_|         ║
 ║      ═══════════════════════      ║
 ╚═══════════════════════════════════╝
 ╔═══════════════════════════════════╗
 ║       _  _  ___  ___ __  __       ║
 ║      | \| |/ _ \| _ \  \/  |      ║
 ║      | .` | (_) |   / |\/| |      ║
 ║      |_|\_|\___/|_|_\_|  |_|      ║
 ║      ═══════════════════════      ║
 ╚═══════════════════════════════════╝
 ╔═══════════════════════════════════╗
 ║        _  _   _   ___  ___        ║
 ║       | || | /_\ | _ \|   \       ║
 ║       | __ |/ _ \|   /| |) |      ║
 ║       |_||_/_/ \_\_|_\|___/       ║
 ║      ═══════════════════════      ║
 ╚═══════════════════════════════════╝
 
 __        ______    _____     __    __   __    ______
/\ \      /\  __ \  /\  __-.  /\ \  /\ "-.\ \  /\  ___\
\ \ \____ \ \ \/\ \ \ \ \/\ \ \ \ \ \ \ \-.  \ \ \ \__ \    __    __    __
 \ \_____\ \ \_____\ \ \____-  \ \_\ \ \_\\"\_\ \ \_____\  /\_\  /\_\  /\_\
  \/_____/  \/_____/  \/____/   \/_/  \/_/ \/_/  \/_____/  \/_/  \/_/  \/_/
 
 ╔════════════════════════════════════════════╗
 ║         ___          _     _               ║
 ║        / __|_  _  __| |___| |___  _        ║
 ║        \__ \ || |/ _` | _ \ / / || |       ║
 ║        |___/\_,_|\__,_|___/_\_\\_,_|       ║
 ║                                            ║
 ╚════════════════════════════════════════════╝

 __  __    ______    __  __      _____    __   _____       __   ______
/\ \_\ \  /\  __ \  /\ \/\ \    /\  __-. /\ \ /\  __-.    /\ \ /\__  _\
\ \____ \ \ \ \/\ \ \ \ \_\ \   \ \ \/\ \\ \ \\ \ \/\ \   \ \ \\/_/\ \/
 \/\_____\ \ \_____\ \ \_____\   \ \____- \ \_\\ \____-    \ \_\  \ \_\
  \/_____/  \/_____/  \/_____/    \/____/  \/_/ \/____/     \/_/   \/_/

 */
