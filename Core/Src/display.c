#include "stm32f3xx_hal.h"
#include "game.h"
#include "display.h"
#include <string.h> // per memset
#include <stdlib.h> // per malloc

extern I2C_HandleTypeDef hi2c1;

//prende in ingresso un Game
//invia al display tramite funzioni hal, la matrice che dobbiamo visualizzare
/*
void updateDisplay(Game* g){
	//SSD1306_ClearScreen();
	uint8_t buffer[1024];
    memset(buffer, 0x00, sizeof(buffer)); // schermo nero

    if (g->stato == IN_GIOCO) {
        // --- Giocatore ---
        int x_player = 5;
        int y = g->y_giocatore;
        if (y >= 0 && y < LCD_HEIGHT) {
            int page = y / 8;
            int bit_in_page = y % 8;
            buffer[page * LCD_WIDTH + x_player] |= (1 << bit_in_page);
        }

        // --- Ostacoli ---
        Nodo* curr = g->lista_ostacoli.head;
        while (curr != NULL) {
            int x = curr->ost.coord_x;
            if (x >= 0 && x < LCD_WIDTH) {
                // Parte alta
                for (int y = curr->ost.estremita_inf; y < curr->ost.estremita_sup; y++) {
                    int page = y / 8;
                    int bit = y % 8;
                    buffer[page * LCD_WIDTH + x] |= (1 << bit);
                }

            }
            curr = curr->next;
        }
    }

    // Imposta indirizzo pagina e colonna (necessario per visualizzare)
    for (int page = 0; page < 8; page++) {
        SSD1306_WriteCommand(0xB0 + page);      // Page address
        SSD1306_WriteCommand(0x00);             // Lower column start address
        SSD1306_WriteCommand(0x10);             // Higher column start address

        SSD1306_WriteData(&buffer[page * LCD_WIDTH], LCD_WIDTH);
    }
}*/
void updateDisplay(Game* g){
    if (g->stato != IN_GIOCO) return;

    // Scrive il pixel del giocatore
    int x_player = 5;
    int y = g->y_giocatore;
    if (y >= 0 && y < LCD_HEIGHT) {
        int page = y / 8;
        int bit_in_page = y % 8;

        SSD1306_WriteCommand(0xB0 + page);   // Page address
        SSD1306_WriteCommand(0x00 + (x_player & 0x0F)); // Lower col
        SSD1306_WriteCommand(0x10 + (x_player >> 4));   // Higher col

        uint8_t pixel = (1 << bit_in_page);
        SSD1306_WriteData(&pixel, 1);
    }

    // Scrive ogni ostacolo
    Nodo* curr = g->lista_ostacoli.head;
    while (curr != NULL) {
        int x = curr->ost.coord_x;
        if (x >= 0 && x < LCD_WIDTH) {
            uint8_t page_data[8] = {0}; // buffer per ogni pagina

            for (int y = curr->ost.estremita_inf; y < curr->ost.estremita_sup; y++) {
                int page = y / 8;
                int bit = y % 8;
                page_data[page] |= (1 << bit);
            }

            for (int page = curr->ost.estremita_inf / 8; page <= (curr->ost.estremita_sup - 1) / 8; page++) {
                SSD1306_WriteCommand(0xB0 + page);
                SSD1306_WriteCommand(0x00 + (x & 0x0F));
                SSD1306_WriteCommand(0x10 + (x >> 4));

                SSD1306_WriteData(&page_data[page], 1);
            }
        }
        curr = curr->next;
    }
}


void cleanDisplay(Game* g){
	//questa funzione è diversa dalla clearScreen che riscrive ogni pixel
	//in questa funzione, per ottimizzare, riscriviamo solo i pixel effettivamente bianchi

    // Rimuovi pixel del giocatore
    int x_player = 5;
    int y = g->y_giocatore;
    if (y >= 0 && y < LCD_HEIGHT) {
        int page = y / 8;
        int bit = y % 8;
        SSD1306_WriteCommand(0xB0 + page);   // Page address
        SSD1306_WriteCommand(0x00 + (x_player & 0x0F)); // Lower col
        SSD1306_WriteCommand(0x10 + (x_player >> 4));   // Upper col

        uint8_t clear_byte = 0x00;  // spegne tutti i bit
        SSD1306_WriteData(&clear_byte, 1);
    }

    // Rimuovi ogni ostacolo
    Nodo* curr = g->lista_ostacoli.head;
    while (curr != NULL) {
        int x = curr->ost.coord_x;
        if (x >= 0 && x < LCD_WIDTH) {
            // Per ogni pagina coinvolta, azzera solo i bit dell'ostacolo
            uint8_t page_data[8] = {0}; // massimo 8 pagine
            for (int y = curr->ost.estremita_inf; y < curr->ost.estremita_sup; y++) {
                int page = y / 8;
                int bit = y % 8;
                page_data[page] &= ~(1 << bit);  // spegne solo quel bit (inutile qui, ma formale)
            }
            // Scrive 0 su ogni pagina toccata
            for (int page = curr->ost.estremita_inf / 8; page <= (curr->ost.estremita_sup - 1) / 8; page++) {
                SSD1306_WriteCommand(0xB0 + page);
                SSD1306_WriteCommand(0x00 + (x & 0x0F));
                SSD1306_WriteCommand(0x10 + (x >> 4));

                uint8_t zero = 0x00;
                SSD1306_WriteData(&zero, 1);
            }
        }
        curr = curr->next;
    }
}

void SSD1306_WriteCommand(uint8_t cmd) {
    uint8_t data[2];
    data[0] = 0x00;  // Control byte: Co = 0, D/C# = 0 (command)
    data[1] = cmd;
    HAL_I2C_Master_Transmit(&hi2c1, 0x78, data, 2, HAL_MAX_DELAY);
}

void SSD1306_WriteData(uint8_t* data, size_t size) {
    uint8_t *buffer = malloc(size + 1);
    buffer[0] = 0x40;  // Control byte: Co = 0, D/C# = 1 (data)
    memcpy(&buffer[1], data, size);
    HAL_I2C_Master_Transmit(&hi2c1, 0x78, buffer, size + 1, HAL_MAX_DELAY);
    free(buffer);
}

void SSD1306_Init(void) {
    HAL_Delay(100); // per sicurezza
    SSD1306_WriteCommand(0xAE); // Display OFF

    SSD1306_WriteCommand(0x20); // Set Memory Addressing Mode
    SSD1306_WriteCommand(0x00); // Horizontal Addressing Mode

    SSD1306_WriteCommand(0xB0); // Page Start Address for Page Addressing Mode
    SSD1306_WriteCommand(0xC8); // COM Output Scan Direction
    SSD1306_WriteCommand(0x00); // Low column address
    SSD1306_WriteCommand(0x10); // High column address
    SSD1306_WriteCommand(0x40); // Start line address

    SSD1306_WriteCommand(0x81); // Set contrast
    SSD1306_WriteCommand(0x7F);

    SSD1306_WriteCommand(0xA1); // Segment Re-map
    SSD1306_WriteCommand(0xA6); // Normal display
    SSD1306_WriteCommand(0xA8); // Multiplex ratio
    SSD1306_WriteCommand(0x3F);

    SSD1306_WriteCommand(0xA4); // Display follows RAM content
    SSD1306_WriteCommand(0xD3); // Display offset
    SSD1306_WriteCommand(0x00); // no offset

    SSD1306_WriteCommand(0xD5); // Display clock divide
    SSD1306_WriteCommand(0xF0); // suggested ratio

    SSD1306_WriteCommand(0xD9); // Pre-charge period
    SSD1306_WriteCommand(0x22);

    SSD1306_WriteCommand(0xDA); // COM pins hardware config
    SSD1306_WriteCommand(0x12);

    SSD1306_WriteCommand(0xDB); // VCOMH deselect level
    SSD1306_WriteCommand(0x20);

    SSD1306_WriteCommand(0x8D); // Charge pump setting
    SSD1306_WriteCommand(0x14);

    SSD1306_WriteCommand(0xAF); // Display ON
}

void SSD1306_ClearScreen(void) {
    SSD1306_WriteCommand(0x21); // Set column address
    SSD1306_WriteCommand(0);    // Start at 0
    SSD1306_WriteCommand(127);  // End at 127

    SSD1306_WriteCommand(0x22); // Set page address
    SSD1306_WriteCommand(0);    // Page 0
    SSD1306_WriteCommand(7);    // Page 7

    uint8_t buffer[128];
    for (int i = 0; i < 128; i++) buffer[i] = 0x00;  // Tutti 0 per spegnere i pixel

    for (int page = 0; page < 8; page++) {
        SSD1306_WriteCommand(0xB0 + page); // Set page
        SSD1306_WriteCommand(0x00);        // Low col
        SSD1306_WriteCommand(0x10);        // High col
        SSD1306_WriteData(buffer, 128);    // Write all columns with 0 (black)
    }
}


// Spegne il display (Display OFF)
void SSD1306_PowerOff(void) {
    SSD1306_WriteCommand(0xAE);
}
