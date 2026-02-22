# WiFi no Raspberry Pi Pico W - Como Funciona

## Visão Geral

O Pico W possui o chip **CYW43439** que fornece conectividade WiFi 802.11n (2.4 GHz). A implementação usa:
- **SDK Pico** (versão 2.1.1) com driver CYW43
- **lwIP** (Lightweight IP stack) para TCP/IP
- **FreeRTOS** para multitarefa
- **HAL WiFi** (camada de abstração de hardware)

## Arquitetura

```
HAL WiFi (hal_wifi_rp2040.cpp)
         ↓
SDK CYW43 (pico_cyw43_arch_lwip_sys_freertos)
         ↓
lwIP Stack (TCP/IP)
         ↓
Driver CYW43439 (Hardware WiFi)
```

## Componentes Principais

### 1. HAL WiFi (`hal_wifi_rp2040.cpp`)

Camada de abstração que encapsula todas as funções do CYW43 SDK:

**Funções principais:**
- `init()` - Inicializa o chip WiFi com código de país
- `connect()` - Conecta a uma rede WiFi
- `disconnect()` - Desconecta da rede
- `scanNetworks()` - Escaneia redes disponíveis
- `getIPAddress()` - Obtém endereço IP
- `getMACAddress()` - Obtém endereço MAC
- `getRSSI()` - Mede intensidade do sinal
- `testDNSConnectivity()` - Testa resolução DNS
- `setHostname()` - Define nome do dispositivo
- `startAP()` / `stopAP()` - Modo Access Point
- `setLED()` - Controla LED do Pico W

**Características:**
- Thread-safe para FreeRTOS
- Retorna `bool` ou códigos de erro claros

### 2. lwIP Configuration (`lwipopts.h`)

Configurações críticas da pilha TCP/IP:

```c
#define MEM_SIZE                    20000   // Pool de memória (20KB) CRÍTICO
#define PBUF_POOL_SIZE              48      // Buffers de pacotes
#define MEMP_NUM_TCP_PCB            10      // Conexões TCP simultâneas
#define LWIP_NETCONN                1       // API Netconn habilitada
#define TCPIP_THREAD_STACKSIZE      1024    // Stack da thread TCP/IP
#define DEFAULT_THREAD_STACKSIZE    1024    // Stack padrão
```

### 3. FreeRTOS Integration

A biblioteca `pico_cyw43_arch_lwip_sys_freertos` integra:
- lwIP roda em thread separada (TCPIP thread)
- WiFi **deve** ser inicializado **após** `vTaskStartScheduler()`
- Callbacks são executados no contexto FreeRTOS

## Fluxo de Inicialização

### Passo 1: Inicialização
```cpp
wifi_hal = new HAL_WiFi_RP2040();
wifi_hal->init("BR");  // Código de país (Brasil)
```

**Internamente:**
1. `cyw43_arch_init_with_country()` - Inicializa driver
2. `cyw43_arch_enable_sta_mode()` - Ativa modo estação
3. Carrega firmware CYW43439
4. Configura país para conformidade regulatória

### Passo 2: Conexão
```cpp
wifi_hal->connect(SSID, PASSWORD, WiFiSecurityMode::WPA2_AES_PSK, 30000);
```

**Internamente:**
1. `cyw43_arch_wifi_connect_async()` - Inicia conexão assíncrona
2. Aguarda conexão com timeout
3. DHCP obtém endereço IP automaticamente
4. Retorna `true` se conectado

### Passo 3: Uso da Rede

Após conectado, usa-se APIs lwIP:
- **DNS:** `dns_gethostbyname()`
- **TCP:** `tcp_new()`, `tcp_connect()`, `tcp_write()`
- **UDP:** `udp_new()`, `udp_sendto()`

## Modos de Operação

### Station Mode (Cliente WiFi)
```cpp
wifi_hal->init("BR");
wifi_hal->connect("MinhaRede", "senha123", WiFiSecurityMode::WPA2_AES_PSK);
```

### Access Point Mode (Servidor WiFi)
```cpp
wifi_hal->init("BR");
wifi_hal->startAP("PicoW-AP", "senha123", WiFiSecurityMode::WPA2_AES_PSK, 6);
```

## Scan de Redes

```cpp
WiFiNetworkInfo networks[10];
int32_t count = wifi_hal->scanNetworks(networks, 10, 10000);

for (int i = 0; i < count; i++) {
    printf("SSID: %s, RSSI: %d dBm, Canal: %d\n", 
           networks[i].ssid, networks[i].rssi, networks[i].channel);
}
```

**Tipos de Segurança:**
- `OPEN` - Sem senha
- `WPA_TKIP_PSK` - WPA1
- `WPA2_AES_PSK` - WPA2 (mais comum)
- `WPA2_MIXED_PSK` - WPA2 modo misto

## DNS e Conectividade

```cpp
// Testa resolução DNS
if (wifi_hal->testDNSConnectivity("google.com", 5000)) {
    printf("DNS funcionando!\n");
}
```

Internamente usa `dns_gethostbyname()` para resolver hostname → IP.

## TCP Echo Server (Exemplo)

```cpp
struct tcp_pcb* pcb = tcp_new();
tcp_bind(pcb, IP_ADDR_ANY, 7);  // Porta 7
pcb = tcp_listen(pcb);
tcp_accept(pcb, accept_callback);

// Cliente conecta: telnet <pico-ip> 7
```

## Indicador LED

O LED do Pico W está conectado ao chip CYW43:

```cpp
wifi_hal->setLED(true);   // Acende
wifi_hal->setLED(false);  // Apaga
```

**Nota:** LED só funciona **após** `init()` WiFi.

## Medição de Sinal (RSSI)

```cpp
int32_t rssi = wifi_hal->getRSSI();
printf("Sinal: %d dBm\n", rssi);
```

**Valores típicos:**
- -30 a -50 dBm: Excelente
- -50 a -70 dBm: Bom
- -70 a -80 dBm: Fraco
- < -80 dBm: Muito fraco

## Configuração de País

Obrigatório para conformidade regulatória:

```cpp
wifi_hal->init("BR");  // Brasil
wifi_hal->init("US");  // Estados Unidos
wifi_hal->init("JP");  // Japão
```

Define canais WiFi permitidos e potência máxima de transmissão.

## Troubleshooting

### Problema: Panic "size > 0"
**Causa:** `MEM_SIZE` muito pequeno em `lwipopts.h`  
**Solução:** Aumentar para `20000` ou mais

### Problema: WiFi não inicializa
**Causa:** Inicialização antes do scheduler FreeRTOS  
**Solução:** Chamar `init()` dentro de uma task FreeRTOS

### Problema: Conexão timeout
**Causas possíveis:**
- SSID ou senha incorretos
- Rede fora de alcance (RSSI < -80 dBm)
- Modo de segurança incompatível
- Timeout muito curto (< 15000 ms)

### Problema: DNS falha
**Causas possíveis:**
- Não conectado à rede
- Servidor DNS não configurado
- Firewall bloqueando porta 53

## Referências

- **Pico SDK:** https://github.com/raspberrypi/pico-sdk
- **CYW43 Driver:** Incluído no Pico SDK
- **lwIP:** https://savannah.nongnu.org/projects/lwip/
- **Datasheet CYW43439:** Broadcom/Infineon
- **HAL WiFi:** `/src/hal/rp2040/hal_wifi_rp2040.cpp`
- **Config lwIP:** `/config/lwipopts.h`
