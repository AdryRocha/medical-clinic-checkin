# Display, HAL, Adapters e LVGL

## 📋 Visão Geral da Arquitetura

O sistema usa **3 camadas** para controlar o display:

```
┌─────────────────────────────────────────┐
│         LVGL (Biblioteca GUI)           │  ← Interface gráfica
├─────────────────────────────────────────┤
│      LVGLDisplayAdapter (Bridge)        │  ← Conecta LVGL ao driver
├─────────────────────────────────────────┤
│   ST7796Driver (Display Controller)     │  ← Controla o display
├─────────────────────────────────────────┤
│   HAL_SPI_RP2040 (Hardware Abstraction) │  ← Acessa hardware SPI
└─────────────────────────────────────────┘
```

---

## 🔧 1. HAL (Hardware Abstraction Layer)

### O que é?
Camada que **isola o hardware específico** (RP2040) do resto do código.

### Interface: `HAL_SPI_Interface`
```cpp
class HAL_SPI_Interface {
    virtual bool init(uint32_t baudrate) = 0;
    virtual size_t write(const uint8_t* data, size_t len) = 0;
    virtual void setCS(bool state) = 0;  // Chip Select
    virtual void setDC(bool state) = 0;  // Data/Command
    virtual void reset() = 0;
};
```

### Implementação: `HAL_SPI_RP2040`
- Usa **hardware/spi.h** do Pico SDK
- Configura pinos GPIO (MOSI, SCK, CS, DC, RST)
- Velocidade: até 20 MHz

**Por que existe?** 
- Trocar de microcontrolador (ex: STM32) só requer reimplementar o HAL
- Resto do código permanece igual

---

## 🖥️ 2. Driver de Display

### Interface: `DisplayInterface`
```cpp
class DisplayInterface {
    virtual bool init() = 0;
    virtual uint16_t getWidth() const = 0;
    virtual uint16_t getHeight() const = 0;
    virtual void drawPixels(uint16_t x1, uint16_t y1, 
                           uint16_t x2, uint16_t y2, 
                           const uint16_t* color_data) = 0;
    virtual void setRotation(uint8_t rotation) = 0;
};
```

### Implementação: `ST7796Driver`
- Controla display **ST7796** (480x320, RGB565)
- Envia **comandos** via SPI:
  - `CMD_SWRESET` - Reset de software
  - `CMD_CASET/RASET` - Define área de desenho
  - `CMD_RAMWR` - Escreve pixels
  - `CMD_MADCTL` - Controla rotação

**Método principal:**
```cpp
void ST7796Driver::drawPixels(uint16_t x1, uint16_t y1, 
                              uint16_t x2, uint16_t y2, 
                              const uint16_t* color_data) {
    setAddressWindow(x1, y1, x2, y2);  // Define região
    hal_spi_->write((uint8_t*)color_data, len * 2);  // Envia pixels
}
```

**Por que Interface?**
- Trocar para outro display (ILI9341, GC9A01) é fácil
- Basta criar novo driver implementando `DisplayInterface`

---

## 🔌 3. Adapter LVGL

### O que é?
**Ponte** entre LVGL (biblioteca genérica) e nosso driver específico.

### `LVGLDisplayAdapter`
```cpp
class LVGLDisplayAdapter {
private:
    DisplayInterface* display_;       // Nosso driver
    lv_disp_drv_t disp_drv_;         // Estrutura LVGL
    lv_disp_draw_buf_t draw_buf_;    // Buffers LVGL
    
    static void flushCallback(lv_disp_drv_t* disp_drv, 
                             const lv_area_t* area, 
                             lv_color_t* color_p);
};
```

### Como funciona?
1. **LVGL desenha** em buffers internos
2. Quando termina, chama `flushCallback()`
3. Callback pega os pixels e chama `display_->drawPixels()`
4. Pixels aparecem no display

**Inicialização:**
```cpp
// 1. Cria HAL
HAL_SPI_RP2040 hal_spi(spi0, pins...);

// 2. Cria driver
ST7796Driver display(&hal_spi, 480, 320);
display.init();

// 3. Cria adapter
LVGLDisplayAdapter adapter(&display, buf1, buf2, buf_size);
adapter.registerDisplay();

// 4. LVGL pronto para usar!
lv_obj_t* btn = lv_btn_create(lv_scr_act());
```

---

## 🎯 4. LVGL (Light and Versatile Graphics Library)

### Configuração: `lv_conf.h`
```c
#define LV_COLOR_DEPTH 16        // RGB565
#define LV_COLOR_16_SWAP 1       // Byte swap para SPI
#define LV_MEM_SIZE (48 * 1024)  // 48KB RAM para widgets
#define LV_TICK_CUSTOM 1         // Usa FreeRTOS tick
```

### Buffers
- **Buffer único**: Menor uso de RAM, mais lento
- **Double buffer**: Mais suave, usa 2x RAM

```cpp
#define LVGL_BUFFER_LINES 40  // 480 * 40 * 2 = ~38KB por buffer
lv_color_t buf1[480 * 40];
lv_color_t buf2[480 * 40];  // Opcional
```

### Com FreeRTOS
```cpp
void lvgl_task(void* param) {
    while (1) {
        xSemaphoreTake(lvgl_mutex, portMAX_DELAY);
        lv_timer_handler();  // Processa eventos e atualiza tela
        xSemaphoreGive(lvgl_mutex);
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
```

---

## 🔄 Fluxo Completo de Renderização

```
1. LVGL decide o que desenhar
   ↓
2. Renderiza em buf1/buf2 (RGB565)
   ↓
3. Chama flushCallback() do Adapter
   ↓
4. Adapter chama display_->drawPixels()
   ↓
5. ST7796Driver define janela (CASET/RASET)
   ↓
6. ST7796Driver envia via hal_spi_->write()
   ↓
7. HAL_SPI_RP2040 usa spi_write_blocking()
   ↓
8. Pixels aparecem no display físico!
```

---

## 🎨 Exemplo Prático

```cpp
// Setup (executar uma vez)
lv_init();

HAL_SPI_RP2040 hal_spi(spi0, mosi, miso, sck, cs, dc, rst);
ST7796Driver display(&hal_spi, 480, 320);
display.init();

LVGLDisplayAdapter adapter(&display, buf1, buf2, 480*40);
adapter.registerDisplay();

// Criar UI
lv_obj_t* label = lv_label_create(lv_scr_act());
lv_label_set_text(label, "Hello World!");
lv_obj_center(label);

// Loop (chamar repetidamente)
while (1) {
    lv_timer_handler();
    sleep_ms(5);
}
```

---

## 📁 Estrutura de Arquivos

```
src/
├── hal/
│   ├── interfaces/
│   │   └── hal_spi_interface.hpp      ← Interface abstrata
│   └── rp2040/
│       ├── hal_spi_rp2040.hpp         ← Implementação RP2040
│       └── hal_spi_rp2040.cpp
│
├── drivers/
│   └── display/
│       ├── interface/
│       │   └── display_interface.hpp   ← Interface abstrata
│       └── st7796/
│           ├── st7796_driver.hpp       ← Driver ST7796
│           └── st7796_driver.cpp
│
└── adapters/
    └── lvgl/
        ├── lvgl_display_adapter.hpp    ← Adapter LVGL
        └── lvgl_display_adapter.cpp

config/
└── lv_conf.h                           ← Configuração LVGL
```

---

## ⚡ Pontos-Chave

1. **Separação de responsabilidades**: cada camada tem uma função clara
2. **Interfaces abstratas**: facilita troca de hardware/displays
3. **Adapter pattern**: desacopla LVGL dos drivers específicos
4. **HAL isola hardware**: portabilidade entre microcontroladores
5. **LVGL é agnóstico**: não sabe nada sobre ST7796 ou RP2040

---

## 🐛 Troubleshooting

| Problema | Causa Provável | Solução |
|----------|---------------|---------|
| Tela branca | `flushCallback` não configurado | Verificar `adapter.registerDisplay()` |
| Cores erradas | Byte swap incorreto | `LV_COLOR_16_SWAP 1` no lv_conf.h |
| Tela lenta | Buffer muito pequeno | Aumentar `LVGL_BUFFER_LINES` |
| Crash | RAM insuficiente | Reduzir `LV_MEM_SIZE` ou buffers |
| SPI não funciona | Pinos errados | Conferir pin_config.hpp |
