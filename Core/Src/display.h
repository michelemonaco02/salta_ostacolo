#ifndef DISPLAY_H
#define DISPLAY_H

#include "game.h"
#include <stdint.h>
#include <stddef.h>

// Inizializza il display OLED SSD1306
void SSD1306_Init(void);

// Pulisce completamente lo schermo del display
void SSD1306_ClearScreen(void);

// Spegne il display (Display OFF)
void SSD1306_PowerOff(void);

// Aggiorna il display con i dati correnti del gioco (giocatore + ostacoli)
void updateDisplay(Game* g);

// Rimuove solo i pixel disegnati sul display (giocatore e ostacoli attuali)
void cleanDisplay(Game* g);


#endif // DISPLAY_H
