"""
Schemas de autenticação
"""
from pydantic import BaseModel


class LoginRequest(BaseModel):
    """Schema para requisição de login"""
    username: str
    password: str


class TokenResponse(BaseModel):
    """Schema para resposta com token"""
    access_token: str
    token_type: str = "bearer"
