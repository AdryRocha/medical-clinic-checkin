import json
from utils import gerar_qr_code_consulta
import logging
import re
from datetime import datetime, date, time, timedelta
from telegram import Update, ReplyKeyboardMarkup, ReplyKeyboardRemove
from telegram.ext import ContextTypes, ConversationHandler, CommandHandler, MessageHandler, filters
from api.api_client import APIClient
from mock.mock_data import MockAPIClient
from config import Config

logger = logging.getLogger(__name__)

CPF, NAME, ACCEPTS_DIGITAL, CATEGORY, PROFESSIONAL, DAY, TIME_SLOT, CONFIRM = range(8)

api_client = None
config = None


def get_category_name(category_id: int, categories: list) -> str:
    """
    Busca o nome da categoria pelo ID.
    
    Args:
        category_id: ID da categoria a ser buscada
        categories: Lista de categorias disponíveis
        
    Returns:
        Nome da categoria ou 'Categoria Desconhecida' se não encontrada
    """
    category = next((c for c in categories if c['id'] == category_id), None)
    return category['nome'] if category else 'Categoria Desconhecida'


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
    dias_semana_nomes = ['Segunda', 'Terça', 'Quarta', 'Quinta', 'Sexta', 'Sábado', 'Domingo']
    
    dica_dias = ""
    for i in range(1, 15):
        data_teste = hoje + timedelta(days=i)
        dia_semana = data_teste.weekday()
        
        try:
            test_result = await api_client.buscar_horarios_disponiveis(profissional['id'], data_teste.isoformat())
            if test_result['success'] and test_result['data']:
                horarios_disponiveis = [h for h in test_result['data'] if h.get('disponivel', True)]
                if horarios_disponiveis:
                    dica_dias = f"\n\n💡 Dica: {dias_semana_nomes[dia_semana]} ({data_teste.strftime('%d/%m/%Y')}) tem horários disponíveis!"
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
            # Filtrar apenas horários disponíveis
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
                # Buscar próximo dia disponível
                dia_atual = datetime.fromisoformat(dia).date()
                dias_semana_nomes = ['Segunda', 'Terça', 'Quarta', 'Quinta', 'Sexta', 'Sábado', 'Domingo']
                sugestao = ""
                
                for i in range(1, 30):
                    data_teste = dia_atual + timedelta(days=i)
                    test_result = await api_client.buscar_horarios_disponiveis(profissional_id, data_teste.isoformat())
                    if test_result['success'] and test_result['data']:
                        horarios_teste = [h for h in test_result['data'] if h.get('disponivel', True)]
                        if horarios_teste:
                            dia_semana = data_teste.weekday()
                            primeiro_horario = horarios_teste[0]['horario']
                            sugestao = f"\n\n💡 Próximo disponível: {dias_semana_nomes[dia_semana]}, {data_teste.strftime('%d/%m/%Y')} às {primeiro_horario}"
                            break
                
                await update.message.reply_text(
                    f"❌ Nenhum horário disponível para este dia.{sugestao}\n\n"
                    "Escolha outro dia (DD/MM/AAAA):"
                )
                return DAY
        else:
            # Buscar próximo dia disponível
            dia_atual = datetime.fromisoformat(dia).date()
            dias_semana_nomes = ['Segunda', 'Terça', 'Quarta', 'Quinta', 'Sexta', 'Sábado', 'Domingo']
            sugestao = ""
            
            for i in range(1, 30):
                data_teste = dia_atual + timedelta(days=i)
                test_result = await api_client.buscar_horarios_disponiveis(profissional_id, data_teste.isoformat())
                if test_result['success'] and test_result['data']:
                    horarios_teste = [h for h in test_result['data'] if h.get('disponivel', True)]
                    if horarios_teste:
                        dia_semana = data_teste.weekday()
                        primeiro_horario = horarios_teste[0]['horario']
                        sugestao = f"\n\n💡 Próximo disponível: {dias_semana_nomes[dia_semana]}, {data_teste.strftime('%d/%m/%Y')} às {primeiro_horario}"
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
    cpf_formatado = f"{dados['cpf'][:3]}.{dados['cpf'][3:6]}.{dados['cpf'][6:9]}-{dados['cpf'][9:]}"
    data_formatada = datetime.fromisoformat(dados['dia']).strftime('%d/%m/%Y')
    
    mensagem = (
        f"📋 Confirme os dados:\n\n"
        f"👤 Paciente: {dados['nome']}\n"
        f"📄 CPF: {cpf_formatado}\n"
        f"🏥 Especialidade: {dados.get('profissional_categoria', 'N/A')}\n"
        f"👨‍⚕️ Profissional: {dados['profissional_nome']}\n"
        f"📅 Data: {data_formatada}\n"
        f"🕐 Horário: {dados['horario']}\n\n"
        "✅ Tudo correto?"
    )
    
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
            qr_code_data = resultado['qr_code']
            qr_code_bytes = gerar_qr_code_consulta(qr_code_data)
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


def setup_handlers(application, app_config: Config):
    """
    Configura os handlers do bot com a configuração centralizada.
    
    Args:
        application: Instância do Application do telegram
        app_config: Instância configurada da classe Config
    """
    global api_client, config
    
    config = app_config
    
    if config.USE_MOCK_DATA:
        api_client = MockAPIClient()
        logger.info("Handlers configurados com MOCK DATA")
    else:
        api_client = APIClient(
            base_url=config.API_BASE_URL,
            username=config.API_USERNAME,
            password=config.API_PASSWORD,
            secret_key=config.QR_SECRET_KEY,
            config=config
        )
        logger.info(f"Handlers configurados com API REST: {config.API_BASE_URL}")
    
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