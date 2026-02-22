# Como Usar SD Card com FatFS no RP2040

## 📖 O que é FatFS?

**FatFS** é uma biblioteca de sistema de arquivos FAT/exFAT genérica e completa para sistemas embarcados. Ela permite que você trabalhe com cartões SD da mesma forma que trabalha com arquivos em um computador - criar, ler, escrever, deletar arquivos e diretórios.

**Biblioteca usada:** [carlk3/no-OS-FatFS-SD-SPI-RPi-Pico](https://github.com/carlk3/no-OS-FatFS-SD-SPI-RPi-Pico)
- ✅ Produção-ready
- ✅ Thread-safe para FreeRTOS
- ✅ Suporte completo a FAT32
- ✅ API padrão POSIX-like

---

## 🔌 Configuração de Hardware

### Pinagem do SD Card

Seu projeto está configurado com os seguintes pinos (veja `config/hw_config.c`):

| Sinal | GPIO | Descrição |
|-------|------|-----------|
| MISO  | 12   | Master In Slave Out (dados do SD para Pico) |
| MOSI  | 15   | Master Out Slave In (dados do Pico para SD) |
| SCK   | 14   | SPI Clock (sincronização) |
| CS    | 13   | Chip Select (habilita o SD card) |

### Preparando o SD Card

1. **Formate como FAT32** (importante!)
2. Insira no módulo SD
3. Conecte os pinos conforme a tabela acima

---

## 🚀 Inicialização Básica

### Passo 1: Incluir os Headers

```cpp
#include "f_util.h"      // Utilitários do FatFS
#include "ff.h"          // API principal do FatFS
#include "hw_config.h"   // Configuração de hardware
```

### Passo 2: Montar o Sistema de Arquivos

Antes de fazer qualquer operação, você precisa **montar** o SD card:

```cpp
// Obter a instância do SD card (definida em hw_config.c)
sd_card_t *pSD = sd_get_by_num(0);  // 0 = primeiro (e único) SD card

// Montar o filesystem
FRESULT fr = f_mount(&pSD->fatfs, pSD->pcName, 1);

if (FR_OK != fr) {
    printf("ERRO ao montar: %s (%d)\n", FRESULT_str(fr), fr);
    // Verifique: SD inserido? Formatado FAT32? Fiação correta?
} else {
    printf("SD card montado com sucesso!\n");
}
```

**O que acontece aqui:**
- `sd_get_by_num(0)` - Pega a configuração do SD card definida em `hw_config.c`
- `f_mount()` - Monta o sistema de arquivos FAT
- `FR_OK` - Código de retorno indicando sucesso

---

## 📝 Operações Comuns com Arquivos

### 1. Criar e Escrever em um Arquivo

```cpp
FIL fil;  // Estrutura de arquivo
FRESULT fr;

// Abrir arquivo para escrita (cria se não existir)
fr = f_open(&fil, "dados.txt", FA_WRITE | FA_CREATE_ALWAYS);

if (FR_OK == fr) {
    // Escrever usando f_printf (similar ao printf)
    f_printf(&fil, "Paciente: João Silva\n");
    f_printf(&fil, "ID: 12345\n");
    f_printf(&fil, "Data: 09/11/2025\n");
    
    // Sempre fechar o arquivo!
    f_close(&fil);
    printf("Arquivo criado com sucesso!\n");
} else {
    printf("Erro ao criar arquivo: %s\n", FRESULT_str(fr));
}
```

**Modos de abertura disponíveis:**
- `FA_READ` - Somente leitura
- `FA_WRITE` - Somente escrita
- `FA_CREATE_NEW` - Cria novo (falha se já existe)
- `FA_CREATE_ALWAYS` - Cria novo (sobrescreve se existe)
- `FA_OPEN_ALWAYS` - Abre existente ou cria novo
- `FA_OPEN_APPEND` - Abre para adicionar no final

Você pode combinar com `|`:
```cpp
FA_WRITE | FA_CREATE_ALWAYS  // Escrever, criar novo (sobrescreve)
FA_WRITE | FA_OPEN_APPEND    // Escrever no final do arquivo
```

### 2. Ler um Arquivo Linha por Linha

```cpp
FIL fil;
FRESULT fr;

fr = f_open(&fil, "dados.txt", FA_READ);

if (FR_OK == fr) {
    char linha[128];
    
    // Ler linha por linha até o fim do arquivo
    while (f_gets(linha, sizeof(linha), &fil)) {
        printf("Lido: %s", linha);  // linha já inclui \n
    }
    
    f_close(&fil);
} else {
    printf("Erro ao abrir arquivo: %s\n", FRESULT_str(fr));
}
```

### 3. Ler Bytes Específicos

```cpp
FIL fil;
FRESULT fr;

fr = f_open(&fil, "dados.bin", FA_READ);

if (FR_OK == fr) {
    uint8_t buffer[512];
    UINT bytes_lidos;
    
    // Ler até 512 bytes
    fr = f_read(&fil, buffer, sizeof(buffer), &bytes_lidos);
    
    if (FR_OK == fr) {
        printf("Lidos %u bytes\n", bytes_lidos);
        // Use os dados em buffer...
    }
    
    f_close(&fil);
}
```

### 4. Adicionar Dados no Final (Append)

```cpp
FIL fil;
FRESULT fr;

// Abrir para append - adiciona no final sem apagar o conteúdo
fr = f_open(&fil, "log.txt", FA_WRITE | FA_OPEN_APPEND);

if (FR_OK == fr) {
    // Obter timestamp atual (você precisa implementar isso)
    uint32_t tempo_ms = to_ms_since_boot(get_absolute_time());
    
    f_printf(&fil, "[%lu] Paciente check-in realizado\n", tempo_ms);
    
    f_close(&fil);
}
```

**Exemplo prático - Log de eventos:**

```cpp
void salvar_log(const char* mensagem) {
    FIL fil;
    
    if (f_open(&fil, "eventos.log", FA_WRITE | FA_OPEN_APPEND) == FR_OK) {
        uint32_t tempo = to_ms_since_boot(get_absolute_time());
        f_printf(&fil, "[%lu ms] %s\n", tempo, mensagem);
        f_close(&fil);
    }
}

// Uso:
salvar_log("Sistema iniciado");
salvar_log("Leitor QR conectado");
salvar_log("Paciente 12345 fez check-in");
```

### 5. Escrever Dados Binários

```cpp
FIL fil;
FRESULT fr;

// Estrutura de exemplo
struct Paciente {
    uint32_t id;
    char nome[64];
    uint8_t idade;
};

Paciente p = {12345, "João Silva", 45};

fr = f_open(&fil, "paciente.dat", FA_WRITE | FA_CREATE_ALWAYS);

if (FR_OK == fr) {
    UINT bytes_escritos;
    
    // Escrever estrutura inteira
    fr = f_write(&fil, &p, sizeof(Paciente), &bytes_escritos);
    
    if (FR_OK == fr && bytes_escritos == sizeof(Paciente)) {
        printf("Dados binários salvos: %u bytes\n", bytes_escritos);
    }
    
    f_close(&fil);
}
```

---

## 📁 Operações com Arquivos e Diretórios

### 6. Verificar se Arquivo Existe

```cpp
FILINFO fno;
FRESULT fr;

fr = f_stat("arquivo.txt", &fno);

if (FR_OK == fr) {
    printf("Arquivo existe!\n");
    printf("Tamanho: %lu bytes\n", (unsigned long)fno.fsize);
} else if (FR_NO_FILE == fr) {
    printf("Arquivo não existe\n");
}
```

### 7. Obter Informações do Arquivo

```cpp
FILINFO fno;

if (f_stat("dados.txt", &fno) == FR_OK) {
    printf("Nome: %s\n", fno.fname);
    printf("Tamanho: %lu bytes\n", (unsigned long)fno.fsize);
    
    // Data de modificação
    int ano = 1980 + ((fno.fdate >> 9) & 0x7F);
    int mes = (fno.fdate >> 5) & 0x0F;
    int dia = fno.fdate & 0x1F;
    printf("Data: %04d-%02d-%02d\n", ano, mes, dia);
    
    // Hora de modificação
    int hora = (fno.ftime >> 11) & 0x1F;
    int min = (fno.ftime >> 5) & 0x3F;
    int seg = (fno.ftime & 0x1F) * 2;
    printf("Hora: %02d:%02d:%02d\n", hora, min, seg);
    
    // Atributos
    if (fno.fattrib & AM_DIR) printf("É um diretório\n");
    if (fno.fattrib & AM_RDO) printf("Somente leitura\n");
    if (fno.fattrib & AM_HID) printf("Oculto\n");
}
```

### 8. Deletar um Arquivo

```cpp
FRESULT fr = f_unlink("arquivo_velho.txt");

if (FR_OK == fr) {
    printf("Arquivo deletado\n");
} else {
    printf("Erro ao deletar: %s\n", FRESULT_str(fr));
}
```

### 9. Renomear/Mover Arquivo

```cpp
// Renomear
FRESULT fr = f_rename("nome_velho.txt", "nome_novo.txt");

// Mover para diretório
fr = f_rename("arquivo.txt", "pasta/arquivo.txt");
```

### 10. Criar Diretório

```cpp
FRESULT fr = f_mkdir("pacientes");

if (FR_OK == fr) {
    printf("Diretório criado\n");
} else if (FR_EXIST == fr) {
    printf("Diretório já existe\n");
}
```

### 11. Listar Arquivos de um Diretório

```cpp
DIR dir;
FILINFO fno;
FRESULT fr;

// Abrir diretório raiz
fr = f_opendir(&dir, "/");

if (FR_OK == fr) {
    printf("Conteúdo do SD card:\n");
    
    // Ler cada entrada
    while (true) {
        fr = f_readdir(&dir, &fno);
        
        // Parar quando não houver mais entradas
        if (fr != FR_OK || fno.fname[0] == 0) break;
        
        // Mostrar informação
        if (fno.fattrib & AM_DIR) {
            printf("  [DIR]  %s\n", fno.fname);
        } else {
            printf("  [FILE] %-20s %8lu bytes\n", 
                   fno.fname, (unsigned long)fno.fsize);
        }
    }
    
    f_closedir(&dir);
}
```

**Listar diretório específico:**

```cpp
DIR dir;
FILINFO fno;

if (f_opendir(&dir, "pacientes") == FR_OK) {
    while (f_readdir(&dir, &fno) == FR_OK && fno.fname[0] != 0) {
        printf("%s\n", fno.fname);
    }
    f_closedir(&dir);
}
```

---

## 💾 Informações de Armazenamento

### 12. Espaço Livre no SD Card

```cpp
FATFS *fs;
DWORD clusters_livres;
FRESULT fr;

// Obter informações do filesystem
fr = f_getfree("0:", &clusters_livres, &fs);

if (FR_OK == fr) {
    // Calcular setores
    DWORD total_setores = (fs->n_fatent - 2) * fs->csize;
    DWORD setores_livres = clusters_livres * fs->csize;
    
    // Converter para MB (cada setor = 512 bytes)
    DWORD total_mb = total_setores / 2048;
    DWORD livre_mb = setores_livres / 2048;
    DWORD usado_mb = total_mb - livre_mb;
    
    printf("SD Card:\n");
    printf("  Total: %lu MB\n", total_mb);
    printf("  Livre: %lu MB\n", livre_mb);
    printf("  Usado: %lu MB\n", usado_mb);
    
    // Percentual de uso
    float uso_pct = ((total_setores - setores_livres) * 100.0f) / total_setores;
    printf("  Uso: %.1f%%\n", uso_pct);
}
```

---

## 🎯 Exemplos Práticos para Sistema Médico

### Exemplo 1: Salvar Check-in em CSV

```cpp
bool salvar_checkin(const char* paciente_id, const char* nome) {
    FIL fil;
    
    // Abrir CSV em modo append
    FRESULT fr = f_open(&fil, "checkins.csv", FA_WRITE | FA_OPEN_APPEND);
    
    if (FR_OK != fr) {
        // Se falhou, talvez seja porque não existe - criar com cabeçalho
        fr = f_open(&fil, "checkins.csv", FA_WRITE | FA_CREATE_NEW);
        if (FR_OK == fr) {
            f_printf(&fil, "Timestamp,ID_Paciente,Nome,Status\n");
        } else {
            return false;
        }
    }
    
    // Obter timestamp
    uint32_t tempo_ms = to_ms_since_boot(get_absolute_time());
    
    // Escrever linha do CSV
    f_printf(&fil, "%lu,%s,%s,CHECKED_IN\n", tempo_ms, paciente_id, nome);
    
    f_close(&fil);
    return true;
}

// Uso:
salvar_checkin("12345", "João Silva");
salvar_checkin("67890", "Maria Santos");
```

### Exemplo 2: Salvar Dados de Paciente

```cpp
bool salvar_dados_paciente(const char* id, const char* dados_json) {
    FIL fil;
    char caminho[80];
    
    // Garantir que diretório existe
    f_mkdir("pacientes");
    
    // Criar nome do arquivo
    snprintf(caminho, sizeof(caminho), "pacientes/%s.json", id);
    
    // Salvar arquivo JSON
    FRESULT fr = f_open(&fil, caminho, FA_WRITE | FA_CREATE_ALWAYS);
    
    if (FR_OK == fr) {
        f_printf(&fil, "%s", dados_json);
        f_close(&fil);
        return true;
    }
    
    return false;
}

// Uso:
const char* dados = "{\n"
    "  \"id\": \"12345\",\n"
    "  \"nome\": \"João Silva\",\n"
    "  \"idade\": 45,\n"
    "  \"telefone\": \"11-98765-4321\"\n"
    "}";
    
salvar_dados_paciente("12345", dados);
```

### Exemplo 3: Ler Dados de Paciente

```cpp
bool ler_dados_paciente(const char* id, char* buffer, size_t buffer_size) {
    FIL fil;
    char caminho[80];
    
    snprintf(caminho, sizeof(caminho), "pacientes/%s.json", id);
    
    FRESULT fr = f_open(&fil, caminho, FA_READ);
    
    if (FR_OK == fr) {
        UINT bytes_lidos;
        fr = f_read(&fil, buffer, buffer_size - 1, &bytes_lidos);
        
        if (FR_OK == fr) {
            buffer[bytes_lidos] = '\0';  // Terminar string
            f_close(&fil);
            return true;
        }
        
        f_close(&fil);
    }
    
    return false;
}

// Uso:
char dados[512];
if (ler_dados_paciente("12345", dados, sizeof(dados))) {
    printf("Dados do paciente:\n%s\n", dados);
}
```

### Exemplo 4: Classe Gerenciadora de Logs

```cpp
class LogManager {
public:
    LogManager(const char* arquivo_log) : arquivo_(arquivo_log) {
        // Montar SD card
        sd_card_t *pSD = sd_get_by_num(0);
        FRESULT fr = f_mount(&pSD->fatfs, pSD->pcName, 1);
        montado_ = (FR_OK == fr);
    }
    
    bool log(const char* nivel, const char* mensagem) {
        if (!montado_) return false;
        
        FIL fil;
        FRESULT fr = f_open(&fil, arquivo_, FA_WRITE | FA_OPEN_APPEND);
        
        if (FR_OK != fr) return false;
        
        uint32_t tempo = to_ms_since_boot(get_absolute_time());
        f_printf(&fil, "[%lu][%s] %s\n", tempo, nivel, mensagem);
        
        f_close(&fil);
        return true;
    }
    
    bool info(const char* msg) { return log("INFO", msg); }
    bool warning(const char* msg) { return log("WARN", msg); }
    bool error(const char* msg) { return log("ERROR", msg); }
    
private:
    const char* arquivo_;
    bool montado_;
};

// Uso:
LogManager logger("sistema.log");
logger.info("Sistema iniciado");
logger.info("QR Scanner conectado");
logger.warning("Fingerprint sensor não respondendo");
logger.error("Falha ao salvar dados");
```

---

## ⚠️ Tratamento de Erros

### Códigos de Retorno Comuns

```cpp
FRESULT fr = f_open(&fil, "arquivo.txt", FA_READ);

switch (fr) {
    case FR_OK:
        printf("Sucesso\n");
        break;
    case FR_NO_FILE:
        printf("Arquivo não encontrado\n");
        break;
    case FR_NO_PATH:
        printf("Caminho não encontrado\n");
        break;
    case FR_DENIED:
        printf("Acesso negado\n");
        break;
    case FR_DISK_ERR:
        printf("Erro no disco\n");
        break;
    case FR_NOT_READY:
        printf("SD card não está pronto\n");
        break;
    default:
        printf("Erro: %s (%d)\n", FRESULT_str(fr), fr);
        break;
}
```

### Função Helper para Erros

```cpp
const char* obter_mensagem_erro(FRESULT fr) {
    switch (fr) {
        case FR_OK: return "Sucesso";
        case FR_DISK_ERR: return "Erro de disco";
        case FR_INT_ERR: return "Erro interno";
        case FR_NOT_READY: return "Disco não está pronto";
        case FR_NO_FILE: return "Arquivo não encontrado";
        case FR_NO_PATH: return "Caminho não encontrado";
        case FR_INVALID_NAME: return "Nome de arquivo inválido";
        case FR_DENIED: return "Acesso negado";
        case FR_EXIST: return "Já existe";
        case FR_INVALID_OBJECT: return "Objeto inválido";
        case FR_WRITE_PROTECTED: return "Protegido contra escrita";
        case FR_INVALID_DRIVE: return "Drive inválido";
        case FR_NOT_ENABLED: return "Não habilitado";
        case FR_NO_FILESYSTEM: return "Sem sistema de arquivos";
        default: return "Erro desconhecido";
    }
}
```

---

## 🔒 Uso com FreeRTOS (Multi-threading)

### Proteger Acesso com Mutex

Se você usa FatFS em múltiplas tasks, proteja com mutex:

```cpp
#include "FreeRTOS.h"
#include "semphr.h"

// Mutex global
static SemaphoreHandle_t sd_mutex = NULL;

// Inicializar no início
void inicializar_sd() {
    sd_mutex = xSemaphoreCreateMutex();
    
    // Montar filesystem...
}

// Usar em cada operação
bool escrever_log_seguro(const char* mensagem) {
    bool sucesso = false;
    
    // Adquirir mutex (espera até 1 segundo)
    if (xSemaphoreTake(sd_mutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        
        // Operação segura aqui
        FIL fil;
        if (f_open(&fil, "log.txt", FA_WRITE | FA_OPEN_APPEND) == FR_OK) {
            f_printf(&fil, "%s\n", mensagem);
            f_close(&fil);
            sucesso = true;
        }
        
        // Liberar mutex
        xSemaphoreGive(sd_mutex);
    }
    
    return sucesso;
}
```

---

## 🧪 Testando o SD Card

Veja o arquivo demo completo em: `src/tests/code_examples/sdcard_fatfs_demo.cpp`

Para rodar o demo:

1. Edite `CMakeLists.txt` na raiz do projeto:
```cmake
set(BUILD_TARGET "SDCARD_DEMO" CACHE STRING "...")
```

2. Compile e faça upload
3. Abra o monitor serial
4. Veja os testes sendo executados

---

## 📚 Referências

- **FatFS Official**: http://elm-chan.org/fsw/ff/00index_e.html
- **carlk3's Library**: https://github.com/carlk3/no-OS-FatFS-SD-SPI-RPi-Pico
- **Demo completo**: `src/tests/code_examples/sdcard_fatfs_demo.cpp`
- **Configuração HW**: `config/hw_config.c`

---

## 🎓 Resumo das Funções Mais Usadas

| Operação | Função | Exemplo |
|----------|--------|---------|
| Montar SD | `f_mount()` | `f_mount(&fs, "0:", 1)` |
| Abrir arquivo | `f_open()` | `f_open(&fil, "file.txt", FA_READ)` |
| Fechar arquivo | `f_close()` | `f_close(&fil)` |
| Ler bytes | `f_read()` | `f_read(&fil, buf, 100, &br)` |
| Ler linha | `f_gets()` | `f_gets(linha, 128, &fil)` |
| Escrever bytes | `f_write()` | `f_write(&fil, data, len, &bw)` |
| Escrever formatado | `f_printf()` | `f_printf(&fil, "ID: %d\n", id)` |
| Info do arquivo | `f_stat()` | `f_stat("file.txt", &fno)` |
| Deletar | `f_unlink()` | `f_unlink("old.txt")` |
| Renomear | `f_rename()` | `f_rename("a.txt", "b.txt")` |
| Criar diretório | `f_mkdir()` | `f_mkdir("pasta")` |
| Abrir diretório | `f_opendir()` | `f_opendir(&dir, "/")` |
| Ler entrada dir | `f_readdir()` | `f_readdir(&dir, &fno)` |
| Espaço livre | `f_getfree()` | `f_getfree("0:", &free, &fs)` |

**Pronto! Agora você sabe como usar FatFS para todas as operações comuns de arquivo! 🎉**
