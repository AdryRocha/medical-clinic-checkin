# Como Funciona o Sistema de Leitura QR Code (GM67)

## Índice
1. [Visão Geral](#visão-geral)
2. [Hardware - GM67](#hardware---gm67)
3. [Camada HAL - UART](#camada-hal---uart)
4. [Driver GM67](#driver-gm67)

---

## Visão Geral

O sistema é dividido em 2 camadas principais:

```
┌─────────────────────────────────────┐
│  DRIVER (gm67_driver.cpp)           │  ← Protocolo GM67
│  - Envia comandos AT para scanner   │
│  - Processa dados recebidos         │
│  - Chama callback quando lê QR      │
└─────────────────────────────────────┘
              ↓
┌─────────────────────────────────────┐
│  HAL (hal_uart_rp2040.cpp)          │  ← Hardware abstrato
│  - Envia/recebe bytes pela UART     │
│  - Específico do RP2040             │
└─────────────────────────────────────┘
              ↓
┌─────────────────────────────────────┐
│  HARDWARE (GM67 Scanner)            │  ← Dispositivo físico
└─────────────────────────────────────┘
```

**Por que essa separação?**
- **HAL**: Isola o hardware. Se trocar de microcontrolador (RP2040 → STM32), só muda o HAL.
- **Driver**: Isolado do hardware. Funciona em qualquer plataforma que tenha UART HAL.

---

## Hardware - GM67

### O que é?

O GM67 é um **módulo scanner de QR code e códigos de barras** que se comunica via UART (serial).

### Características

- **Comunicação**: UART (serial)
- **Velocidade padrão**: 115200 baud (alguns modelos vêm em 9600)
- **Protocolo**: Comandos AT (como modem)
- **Alimentação**: 5V 
- **Pinos Pico W**:
  - VCC (vermelho): 5V
  - GND (preto): terra
  - TX (branco): transmite dados para o scanner
  - RX (verde): recebe comandos do scanner

### Como funciona fisicamente?

1. **Você liga o scanner** (5V + GND)
2. **LED acende** (scanner está ligado)
3. **Aponta para um QR code**
4. **LED muda de cor/pisca** (está lendo)
5. **Scanner envia os dados pela UART** (pelo pino TX)
6. **Você recebe os dados** (pelo pino RX do microcontrolador)

### Modos de Operação

O GM67 tem 2 modos principais:

#### Modo 1: Contínuo (Auto-scan)
```
Scanner fica sempre ligado
    ↓
Aponta para QR code
    ↓
Lê automaticamente
    ↓
Envia dados pela UART
```

**Comando**: `AT+MODE=1\r\n`

#### Modo 2: Trigger (Sob demanda)
```
Scanner fica em standby
    ↓
Você envia comando AT+SCAN
    ↓
Scanner lê uma vez
    ↓
Volta para standby
```

**Comando**: `AT+MODE=0\r\n`

### Protocolo de Comunicação

O GM67 usa **comandos AT** (tipo modem antigo):

| Comando | Descrição |
|---------|-----------|
| `AT+MODE=1\r\n` | Modo contínuo |
| `AT+MODE=0\r\n` | Modo trigger |
| `AT+SCAN\r\n` | Scan uma vez |
| `AT+STOP\r\n` | Para de escanear |

**Formato dos dados recebidos:**
```
DADOS_DO_QR_CODE\r\n
```

Exemplo: Se você escanear um QR com "TESTE123", o scanner envia:
```
T E S T E 1 2 3 \r \n
```

O `\r\n` (CR+LF) indica o fim dos dados.

---

## Camada HAL - UART

### O que é HAL?

HAL = **Hardware Abstraction Layer** (Camada de Abstração de Hardware)

**Propósito**: Esconder os detalhes do hardware para que o driver seja portável.

### Estrutura

```cpp
// Interface abstrata (não depende de hardware)
class HAL_UART_Interface {
    virtual bool init(baudrate, ...) = 0;
    virtual size_t write(data, len) = 0;
    virtual size_t read(data, len) = 0;
    virtual size_t available() = 0;
    // ...
};

// Implementação para RP2040
class HAL_UART_RP2040 : public HAL_UART_Interface {
    // Usa funções específicas do RP2040 SDK
    uart_init(uart_instance, baudrate);
    uart_write_blocking(...);
    uart_read_blocking(...);
};
```

### Como funciona?

1. **Você cria uma instância do HAL**
```cpp
HAL_UART_RP2040* uart = new HAL_UART_RP2040(uart1, GPIO_TX, GPIO_RX);
```

2. **Inicializa a UART com parâmetros**
```cpp
uart->init(
    115200,  // baudrate (velocidade)
    8,       // data bits
    1,       // stop bits
    0        // parity (sem paridade)
);
```

3. **HAL configura o hardware do RP2040**
```cpp
uart_init(uart1, 115200);
uart_set_format(uart1, 8, 1, UART_PARITY_NONE);
gpio_set_function(GPIO_TX, GPIO_FUNC_UART);
gpio_set_function(GPIO_RX, GPIO_FUNC_UART);
```

4. **Agora você pode usar métodos abstratos**
```cpp
uart->write(data, len);     // Envia bytes
uart->read(buffer, len);    // Recebe bytes
uart->available();          // Quantos bytes tem para ler
```

### Por que usar HAL?

**Sem HAL** (código acoplado ao hardware):
```cpp
// Driver diretamente usa funções do RP2040
uart_write_blocking(uart1, data, len);  // Só funciona no RP2040!
```

**Com HAL** (código portável):
```cpp
// Driver usa interface abstrata
uart_hal->write(data, len);  // Funciona em qualquer plataforma!
```

Se você trocar de microcontrolador (RP2040 → STM32), basta criar um novo HAL:
```cpp
class HAL_UART_STM32 : public HAL_UART_Interface {
    // Implementa com funções da STM32
    HAL_UART_Transmit(...);
    HAL_UART_Receive(...);
};
```

O driver **não muda nada**!

---

## Driver GM67

### O que faz?

O driver é responsável por:
1. Enviar comandos AT para o scanner
2. Receber dados da UART
3. Processar os dados (identificar fim de linha, etc.)
4. Chamar callback quando um QR code completo é lido

### Estrutura

```cpp
class GM67_Driver : public QR_Interface {
private:
    HAL_UART_Interface* uart_hal_;  // Interface abstrata (não sabe se é RP2040 ou STM32)
    ScanCallback scan_callback_;    // Função chamada quando lê QR
    char rx_buffer_[256];           // Buffer para dados recebidos
    size_t rx_index_;               // Posição atual no buffer
    
public:
    GM67_Driver(HAL_UART_Interface* uart_hal);
    
    bool init();
    bool setContinuousMode();
    void setScanCallback(ScanCallback callback);
    void process();
};
```

### Fluxo de Funcionamento

#### 1. Inicialização

```cpp
bool GM67_Driver::init() {
    // Limpa buffer UART (dados antigos)
    uart_hal_->clearRxBuffer();
    
    // Aguarda scanner estabilizar
    sleep_ms(100);
    
    is_initialized_ = true;
    return true;
}
```

**Não inicializa UART!** O HAL já foi inicializado antes.

#### 2. Configuração do Modo

```cpp
bool GM67_Driver::setContinuousMode() {
    return sendCommand("AT+MODE=1\r\n");
}

bool GM67_Driver::sendCommand(const char* command) {
    // Converte string para bytes
    // Envia pela UART usando o HAL
    uart_hal_->write((uint8_t*)command, strlen(command));
    uart_hal_->flush();
}
```

Quando você chama `setContinuousMode()`:
1. Driver monta o comando "AT+MODE=1\r\n"
2. Envia para o scanner via HAL
3. Scanner recebe e entra em modo contínuo

#### 3. Processamento de Dados

```cpp
void GM67_Driver::process() {
    // Verifica se tem bytes disponíveis na UART
    while (uart_hal_->available()) {
        // Lê um byte
        char ch = uart_hal_->readByte();
        
        // Processa o caractere
        processChar(ch);
    }
}
```

```cpp
void GM67_Driver::processChar(char ch) {
    // Se é fim de linha (\r ou \n)
    if (ch == '\r' || ch == '\n') {
        if (rx_index_ > 0) {
            // Termina a string
            rx_buffer_[rx_index_] = '\0';
            
            // Chama callback com os dados
            handleScanComplete();
            
            // Reseta buffer para próxima leitura
            rx_index_ = 0;
        }
        return;
    }
    
    // Adiciona caractere ao buffer
    if (rx_index_ < BUFFER_SIZE - 1) {
        rx_buffer_[rx_index_++] = ch;
    }
}
```

**Exemplo prático:**

Scanner lê QR code "TESTE" e envia: `T E S T E \r \n`

```
Byte recebido: 'T'  → rx_buffer_ = "T"       rx_index_ = 1
Byte recebido: 'E'  → rx_buffer_ = "TE"      rx_index_ = 2
Byte recebido: 'S'  → rx_buffer_ = "TES"     rx_index_ = 3
Byte recebido: 'T'  → rx_buffer_ = "TEST"    rx_index_ = 4
Byte recebido: 'E'  → rx_buffer_ = "TESTE"   rx_index_ = 5
Byte recebido: '\r' → Fim! Chama callback com "TESTE"
```

#### 4. Callback

```cpp
void GM67_Driver::handleScanComplete() {
    if (scan_callback_) {
        std::string data(rx_buffer_, rx_index_);
        scan_callback_(data);  // Chama sua função!
    }
}
```
---