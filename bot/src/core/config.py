import os
from typing import Optional


class Config:
    """
    Classe de configuração do bot carregando variáveis de ambiente.
    
    Attributes:
        TELEGRAM_TOKEN: Token de autenticação do bot do Telegram
        API_BASE_URL: URL base da API REST
        JWT_TOKEN: Token JWT para autenticação na API
        QR_SECRET_KEY: Chave secreta para geração de QR codes
        LOG_LEVEL: Nível de logging (INFO, DEBUG, etc.)
        API_TIMEOUT: Timeout em segundos para requisições à API
    """
    
    def __init__(self):
        self.TELEGRAM_TOKEN: Optional[str] = os.getenv('TELEGRAM_TOKEN')
        self.API_BASE_URL: str = os.getenv('API_BASE_URL', 'http://localhost:8000')
        self.API_USERNAME: str = os.getenv('API_USERNAME', 'bot_user')
        self.API_PASSWORD: str = os.getenv('API_PASSWORD', 'bot_password')
        self.QR_SECRET_KEY: Optional[str] = os.getenv('QR_SECRET_KEY')
        self.LOG_LEVEL: str = os.getenv('LOG_LEVEL', 'INFO')
        self.API_TIMEOUT: int = int(os.getenv('API_TIMEOUT', '30'))
        self.USE_MOCK_DATA: bool = os.getenv('USE_MOCK_DATA', 'false').lower() in ('true', '1', 'yes')

    def validate(self) -> bool:
        """
        Valida se as configurações obrigatórias estão presentes.
        
        Returns:
            True se válido
            
        Raises:
            ValueError: Se alguma configuração obrigatória estiver faltando
        """
        if not self.TELEGRAM_TOKEN:
            raise ValueError("TELEGRAM_TOKEN é obrigatório")
        if not self.QR_SECRET_KEY:
            raise ValueError("QR_SECRET_KEY é obrigatório")
        return True

    def __str__(self) -> str:
        """
        Representação em string da configuração (sem dados sensíveis).
        
        Returns:
            String com informações básicas da configuração
        """
        return f"Config(API_BASE_URL={self.API_BASE_URL}, LOG_LEVEL={self.LOG_LEVEL})"