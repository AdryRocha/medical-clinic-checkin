"""
Serviço de geração e validação de QR Codes para check-in.

Este módulo fornece funcionalidades para:
- Preparar dados de consultas para QR Code
- Gerar hash de validação antifraude
- Criar imagens PNG de QR Code
- Validar autenticidade de QR Codes
"""

from typing import Dict, Any
import json
import hashlib
import segno
from io import BytesIO


def prepare_qrcode_data(consulta: Dict[str, Any], paciente: Dict[str, Any], secret_key: str) -> Dict[str, Any]:
    """
    Prepara os dados mínimos para o QR code de check-in.
    
    Args:
        consulta: Dicionário com dados da consulta (deve conter 'id')
        paciente: Dicionário com dados do paciente (deve conter 'cpf' e 'nome')
        secret_key: Chave secreta para geração do hash de validação
        
    Returns:
        Dicionário para QR Code com hash de validação
        
    Example:
        >>> consulta = {'id': 123}
        >>> paciente = {'cpf': '12345678900', 'nome': 'João Silva'}
        >>> data = prepare_qrcode_data(consulta, paciente, 'secret')
        >>> 'hash' in data
        True
    """
    data = {
        "cmd": "checkin",
        "appt_id": consulta["id"],
        "cpf": paciente["cpf"],
        "name": paciente["nome"],
    }
    data["hash"] = generate_validation_hash(data, secret_key)
    return data


def generate_validation_hash(data: Dict[str, Any], secret_key: str) -> str:
    """
    Gera hash SHA256 dos dados + chave secreta, retorna os primeiros 16 caracteres.
    
    Args:
        data: Dicionário com os dados do QR (cmd, appt_id, cpf, name)
        secret_key: Chave secreta para geração do hash
        
    Returns:
        Hash antifraude (primeiros 16 caracteres do SHA256)
        
    Note:
        O hash é gerado concatenando: cmd:appt_id:cpf:name:secret_key
    """
    base = f"{data['cmd']}:{data['appt_id']}:{data['cpf']}:{data['name']}:{secret_key}"
    full_hash = hashlib.sha256(base.encode()).hexdigest()
    return full_hash[:16]


def gerar_qr_code_consulta(
    dados_qrcode: Dict[str, Any],
    versao: int | None = None
) -> bytes:
    """
    Gera imagem PNG de QR Code em memória para check-in no embarcado.
    
    Args:
        dados_qrcode: Dicionário já formatado para QR Code (com hash)
        versao: Força uma versão específica do QR Code (2-40), opcional
        
    Returns:
        Bytes da imagem PNG do QR Code
        
    Example:
        >>> data = {
        ...     'cmd': 'checkin',
        ...     'appt_id': 123,
        ...     'cpf': '12345678900',
        ...     'name': 'João',
        ...     'hash': 'abcd1234'
        ... }
        >>> qr_bytes = gerar_qr_code_consulta(data)
        >>> type(qr_bytes)
        <class 'bytes'>
    """
    conteudo_json = json.dumps(
        dados_qrcode,
        ensure_ascii=False,
        separators=(',', ':')
    )
    qr = segno.make(
        conteudo_json,
        error='M',
        micro=False,
        boost_error=False,
        version=versao
    )
    buffer = BytesIO()
    qr.save(buffer,
        kind='png',
        scale=8,
        border=4,
        dark='black',
        light='white',
    )
    buffer.seek(0)
    return buffer.getvalue()


def validate_qrcode_hash(qrcode_data: Dict[str, Any], secret_key: str) -> bool:
    """
    Valida o hash de um QR Code (útil para testes).
    
    Args:
        qrcode_data: Dados extraídos do QR Code (deve conter hash)
        secret_key: Chave secreta para validação do hash
        
    Returns:
        True se o hash é válido, False caso contrário
        
    Example:
        >>> data = {
        ...     'cmd': 'checkin',
        ...     'appt_id': 123,
        ...     'cpf': '12345678900',
        ...     'name': 'João',
        ...     'hash': generate_validation_hash({...}, 'secret')
        ... }
        >>> validate_qrcode_hash(data, 'secret')
        True
    """
    received_hash = qrcode_data.get('hash', '')
    data = {
        "cmd": qrcode_data.get("cmd"),
        "appt_id": qrcode_data.get("appt_id"),
        "cpf": qrcode_data.get("cpf"),
        "name": qrcode_data.get("name"),
    }
    calculated_hash = generate_validation_hash(data, secret_key)
    return received_hash == calculated_hash
