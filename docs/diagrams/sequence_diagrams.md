# Sequence Diagrams

Diagramas de sequência do sistema de check-in médico.

---

## 1. Leitura do QR Code

Do scan até a fila FreeRTOS.

```mermaid
sequenceDiagram
    autonumber
    participant P as Paciente
    participant QR as GM67 Scanner
    participant QRT as task_qr_reader
    participant Q as FreeRTOS Queue

    Note over P,Q: Estado: IDLE

    P->>QR: Apresenta QR Code
    QR->>QRT: Dados via UART
    QRT->>QRT: Acumula bytes, valida JSON
    QRT->>Q: xQueueSend(qr_data)
```

---

## 2. Validacao do QR Code

Parse do JSON recebido.

```mermaid
sequenceDiagram
    autonumber
    participant Q as FreeRTOS Queue
    participant QRV as task_qr_validator
    participant AS as AppointmentService
    participant SM as StateMachine

    Q->>QRV: xQueueReceive(qr_data)
    QRV->>SM: setState(VALIDATING)
    QRV->>AS: validateAndParseQRCode(json)

    alt Parse falhou
        AS-->>QRV: false
        QRV->>SM: setState(ERROR) -> IDLE
    else Parse OK
        AS-->>QRV: QRCodeData
        Note over QRV: Prossegue para validacao do agendamento
    end
```

---

## 3. Validacao do Agendamento

Busca e validacao da consulta no SD Card.

```mermaid
sequenceDiagram
    autonumber
    participant QRV as task_qr_validator
    participant AS as AppointmentService
    participant SM as StateMachine

    QRV->>AS: validateAppointmentById(qr_data)
    Note over AS: Busca por ID, valida CPF, horario e status

    alt Invalido
        AS-->>QRV: false
        QRV->>SM: setState(ERROR) -> IDLE
    else Valido
        AS-->>QRV: AppointmentInfo
        Note over QRV: Prossegue (check-in ou biometria)
    end
```

---

## 4. Check-in com Sucesso

Fluxo apos validacao, incluindo decisao de biometria.

```mermaid
sequenceDiagram
    autonumber
    participant QRV as task_qr_validator
    participant AS as AppointmentService
    participant SM as StateMachine
    participant UI as UI Screens

    Note over QRV: AppointmentInfo valido

    alt requires_fingerprint_verification
        Note over QRV: -> Diagrama 6 (Verificacao)
    else requires_fingerprint_enrollment
        Note over QRV: -> Diagrama 7 (Cadastro)
    end

    QRV->>UI: appointment_screen(paciente, horario)
    QRV->>SM: setState(APPOINTMENT)

    QRV->>AS: markAppointmentCompleted(id)
    Note over AS: Atualiza SD Card (local) + API (remoto) em paralelo

    Note over UI: Timeout 5s
    QRV->>SM: setState(IDLE)
```

---

## 5. Tratamento de Erros

Cenarios de erro e retorno ao IDLE.

```mermaid
sequenceDiagram
    autonumber
    participant QRV as task_qr_validator
    participant SM as StateMachine
    participant UI as UI Screens

    alt QR invalido
        QRV->>UI: "QR Code invalido"
    else Consulta nao encontrada
        QRV->>UI: "Consulta nao encontrada"
    else Check-in ja realizado
        QRV->>UI: "Check-in ja realizado"
    else Digital nao reconhecida
        QRV->>UI: "Digital nao reconhecida"
    else Falha no cadastro
        QRV->>UI: "Falha no cadastro"
    end

    QRV->>SM: setState(ERROR)
    Note over UI: Timeout 3-5s
    QRV->>SM: setState(IDLE)
```

---

## 6. Verificacao Biometrica

Paciente ja tem template cadastrado -- verificacao 1:1.

```mermaid
sequenceDiagram
    autonumber
    participant QRV as task_qr_validator
    participant FPS as FingerprintService
    participant SM as StateMachine
    participant UI as UI Screens

    QRV->>SM: setState(FINGERPRINT_VERIFYING)
    QRV->>UI: fingerprint_screen(VERIFY)

    QRV->>FPS: verifyFingerprint(patient_id, 15s)
    Note over FPS: Carrega template do SD ao sensor<br/>Aguarda dedo, captura e compara

    alt Match OK
        FPS-->>QRV: true + confidence
        QRV->>UI: "Digital verificada"
        Note over QRV: Prossegue para check-in
    else Nao reconhecida / timeout
        FPS-->>QRV: false
        QRV->>SM: setState(ERROR) -> IDLE
    end
```

---

## 7. Cadastro Biometrico

Paciente sem template -- cadastro e upload para API.

```mermaid
sequenceDiagram
    autonumber
    participant QRV as task_qr_validator
    participant FPS as FingerprintService
    participant SM as StateMachine
    participant UI as UI Screens

    QRV->>SM: setState(FINGERPRINT_ENROLLING)
    QRV->>UI: fingerprint_screen(ENROLL)

    QRV->>FPS: enrollFingerprint(template_data, 30s)
    Note over FPS: Captura dedo (2 etapas)<br/>Cria modelo e extrai template

    alt Cadastro OK
        FPS-->>QRV: true + template_data
        Note over QRV: Salva template no SD Card
        QRV->>SM: setState(FINGERPRINT_UPLOADING)
        Note over QRV: Upload template para API<br/>(falha nao bloqueia check-in)
        Note over QRV: Prossegue para check-in
    else Cadastro falhou
        FPS-->>QRV: false
        QRV->>SM: setState(ERROR) -> IDLE
    end
```

---

## Fluxo Geral (Resumo)

```mermaid
flowchart LR
    A[1. Leitura QR] --> B[2. Validacao QR]
    B --> C[3. Validacao Agendamento]
    C --> D{Valido?}
    D -->|Nao| E[5. Erro]
    D -->|Sim| F{Biometria?}
    F -->|Verificar| G[6. Verificacao]
    F -->|Cadastrar| H[7. Cadastro]
    F -->|Nao| I[4. Check-in OK]
    G -->|OK| I
    G -->|Falha| E
    H -->|OK| I
    H -->|Falha| E
    I --> J[Fim]
    E --> J

    style A fill:#e3f2fd
    style B fill:#e3f2fd
    style C fill:#fff9c4
    style D fill:#fff9c4
    style E fill:#ffcdd2
    style F fill:#f3e5f5
    style G fill:#f3e5f5
    style H fill:#f3e5f5
    style I fill:#c8e6c9
```
