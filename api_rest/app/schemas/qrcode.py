"""
Schemas Pydantic para QR Code
"""
from pydantic import BaseModel


class QRCodeResponse(BaseModel):
    """Schema de resposta para geração de QR Code"""
    consulta_id: int
    qr_code: str
    
    class Config:
        from_attributes = True
