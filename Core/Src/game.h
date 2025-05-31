#ifndef SRC_GAME_H_
#define SRC_GAME_H_

#define LCD_WIDTH 128
#define LCD_HEIGHT 64
#define OBS_MAX_LEN 10
#define OBS_MIN_LEN 5
#define COL_GIOC 5

#define INATTIVO 0
#define IN_GIOCO 1
#define OVER 2

typedef struct {
	int estremita_sup;
	int estremita_inf;
	int coord_x;
} Ostacolo;

typedef struct Nodo {
	Ostacolo ost;
	struct Nodo* next;
} Nodo;

typedef struct {
	Nodo* head;
} ListaOstacoli;

typedef struct {
	int y_giocatore;
	float v_giocatore;
	ListaOstacoli lista_ostacoli;
	int stato;  //0 inattivo, 1 in gioco, 2 game over
	float durata_corrente;
} Game;

void initGame(Game* g);
void updateGame(Game* g,int new_y_giocatore,int new_v_giocatore);
void endGame(Game* g);

#endif /* SRC_GAME_H_ */
