"""
Rotas da API para operações com Horários de Profissionais
"""
from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy.orm import Session
from typing import List
from app.core.database import get_db
from app.models.horario_profissional import HorarioProfissional
from app.models.profissional import Profissional
from app.schemas.horario_profissional import (
    HorarioProfissionalCreate,
    HorarioProfissionalUpdate,
    HorarioProfissionalResponse
)

router = APIRouter(tags=["Horários de Profissionais"])


@router.post("/horarios-profissionais", response_model=HorarioProfissionalResponse, status_code=status.HTTP_201_CREATED)
async def criar_horario_profissional(horario_data: HorarioProfissionalCreate, db: Session = Depends(get_db)):
    """
    Cria um novo horário de atendimento para um profissional
    """
    profissional = db.query(Profissional).filter(Profissional.id == horario_data.profissional_id).first()
    if not profissional:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Profissional com ID {horario_data.profissional_id} não encontrado"
        )
    
    horario = HorarioProfissional(**horario_data.dict())
    db.add(horario)
    db.commit()
    db.refresh(horario)
    
    return horario


@router.get("/horarios-profissionais/{profissional_id}", response_model=List[HorarioProfissionalResponse])
async def listar_horarios_profissional(profissional_id: int, db: Session = Depends(get_db)):
    """
    Lista todos os horários de atendimento de um profissional
    """
    profissional = db.query(Profissional).filter(Profissional.id == profissional_id).first()
    if not profissional:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Profissional com ID {profissional_id} não encontrado"
        )
    
    horarios = db.query(HorarioProfissional).filter(
        HorarioProfissional.profissional_id == profissional_id
    ).order_by(HorarioProfissional.dia_semana, HorarioProfissional.hora_inicio).all()
    
    return horarios


@router.get("/horarios-profissionais/detalhe/{horario_id}", response_model=HorarioProfissionalResponse)
async def buscar_horario_profissional(horario_id: int, db: Session = Depends(get_db)):
    """
    Busca um horário específico pelo ID
    """
    horario = db.query(HorarioProfissional).filter(HorarioProfissional.id == horario_id).first()
    
    if not horario:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Horário com ID {horario_id} não encontrado"
        )
    
    return horario


@router.put("/horarios-profissionais/{horario_id}", response_model=HorarioProfissionalResponse)
async def atualizar_horario_profissional(
    horario_id: int,
    horario_data: HorarioProfissionalUpdate,
    db: Session = Depends(get_db)
):
    """
    Atualiza um horário de atendimento
    """
    horario = db.query(HorarioProfissional).filter(HorarioProfissional.id == horario_id).first()
    
    if not horario:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Horário com ID {horario_id} não encontrado"
        )
    
    for key, value in horario_data.dict(exclude_unset=True).items():
        setattr(horario, key, value)
    
    db.commit()
    db.refresh(horario)
    
    return horario


@router.delete("/horarios-profissionais/{horario_id}", status_code=status.HTTP_204_NO_CONTENT)
async def deletar_horario_profissional(horario_id: int, db: Session = Depends(get_db)):
    """
    Deleta um horário de atendimento
    """
    horario = db.query(HorarioProfissional).filter(HorarioProfissional.id == horario_id).first()
    
    if not horario:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Horário com ID {horario_id} não encontrado"
        )
    
    db.delete(horario)
    db.commit()
    
    return None
