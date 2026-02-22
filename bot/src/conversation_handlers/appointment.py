"""
Handler de agendamento de consultas médicas.

Gerencia todo o fluxo de conversação para agendamento de consultas,
desde a coleta do CPF até a confirmação final e geração do QR Code.
"""

import logging
import re
from datetime import datetime, date, timedelta
from telegram import Update, ReplyKeyboardMarkup, ReplyKeyboardRemove
from telegram.ext import ContextTypes, ConversationHandler, CommandHandler, MessageHandler, filters

from core.constants import (
    CPF, NAME, ACCEPTS_DIGITAL, CATEGORY, PROFESSIONAL, DAY, TIME_SLOT, CONFIRM, 
    RECOVER_CPF, SELECT_CONSULTATION, DIAS_SEMANA
)
from core.config import Config
from services.formatting import get_category_name, format_cpf, format_appointment_summary
from api.api_client import APIClient
from mock.mock_data import MockAPIClient

logger = logging.getLogger(__name__)

api_client = None
config = None


async def schedule_appointment(update: Update, context: ContextTypes.DEFAULT_TYPE) -> int:
    """
    Inicia o processo de agendamento de consulta.
    
    Args:
        update: Objeto Update do Telegram
        context: Contexto da conversa
        
    Returns:
        Próximo estado da conversa (CPF)
    """
    await update.message.reply_text(
        "Vamos agendar sua consulta! 📅\n\n"
        "Primeiro, qual é o seu CPF (apenas números)?"
    )
    return CPF


async def receive_cpf(update: Update, context: ContextTypes.DEFAULT_TYPE) -> int:
    """
    Recebe e valida o CPF do paciente.
    
    Args:
        update: Objeto Update do Telegram
        context: Contexto da conversa
        
    Returns:
        Próximo estado da conversa (NAME ou CATEGORY)
    """
    cpf = re.sub(r'\D', '', update.message.text)

    if len(cpf) != 11:
        await update.message.reply_text("CPF deve ter 11 dígitos. Digite novamente:")
        return CPF

    if cpf == cpf[0] * 11:
        await update.message.reply_text("CPF inválido. Digite novamente:")
        return CPF

    context.user_data['cpf'] = cpf

    try:
        patient_result = await api_client.buscar_paciente_por_cpf(cpf)
        if patient_result['success'] and patient_result['data']:
            context.user_data['paciente_id'] = patient_result['data']['id']
            context.user_data['nome'] = patient_result['data']['nome']
            await update.message.reply_text(
                f"Olá {patient_result['data']['nome']}! Encontrei seu cadastro. ✓\n\n"
                "Agora, escolha a especialidade:"
            )
            return await show_categories(update, context)
        else:
            await update.message.reply_text(
                "CPF não encontrado. Vamos fazer seu cadastro!\n\n"
                "Qual é o seu nome completo?"
            )
            return NAME

    except Exception as e:
        logger.error(f"Erro ao buscar paciente: {e}")
        await update.message.reply_text(
            "Erro ao verificar cadastro. Qual é o seu nome completo?"
        )
        return NAME


async def receive_name(update: Update, context: ContextTypes.DEFAULT_TYPE) -> int:
    """
    Recebe e valida o nome completo do paciente.
    
    Args:
        update: Objeto Update do Telegram
        context: Contexto da conversa
        
    Returns:
        Próximo estado da conversa (ACCEPTS_DIGITAL)
    """
    nome = update.message.text.strip()

    if len(nome) < 2:
        await update.message.reply_text("Nome muito curto. Digite seu nome completo:")
        return NAME

    context.user_data['nome'] = nome
    
    keyboard = [['Sim', 'Não']]
    await update.message.reply_text(
        f"Nome: {nome} ✓\n\n"
        "🔐 Para agilizar o check-in na recepção, podemos coletar sua digital biométrica no dia da consulta.\n\n"
        "Você autoriza a coleta da sua digital?",
        reply_markup=ReplyKeyboardMarkup(keyboard, one_time_keyboard=True, resize_keyboard=True)
    )
    return ACCEPTS_DIGITAL


async def receive_accepts_digital(update: Update, context: ContextTypes.DEFAULT_TYPE) -> int:
    """
    Recebe a resposta sobre autorização de coleta de digital biométrica.
    
    Args:
        update: Objeto Update do Telegram
        context: Contexto da conversa
        
    Returns:
        Próximo estado da conversa (CATEGORY)
    """
    resposta = update.message.text.strip().lower()
    
    aceita_digital = (resposta == 'sim')
    context.user_data['aceita_digital'] = aceita_digital
    
    try:
        nome = context.user_data['nome']
        cpf = context.user_data['cpf']
        
        resultado = await api_client.cadastrar_paciente(nome, cpf, aceita_digital)
        
        if resultado['success']:
            context.user_data['paciente_id'] = resultado['data']['id']
            
            if aceita_digital:
                await update.message.reply_text(
                    "✓ Cadastro realizado! Sua digital será coletada no dia da consulta.\n\n"
                    "Agora, escolha a especialidade:",
                    reply_markup=ReplyKeyboardRemove()
                )
            else:
                await update.message.reply_text(
                    "✓ Cadastro realizado! Você fará check-in pela forma tradicional.\n\n"
                    "Agora, escolha a especialidade:",
                    reply_markup=ReplyKeyboardRemove()
                )
        else:
            await update.message.reply_text(
                f"Erro ao cadastrar: {resultado.get('error', 'Erro desconhecido')}\n\n"
                "Continuando com o agendamento...",
                reply_markup=ReplyKeyboardRemove()
            )
    except Exception as e:
        logger.error(f"Erro ao cadastrar paciente: {e}")
        await update.message.reply_text(
            "Erro ao cadastrar, mas continuando com o agendamento...",
            reply_markup=ReplyKeyboardRemove()
        )
    
    return await show_categories(update, context)


async def show_categories(update: Update, context: ContextTypes.DEFAULT_TYPE) -> int:
    """
    Exibe as categorias/especialidades médicas disponíveis.
    
    Args:
        update: Objeto Update do Telegram
        context: Contexto da conversa
        
    Returns:
        Próximo estado da conversa (CATEGORY ou PROFESSIONAL)
    """
    try:
        categories_result = await api_client.buscar_categorias()
        if categories_result['success'] and categories_result['data']:
            categorias = categories_result['data']
            keyboard = [[cat['nome']] for cat in categorias]
            keyboard.append(['Ver Todos', 'Cancelar'])

            await update.message.reply_text(
                "🏥 Escolha a especialidade médica:",
                reply_markup=ReplyKeyboardMarkup(keyboard, one_time_keyboard=True, resize_keyboard=True)
            )
            context.user_data['categorias_disponiveis'] = categorias
            return CATEGORY
        else:
            await update.message.reply_text(
                "Erro ao carregar especialidades. Vamos direto para os profissionais.",
                reply_markup=ReplyKeyboardRemove()
            )
            return await show_professionals(update, context)

    except Exception as e:
        logger.error(f"Erro ao buscar categorias: {e}")
        await update.message.reply_text(
            "Erro ao carregar especialidades. Vamos direto para os profissionais.",
            reply_markup=ReplyKeyboardRemove()
        )
        return await show_professionals(update, context)


async def receive_category(update: Update, context: ContextTypes.DEFAULT_TYPE) -> int:
    """
    Recebe a categoria escolhida e avança para seleção de profissionais.
    
    Args:
        update: Objeto Update do Telegram
        context: Contexto da conversa
        
    Returns:
        Próximo estado da conversa (PROFESSIONAL ou END)
    """
    categoria_nome = update.message.text.strip()

    if categoria_nome == 'Cancelar':
        await update.message.reply_text(
            "Agendamento cancelado.",
            reply_markup=ReplyKeyboardRemove()
        )
        return ConversationHandler.END

    if categoria_nome == 'Ver Todos':
        context.user_data['categoria_selecionada'] = None
        return await show_professionals(update, context)

    categorias = context.user_data.get('categorias_disponiveis', [])
    categoria = next((c for c in categorias if c['nome'] == categoria_nome), None)

    if not categoria:
        await update.message.reply_text("Especialidade não encontrada. Escolha novamente:")
        return await show_categories(update, context)

    context.user_data['categoria_selecionada'] = categoria['id']
    context.user_data['categoria_nome'] = categoria['nome']
    
    await update.message.reply_text(f"Especialidade: {categoria['nome']} ✓")
    return await show_professionals(update, context)


async def show_professionals(update: Update, context: ContextTypes.DEFAULT_TYPE) -> int:
    """
    Exibe os profissionais disponíveis, opcionalmente filtrados por categoria.
    
    Args:
        update: Objeto Update do Telegram
        context: Contexto da conversa
        
    Returns:
        Próximo estado da conversa (PROFESSIONAL, CATEGORY ou END)
    """
    try:
        categoria_id = context.user_data.get('categoria_selecionada')
        professionals_result = await api_client.buscar_profissionais(categoria_id)
        
        if professionals_result['success'] and professionals_result['data']:
            profissionais = professionals_result['data']
            
            if not profissionais:
                await update.message.reply_text(
                    "Nenhum profissional disponível para esta especialidade.\n"
                    "Escolha outra especialidade:",
                    reply_markup=ReplyKeyboardRemove()
                )
                return await show_categories(update, context)
            
            categorias = context.user_data.get('categorias_disponiveis', [])
            
            keyboard = [
                [f"{prof['nome']} - {get_category_name(prof['categoria_id'], categorias)}"] 
                for prof in profissionais
            ]
            keyboard.append(['Voltar', 'Cancelar'])

            mensagem = "👨‍⚕️ Escolha o profissional:"
            if categoria_id:
                categoria_nome = context.user_data.get('categoria_nome', '')
                mensagem = f"👨‍⚕️ Profissionais de {categoria_nome}:"

            await update.message.reply_text(
                mensagem,
                reply_markup=ReplyKeyboardMarkup(keyboard, one_time_keyboard=True, resize_keyboard=True)
            )
            context.user_data['profissionais_disponiveis'] = profissionais
            context.user_data['categorias_disponiveis'] = categorias
            return PROFESSIONAL
        else:
            await update.message.reply_text(
                "Erro ao carregar profissionais. Tente novamente mais tarde.",
                reply_markup=ReplyKeyboardRemove()
            )
            return ConversationHandler.END

    except Exception as e:
        logger.error(f"Erro ao buscar profissionais: {e}")
        await update.message.reply_text(
            "Erro interno. Tente novamente mais tarde.",
            reply_markup=ReplyKeyboardRemove()
        )
        return ConversationHandler.END


async def receive_professional(update: Update, context: ContextTypes.DEFAULT_TYPE) -> int:
    """
    Recebe a escolha do profissional selecionado pelo usuário.
    
    Args:
        update: Objeto Update do Telegram
        context: Contexto da conversa
        
    Returns:
        Próximo estado da conversa (DAY, CATEGORY ou END)
    """
    profissional_texto = update.message.text.strip()

    if profissional_texto == 'Cancelar':
        await update.message.reply_text(
            "Agendamento cancelado.",
            reply_markup=ReplyKeyboardRemove()
        )
        return ConversationHandler.END

    if profissional_texto == 'Voltar':
        await update.message.reply_text("Voltando para especialidades...")
        return await show_categories(update, context)

    profissionais = context.user_data.get('profissionais_disponiveis', [])
    categorias = context.user_data.get('categorias_disponiveis', [])
    
    profissional = None
    for p in profissionais:
        categoria_nome = get_category_name(p['categoria_id'], categorias)
        texto_completo = f"{p['nome']} - {categoria_nome}"
        if texto_completo == profissional_texto:
            profissional = p
            break

    if not profissional:
        await update.message.reply_text("Profissional não encontrado. Escolha novamente:")
        return await show_professionals(update, context)

    context.user_data['profissional_id'] = profissional['id']
    context.user_data['profissional_nome'] = profissional['nome']
    categoria_nome = get_category_name(profissional['categoria_id'], categorias)
    context.user_data['profissional_categoria'] = categoria_nome

    hoje = date.today()
    
    dica_dias = ""
    for i in range(1, 15):
        data_teste = hoje + timedelta(days=i)
        dia_semana = data_teste.weekday()
        
        try:
            test_result = await api_client.buscar_horarios_disponiveis(profissional['id'], data_teste.isoformat())
            if test_result['success'] and test_result['data']:
                horarios_disponiveis = [h for h in test_result['data'] if h.get('disponivel', True)]
                if horarios_disponiveis:
                    dica_dias = f"\n\n💡 Dica: {DIAS_SEMANA[dia_semana]} ({data_teste.strftime('%d/%m/%Y')}) tem horários disponíveis!"
                    break
        except:
            pass
    
    await update.message.reply_text(
        f"Profissional: {profissional['nome']} ({categoria_nome}) ✓\n\n"
        f"📅 Agora, qual dia você deseja? (formato: DD/MM/AAAA){dica_dias}"
    )
    return DAY


async def receive_day(update: Update, context: ContextTypes.DEFAULT_TYPE) -> int:
    """
    Recebe e valida a data escolhida pelo usuário.
    
    Args:
        update: Objeto Update do Telegram
        context: Contexto da conversa
        
    Returns:
        Próximo estado da conversa (TIME_SLOT ou DAY)
    """
    try:
        dia_str = update.message.text.strip()
        dia = datetime.strptime(dia_str, "%d/%m/%Y").date()

        hoje = date.today()
        if dia <= hoje:
            await update.message.reply_text("Data deve ser futura. Digite novamente:")
            return DAY

        context.user_data['dia'] = dia.isoformat()
        await update.message.reply_text(
            f"Dia: {dia.strftime('%d/%m/%Y')}\n\n"
            "Verificando horários disponíveis..."
        )
        return await show_time_slots(update, context)

    except ValueError:
        await update.message.reply_text(
            "Formato inválido. Use: DD/MM/AAAA\nExemplo: 25/11/2025"
        )
        return DAY


async def show_time_slots(update: Update, context: ContextTypes.DEFAULT_TYPE) -> int:
    """
    Exibe os horários disponíveis para o profissional e dia selecionados.
    
    Args:
        update: Objeto Update do Telegram
        context: Contexto da conversa
        
    Returns:
        Próximo estado da conversa (TIME_SLOT, DAY ou END)
    """
    try:
        profissional_id = context.user_data['profissional_id']
        dia = context.user_data['dia']

        time_slots_result = await api_client.buscar_horarios_disponiveis(profissional_id, dia)
        if time_slots_result['success'] and time_slots_result['data']:
            horarios_disponiveis = [h for h in time_slots_result['data'] if h.get('disponivel', True)]
            
            if horarios_disponiveis:
                keyboard = [[h['horario']] for h in horarios_disponiveis]
                keyboard.append(['Voltar'])

                await update.message.reply_text(
                    "Escolha o horário:",
                    reply_markup=ReplyKeyboardMarkup(keyboard, one_time_keyboard=True)
                )
                context.user_data['horarios_disponiveis'] = horarios_disponiveis
                return TIME_SLOT
            else:
                dia_atual = datetime.fromisoformat(dia).date()
                sugestao = ""
                
                for i in range(1, 30):
                    data_teste = dia_atual + timedelta(days=i)
                    test_result = await api_client.buscar_horarios_disponiveis(profissional_id, data_teste.isoformat())
                    if test_result['success'] and test_result['data']:
                        horarios_teste = [h for h in test_result['data'] if h.get('disponivel', True)]
                        if horarios_teste:
                            dia_semana = data_teste.weekday()
                            primeiro_horario = horarios_teste[0]['horario']
                            sugestao = f"\n\n💡 Próximo disponível: {DIAS_SEMANA[dia_semana]}, {data_teste.strftime('%d/%m/%Y')} às {primeiro_horario}"
                            break
                
                await update.message.reply_text(
                    f"❌ Nenhum horário disponível para este dia.{sugestao}\n\n"
                    "Escolha outro dia (DD/MM/AAAA):"
                )
                return DAY
        else:
            dia_atual = datetime.fromisoformat(dia).date()
            sugestao = ""
            
            for i in range(1, 30):
                data_teste = dia_atual + timedelta(days=i)
                test_result = await api_client.buscar_horarios_disponiveis(profissional_id, data_teste.isoformat())
                if test_result['success'] and test_result['data']:
                    horarios_teste = [h for h in test_result['data'] if h.get('disponivel', True)]
                    if horarios_teste:
                        dia_semana = data_teste.weekday()
                        primeiro_horario = horarios_teste[0]['horario']
                        sugestao = f"\n\n💡 Próximo disponível: {DIAS_SEMANA[dia_semana]}, {data_teste.strftime('%d/%m/%Y')} às {primeiro_horario}"
                        break
            
            await update.message.reply_text(
                f"❌ Nenhum horário disponível para este dia.{sugestao}\n\n"
                "Escolha outro dia (DD/MM/AAAA):"
            )
            return DAY

    except Exception as e:
        logger.error(f"Erro ao buscar horários: {e}")
        await update.message.reply_text(
            "Erro ao carregar horários. Tente novamente.",
            reply_markup=ReplyKeyboardRemove()
        )
        return ConversationHandler.END


async def receive_time_slot(update: Update, context: ContextTypes.DEFAULT_TYPE) -> int:
    """
    Recebe o horário escolhido e exibe resumo para confirmação.
    
    Args:
        update: Objeto Update do Telegram
        context: Contexto da conversa
        
    Returns:
        Próximo estado da conversa (CONFIRM ou DAY)
    """
    horario_str = update.message.text.strip()

    if horario_str == 'Voltar':
        await update.message.reply_text("Qual dia você deseja? (DD/MM/AAAA)")
        return DAY

    horarios = context.user_data.get('horarios_disponiveis', [])
    horario = next((h for h in horarios if h['horario'] == horario_str), None)

    if not horario:
        await update.message.reply_text("Horário não encontrado. Escolha novamente:")
        return await show_time_slots(update, context)

    context.user_data['horario'] = horario['horario']

    dados = context.user_data
    mensagem = format_appointment_summary(dados)
    
    await update.message.reply_text(
        mensagem,
        reply_markup=ReplyKeyboardMarkup([['Sim', 'Não']], one_time_keyboard=True, resize_keyboard=True)
    )
    return CONFIRM


async def confirm_appointment(update: Update, context: ContextTypes.DEFAULT_TYPE) -> int:
    """
    Confirma o agendamento e cria a consulta no sistema.
    
    Args:
        update: Objeto Update do Telegram
        context: Contexto da conversa
        
    Returns:
        END da conversa
    """
    resposta = update.message.text.lower().strip()

    if resposta not in ['sim', 's', 'yes', 'y']:
        await update.message.reply_text(
            "❌ Agendamento cancelado.\n\n"
            "Use /agendar para fazer um novo agendamento.",
            reply_markup=ReplyKeyboardRemove()
        )
        return ConversationHandler.END

    try:
        dados = context.user_data
        consulta_data = {
            'paciente_id': dados.get('paciente_id'),
            'cpf': dados['cpf'],
            'nome': dados['nome'],
            'profissional_id': dados['profissional_id'],
            'data': dados['dia'],
            'horario': dados['horario']
        }

        resultado = await api_client.agendar_consulta(consulta_data)

        if resultado['success']:
            import base64
            qr_code_base64 = resultado['qr_code']
            qr_code_bytes = base64.b64decode(qr_code_base64)
            await update.message.reply_text(
                "✅ Agendamento realizado com sucesso!\n\n"
                f"ID da Consulta: {resultado['consulta_id']}\n\n"
                "📱 Aqui está seu QR Code para check-in:",
                reply_markup=ReplyKeyboardRemove()
            )
            await update.message.reply_photo(qr_code_bytes, caption="Apresente este QR Code no dia da consulta.")
        else:
            await update.message.reply_text(
                f"❌ Erro no agendamento: {resultado['error']}\n\n"
                "Tente novamente com /agendar",
                reply_markup=ReplyKeyboardRemove()
            )

    except Exception as e:
        logger.error(f"Erro no agendamento: {e}")
        await update.message.reply_text(
            "❌ Erro interno. Tente novamente mais tarde.",
            reply_markup=ReplyKeyboardRemove()
        )

    return ConversationHandler.END


async def cancel_appointment(update: Update, context: ContextTypes.DEFAULT_TYPE) -> int:
    """
    Cancela o processo de agendamento.
    
    Args:
        update: Objeto Update do Telegram
        context: Contexto da conversa
        
    Returns:
        END da conversa
    """
    await update.message.reply_text(
        "❌ Agendamento cancelado.\n\n"
        "Use /agendar quando quiser tentar novamente.\n"
        "Use /ajuda para ver todos os comandos disponíveis.",
        reply_markup=ReplyKeyboardRemove()
    )
    return ConversationHandler.END


def setup_appointment_handler(application, app_config: Config):
    """
    Configura o conversation handler de agendamento e inicializa o API client.
    
    Args:
        application: Instância do Application do telegram
        app_config: Instância configurada da classe Config
    """
    global api_client, config
    
    config = app_config
    
    if config.USE_MOCK_DATA:
        api_client = MockAPIClient()
        logger.info("Appointment handler configurado com MOCK DATA")
    else:
        api_client = APIClient(
            base_url=config.API_BASE_URL,
            username=config.API_USERNAME,
            password=config.API_PASSWORD,
            secret_key=config.QR_SECRET_KEY,
            config=config
        )
        logger.info(f"Appointment handler configurado com API REST: {config.API_BASE_URL}")
    
    appointment_handler = ConversationHandler(
        entry_points=[CommandHandler("agendar", schedule_appointment)],
        states={
            CPF: [MessageHandler(filters.TEXT & ~filters.COMMAND, receive_cpf)],
            NAME: [MessageHandler(filters.TEXT & ~filters.COMMAND, receive_name)],
            ACCEPTS_DIGITAL: [MessageHandler(filters.TEXT & ~filters.COMMAND, receive_accepts_digital)],
            CATEGORY: [MessageHandler(filters.TEXT & ~filters.COMMAND, receive_category)],
            PROFESSIONAL: [MessageHandler(filters.TEXT & ~filters.COMMAND, receive_professional)],
            DAY: [MessageHandler(filters.TEXT & ~filters.COMMAND, receive_day)],
            TIME_SLOT: [MessageHandler(filters.TEXT & ~filters.COMMAND, receive_time_slot)],
            CONFIRM: [MessageHandler(filters.TEXT & ~filters.COMMAND, confirm_appointment)],
        },
        fallbacks=[CommandHandler("cancelar", cancel_appointment)],
    )

    application.add_handler(appointment_handler)
    logger.info("Conversation handler de agendamento registrado com sucesso")


async def recover_appointments(update: Update, context: ContextTypes.DEFAULT_TYPE) -> int:
    """
    Inicia o processo de recuperação de consultas agendadas.
    
    Args:
        update: Objeto Update do Telegram
        context: Contexto da conversa
        
    Returns:
        Próximo estado da conversa (RECOVER_CPF)
    """
    await update.message.reply_text(
        "🔍 Vamos recuperar suas consultas agendadas!\n\n"
        "Digite o seu CPF (apenas números):"
    )
    return RECOVER_CPF


async def receive_recover_cpf(update: Update, context: ContextTypes.DEFAULT_TYPE) -> int:
    """
    Recebe e valida o CPF para buscar consultas diretamente.
    
    Args:
        update: Objeto Update do Telegram
        context: Contexto da conversa
        
    Returns:
        Próximo estado da conversa (SELECT_CONSULTATION ou END)
    """
    cpf = re.sub(r'\D', '', update.message.text)

    if len(cpf) != 11:
        await update.message.reply_text("CPF deve ter 11 dígitos. Digite novamente:")
        return RECOVER_CPF

    if cpf == cpf[0] * 11:
        await update.message.reply_text("CPF inválido. Digite novamente:")
        return RECOVER_CPF

    try:
        patient_result = await api_client.buscar_paciente_por_cpf(cpf)
        
        if not patient_result['success'] or not patient_result['data']:
            await update.message.reply_text(
                "❌ CPF não encontrado em nossa base de dados.\n\n"
                "Verifique se digitou corretamente ou faça um novo agendamento com /agendar",
                reply_markup=ReplyKeyboardRemove()
            )
            return ConversationHandler.END

        patient = patient_result['data']

        consultas_result = await api_client.buscar_consultas_por_cpf(cpf)
        
        if not consultas_result['success']:
            await update.message.reply_text(
                "❌ Erro ao buscar suas consultas.\n\n"
                "Tente novamente mais tarde.",
                reply_markup=ReplyKeyboardRemove()
            )
            return ConversationHandler.END

        consultas = consultas_result['data']
        
        if not consultas or len(consultas) == 0:
            await update.message.reply_text(
                "📭 Você não possui consultas agendadas.\n\n"
                "Use /agendar para marcar uma nova consulta!",
                reply_markup=ReplyKeyboardRemove()
            )
            return ConversationHandler.END

        hoje = datetime.now().date()
        consultas_futuras = []
        
        for consulta in consultas:
            try:
                data_consulta = datetime.strptime(consulta['data'], '%Y-%m-%d').date()
                if data_consulta >= hoje:
                    consultas_futuras.append(consulta)
            except:
                continue

        if not consultas_futuras:
            await update.message.reply_text(
                "📭 Você não possui consultas futuras agendadas.\n\n"
                "Use /agendar para marcar uma nova consulta!",
                reply_markup=ReplyKeyboardRemove()
            )
            return ConversationHandler.END

        context.user_data['consultas_encontradas'] = consultas_futuras

        msg = f"✅ Encontrei {len(consultas_futuras)} consulta(s) agendada(s) para você:\n\n"
        
        keyboard_options = []
        for i, consulta in enumerate(consultas_futuras, 1):
            try:
                data_obj = datetime.strptime(consulta['data'], '%Y-%m-%d')
                data_formatada = data_obj.strftime('%d/%m/%Y')
                
                profissional_nome = consulta.get('profissional', {}).get('nome', 'N/A')
                categoria_nome = consulta.get('profissional', {}).get('categoria', {}).get('nome', 'N/A')
                
                msg += f"{i}. 📅 {data_formatada} às {consulta['horario']}\n"
                msg += f"   👨‍⚕️ {profissional_nome} - {categoria_nome}\n\n"
                
                keyboard_options.append([f"{i} - {data_formatada} {consulta['horario']}"])
            except Exception as e:
                logger.error(f"Erro ao formatar consulta: {e}")
                continue

        msg += "Selecione o número da consulta para gerar o QR Code:"
        
        keyboard = ReplyKeyboardMarkup(keyboard_options, one_time_keyboard=True, resize_keyboard=True)
        await update.message.reply_text(msg, reply_markup=keyboard)
        
        return SELECT_CONSULTATION

    except Exception as e:
        logger.error(f"Erro ao recuperar consultas: {e}")
        await update.message.reply_text(
            "❌ Erro ao buscar suas consultas.\n\n"
            "Tente novamente mais tarde.",
            reply_markup=ReplyKeyboardRemove()
        )
        return ConversationHandler.END


async def select_consultation_for_qr(update: Update, context: ContextTypes.DEFAULT_TYPE) -> int:
    """
    Processa seleção de consulta e gera QR code.
    
    Args:
        update: Objeto Update do Telegram
        context: Contexto da conversa
        
    Returns:
        END da conversa
    """
    try:
        texto = update.message.text
        numero = int(texto.split()[0]) if texto else 0
        
        consultas = context.user_data.get('consultas_encontradas', [])
        
        if numero < 1 or numero > len(consultas):
            await update.message.reply_text(
                "❌ Opção inválida. Use /minhas_consultas para tentar novamente.",
                reply_markup=ReplyKeyboardRemove()
            )
            return ConversationHandler.END

        consulta = consultas[numero - 1]
        consulta_id = consulta['id']

        # Gera QR code
        await update.message.reply_text("⏳ Gerando seu QR Code...")
        
        qr_result = await api_client.gerar_qrcode(consulta_id)
        
        if not qr_result['success']:
            await update.message.reply_text(
                "❌ Erro ao gerar QR Code.\n\n"
                "Tente novamente mais tarde.",
                reply_markup=ReplyKeyboardRemove()
            )
            return ConversationHandler.END

        # Formata resumo da consulta
        data_obj = datetime.strptime(consulta['data'], '%Y-%m-%d')
        data_formatada = data_obj.strftime('%d/%m/%Y')
        profissional_nome = consulta.get('profissional', {}).get('nome', 'N/A')
        categoria_nome = consulta.get('profissional', {}).get('categoria', {}).get('nome', 'N/A')
        
        resumo = (
            f"✅ QR Code gerado com sucesso!\n\n"
            f"📋 Detalhes da consulta:\n"
            f"📅 Data: {data_formatada}\n"
            f"🕐 Horário: {consulta['horario']}\n"
            f"👨‍⚕️ Profissional: {profissional_nome}\n"
            f"🏥 Especialidade: {categoria_nome}\n\n"
            f"💾 Salve este QR Code para fazer check-in no dia da consulta!"
        )

        await update.message.reply_text(resumo, reply_markup=ReplyKeyboardRemove())
        
        import base64
        from io import BytesIO
        
        qr_base64 = qr_result['data']['qr_code']
        qr_bytes = base64.b64decode(qr_base64)
        qr_image = BytesIO(qr_bytes)
        qr_image.seek(0)
        
        await update.message.reply_photo(
            photo=qr_image,
            caption=f"🎫 QR Code - Consulta #{consulta_id}"
        )

        return ConversationHandler.END

    except Exception as e:
        logger.error(f"Erro ao gerar QR code: {e}")
        await update.message.reply_text(
            "❌ Erro ao processar sua solicitação.\n\n"
            "Tente novamente mais tarde.",
            reply_markup=ReplyKeyboardRemove()
        )
        return ConversationHandler.END


def setup_recover_handler(application, app_config: Config):
    """
    Configura o conversation handler de recuperação de consultas.
    
    Args:
        application: Instância do Application do telegram
        app_config: Instância configurada da classe Config
    """
    global api_client, config
    
    if not api_client:
        config = app_config
        if config.USE_MOCK_DATA:
            api_client = MockAPIClient()
            logger.info("Recover handler configurado com MOCK DATA")
        else:
            api_client = APIClient(
                base_url=config.API_BASE_URL,
                username=config.API_USERNAME,
                password=config.API_PASSWORD,
                secret_key=config.QR_SECRET_KEY,
                config=config
            )
            logger.info(f"Recover handler configurado com API REST: {config.API_BASE_URL}")
    
    recover_handler = ConversationHandler(
        entry_points=[CommandHandler("minhas_consultas", recover_appointments)],
        states={
            RECOVER_CPF: [MessageHandler(filters.TEXT & ~filters.COMMAND, receive_recover_cpf)],
            SELECT_CONSULTATION: [MessageHandler(filters.TEXT & ~filters.COMMAND, select_consultation_for_qr)],
        },
        fallbacks=[CommandHandler("cancelar", cancel_appointment)],
    )

    application.add_handler(recover_handler)
    logger.info("Conversation handler de recuperação de consultas registrado com sucesso")
