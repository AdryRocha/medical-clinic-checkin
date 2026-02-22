"""
Configuração do banco de dados SQLAlchemy
"""
from sqlalchemy import create_engine
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker
from app.core.config import settings

engine = create_engine(
    settings.DATABASE_URL,
    pool_pre_ping=True,
    echo=False
)

SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)

Base = declarative_base()


def get_db():
    """
    Dependency para obter sessão do banco de dados
    """
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()


def init_db():
    """
    Inicializa o banco de dados criando todas as tabelas.
    Importa todos os modelos para garantir que sejam registrados
    no metadata antes da criação das tabelas.
    """
    from app.models.paciente import Paciente
    from app.models.categoria import Categoria
    from app.models.profissional import Profissional
    from app.models.horario_profissional import HorarioProfissional
    from app.models.consulta import Consulta
    
    Base.metadata.create_all(bind=engine)
