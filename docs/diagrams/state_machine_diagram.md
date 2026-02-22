# State Machine Diagram

## 1. Fluxo Principal

Inicialização, check-in e tratamento de erros.

```mermaid
stateDiagram-v2
    [*] --> INITIALIZING : Power On

    INITIALIZING : Conecta WiFi, NTP e drivers

    DOWNLOADING_APPOINTMENTS : Autentica API e baixa agendamentos

    IDLE : Aguardando QR Code
    IDLE : Welcome Screen + data/hora

    VALIDATING : Parse JSON + valida agendamento

    APPOINTMENT : Exibe confirmação de check-in

    ERROR : Erro recuperável (3-5s)

    ERROR_CRITICAL : Falha de hardware / WiFi / SD

    RESTARTING : Reiniciando via watchdog

    %% Transições
    INITIALIZING --> DOWNLOADING_APPOINTMENTS : WiFi + NTP OK
    INITIALIZING --> ERROR_CRITICAL : Falha crítica

    DOWNLOADING_APPOINTMENTS --> IDLE : Download OK
    DOWNLOADING_APPOINTMENTS --> ERROR_CRITICAL : Falha persistente

    IDLE --> VALIDATING : QR Code recebido

    VALIDATING --> APPOINTMENT : Válida (sem biometria)
    VALIDATING --> FINGERPRINT : Requer biometria
    VALIDATING --> ERROR : Inválido

    state FINGERPRINT {
        direction LR
        [*] --> Ver_Diagrama_2
    }

    FINGERPRINT --> APPOINTMENT : Biometria OK
    FINGERPRINT --> ERROR : Biometria falhou

    APPOINTMENT --> IDLE : Timeout (5s)
    ERROR --> IDLE : Timeout (3s)
    ERROR_CRITICAL --> RESTARTING : Timeout ou botão
    RESTARTING --> [*] : Watchdog reset
```

---

## 2. Sub-estados Biométricos

Detalhamento do fluxo de fingerprint dentro do check-in.

```mermaid
stateDiagram-v2
    state "Biometria" as BIO {
        [*] --> DECISION

        state DECISION <<choice>>
        DECISION --> FINGERPRINT_VERIFYING : Template existe
        DECISION --> FINGERPRINT_ENROLLING : Sem template

        FINGERPRINT_VERIFYING : Carrega template do SD
        FINGERPRINT_VERIFYING : Captura dedo + compara

        FINGERPRINT_ENROLLING : Captura em 2 etapas
        FINGERPRINT_ENROLLING : Gera modelo + salva SD

        FINGERPRINT_UPLOADING : Envia template para API

        FINGERPRINT_VERIFYING --> [*] : Digital verificada
        FINGERPRINT_VERIFYING --> ERROR : Não reconhecida

        FINGERPRINT_ENROLLING --> FINGERPRINT_UPLOADING : Cadastro OK
        FINGERPRINT_ENROLLING --> ERROR : Falha no cadastro

        FINGERPRINT_UPLOADING --> [*] : Upload concluído/offline

        ERROR --> [*] : Retorna erro
    }
```

## Descrição dos Estados

| Estado | Descrição | Próximos Estados |
|--------|-----------|------------------|
| `INITIALIZING` | Boot do sistema, conexão WiFi e sincronização NTP | `DOWNLOADING_APPOINTMENTS`, `ERROR_CRITICAL` |
| `DOWNLOADING_APPOINTMENTS` | Autenticação na API e download dos agendamentos do dia | `IDLE`, `ERROR_CRITICAL` |
| `IDLE` | Aguardando leitura de QR Code na Welcome Screen | `VALIDATING` |
| `VALIDATING` | Processando e validando dados do QR Code | `FINGERPRINT_VERIFYING`, `FINGERPRINT_ENROLLING`, `APPOINTMENT`, `ERROR` |
| `FINGERPRINT_VERIFYING` | Verificando digital do paciente contra template armazenado | `APPOINTMENT`, `ERROR` |
| `FINGERPRINT_ENROLLING` | Cadastrando digital do paciente (captura em 2 etapas) | `FINGERPRINT_UPLOADING`, `ERROR` |
| `FINGERPRINT_UPLOADING` | Enviando template biométrico para API | `APPOINTMENT` |
| `APPOINTMENT` | Exibindo confirmação de check-in ao paciente | `IDLE` |
| `ERROR` | Erro recuperável (QR inválido, consulta não encontrada, digital falhou) | `IDLE` |
| `ERROR_CRITICAL` | Erro crítico que requer intervenção | `RESTARTING` |
| `RESTARTING` | Sistema reiniciando via watchdog | `INITIALIZING` |
