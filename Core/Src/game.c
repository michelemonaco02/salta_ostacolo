/*
 * game.c
 *
 *  Created on: May 29, 2025
 *      Author: angio
 */

#include "game.h"
#include <stdlib.h>  // per malloc e free
#include <stdio.h>   // per debug printf se serve


void initGame(Game* g){
	g->y_giocatore = LCD_HEIGTH / 2;
	//generiamo randomicamente l'altezza
	int lunghezza = rand_in_range(OBS_MIN_LEN,OBS_MAX_LEN);
	//generiamo randomicamente l'estremita superiore dell'ostacolo
	int es_sup = rand_in_range(altezza,LCD_HEIGHT-1);
	int es_inf = es_sup - lunghezza;
	addOstacolo(g,es_sup,es_inf);
	g->stato = IN_GIOCO;
	g->durata_corrente = 0;

}

void updateGame(Game* g, int new_y_giocatore) {
    if (!g || g->stato != IN_GIOCO) return;

    // Aggiorna la posizione del giocatore
    g->y_giocatore = new_y_giocatore;

    Nodo* head = g->lista_ostacoli.head;

    // Controllo se il giocatore muore
    // Se l'ostacolo in testa ha coord_x == 0 e y_giocatore è compreso tra estremita_inf e estremita_sup
    if (head && head->ost.coord_x == 0) {
        if (g->y_giocatore >= head->ost.estremita_inf && g->y_giocatore <= head->ost.estremita_sup) {
            endGame(g);
            return;
        }
    }

    // Se il primo ostacolo è a coord_x == 0 e il giocatore non muore, lo rimuovo
    if (head && head->ost.coord_x == 0) {
        removeOstacolo(g);
        head = g->lista_ostacoli.head;  // aggiorno la testa dopo la rimozione
    }

    // Aggiorno la posizione di tutti gli ostacoli spostandoli a sinistra (coord_x - 1)
    Nodo* current = head;
    while (current) {
        current->ost.coord_x -= 1;
        current = current->next;
    }

    // Controllo se devo aggiungere un nuovo ostacolo
    // (qui chiamiamo la funzione placeholder check_aggiungi_ostacolo che ritorna int)
    if (check_aggiungi_ostacolo(g)) {
        // genero valori random per il nuovo ostacolo
        int lunghezza = rand_in_range(OBS_MIN_LEN, OBS_MAX_LEN);
        int es_sup = rand_in_range(0, LCD_HEIGHT - lunghezza - 1);
        int es_inf = es_sup + lunghezza;  // attenzione a estremità inf = sup + lunghezza (non sottrazione)

        addOstacolo(g, es_inf, es_sup);  // attenzione all'ordine parametri di addOstacolo (supponendo estremita_sup, estremita_inf)
    }

    // Aggiorno durata corrente
    g->durata_corrente += 1;  // o incrementa in base a un delta time passato o fisso
}


void endGame(Game* g){
	if (!g) return;

	// Imposta stato game over
	g->stato = OVER;

	// Libera tutta la lista di ostacoli
	Nodo* current = g->lista_ostacoli.head;
	while (current) {
		Nodo* temp = current;
		current = current->next;
		free(temp);
	}
	g->lista_ostacoli.head = NULL;

}


int rand_in_range(int min, int max) {
    return min + rand() % (max - min + 1);
}

// Aggiunge un ostacolo in coda nella lista di ostacoli del gioco
void addOstacolo(Game* g, int estremita_sup, int es_inf) {
    if (!g) return;

    Nodo* nuovo = (Nodo*)malloc(sizeof(Nodo));
    if (!nuovo) return; // errore allocazione

    nuovo->ost.estremita_sup = estremita_sup;
    nuovo->ost.estremita_inf = es_inf;
    // Coord_x non viene passato, presumibilmente va settato a 0 o lasciato così?
    nuovo->ost.coord_x = LCD_WIDTH-1;
    nuovo->next = NULL;

    if (g->lista_ostacoli.head == NULL) {
        // lista vuota -> nuovo nodo diventa head
        g->lista_ostacoli.head = nuovo;
    } else {
        // scorro fino alla fine e inserisco
        Nodo* temp = g->lista_ostacoli.head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = nuovo;
    }
}

// Rimuove il primo ostacolo in testa alla lista di ostacoli
void removeOstacolo(Game* g) {


    if (!g || !g->lista_ostacoli.head) return;

    Nodo* temp = g->lista_ostacoli.head;
    g->lista_ostacoli.head = temp->next;
    free(temp);
}
void check_aggiungi_ostacolo(Game* g);
