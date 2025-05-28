/*
 * game.h
 *
 *  Created on: May 28, 2025
 *      Author: angio
 */

#ifndef SRC_GAME_H_
#define SRC_GAME_H_


typedef struct{
	float estremita_sup;
	float lunghezza;
	float coord_x;
}Ostacolo;

typedef struct ListaOstacoli{
	Ostacolo ost;
	struct ListaOstacoli* next;
}ListaOstacoli;

typedef struct{
	float altezza_giocatore;
	ListaOstacoli* lista_ostacoli;
	int stato;  //0 inattivo, 1 in gioco, 2 game over
	float durata_corrente;
}Game;

void initGame(Game* g);

void updateGame(Game* g);

void endGame(Game* g);



#endif /* SRC_GAME_H_ */
