"""
Constantes globais do bot de agendamento médico.

Define estados da conversa, formatos de data e outras constantes
usadas em todo o sistema.
"""

# ============================================================================
# ESTADOS DA CONVERSA
# ============================================================================

CPF = 0
"""Estado: Aguardando CPF do paciente"""

NAME = 1
"""Estado: Aguardando nome completo do paciente"""

ACCEPTS_DIGITAL = 2
"""Estado: Aguardando autorização para coleta de digital biométrica"""

CATEGORY = 3
"""Estado: Aguardando seleção de categoria/especialidade médica"""

PROFESSIONAL = 4
"""Estado: Aguardando seleção de profissional"""

DAY = 5
"""Estado: Aguardando seleção de data da consulta"""

TIME_SLOT = 6
"""Estado: Aguardando seleção de horário"""

CONFIRM = 7
"""Estado: Aguardando confirmação final do agendamento"""

RECOVER_CPF = 8
"""Estado: Aguardando CPF para buscar consultas existentes"""

SELECT_CONSULTATION = 9
"""Estado: Aguardando seleção de consulta para gerar QR code"""


# ============================================================================
# FORMATOS DE DATA E HORA
# ============================================================================

DATE_FORMAT = "%d/%m/%Y"
"""Formato de data para exibição e entrada do usuário (ex: 25/12/2025)"""

DATETIME_FORMAT = "%d/%m/%Y %H:%M"
"""Formato de data e hora completo (ex: 25/12/2025 14:30)"""


# ============================================================================
# CONSTANTES DE CALENDÁRIO
# ============================================================================

DIAS_SEMANA = [
    'Segunda',
    'Terça',
    'Quarta',
    'Quinta',
    'Sexta',
    'Sábado',
    'Domingo'
]
"""Lista de nomes dos dias da semana (índice 0 = Segunda-feira)"""


# ============================================================================
# LIMITES E VALIDAÇÕES
# ============================================================================

CPF_LENGTH = 11
"""Tamanho esperado do CPF (apenas dígitos)"""

MIN_NAME_LENGTH = 2
"""Tamanho mínimo aceitável para nome do paciente"""

MAX_DAYS_AHEAD = 30
"""Máximo de dias no futuro para buscar horários disponíveis"""

NEXT_AVAILABLE_SEARCH_DAYS = 14
"""Quantidade de dias para buscar próxima data disponível"""
