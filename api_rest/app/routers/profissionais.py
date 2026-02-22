"""
Rotas da API para operações com Profissionais
"""
from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy.orm import Session
from typing import List, Optional
from app.core.database import get_db
from app.core.security import get_current_user
from app.models.categoria import Categoria
from app.models.profissional import Profissional
from app.schemas.profissional import ProfissionalCreate, ProfissionalResponse

router = APIRouter(prefix="/profissionais", tags=["Profissionais"])


@router.get("/", response_model=List[ProfissionalResponse])
async def listar_profissionais(
    categoria_id: Optional[int] = None,
    db: Session = Depends(get_db),
    current_user: dict = Depends(get_current_user)
):
    """
    Lista todos os profissionais, opcionalmente filtrados por categoria
    """
    query = db.query(Profissional)
    
    if categoria_id:
        query = query.filter(Profissional.categoria_id == categoria_id)
    
    profissionais = query.all()
    return profissionais


@router.post("/", response_model=ProfissionalResponse, status_code=status.HTTP_201_CREATED)
async def criar_profissional(
    profissional_data: ProfissionalCreate, 
    db: Session = Depends(get_db),
    current_user: dict = Depends(get_current_user)
):
    """
    Cadastra um novo profissional de saúde
    """
    categoria = db.query(Categoria).filter(Categoria.id == profissional_data.categoria_id).first()
    if not categoria:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Categoria com ID {profissional_data.categoria_id} não encontrada"
        )
    
    existing = db.query(Profissional).filter(Profissional.registro == profissional_data.registro).first()
    if existing:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail=f"Profissional com registro {profissional_data.registro} já cadastrado"
        )
    
    profissional = Profissional(
        nome=profissional_data.nome,
        registro=profissional_data.registro,
        categoria_id=profissional_data.categoria_id
    )
    
    db.add(profissional)
    db.commit()
    db.refresh(profissional)
    
    return profissional


@router.get("/{profissional_id}", response_model=ProfissionalResponse)
async def buscar_profissional(
    profissional_id: int, 
    db: Session = Depends(get_db),
    current_user: dict = Depends(get_current_user)
):
    """
    Busca um profissional pelo ID
    """
    profissional = db.query(Profissional).filter(Profissional.id == profissional_id).first()
    
    if not profissional:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Profissional com ID {profissional_id} não encontrado"
        )
    
    return profissional