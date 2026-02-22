"""
Configurações da API REST
"""
import re
from pydantic import Field, validator
from pydantic_settings import BaseSettings


class Settings(BaseSettings):
    """
    Configurações da aplicação

    OBRIGATÓRIAS (devem estar no .env):
    - DATABASE_URL
    - JWT_SECRET_KEY
    - JWT_ALGORITHM
    - JWT_ACCESS_TOKEN_EXPIRE_MINUTES
    - QR_SECRET_KEY
    - ADMIN_USERNAME, ADMIN_PASSWORD
    - BOT_USERNAME, BOT_PASSWORD
    - DEVICE_USERNAME, DEVICE_PASSWORD

    OPCIONAIS (têm valores padrão):
    - API_TITLE, API_VERSION, API_HOST, API_PORT
    - ENVIRONMENT
    """

    API_TITLE: str = "Clínica Médica API"
    API_VERSION: str = "1.0.0"
    API_HOST: str = "0.0.0.0"
    API_PORT: int = 8000

    DATABASE_URL: str = Field(..., description="URL de conexão do PostgreSQL")

    @validator("DATABASE_URL")
    def validate_database_url(cls, v):
        """Valida se é uma URL de conexão PostgreSQL válida"""
        if not v or v.strip() == "":
            raise ValueError("DATABASE_URL não configurada!")

        #postgres_pattern = r'^postgresql://[^:]+:[^@]+@[^:]+:\d+/[^/]+$'

        #if not re.match(postgres_pattern, v):
        #   raise ValueError(
        #      "DATABASE_URL deve ter formato: postgresql://user:password@host:port/database\n"
        #        f"Valor atual: {v}"
        #    )

        return v

    JWT_SECRET_KEY: str = Field(
        ...,
        min_length=32,
        description="Chave secreta para assinar tokens JWT (mínimo 32 caracteres)"
    )
    JWT_ALGORITHM: str = Field(..., description="Algoritmo de assinatura JWT")
    JWT_ACCESS_TOKEN_EXPIRE_MINUTES: int = Field(..., description="Tempo de expiração do token em minutos")
    
    ADMIN_USERNAME: str = Field(..., description="Username do administrador")
    ADMIN_PASSWORD: str = Field(..., description="Password do admin")
    
    BOT_USERNAME: str = Field(..., description="Username do bot Telegram")
    BOT_PASSWORD: str = Field(..., description="Password do bot")
    
    DEVICE_USERNAME: str = Field(..., description="Username do dispositivo embarcado")
    DEVICE_PASSWORD: str = Field(..., description="Password do dispositivo")

    @validator("JWT_SECRET_KEY")
    def validate_jwt_secret(cls, v):
        if not v or len(v) < 32:
            raise ValueError("JWT_SECRET_KEY deve ter no mínimo 32 caracteres!")
        return v

    QR_SECRET_KEY: str = Field(..., min_length=16, description="Chave secreta QR Code")

    @validator("QR_SECRET_KEY")
    def validate_qr_secret(cls, v):
        if not v or len(v) < 16:
            raise ValueError("QR_SECRET_KEY deve ter no mínimo 16 caracteres!")
        return v

    ENVIRONMENT: str = Field(default="development", description="Ambiente de execução")

    @validator("ENVIRONMENT")
    def validate_environment(cls, v):
        """Valida ambiente"""
        valid_envs = ["development", "staging", "production"]
        if v not in valid_envs:
            raise ValueError(f"ENVIRONMENT deve ser: {', '.join(valid_envs)}")
        return v

    class Config:
        env_file = ".env"
        env_file_encoding = "utf-8"
        case_sensitive = True


settings = Settings()
