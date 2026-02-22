"""
Módulo mock - Dados simulados para testes sem API.

Contém dados mockados e utilitários de QR Code usados apenas
para testes e desenvolvimento local.
"""

from .mock_qrcode_service import (
    prepare_qrcode_data,
    generate_validation_hash,
    gerar_qr_code_consulta,
    validate_qrcode_hash
)

__all__ = [
    'prepare_qrcode_data',
    'generate_validation_hash',
    'gerar_qr_code_consulta',
    'validate_qrcode_hash'
]