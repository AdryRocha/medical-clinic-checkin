"""
Rotas de autenticação JWT
"""
from fastapi import APIRouter, HTTPException, status
from datetime import timedelta
from app.core.security import criar_token_acesso
from app.core.config import settings
from app.schemas.auth import LoginRequest, TokenResponse


router = APIRouter(prefix="/auth", tags=["Autenticação"])


@router.post("/token", response_model=TokenResponse)
async def login(credentials: LoginRequest):
    """
    Endpoint de autenticação para obter token JWT.
    
    Retorna um token de acesso válido para uso nas requisições autenticadas.
    """
    users = {
        settings.ADMIN_USERNAME: {
            "password": settings.ADMIN_PASSWORD,
            "role": "admin",
            "permissions": ["*"]
        },
        settings.BOT_USERNAME: {
            "password": settings.BOT_PASSWORD,
            "role": "bot",
            "permissions": ["read:categorias", "read:profissionais", "read:horarios", 
                          "write:pacientes", "write:consultas", "read:consultas"]
        },
        settings.DEVICE_USERNAME: {
            "password": settings.DEVICE_PASSWORD,
            "role": "device",
            "permissions": ["read:consultas", "update:consultas_status"]
        }
    }
    
    if credentials.username not in users:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Credenciais inválidas",
            headers={"WWW-Authenticate": "Bearer"},
        )
    
    user = users[credentials.username]
    if user["password"] != credentials.password:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Credenciais inválidas",
            headers={"WWW-Authenticate": "Bearer"},
        )
    
    access_token = criar_token_acesso(
        data={
            "sub": credentials.username,
            "role": user["role"],
            "permissions": user["permissions"]
        },
        expires_delta=timedelta(days=30)
    )
    
    return TokenResponse(access_token=access_token)
