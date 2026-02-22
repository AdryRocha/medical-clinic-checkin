#pragma once

// ------------------------------------------------------------
// ESTADOS DO SISTEMA
// ------------------------------------------------------------
enum class CheckinState {
    AGUARDANDO_QR,              // Tela inicial
    PROCESSANDO_DADOS_PACIENTE, // Validando leitura
    
    // Fluxo Cadastro
    REGISTRO_INICIADO,
    COLETANDO_DIGITAL_CADASTRO,
    PERSISTINDO_DADOS,
    CADASTRO_CONCLUIDO,

    // Fluxo Consulta
    VALIDACAO_BIOMETRICA,
    CONSULTA_CONFIRMADA,

    // Erros
    EM_ERRO
};

// ------------------------------------------------------------
// EVENTOS DO SISTEMA
// ------------------------------------------------------------
enum class CheckinEvent {
    // UI e QR
    INICIAR_CHECKIN,
    QR_CODE_LIDO,
    DADOS_INVALIDOS,

    // Lógica de Negócio
    PACIENTE_NAO_ENCONTRADO,
    PACIENTE_ENCONTRADO,

    // Biometria
    DIGITAL_COLETADA,
    DIGITAL_MATCH_OK,
    DIGITAL_MATCH_FAIL,

    // Sistema
    ARQUIVO_SALVO_SUCESSO,
    ERRO_GENERICO,
    TIMEOUT_INTERFACE
};
   
