"""
Serviço de formatação e validação de dados.

Fornece funções puras para formatação de CPF, datas e outros dados,
independentes da lógica de apresentação do Telegram.
"""

import re
from datetime import datetime, date
from typing import List, Dict, Any, Optional
from core.constants import DATE_FORMAT, CPF_LENGTH


def clean_cpf(cpf: str) -> str:
    """
    Remove caracteres não numéricos do CPF.
    
    Args:
        cpf: CPF em qualquer formato (com ou sem pontuação)
        
    Returns:
        CPF contendo apenas dígitos
        
    Example:
        >>> clean_cpf("123.456.789-00")
        '12345678900'
        >>> clean_cpf("12345678900")
        '12345678900'
    """
    return re.sub(r'\D', '', cpf)


def format_cpf(cpf: str) -> str:
    """
    Formata CPF no padrão XXX.XXX.XXX-XX.
    
    Args:
        cpf: CPF com 11 dígitos (apenas números)
        
    Returns:
        CPF formatado com pontuação
        
    Example:
        >>> format_cpf("12345678900")
        '123.456.789-00'
    """
    if len(cpf) != CPF_LENGTH:
        return cpf
    return f"{cpf[:3]}.{cpf[3:6]}.{cpf[6:9]}-{cpf[9:]}"


def validate_cpf(cpf: str) -> bool:
    """
    Valida se o CPF tem formato válido (apenas verifica dígitos repetidos).
    
    Args:
        cpf: CPF limpo (apenas dígitos)
        
    Returns:
        True se válido, False se inválido
        
    Example:
        >>> validate_cpf("12345678900")
        True
        >>> validate_cpf("11111111111")
        False
        >>> validate_cpf("123")
        False
    """
    if len(cpf) != CPF_LENGTH:
        return False
    
    # Verifica se todos os dígitos são iguais
    if cpf == cpf[0] * CPF_LENGTH:
        return False
    
    return True


def format_date(date_obj: date) -> str:
    """
    Formata objeto date para string no formato brasileiro.
    
    Args:
        date_obj: Objeto date ou datetime
        
    Returns:
        Data formatada como DD/MM/AAAA
        
    Example:
        >>> from datetime import date
        >>> format_date(date(2025, 12, 25))
        '25/12/2025'
    """
    return date_obj.strftime(DATE_FORMAT)


def parse_date(date_str: str) -> Optional[date]:
    """
    Converte string no formato DD/MM/AAAA para objeto date.
    
    Args:
        date_str: Data como string (DD/MM/AAAA)
        
    Returns:
        Objeto date se válido, None se inválido
        
    Example:
        >>> parse_date("25/12/2025")
        datetime.date(2025, 12, 25)
        >>> parse_date("invalid") is None
        True
    """
    try:
        return datetime.strptime(date_str, DATE_FORMAT).date()
    except ValueError:
        return None


def get_category_name(category_id: int, categories: List[Dict[str, Any]]) -> str:
    """
    Busca o nome da categoria pelo ID.
    
    Args:
        category_id: ID da categoria a ser buscada
        categories: Lista de categorias disponíveis (dicts com 'id' e 'nome')
        
    Returns:
        Nome da categoria ou 'Categoria Desconhecida' se não encontrada
        
    Example:
        >>> cats = [{'id': 1, 'nome': 'Cardiologia'}, {'id': 2, 'nome': 'Pediatria'}]
        >>> get_category_name(1, cats)
        'Cardiologia'
        >>> get_category_name(999, cats)
        'Categoria Desconhecida'
    """
    category = next((c for c in categories if c['id'] == category_id), None)
    return category['nome'] if category else 'Categoria Desconhecida'


def get_weekday_name(date_obj: date) -> str:
    """
    Retorna o nome do dia da semana em português.
    
    Args:
        date_obj: Objeto date
        
    Returns:
        Nome do dia da semana (Segunda, Terça, etc)
        
    Example:
        >>> from datetime import date
        >>> get_weekday_name(date(2025, 12, 15))  # Segunda-feira
        'Segunda'
    """
    from core.constants import DIAS_SEMANA
    return DIAS_SEMANA[date_obj.weekday()]


def format_appointment_summary(dados: Dict[str, Any]) -> str:
    """
    Formata um resumo legível do agendamento.
    
    Args:
        dados: Dicionário com dados do agendamento
        
    Returns:
        String formatada com resumo da consulta
        
    Example:
        >>> dados = {
        ...     'nome': 'João Silva',
        ...     'cpf': '12345678900',
        ...     'profissional_nome': 'Dr. Pedro',
        ...     'profissional_categoria': 'Cardiologia',
        ...     'dia': '2025-12-25',
        ...     'horario': '14:30'
        ... }
        >>> summary = format_appointment_summary(dados)
        >>> 'João Silva' in summary
        True
    """
    cpf_formatado = format_cpf(dados['cpf'])
    data_obj = datetime.fromisoformat(dados['dia']).date()
    data_formatada = format_date(data_obj)
    
    return (
        f"📋 Confirme os dados:\n\n"
        f"👤 Paciente: {dados['nome']}\n"
        f"📄 CPF: {cpf_formatado}\n"
        f"🏥 Especialidade: {dados.get('profissional_categoria', 'N/A')}\n"
        f"👨‍⚕️ Profissional: {dados['profissional_nome']}\n"
        f"📅 Data: {data_formatada}\n"
        f"🕐 Horário: {dados['horario']}\n\n"
        f"✅ Tudo correto?"
    )
