# R307S Fingerprint Sensor - Como Funciona

## O que é o Sensor R307S?

O R307S é um sensor de impressão digital que funciona como um "mini computador especializado" em reconhecer dedos. Ele tem sua própria memória interna onde guarda as impressões digitais cadastradas (até 1000 fingerprints).

**Analogia**: Pense no sensor como um cofre digital. Ele:
- Tira "fotos" do seu dedo
- Transforma essas fotos em "códigos secretos" (templates)
- Guarda esses códigos na sua memória interna
- Compara novos dedos com os códigos guardados

## Conexão e Hardware

### Pinos de Conexão
```
Sensor R307S          Raspberry Pi Pico
┌─────────────┐       ┌─────────────┐
│  VCC (Red)  │ ───── │ 5V          │
│  GND (Black)│ ───── │ GND         │
│  TX (White) │ ───── │ GPIO 5 (RX) │
│  RX (Green) │ ───── │ GPIO 4 (TX) │
└─────────────┘       └─────────────┘
```

**Importante**: 
- VCC precisa de **5V**
- A comunicação é **serial UART** a 57600 baud
- TX do sensor conecta no RX do Pico (e vice-versa)

### Protocolo de Comunicação

O sensor não entende português - ele só fala "pacotes". Cada pacote tem uma estrutura específica:

```
┌────────┬─────────┬─────┬────────┬──────────┬──────────┐
│ Header │ Address │ PID │ Length │   Data   │ Checksum │
│  2B    │   4B    │ 1B  │   2B   │    nB    │    2B    │
└────────┴─────────┴─────┴────────┴──────────┴──────────┘
```

**Explicação dos campos**:

1. **Header (0xEF01)**: É como dizer "Atenção, começa uma mensagem aqui!"
2. **Address (0xFFFFFFFF)**: Endereço do sensor (padrão aceita qualquer)
3. **PID (Package Identifier)**:
   - `0x01` = Comando (você mandando uma ordem)
   - `0x07` = Resposta (sensor te respondendo)
   - `0x02` / `0x08` = Dados grandes sendo enviados em partes
4. **Length**: Tamanho do que vem depois
5. **Data**: O comando/dados em si
6. **Checksum**: Soma para verificar se a mensagem não foi corrompida

## Como o Sensor Processa uma Digital

### Entendendo o Fluxo Completo

O sensor não guarda "fotos" das digitais. Ele trabalha em 3 etapas:

```
1. CAPTURAR           2. EXTRAIR              3. GUARDAR/COMPARAR
   [Foto]     ──►    [Características]   ──►    [Template]
                      (pontos únicos)           (código final)
```

**Passo a passo detalhado**:

#### 1️⃣ Captura de Imagem (`getImage()`)
O sensor tira uma "foto" do dedo colocado no sensor.
- **Tempo**: Espera até 5 segundos para detectar um dedo
- **Retorno**: OK se conseguiu, ERROR_NO_FINGER se ninguém colocou o dedo

#### 2️⃣ Extração de Características (`image2Tz(slot)`)
Transforma a foto em características únicas (pontos marcantes da digital).

**O que é o "slot"?**
- O sensor tem 2 "gavetas" temporárias de memória (CharBuffer1 e CharBuffer2)
- Slot 1 = CharBuffer1
- Slot 2 = CharBuffer2
- Você precisa escolher em qual gaveta vai guardar temporariamente

**Por que 2 gavetas?**
- Para cadastrar: você compara 2 scans do mesmo dedo
- Para verificar: você compara o dedo atual com um template guardado

#### 3️⃣ Criar Modelo (`createModel()`)
Combina as características das 2 gavetas em um modelo final confiável.
- Pega CharBuffer1 + CharBuffer2 → cria template final
- Se os 2 scans forem muito diferentes, falha (ERROR_MERGE_FAIL)

#### 4️⃣ Guardar na Memória (`storeModel(id)`)
Salva o template final na memória permanente do sensor.
- Você escolhe um ID (1 a 1000)
- O sensor guarda esse template com esse ID

---

## As 3 Operações Principais

### 🆕 CADASTRAR (Enroll)

**Objetivo**: Cadastrar uma nova impressão digital no banco de dados do sensor.

**Fluxograma**:
```
┌─────────────────────────────────────────────┐
│ 1. Coloque o dedo                           │
│    └─► getImage() → image2Tz(slot=1)       │
├─────────────────────────────────────────────┤
│ 2. REMOVA o dedo                            │
│    └─► Aguarda sensor não detectar dedo    │
├─────────────────────────────────────────────┤
│ 3. Coloque o MESMO dedo novamente           │
│    └─► getImage() → image2Tz(slot=2)       │
├─────────────────────────────────────────────┤
│ 4. Combinar os 2 scans                      │
│    └─► createModel()                        │
├─────────────────────────────────────────────┤
│ 5. Salvar no ID escolhido                   │
│    └─► storeModel(id)                       │
└─────────────────────────────────────────────┘
```

**Por que escanear 2 vezes?**
- Aumenta a precisão (evita falsos positivos)
- Se você colocar o dedo de forma diferente nas 2 vezes, o sensor percebe e rejeita
- O modelo final é uma "média" dos 2 scans

**Código**:
```cpp
// Método de alto nível (recomendado - já faz tudo)
FingerprintStatus status = fp_sensor->enrollFingerprint(100);  // Cadastra no ID 100

// Método de baixo nível (controle manual)
fp_sensor->getImage();           // 1. Primeira captura
fp_sensor->image2Tz(1);          //    Guarda na gaveta 1

// Usuário remove e recoloca o dedo

fp_sensor->getImage();           // 2. Segunda captura  
fp_sensor->image2Tz(2);          //    Guarda na gaveta 2
fp_sensor->createModel();        // 3. Combina gaveta 1 + gaveta 2
fp_sensor->storeModel(100);      // 4. Salva como ID 100
```

---

### 🔍 BUSCAR (Search - 1:N)

**Objetivo**: "Quem é essa pessoa?" - busca em TODAS as digitais cadastradas.

**Fluxograma**:
```
┌─────────────────────────────────────────────┐
│ 1. Coloque o dedo                           │
│    └─► getImage() → image2Tz(slot=1)       │
├─────────────────────────────────────────────┤
│ 2. Buscar em TODO o banco de dados          │
│    └─► fingerFastSearch()                   │
│        ├─► Se encontrou: retorna ID         │
│        └─► Se não: ERROR_NO_MATCH           │
└─────────────────────────────────────────────┘
```

**O que acontece internamente?**
1. Sensor compara CharBuffer1 com TODOS os templates salvos
2. Se achar correspondência: retorna o ID + score de confiança
3. Se não achar ninguém: retorna ERROR_NO_MATCH

**Código**:
```cpp
// Método de alto nível
FingerprintMatch match;
FingerprintStatus status = fp_sensor->matchFingerprint(match);

if (status == FingerprintStatus::OK && match.matched) {
    printf("Encontrado: ID %d (confiança: %d)\n", match.id, match.confidence);
}

// Método de baixo nível
fp_sensor->getImage();
fp_sensor->image2Tz(1);
FingerprintMatch match;
fp_sensor->fingerFastSearch(match);  // Busca no banco todo
```

**Confiança (Confidence Score)**:
- Valor entre 0-65535 (quanto maior, melhor a correspondência)
- Valores típicos: 100-200 = boa correspondência
- Abaixo de 50 = duvidoso

---

### ✅ VERIFICAR (Verify - 1:1)

**Objetivo**: "Esse dedo é realmente do ID 100?" - compara com 1 digital específica.

**Diferença para BUSCAR**:
- **Buscar (Search)**: "Quem é você?" (compara com todos)
- **Verificar (Verify)**: "Você é o João (ID 100)?" (compara só com 1)

**Fluxograma**:
```
┌─────────────────────────────────────────────┐
│ 1. Coloque o dedo                           │
│    └─► getImage() → image2Tz(slot=1)       │
├─────────────────────────────────────────────┤
│ 2. Carregar template do ID específico       │
│    └─► loadTemplate(id, slot=2)            │
├─────────────────────────────────────────────┤
│ 3. Comparar gaveta 1 com gaveta 2           │
│    └─► compareTemplates()                   │
│        ├─► Match: retorna confiança         │
│        └─► No match: ERROR_NO_MATCH         │
└─────────────────────────────────────────────┘
```

**Código**:
```cpp
// Verificar se o dedo atual é do ID 100
fp_sensor->getImage();
fp_sensor->image2Tz(1);                    // Dedo atual → CharBuffer1
fp_sensor->loadTemplate(100, 2);           // ID 100 → CharBuffer2

uint16_t confidence = 0;
FingerprintStatus status = fp_sensor->compareTemplates(confidence);

if (status == FingerprintStatus::OK) {
    printf("Verificado! Confiança: %d\n", confidence);
} else {
    printf("NÃO é o ID 100\n");
}
```

---

## Gerenciamento do Banco de Dados

### Quantos templates estão salvos?
```cpp
uint16_t count;
fp_sensor->getTemplateCount(count);
printf("Total cadastrados: %d\n", count);
```

### Deletar uma digital específica
```cpp
fp_sensor->deleteModel(100);  // Deleta o ID 100
```

### Limpar TUDO
```cpp
fp_sensor->emptyDatabase();  // APAGA TODAS as digitais!
```

### Ver configurações do sensor
```cpp
uint16_t status_reg, sys_id, lib_size, sec_level;
fp_sensor->readSysPara(status_reg, sys_id, lib_size, sec_level);

printf("Capacidade: %d templates\n", lib_size);      // Ex: 1000
printf("Nível de segurança: %d\n", sec_level);       // 1-5 (quanto maior, mais rigoroso)
```

---

## Códigos de Status (Erros Comuns)

| Código | Nome | O que significa | O que fazer |
|--------|------|-----------------|-------------|
| `0x00` | `OK` | ✅ Sucesso | Tudo certo! |
| `0x02` | `ERROR_NO_FINGER` | Nenhum dedo detectado | Coloque o dedo no sensor |
| `0x06` | `ERROR_BAD_IMAGE` | Imagem não ficou boa | Limpe o dedo e tente de novo |
| `0x07` | `ERROR_TOO_MESSY` | Imagem muito confusa | Dedo pode estar sujo ou molhado |
| `0x08` | `ERROR_FEATURE_FAIL` | Não conseguiu extrair características | Pressione o dedo com mais força |
| `0x09` | `ERROR_NO_MATCH` | Nenhuma correspondência | Dedo não está cadastrado |
| `0x0A` | `ERROR_NOT_FOUND` ou `ERROR_MERGE_FAIL` | **No cadastro**: Os 2 scans estão muito diferentes<br>**Na busca**: ID não existe | Use a mesma posição do dedo<br>ou verifique se o ID existe |
| `0x10` | `ERROR_DELETE_FAIL` | Falha ao deletar | ID pode não existir |
| `0x11` | `ERROR_CLEAR_FAIL` | Falha ao limpar banco | Problema no sensor |
| `0xFF` | `ERROR_TIMEOUT` | Operação demorou muito | Verifique a conexão |

---

## 🔧 Configurações Importantes

**Nível de Segurança (1-5)**:
- **1**: Muito permissivo (aceita fácil, mais falsos positivos)
- **3**: Balanceado (padrão recomendado)
- **5**: Muito rigoroso (pode rejeitar o próprio dono)

**Baud Rate**:
- Padrão: 57600
- Pode ser mudado, mas 57600 é o mais confiável

**Capacidade**:
- R307S típico: 1000 templates
- Cada template ocupa ~512 bytes na memória do sensor

---

## Diferenças: Buscar vs Verificar

| Aspecto | **BUSCAR (Search)** | **VERIFICAR (Verify)** |
|---------|---------------------|------------------------|
| **Pergunta** | "Quem é você?" | "Você é o João?" |
| **Compara com** | TODOS os templates | 1 template específico |
| **Função** | `fingerFastSearch()` | `loadTemplate() + compareTemplates()` |
| **Velocidade** | Mais lento (1-2 segundos) | Rápido (milissegundos) |
| **Uso típico** | Identificação (ex: relógio de ponto) | Autenticação (ex: confirmar senha) |
| **Retorna** | ID encontrado | Match/No match |

**Quando usar cada um?**

- **Search**: Quando você quer saber QUEM é a pessoa
  - Exemplo: Portaria, relógio de ponto, lista de presença
  
- **Verify**: Quando você já sabe QUEM deveria ser, só quer confirmar
  - Exemplo: "Confirme sua identidade para fazer esta operação", 2FA

---
