"""
Funções auxiliares para o fluxo de agendamento.

Contém lógica específica do fluxo de conversa do bot,
como buscar próximas datas disponíveis e gerar dicas.
"""

import logging
from datetime import date, timedelta
from typing import Dict, Any, Optional, Tuple
from core.constants import DIAS_SEMANA, NEXT_AVAILABLE_SEARCH_DAYS, MAX_DAYS_AHEAD
from services.formatting import format_date, get_weekday_name

logger = logging.getLogger(__name__)


async def find_next_available_slot(
    api_client,
    profissional_id: int,
    start_date: date,
    max_days: int = MAX_DAYS_AHEAD
) -> Optional[Tuple[date, str]]:
    """
    Busca a próxima data e horário disponível para um profissional.
    
    Args:
        api_client: Cliente da API para buscar horários
        profissional_id: ID do profissional
        start_date: Data inicial para busca
        max_days: Máximo de dias para buscar (padrão: 30)
        
    Returns:
        Tupla (data, primeiro_horario) se encontrar, None caso contrário
        
    Example:
        >>> slot = await find_next_available_slot(api, 1, date.today())
        >>> if slot:
        ...     data, horario = slot
        ...     print(f"Disponível em {data} às {horario}")
    """
    for i in range(1, max_days + 1):
        data_teste = start_date + timedelta(days=i)
        
        try:
            result = await api_client.buscar_horarios_disponiveis(
                profissional_id,
                data_teste.isoformat()
            )
            
            if result['success'] and result['data']:
                horarios_disponiveis = [
                    h for h in result['data'] 
                    if h.get('disponivel', True)
                ]
                
                if horarios_disponiveis:
                    primeiro_horario = horarios_disponiveis[0]['horario']
                    return (data_teste, primeiro_horario)
                    
        except Exception as e:
            logger.warning(f"Erro ao buscar horário para {data_teste}: {e}")
            continue
    
    return None


async def get_next_available_hint(
    api_client,
    profissional_id: int,
    days_ahead: int = NEXT_AVAILABLE_SEARCH_DAYS
) -> str:
    """
    Gera uma dica sobre a próxima data disponível.
    
    Args:
        api_client: Cliente da API
        profissional_id: ID do profissional
        days_ahead: Quantos dias buscar (padrão: 14)
        
    Returns:
        String com dica formatada ou string vazia
        
    Example:
        >>> hint = await get_next_available_hint(api, 1)
        >>> print(hint)
        '💡 Dica: Segunda (15/12/2025) tem horários disponíveis!'
    """
    hoje = date.today()
    slot = await find_next_available_slot(api_client, profissional_id, hoje, days_ahead)
    
    if slot:
        data, horario = slot
        dia_semana = get_weekday_name(data)
        data_formatada = format_date(data)
        return f"\n\n💡 Dica: {dia_semana} ({data_formatada}) tem horários disponíveis!"
    
    return ""


async def get_next_available_suggestion(
    api_client,
    profissional_id: int,
    current_date: date,
    max_days: int = MAX_DAYS_AHEAD
) -> str:
    """
    Gera sugestão completa de próximo horário disponível.
    
    Args:
        api_client: Cliente da API
        profissional_id: ID do profissional
        current_date: Data atual que não tinha horários
        max_days: Máximo de dias para buscar
        
    Returns:
        String com sugestão formatada ou string vazia
        
    Example:
        >>> sugestao = await get_next_available_suggestion(api, 1, date.today())
        >>> print(sugestao)
        '💡 Próximo disponível: Segunda, 15/12/2025 às 14:30'
    """
    slot = await find_next_available_slot(api_client, profissional_id, current_date, max_days)
    
    if slot:
        data, primeiro_horario = slot
        dia_semana = get_weekday_name(data)
        data_formatada = format_date(data)
        return f"\n\n💡 Próximo disponível: {dia_semana}, {data_formatada} às {primeiro_horario}"
    
    return ""


def build_professional_display_text(professional: Dict[str, Any], categories: list) -> str:
    """
    Constrói o texto de exibição de um profissional.
    
    Args:
        professional: Dicionário com dados do profissional
        categories: Lista de categorias disponíveis
        
    Returns:
        String formatada "Nome - Especialidade"
        
    Example:
        >>> prof = {'nome': 'Dr. João', 'categoria_id': 1}
        >>> cats = [{'id': 1, 'nome': 'Cardiologia'}]
        >>> build_professional_display_text(prof, cats)
        'Dr. João - Cardiologia'
    """
    from services.formatting import get_category_name
    categoria_nome = get_category_name(professional['categoria_id'], categories)
    return f"{professional['nome']} - {categoria_nome}"


def find_professional_by_text(
    text: str,
    professionals: list,
    categories: list
) -> Optional[Dict[str, Any]]:
    """
    Encontra um profissional pelo texto de exibição.
    
    Args:
        text: Texto selecionado pelo usuário
        professionals: Lista de profissionais
        categories: Lista de categorias
        
    Returns:
        Dicionário do profissional ou None se não encontrado
        
    Example:
        >>> text = "Dr. João - Cardiologia"
        >>> prof = find_professional_by_text(text, profissionais, categorias)
        >>> prof['nome']
        'Dr. João'
    """
    for prof in professionals:
        display_text = build_professional_display_text(prof, categories)
        if display_text == text:
            return prof
    return None
