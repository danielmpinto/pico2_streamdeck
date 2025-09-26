#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

// Bibliotecas para o display ILI9341
#include "tft_lcd_ili9341/ili9341/ili9341.h"
#include "tft_lcd_ili9341/gfx/gfx.h"

// Biblioteca para o touch resistivo
#include "tft_lcd_ili9341/touch_resistive/touch_resistive.h"

// === Definições para ILI9341 ===
const uint LITE = 15;         // Pino de controle da luz de fundo (backlight)
#define SCREEN_WIDTH 240      // Largura da tela em pixels


#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F
#define COLOR_YELLOW  0xFFE0
#define COLOR_ORANGE  0xFD20
#define COLOR_GRAY    0x8410

void drawPlay(int x, int y, int size, uint16_t color) {
    int x0 = x,       y0 = y;
    int x1 = x,       y1 = y + size;
    int x2 = x + size, y2 = y + size/2;

    // Preenchimento: percorre de cima até embaixo
    for (int yy = y0; yy <= y1; yy++) {
        // calcular interpolação dos lados
        float t = (float)(yy - y0) / (float)(y1 - y0);

        // lado esquerdo (linha entre x0,y0 e x1,y1) → sempre x = x
        int xa = x0;
        
        // lado direito (linha entre x0,y0 e x2,y2 até o meio, 
        // depois linha entre x1,y1 e x2,y2)
        int xb;
        if (yy < y2) {
            // parte superior
            float t2 = (float)(yy - y0) / (float)(y2 - y0);
            xb = x0 + (int)(t2 * (x2 - x0));
        } else {
            // parte inferior
            float t2 = (float)(yy - y2) / (float)(y1 - y2);
            xb = x2 + (int)(t2 * (x1 - x2));
        }

        // desenha linha horizontal de xa até xb
        if (xa > xb) { int tmp = xa; xa = xb; xb = tmp; }
        GFX_drawFastHLine(xa, yy, xb - xa + 1, color);
    }

    // borda do triângulo
    GFX_drawLine(x0, y0, x1, y1, COLOR_WHITE);
    GFX_drawLine(x1, y1, x2, y2, COLOR_WHITE);
    GFX_drawLine(x2, y2, x0, y0, COLOR_WHITE);
}


void drawStop(int x, int y, int size, uint16_t color) {
    // quadrado sólido
    GFX_fillRect(x, y, size, size, color);

    // borda branca para destacar
    GFX_drawRect(x, y, size, size, COLOR_WHITE);
}


void drawStep(int x, int y, int size, uint16_t color) {
    int x0 = x, y0 = y;
    int x1 = x, y1 = y + size;
    int x2 = x + size, y2 = y + size/2;

    // Preenchimento do triângulo
    for (int yy = y0; yy <= y1; yy++) {
        // lado esquerdo é sempre x0
        int xa = x0;
        int xb;

        if (yy < y2) {
            // parte superior (entre x0,y0 e x2,y2)
            float t = (float)(yy - y0) / (float)(y2 - y0);
            xb = x0 + (int)(t * (x2 - x0));
        } else {
            // parte inferior (entre x1,y1 e x2,y2)
            float t = (float)(yy - y2) / (float)(y1 - y2);
            xb = x2 + (int)(t * (x1 - x2));
        }

        if (xa > xb) { int tmp = xa; xa = xb; xb = tmp; }
        GFX_drawFastHLine(xa, yy, xb - xa + 1, color);
    }

    // Barrinha ao lado
    GFX_fillRect(x + size + 2, y, 6, size, color);

    // borda branca
    GFX_drawLine(x0, y0, x1, y1, COLOR_WHITE);
    GFX_drawLine(x1, y1, x2, y2, COLOR_WHITE);
    GFX_drawLine(x2, y2, x0, y0, COLOR_WHITE);
    GFX_drawRect(x + size + 2, y, 6, size, COLOR_WHITE);
}


// ⚙ COMPILE (caixa azul com texto "COMPILE")
// ⚙ COMPILE (caixa azul com texto "COMPILE")
void drawCompile(int x, int y, int w, int h) {
    // fundo azul
    GFX_fillRect(x, y, w, h, COLOR_BLUE);

    // borda branca
    GFX_drawRect(x, y, w, h, COLOR_WHITE);

    // texto centralizado (aproximado)
    int text_x = x + (w/2) - 24;  // ajuste fino horizontal
    int text_y = y + (h/2) - 4;   // ajuste fino vertical

    GFX_setCursor(text_x, text_y);
    GFX_setTextColor(COLOR_WHITE);
    GFX_printf("COMPILE");
}


 



int main(void) {
    stdio_init_all();         // Inicializa entrada/saída padrão (USB serial)

    // Inicialização do display LCD e sistema gráfico
    LCD_initDisplay();        // Inicializa o controlador do display
    LCD_setRotation(3);       // Define a rotação da tela (0 = retrato padrão)
    GFX_createFramebuf();     // Cria um framebuffer em memória para renderização

    configure_touch();        // Inicializa o sistema de leitura do touch resistivo

    // Configura o pino de backlight como saída e ativa a luz de fundo
    gpio_init(LITE);
    gpio_set_dir(LITE, GPIO_OUT);
    gpio_put(LITE, 1);        // Liga o backlight (nível alto)

    int px, py;               // Variáveis para armazenar coordenadas do toque
    // === Cores úteis ===
    int boxW = 120;
    int boxH = 30;
    int posX = (320 - boxW) / 2;  // centraliza horizontal
    int posY = 150;
    while (true) {
        GFX_clearScreen();

        // Título
        GFX_setCursor(120, 10);
        GFX_setTextColor(COLOR_WHITE);
        GFX_printf("Daniboy's Deck\n");

        // === Desenha ícones ===
        // Botão Debug (no lugar do Play)
        drawPlay(50, 50, 40, COLOR_GREEN);   // NOVO ÍCONE (substitui Play)
        drawStop(150, 50, 40, COLOR_RED);        
        drawStep(240, 50, 40, COLOR_YELLOW);

        // Botão Compile (abaixo da linha de ícones)
        drawCompile(posX, posY, boxW, boxH);

        // === Leitura do toque ===
        if (readPoint(&px, &py)) {
            GFX_setCursor(10, 200);
            GFX_setTextColor(COLOR_YELLOW);
            GFX_printf("X:%03d Y:%03d\n", px, py);

            // --- Área do botão Debug (quadrado centrado em 50,50 com lado=40) ---
            if (px<= 200 && px >=135 &&
                py >= (10) && py <= 150) {
                printf("DEBUG\n");   // Envia comando de Debug (F5) para o PC
                sleep_ms(100); // Debounce simples
            }

            // --- Área do botão Stop ---
            if (px >= (140) && px <= (240) &&
                py >= (149) && py <= (220)) {
                printf("STOP\n");
                sleep_ms(100); // Debounce simples
            }

            // --- Área do botão Step ---
            if (px >= (145) && px <= (240) &&
                py >= (230) && py <= (320)) {
                printf("STEP\n");
                sleep_ms(100); // Debounce simples

            }

            // --- Área do botão Compile (caixa definida por posX,posY,boxW,boxH) ---
            if (px >= 40 && px <= (100) &&
                py >= 80 && py <= (250)) {
                printf("COMPILAR\n");
                sleep_ms(100); // Debounce simples

            }
        }

        GFX_flush();
        sleep_ms(20);
    }

}