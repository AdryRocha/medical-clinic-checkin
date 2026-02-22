"""
Rotas da API para operações com QR Code
"""
from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy.orm import Session
from app.core.database import get_db
from app.models.consulta import Consulta
from app.schemas.qrcode import QRCodeResponse
from app.utils.qrcode_utils import gerar_dados_qrcode, criar_qrcode_image
import json

router = APIRouter(prefix="/qrcode", tags=["QR Code"])


@router.get("/generate/{consulta_id}", response_model=QRCodeResponse)
async def gerar_qrcode(consulta_id: int, db: Session = Depends(get_db)):
    """
    Gera um QR Code para check-in da consulta.
    
    Retorna o QR Code codificado em base64 para ser exibido ao paciente.
    """
    consulta = db.query(Consulta).filter(Consulta.id == consulta_id).first()
    if not consulta:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Consulta com ID {consulta_id} não encontrada"
        )
    
    dados_qrcode = gerar_dados_qrcode({
        "appt_id": consulta.id,
        "cpf": consulta.paciente.cpf,
        "name": consulta.paciente.nome
    })
    
    qrcode_json = json.dumps(dados_qrcode, ensure_ascii=False, separators=(',', ':'))
    
    qr_image_base64 = criar_qrcode_image(qrcode_json)
    
    return {
        "consulta_id": consulta.id,
        "qr_code": qr_image_base64
    }
