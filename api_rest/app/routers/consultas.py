"""
Rotas da API para operações com Consultas e Horários
"""
from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy.orm import Session
from typing import List
from datetime import datetime, timedelta
from app.core.database import get_db
from app.core.security import get_current_user
from app.models.consulta import Consulta
from app.models.paciente import Paciente
from app.models.profissional import Profissional
from app.models.horario_profissional import HorarioProfissional
from app.schemas.consulta import ConsultaCreate, ConsultaResponse, HorarioDisponivel

router = APIRouter(tags=["Consultas e Horários"])


def gerar_slots_horarios(hora_inicio: str, hora_fim: str, duracao_minutos: int) -> List[str]:
    """
    Gera lista de horários baseado no intervalo e duração.
    Ex: ('08:00', '10:00', 30) -> ['08:00', '08:30', '09:00', '09:30']
    """
    formato = '%H:%M'
    inicio = datetime.strptime(hora_inicio, formato)
    fim = datetime.strptime(hora_fim, formato)
    
    slots = []
    horario_atual = inicio
    
    while horario_atual < fim:
        slots.append(horario_atual.strftime(formato))
        horario_atual += timedelta(minutes=duracao_minutos)
    
    return slots


@router.get("/horarios/disponiveis/{profissional_id}/{data}", response_model=List[HorarioDisponivel])
async def buscar_horarios_disponiveis(
    profissional_id: int,
    data: str,
    db: Session = Depends(get_db),
    current_user: dict = Depends(get_current_user)
):
    """
    Busca horários disponíveis para um profissional em uma data específica.
    Data no formato: YYYY-MM-DD
    """
    profissional = db.query(Profissional).filter(Profissional.id == profissional_id).first()
    if not profissional:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Profissional com ID {profissional_id} não encontrado"
        )
    
    try:
        data_obj = datetime.strptime(data, "%Y-%m-%d")
    except ValueError:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail="Data deve estar no formato YYYY-MM-DD"
        )
    
    dia_semana = data_obj.weekday()
    
    horarios_config = db.query(HorarioProfissional).filter(
        HorarioProfissional.profissional_id == profissional_id,
        HorarioProfissional.dia_semana == dia_semana
    ).all()
    
    if not horarios_config:
        return []
    
    todos_horarios = []
    for periodo in horarios_config:
        slots = gerar_slots_horarios(
            periodo.hora_inicio,
            periodo.hora_fim,
            periodo.duracao_minutos
        )
        todos_horarios.extend(slots)
    
    todos_horarios = sorted(set(todos_horarios))
    
    consultas_agendadas = db.query(Consulta).filter(
        Consulta.profissional_id == profissional_id,
        Consulta.data == data,
        Consulta.status.in_(["agendada"])
    ).all()
    
    horarios_ocupados = {consulta.horario for consulta in consultas_agendadas}
    
    horarios = [
        HorarioDisponivel(
            horario=horario,
            disponivel=horario not in horarios_ocupados
        )
        for horario in todos_horarios
    ]
    
    return horarios


@router.post("/consultas", response_model=ConsultaResponse, status_code=status.HTTP_201_CREATED)
async def agendar_consulta(
    consulta_data: ConsultaCreate, 
    db: Session = Depends(get_db),
    current_user: dict = Depends(get_current_user)
):
    """
    Agenda uma nova consulta
    """
    paciente = None
    if consulta_data.paciente_id:
        paciente = db.query(Paciente).filter(Paciente.id == consulta_data.paciente_id).first()
    else:
        paciente = db.query(Paciente).filter(Paciente.cpf == consulta_data.cpf).first()
    
    if not paciente:
        paciente = Paciente(
            nome=consulta_data.nome,
            cpf=consulta_data.cpf,
            aceita_digital=False
        )
        db.add(paciente)
        db.flush()
    
    profissional = db.query(Profissional).filter(Profissional.id == consulta_data.profissional_id).first()
    if not profissional:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Profissional com ID {consulta_data.profissional_id} não encontrado"
        )
    
    try:
        data_obj = datetime.strptime(consulta_data.data, "%Y-%m-%d")
    except ValueError:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail="Data deve estar no formato YYYY-MM-DD"
        )
    
    dia_semana = data_obj.weekday()
    
    horarios_config = db.query(HorarioProfissional).filter(
        HorarioProfissional.profissional_id == consulta_data.profissional_id,
        HorarioProfissional.dia_semana == dia_semana
    ).all()
    
    if not horarios_config:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail=f"Profissional não atende neste dia da semana"
        )
    
    horarios_validos = []
    for periodo in horarios_config:
        slots = gerar_slots_horarios(
            periodo.hora_inicio,
            periodo.hora_fim,
            periodo.duracao_minutos
        )
        horarios_validos.extend(slots)
    
    horarios_validos = set(horarios_validos)
    
    if consulta_data.horario not in horarios_validos:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail=f"Horário {consulta_data.horario} não está disponível para este profissional neste dia"
        )
    
    consulta_existente = db.query(Consulta).filter(
        Consulta.profissional_id == consulta_data.profissional_id,
        Consulta.data == consulta_data.data,
        Consulta.horario == consulta_data.horario,
        Consulta.status.in_(["agendada"])
    ).first()
    
    if consulta_existente:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail=f"Horário {consulta_data.horario} já está ocupado para este profissional nesta data"
        )
    
    consulta = Consulta(
        paciente_id=paciente.id,
        profissional_id=consulta_data.profissional_id,
        data=consulta_data.data,
        horario=consulta_data.horario,
        status="agendada"
    )
    
    db.add(consulta)
    db.commit()
    db.refresh(consulta)
    
    return consulta


@router.get("/consultas/{consulta_id}", response_model=ConsultaResponse)
async def buscar_consulta(
    consulta_id: int, 
    db: Session = Depends(get_db),
    current_user: dict = Depends(get_current_user)
):
    """
    Busca uma consulta pelo ID
    """
    consulta = db.query(Consulta).filter(Consulta.id == consulta_id).first()
    
    if not consulta:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Consulta com ID {consulta_id} não encontrada"
        )
    
    return consulta


@router.get("/consultas", response_model=List[ConsultaResponse])
async def listar_consultas(
    paciente_id: int = None,
    profissional_id: int = None,
    data: str = None,
    status: str = None,
    skip: int = 0,
    limit: int = 100,
    db: Session = Depends(get_db),
    current_user: dict = Depends(get_current_user)
):
    """
    Lista consultas com filtros opcionais.
    
    Parâmetros:
    - paciente_id: Filtrar por ID do paciente
    - profissional_id: Filtrar por ID do profissional
    - data: Filtrar por data (formato YYYY-MM-DD)
    - status: Filtrar por status (agendada, cancelada, realizada)
    - skip: Número de registros para pular (paginação)
    - limit: Número máximo de registros a retornar
    """
    query = db.query(Consulta)
    
    if paciente_id:
        query = query.filter(Consulta.paciente_id == paciente_id)
    if profissional_id:
        query = query.filter(Consulta.profissional_id == profissional_id)
    if data:
        query = query.filter(Consulta.data == data)
    if status:
        status_validos = ["agendada", "cancelada", "realizada"]
        if status not in status_validos:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail=f"Status deve ser um dos seguintes: {', '.join(status_validos)}"
            )
        query = query.filter(Consulta.status == status)
    
    consultas = query.offset(skip).limit(limit).all()
    return consultas


@router.patch("/consultas/{consulta_id}/status")
async def atualizar_status_consulta(
    consulta_id: int,
    novo_status: str,
    db: Session = Depends(get_db),
    current_user: dict = Depends(get_current_user)
):
    """
    Atualiza o status de uma consulta
    """
    status_validos = ["agendada", "cancelada", "realizada"]
    if novo_status not in status_validos:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail=f"Status deve ser um dos seguintes: {', '.join(status_validos)}"
        )
    
    consulta = db.query(Consulta).filter(Consulta.id == consulta_id).first()
    if not consulta:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Consulta com ID {consulta_id} não encontrada"
        )
    
    consulta.status = novo_status
    db.commit()
    db.refresh(consulta)
    
    return {"message": f"Status atualizado para '{novo_status}'", "consulta_id": consulta_id}
