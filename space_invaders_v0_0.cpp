#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <conio.h>

#define WIDTH  20
#define HEIGHT  40
//====================================================
class invader {
protected:
    int speed;
    int column;
    int line;
    bool shoot_down;
    bool succeed;
public:
    invader(int in_column = -1);
    int getC(void);
    int getL(void);
    void fall(void);
    void get_destroyed(void);
    bool is_destroyed(void);
};

invader::invader(int in_column) {
    shoot_down = false;
    succeed = false;
    if (in_column == -1) {
        column = (int) rand()%(WIDTH-1);
    } else {column = in_column;}
    line = HEIGHT;
    speed = 1;
}

bool invader::is_destroyed(void) {
    return shoot_down;
}

int invader::getC(void) {
    return column;
}

int invader::getL(void) {
    return line;
}

void invader::get_destroyed(void) {
    succeed = false;
    shoot_down = true;
}

void invader::fall(void) {
    if (line == 0) {
        succeed = true;
        shoot_down = false;
    } else line--;
}


//====================================================
class missile {
protected:
    double column;
    int line;
    bool fired;
    bool destroyed;
public:
    missile(double in_column = 12);
    void change_col(double in_column);
    void rise(void);
    int getC(void);
    int getL(void);
    void get_destroyed(void);
    bool is_destroyed(void);
};

missile::missile(double in_column) {
    destroyed = false;
    column = in_column;
    line = 0;
}

void missile::change_col(double in_column) {
    column = in_column;
}
void missile::rise(void) {
    line++;;
}

void missile::get_destroyed(void) {
    destroyed = true;
}

bool missile::is_destroyed(void) {
    return destroyed;
}

int missile::getC(void) {
    return column;
}

int missile::getL(void) {
    return line;
}
//====================================================
class robot {
protected:
    double column;
    double speed;
public:
    robot(double in_speed);
    void move(double how_many);
    missile* shoot(void);
    double getC(void);
};

robot::robot(double in_speed) {
    speed = in_speed;
    column = WIDTH/2;
}

double robot::getC(void) {
    return column;
}

void robot::move(double how_many) {
    if (how_many > 0) {
        if (how_many < -speed)
            column += speed;
        else column += how_many;
    } else {
        if (how_many > speed)
            column += speed;
        else column += how_many;
    }
    if (column == WIDTH) column--;
    if (column == -1) column++;
}

missile* robot::shoot(void) {
    return new missile(column);
}


//====================================================
class invasion {
protected:
    int nb_soldiers;
    robot** soldiers;
    int nb_invaders;
    invader** invaders;
    int nb_missiles;
    missile** missiles;
    //      ligne    colonne
    char map[HEIGHT][WIDTH];
public:
    invasion(int in_nb_soldiers = 1);
    void print(void);
    void soldier_moves(int id_soldier, double how_many);
    void invades(void);
    bool cycle(void);
    void soldier_shoots(int id_soldier);
    int getdistanceWorstInvader(int id_soldier);
    bool isRobotAligned(int id_soldier);
};

invasion::invasion(int in_nb_soldiers) {
    nb_soldiers = in_nb_soldiers;
    soldiers =  (robot**) malloc(nb_soldiers*sizeof(robot*));
    for (int i = 0; i < nb_soldiers ; i++) {
        soldiers[i] = new robot(10.0);
    }
    invaders = NULL;
    missiles = NULL;
    nb_invaders = 0;
    nb_missiles = 0;
}

bool invasion::isRobotAligned(int id_soldier) {
    if (id_soldier < nb_soldiers && id_soldier >= 0) {
        // test des collisions missile-invader
        int col_robot = soldiers[id_soldier]->getC();
        for (int i = 0; i < nb_invaders; i++) {
            if(!invaders[i]->is_destroyed()) {
                if (invaders[i]->getC() == col_robot) return true;
            }
        }
    }
    return false;
}

int invasion::getdistanceWorstInvader(int id_soldier) {
    if (id_soldier < nb_soldiers && id_soldier >= 0) {
        // test des collisions missile-invader
        int col_robot = soldiers[id_soldier]->getC();
        int lower_line = 1000;
        int index_worst = -1;
        for (int i = 0; i < nb_invaders; i++) {
            if(!invaders[i]->is_destroyed()) {
                if(invaders[i]->getL()<lower_line) {
                    lower_line = invaders[i]->getL();
                    index_worst = i;
                }
            }
        }
        if (index_worst != -1) {
            return invaders[index_worst]->getC() - col_robot;
        } else return 0;
    }
    return 0;
}

bool invasion::cycle(void) {
    bool robot_killed = false;
    for (int i = 0; i < nb_invaders; i++) {
        if(!invaders[i]->is_destroyed()) {
            invaders[i]->fall();
        }
    }
    for (int i = 0; i < nb_missiles; i++) {        
        if(!missiles[i]->is_destroyed()) {
            missiles[i]->rise();
        }
    }
    // test des collisions missile-invader
    for (int i = 0; i < nb_invaders; i++) {
        int col_i = invaders[i]->getC();
        int lig_i = invaders[i]->getL();
        for (int j = 0; j < nb_missiles; j++) {
            int col_m = missiles[j]->getC();
            int lig_m = missiles[j]->getL();    
            if (col_i==col_m) {
                if (lig_m > lig_i) {
                    invaders[i]->get_destroyed();
                    missiles[j]->get_destroyed();
                    invades();
                }
            }
        }
    }
    return robot_killed;
}

void invasion::invades(void) {
    if(invaders != NULL) {
        invaders = (invader**) realloc(invaders,(nb_invaders+1)*sizeof(invader*));
        invaders[nb_invaders] = new invader();
        nb_invaders++;
    } else {
        invaders = (invader**) malloc(sizeof(invader*));
        invaders[0] = new invader();
        nb_invaders = 1;
    }

}

void invasion::soldier_moves(int id_soldier, double how_many) {
    if (id_soldier < nb_soldiers && id_soldier >= 0)
        soldiers[id_soldier]->move(how_many);
}

void invasion::soldier_shoots(int id_soldier) {
    if (id_soldier < nb_soldiers && id_soldier >= 0) {
        if(missiles != NULL) {
            missiles = (missile**) realloc(missiles,(nb_missiles+1)*sizeof(missile*));
            missiles[nb_missiles] = soldiers[id_soldier]->shoot();
            nb_missiles++;
        } else {
            missiles = (missile**) malloc(sizeof(missile*));
            missiles[0] = soldiers[id_soldier]->shoot();
            nb_missiles = 1;
        }
    }
}

void invasion::print(void) {
    // system("cls");
    // char str_map[WIDTH*(HEIGHT+1)] = {' '};
    memset(map,' ', sizeof(char)*WIDTH*HEIGHT);
    for (int i = 0; i < nb_invaders; i++) {
        if(!invaders[i]->is_destroyed()) {
            map[invaders[i]->getL()][invaders[i]->getC()] = 'w';
        }
    }
    for (int i = 0; i < nb_missiles; i++) {
        if(missiles[i]->getL() <= HEIGHT && !missiles[i]->is_destroyed()) {
            map[missiles[i]->getL()][missiles[i]->getC()] = '.';
        }
    }
    for (int i = 0 ; i < nb_soldiers ; i++) {
        int col_robot = soldiers[i]->getC();
        if (col_robot==0) col_robot++;
        if (col_robot==WIDTH) col_robot--;
        map[1][col_robot] = 'r';
        map[0][col_robot-1] = 'r';
        map[0][col_robot-1] = 'r';
        map[0][col_robot+1] = 'r';
    }
    
    printf("r\n\n\n\n\n\n\n");
    for (int i = HEIGHT-1 ; i >= 0 ; i--) {
        for (int j = 0 ; j <WIDTH ; j++) {
            // sprintf(str_map,"%s%c",str_map,map[j][i]);
            printf("%c",map[i][j]);
        }
        // sprintf(str_map,"%s\n",str_map);
        printf("|\n");
    }
    
    printf("%d %d\n",(int)isRobotAligned(0),getdistanceWorstInvader(0));
    // printf("%s",str_map);
}

void play(int nb_invaders) {
    invasion game;
    char c = '0';
    int icycle = 0;
    srand(NULL);
    while (icycle < nb_invaders) {
        game.cycle();
        game.print();
        while(!kbhit()); // wait for input
        c = getch();       // read input
        if(c == 'q') game.soldier_moves(0,-1.0);
        if(c == 'd') game.soldier_moves(0,1.0);
        if(c == 'i') game.invades();
        if(c == ' ') game.soldier_shoots(0);
        if(c == 'f') icycle = nb_invaders;
    }
}


int main (void) {
    play(1);
}
