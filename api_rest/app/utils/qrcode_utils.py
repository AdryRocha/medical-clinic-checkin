"""
Utilitários para geração e manipulação de QR Codes
"""
import segno
import io
import base64
import hashlib
import json
from typing import Dict, Any
from app.core.config import settings


def gerar_dados_qrcode(dados_input: Dict[str, Any]) -> Dict[str, Any]:
    """
    Gera os dados estruturados para o QR Code com validação.
    
    Args:
        dados_input: Dados da consulta (appt_id, cpf, name)
    
    Returns:
        Dicionário com dados do QR Code e hash de validação
    """
    dados = {
        "cmd": "checkin",
        **dados_input
    }
    
    base = f"{dados['cmd']}:{dados['appt_id']}:{dados['cpf']}:{dados['name']}:{settings.QR_SECRET_KEY}"
    hash_completo = hashlib.sha256(base.encode()).hexdigest()
    dados["hash"] = hash_completo[:16]
    
    return dados


def validar_dados_qrcode(dados: Dict[str, Any]) -> bool:
    """
    Valida a autenticidade dos dados do QR Code.
    
    Args:
        dados: Dicionário extraído do QR Code
    
    Returns:
        True se válido, False caso contrário
    """
    try:
        hash_recebido = dados.get('hash', '')
        
        base = f"{dados['cmd']}:{dados['appt_id']}:{dados['cpf']}:{dados['name']}:{settings.QR_SECRET_KEY}"
        hash_esperado = hashlib.sha256(base.encode()).hexdigest()[:16]
        
        return hash_recebido == hash_esperado
        
    except (KeyError, TypeError) as e:
        raise ValueError(f"Dados de QR Code incompletos: {str(e)}")


def criar_qrcode_image(token: str) -> str:
    """
    Cria imagem QR Code e retorna em base64
    """
    qr = segno.make(token, micro=False, error='m', boost_error=False)
    
    buffer = io.BytesIO()
    qr.save(
        buffer,
        kind='png',
        scale=10,
        border=4,
        dark='#000000',
        light='#FFFFFF'
    )
    
    img_str = base64.b64encode(buffer.getvalue()).decode()
    
    return img_str
