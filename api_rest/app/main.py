"""
API REST para Sistema de Clínicas Médicas
Desenvolvido com FastAPI
"""
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from contextlib import asynccontextmanager
from datetime import datetime
from app.core.config import settings
from app.core.database import init_db
from app.routers import pacientes, categorias, profissionais, consultas, horarios_profissionais, qrcode, auth


@asynccontextmanager
async def lifespan(app: FastAPI):
    """
    Gerenciador de ciclo de vida da aplicação
    """
    print("Inicializando banco de dados...")
    init_db()
    print("Banco de dados inicializado!")
    
    yield
    
    print("Encerrando aplicação...")


app = FastAPI(
    title=settings.API_TITLE,
    version=settings.API_VERSION,
    description="API REST para gerenciamento de clínica médica, agendamento de consultas e geração de QR Code para check-in.",
    lifespan=lifespan,
    docs_url="/docs",
    redoc_url="/redoc",
    openapi_url="/openapi.json"
)

allowed_origins = [
    "http://localhost:8000",
]

if settings.ENVIRONMENT == "production":
    allowed_origins.append("https://clinicas-api.railway.app")

app.add_middleware(
    CORSMiddleware,
    allow_origins=allowed_origins,
    allow_credentials=True,
    allow_methods=["GET", "POST", "PUT", "PATCH", "DELETE"],
    allow_headers=["*"],
)

app.include_router(auth.router)
app.include_router(pacientes.router)
app.include_router(categorias.router)
app.include_router(profissionais.router)
app.include_router(consultas.router)
app.include_router(horarios_profissionais.router)
app.include_router(qrcode.router)


@app.get("/")
async def root():
    """
    Endpoint raiz - informações da API
    """
    return {
        "api": settings.API_TITLE,
        "version": settings.API_VERSION,
        "status": "online",
        "docs": "/docs",
        "redoc": "/redoc"
    }


@app.get("/health")
async def health_check():
    """
    Endpoint de health check
    """
    return {
        "status": "healthy", 
        "timestamp": datetime.now().isoformat()
    }



