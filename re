import logging
import random
import json
import os
import asyncio
from datetime import datetime
from telegram import Update, InlineKeyboardButton, InlineKeyboardMarkup, ReplyKeyboardMarkup, KeyboardButton
from telegram.ext import Application, CommandHandler, CallbackQueryHandler, ContextTypes, MessageHandler, filters

# Настройка логирования
logging.basicConfig(
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    level=logging.INFO
)

# Файл для хранения данных пользователей
DATA_FILE = 'user_data.json'

# ЗДЕСЬ ВСТАВЬТЕ ВАШ ТОКЕН!
BOT_TOKEN = "8273152668:AAGMbDcVUlPJJ275lGeP5AiIq3wK_wdPa4Y"  # ← ЗАМЕНИТЕ НА ВАШ ТОКЕН


class GameBot:
    def __init__(self):
        self.user_data = self.load_data()
        self.captcha_challenges = {}
        self.work_sessions = {}

    def load_data(self):
        if os.path.exists(DATA_FILE):
            with open(DATA_FILE, 'r', encoding='utf-8') as f:
                return json.load(f)
        return {}

    def save_data(self):
        with open(DATA_FILE, 'w', encoding='utf-8') as f:
            json.dump(self.user_data, f, ensure_ascii=False, indent=2)

    def get_user_data(self, user_id):
        user_id_str = str(user_id)
        if user_id_str not in self.user_data:
            # Создаем нового пользователя
            self.user_data[user_id_str] = {
                'money': 100,
                'health': 100,
                'level': 1,
                'xp': 0,
                'location': '🏠 Дом',
                'inventory': [],
                'last_work': None,
                'last_taxi': None,
                'last_heal': None,
                'pvp_wins': 0,
                'pvp_losses': 0,
                'quests_completed': 0,
                'energy': 100,
                'name': None
            }
        else:
            # Обновляем данные существующего пользователя, добавляя недостающие поля
            user_data = self.user_data[user_id_str]
            default_data = {
                'energy': 100,
                'pvp_wins': 0,
                'pvp_losses': 0,
                'quests_completed': 0,
                'last_heal': None,
                'name': None
            }

            for key, value in default_data.items():
                if key not in user_data:
                    user_data[key] = value

            # Гарантируем, что здоровье не превышает 100
            if user_data['health'] > 100:
                user_data['health'] = 100

            self.user_data[user_id_str] = user_data

        return self.user_data[user_id_str]

    def add_xp(self, user_id, amount):
        user = self.get_user_data(user_id)
        user['xp'] += amount
        while user['xp'] >= user['level'] * 100:
            user['xp'] -= user['level'] * 100
            user['level'] += 1
            return True  # Уровень повышен
        return False

    def generate_captcha(self):
        """Генерация простой капчи"""
        a = random.randint(1, 15)
        b = random.randint(1, 15)
        operation = random.choice(['+', '-', '*'])

        if operation == '+':
            answer = a + b
            question = f"{a} + {b}"
        elif operation == '-':
            # Избегаем отрицательных чисел
            a, b = max(a, b), min(a, b)
            answer = a - b
            question = f"{a} - {b}"
        else:
            answer = a * b
            question = f"{a} × {b}"

        return question, answer

    def get_location_actions(self, location):
        """Возвращает доступные действия в зависимости от локации"""
        location_specific = {
            '🏠 Дом': {
                'rows': [
                    ["❤️ Отдых", "🏪 Магазин", "🎒 Инвентарь"],
                    ["🚕 Такси", "💰 Работа", "🎯 Квесты"],
                    ["👤 Профиль", "📊 Статистика", "❓ Помощь"]
                ]
            },
            '🏢 Офис': {
                'rows': [
                    ["💼 Работать", "📊 Отчет", "☕ Перерыв"],
                    ["🚕 Такси", "🎯 Квесты", "👤 Профиль"],
                    ["📊 Статистика", "🎒 Инвентарь", "❓ Помощь"]
                ]
            },
            '🏪 Магазин': {
                'rows': [
                    ["🛒 Покупки", "💵 Продать", "💰 Торговля"],
                    ["🚕 Такси", "🎯 Квесты", "👤 Профиль"],
                    ["📊 Статистика", "🎒 Инвентарь", "❓ Помощь"]
                ]
            },
            '🎮 Аркада': {
                'rows': [
                    ["🎮 Играть", "🏆 Турнир", "🎰 Слоты"],
                    ["🚕 Такси", "💰 Работа", "👤 Профиль"],
                    ["📊 Статистика", "🎒 Инвентарь", "❓ Помощь"]
                ]
            },
            '🌳 Парк': {
                'rows': [
                    ["🚶 Прогулка", "🎣 Рыбалка", "🌿 Отдых"],
                    ["🚕 Такси", "💰 Работа", "🎯 Квесты"],
                    ["👤 Профиль", "📊 Статистика", "❓ Помощь"]
                ]
            },
            '⚔️ Арена': {
                'rows': [
                    ["⚔️ PvP", "🏆 Рейтинг", "🥊 Тренировка"],
                    ["🚕 Такси", "💰 Работа", "🎯 Квесты"],
                    ["👤 Профиль", "📊 Статистика", "❓ Помощь"]
                ]
            },
            '🏥 Больница': {
                'rows': [
                    ["❤️ Лечение", "💊 Аптека", "🩺 Диагностика"],
                    ["🚕 Такси", "💰 Работа", "🎯 Квесты"],
                    ["👤 Профиль", "📊 Статистика", "❓ Помощь"]
                ]
            }
        }

        return location_specific.get(location, {
            'rows': [
                ["🚕 Такси", "💰 Работа", "🎯 Квесты"],
                ["⚔️ PvP", "👤 Профиль", "📊 Статистика"],
                ["🎒 Инвентарь", "❓ Помощь", "🏠 Дом"]
            ]
        })


game_bot = GameBot()


# Создаем адаптивную клавиатуру в зависимости от локации
def get_main_keyboard(user_id):
    user_data = game_bot.get_user_data(user_id)
    location = user_data['location']
    actions_config = game_bot.get_location_actions(location)

    # Создаем клавиатуру из готовых рядов
    keyboard = []
    for row in actions_config['rows']:
        keyboard_row = []
        for button_text in row:
            keyboard_row.append(KeyboardButton(button_text))
        keyboard.append(keyboard_row)

    return ReplyKeyboardMarkup(keyboard, resize_keyboard=True)


# Команда помощи
async def help_command(update: Update, context: ContextTypes.DEFAULT_TYPE):
    help_text = """
🎮 *РПГ Бот - Помощь и руководство*

*Основные команды:*
/start - Начать игру
/help - Это сообщение
/pvp - Быстрый поиск PvP боя

*Система игры:*
💵 *Деньги* - Зарабатывайте работой и квестами
❤️ *Здоровье* - Восстанавливайте в больнице
⚡ *Энергия* - Тратится на действия, восстанавливается отдыхом
🎯 *Уровень* - Повышайте выполняя задания
⭐ *Опыт* - Накопите для повышения уровня

*Локации и их особенности:*
🏠 *Дом* - Отдых и восстановление
🏢 *Офис* - Работа с бонусами
🏪 *Магазин* - Покупка и продажа предметов
🎮 *Аркада* - Мини-игры и развлечения
🌳 *Парк* - Прогулки и рыбалка
⚔️ *Арена* - PvP бои и рейтинг
🏥 *Больница* - Лечение и аптека

*Советы:*
• Используйте такси для перемещения
• Решайте капчи для работы
• Участвуйте в PvP для быстрого заработка
• Следите за энергией
• Выполняйте квесты для опыта
    """

    await update.message.reply_text(help_text, parse_mode='Markdown')


# Команда быстрого PvP
async def pvp_command(update: Update, context: ContextTypes.DEFAULT_TYPE):
    user_id = update.effective_user.id
    user_data = game_bot.get_user_data(user_id)

    # Если игрок не на арене, предлагаем поехать
    if user_data['location'] != '⚔️ Арена':
        keyboard = [
            [InlineKeyboardButton("🚕 Поехать на Арену", callback_data="taxi_⚔️ Арена")],
            [InlineKeyboardButton("❌ Отмена", callback_data="back_main")]
        ]
        reply_markup = InlineKeyboardMarkup(keyboard)

        await update.message.reply_text(
            "⚔️ Для PvP нужно быть на Арене! Хотите поехать?",
            reply_markup=reply_markup
        )
        return

    await pvp_menu_message(update, context)


# Команда старта
async def start(update: Update, context: ContextTypes.DEFAULT_TYPE):
    user = update.effective_user
    user_data = game_bot.get_user_data(user.id)

    # Сохраняем имя пользователя при первом входе
    if not user_data.get('name'):
        user_data['name'] = user.first_name
        game_bot.save_data()

    welcome_text = f"""
👋 Добро пожаловать в RPG мир, {user.first_name}!

📍 *Локация:* {user_data['location']}
💵 *Деньги:* {user_data['money']} ₽
❤️ *Здоровье:* {user_data['health']}/100
⚡ *Энергия:* {user_data.get('energy', 100)}/100
🎯 *Уровень:* {user_data['level']}

*Используйте кнопки ниже для взаимодействия с миром!*
Для справки используйте /help
    """

    await update.message.reply_text(welcome_text, parse_mode='Markdown', reply_markup=get_main_keyboard(user.id))


# Обработчик текстовых сообщений (кнопки клавиатуры)
async def handle_text(update: Update, context: ContextTypes.DEFAULT_TYPE):
    text = update.message.text
    user_id = update.effective_user.id
    user_data = game_bot.get_user_data(user_id)

    # Проверка на ответ капчи
    if user_id in game_bot.captcha_challenges:
        await check_captcha(update, context)
        return

    location_handlers = {
        '🏠 Дом': handle_home_actions,
        '🏢 Офис': handle_office_actions,
        '🏪 Магазин': handle_shop_actions,
        '🎮 Аркада': handle_arcade_actions,
        '🌳 Парк': handle_park_actions,
        '⚔️ Арена': handle_arena_actions,
        '🏥 Больница': handle_hospital_actions
    }

    handler = location_handlers.get(user_data['location'], handle_global_actions)
    await handler(update, context, text)


# Глобальные действия (доступны везде)
async def handle_global_actions(update: Update, context: ContextTypes.DEFAULT_TYPE, text: str):
    user_id = update.effective_user.id

    if text == "🚕 Такси":
        await taxi_menu_message(update, context)
    elif text == "💰 Работа":
        await start_work_session(update, context)
    elif text == "🎯 Квесты":
        await quests_menu_message(update, context)
    elif text == "👤 Профиль":
        await profile_message(update, context)
    elif text == "📊 Статистика":
        await stats_message(update, context)
    elif text == "🎒 Инвентарь":
        await inventory_message(update, context)
    elif text == "⚔️ PvP":
        await pvp_menu_message(update, context)
    elif text == "❓ Помощь":
        await help_command(update, context)
    elif text == "🏠 Дом":
        await taxi_to_home(update, context)
    else:
        await update.message.reply_text("Неизвестная команда! Используйте /help для справки.",
                                        reply_markup=get_main_keyboard(user_id))


# Действия в доме
async def handle_home_actions(update: Update, context: ContextTypes.DEFAULT_TYPE, text: str):
    user_id = update.effective_user.id
    user_data = game_bot.get_user_data(user_id)

    if text == "❤️ Отдых":
        energy = user_data.get('energy', 100)
        if energy < 100:
            user_data['energy'] = min(100, energy + 30)
            user_data['health'] = min(100, user_data['health'] + 10)
            game_bot.save_data()
            await update.message.reply_text(
                f"💤 Вы отдохнули дома!\n"
                f"❤️ +10 здоровья\n"
                f"⚡ +30 энергии",
                reply_markup=get_main_keyboard(user_id)
            )
        else:
            await update.message.reply_text("Вы уже полны энергии!", reply_markup=get_main_keyboard(user_id))
    elif text == "🏪 Магазин":
        await shop_menu_message(update, context)
    else:
        await handle_global_actions(update, context, text)


# Действия в офисе
async def handle_office_actions(update: Update, context: ContextTypes.DEFAULT_TYPE, text: str):
    user_id = update.effective_user.id
    user_data = game_bot.get_user_data(user_id)

    if text == "💼 Работать":
        await start_work_session(update, context)
    elif text == "📊 Отчет":
        user_data = game_bot.get_user_data(user_id)
        await update.message.reply_text(
            f"📊 *Офисный отчет:*\n"
            f"💼 Доступные вакансии: 3\n"
            f"📈 Бонус к зарплате: +20%\n"
            f"⚡ Расход энергии: -10%\n"
            f"⭐ Доп. опыт: +5",
            parse_mode='Markdown',
            reply_markup=get_main_keyboard(user_id)
        )
    elif text == "☕ Перерыв":
        user_data['energy'] = min(100, user_data.get('energy', 100) + 10)
        game_bot.save_data()
        await update.message.reply_text(
            "☕ Вы выпили кофе на перерыве! +10 ⚡",
            reply_markup=get_main_keyboard(user_id)
        )
    else:
        await handle_global_actions(update, context, text)


# Действия в магазине
async def handle_shop_actions(update: Update, context: ContextTypes.DEFAULT_TYPE, text: str):
    user_id = update.effective_user.id

    if text == "🛒 Покупки":
        await shop_menu_message(update, context)
    elif text == "💵 Продать":
        await sell_menu_message(update, context)
    elif text == "💰 Торговля":
        await update.message.reply_text(
            "💰 *Биржа торговли*\n\n"
            "Здесь вы можете торговать с другими игроками.\n"
            "Функция в разработке!",
            parse_mode='Markdown',
            reply_markup=get_main_keyboard(user_id)
        )
    else:
        await handle_global_actions(update, context, text)


# Действия в аркаде
async def handle_arcade_actions(update: Update, context: ContextTypes.DEFAULT_TYPE, text: str):
    user_id = update.effective_user.id
    user_data = game_bot.get_user_data(user_id)

    if text == "🎮 Играть":
        if user_data['money'] >= 10:
            user_data['money'] -= 10
            prize = random.randint(0, 50)
            if prize > 0:
                user_data['money'] += prize
                await update.message.reply_text(
                    f"🎮 Вы сыграли в аркаду!\n"
                    f"💵 Выигрыш: {prize} ₽\n"
                    f"⭐ +5 опета",
                    reply_markup=get_main_keyboard(user_id)
                )
            else:
                await update.message.reply_text(
                    "🎮 Вы сыграли в аркаду, но не выиграли ничего 😢",
                    reply_markup=get_main_keyboard(user_id)
                )
            game_bot.add_xp(user_id, 5)
            game_bot.save_data()
        else:
            await update.message.reply_text("Недостаточно денег для игры!", reply_markup=get_main_keyboard(user_id))
    elif text == "🏆 Турнир":
        await update.message.reply_text(
            "🏆 *Аркадный турнир*\n\n"
            "Следующий турнир начнется через 2 часа!\n"
            "Призовой фонд: 500 ₽\n"
            "Вход: 25 ₽",
            parse_mode='Markdown',
            reply_markup=get_main_keyboard(user_id)
        )
    elif text == "🎰 Слоты":
        await play_slots(update, context)
    else:
        await handle_global_actions(update, context, text)


# Действия в парке
async def handle_park_actions(update: Update, context: ContextTypes.DEFAULT_TYPE, text: str):
    user_id = update.effective_user.id
    user_data = game_bot.get_user_data(user_id)

    if text == "🚶 Прогулка":
        user_data['health'] = min(100, user_data['health'] + 5)
        user_data['energy'] = min(100, user_data.get('energy', 100) + 10)
        game_bot.add_xp(user_id, 3)
        game_bot.save_data()
        await update.message.reply_text(
            f"🌳 Приятная прогулка в парке!\n"
            f"❤️ +5 здоровья\n"
            f"⚡ +10 энергии\n"
            f"⭐ +3 опыта",
            reply_markup=get_main_keyboard(user_id)
        )
    elif text == "🎣 Рыбалка":
        await start_fishing(update, context)
    elif text == "🌿 Отдых":
        user_data['energy'] = min(100, user_data.get('energy', 100) + 15)
        game_bot.save_data()
        await update.message.reply_text(
            "🌿 Вы расслабились на природе! +15 ⚡",
            reply_markup=get_main_keyboard(user_id)
        )
    else:
        await handle_global_actions(update, context, text)


# Действия на арене
async def handle_arena_actions(update: Update, context: ContextTypes.DEFAULT_TYPE, text: str):
    user_id = update.effective_user.id

    if text == "⚔️ PvP":
        await pvp_menu_message(update, context)
    elif text == "🏆 Рейтинг":
        await show_arena_ranking(update, context)
    elif text == "🥊 Тренировка":
        user_data = game_bot.get_user_data(user_id)
        if user_data.get('energy', 100) >= 15:
            user_data['energy'] = max(0, user_data['energy'] - 15)
            xp_gained = random.randint(5, 15)
            game_bot.add_xp(user_id, xp_gained)
            game_bot.save_data()
            await update.message.reply_text(
                f"🥊 Вы потренировались на манекенах!\n"
                f"⚡ -15 энергии\n"
                f"⭐ +{xp_gained} опыта",
                reply_markup=get_main_keyboard(user_id)
            )
        else:
            await update.message.reply_text("❌ Недостаточно энергии для тренировки!")
    else:
        await handle_global_actions(update, context, text)


# Действия в больнице
async def handle_hospital_actions(update: Update, context: ContextTypes.DEFAULT_TYPE, text: str):
    user_id = update.effective_user.id
    user_data = game_bot.get_user_data(user_id)

    if text == "❤️ Лечение":
        heal_cost = (100 - user_data['health']) * 1  # 1 монета за 1 HP
        if user_data['money'] >= heal_cost:
            user_data['money'] -= heal_cost
            user_data['health'] = 100
            game_bot.save_data()
            await update.message.reply_text(
                f"🏥 Вы полностью вылечились!\n"
                f"💵 Стоимость: {heal_cost} ₽",
                reply_markup=get_main_keyboard(user_id)
            )
        else:
            await update.message.reply_text(f"Недостаточно денег для лечения! Нужно {heal_cost} ₽")
    elif text == "💊 Аптека":
        await pharmacy_menu(update, context)
    elif text == "🩺 Диагностика":
        await update.message.reply_text(
            f"🩺 *Медицинская диагностика:*\n\n"
            f"❤️ Здоровье: {user_data['health']}/100\n"
            f"⚡ Энергия: {user_data.get('energy', 100)}/100\n"
            f"💊 Рекомендации: {'Отлично!' if user_data['health'] > 80 else 'Нужен отдых' if user_data['health'] > 50 else 'Срочное лечение!'}",
            parse_mode='Markdown',
            reply_markup=get_main_keyboard(user_id)
        )
    else:
        await handle_global_actions(update, context, text)


# ========== СИСТЕМА КАПЧИ ДЛЯ РАБОТЫ ==========
async def start_work_session(update: Update, context: ContextTypes.DEFAULT_TYPE):
    user_id = update.effective_user.id
    user_data = game_bot.get_user_data(user_id)

    # Проверка кулдауна работы
    current_time = datetime.now()
    if user_data.get('last_work'):
        last_work = datetime.fromisoformat(user_data['last_work'])
        if (current_time - last_work).seconds < 300:  # 5 минут
            await update.message.reply_text("⏳ Вы уже работали недавно! Подождите 5 минут.")
            return

    # Проверка энергии
    energy = user_data.get('energy', 100)
    if energy < 20:
        await update.message.reply_text("⚡ Недостаточно энергии для работы! Отдохните.")
        return

    # Генерация капчи
    question, answer = game_bot.generate_captcha()
    game_bot.captcha_challenges[user_id] = answer
    game_bot.work_sessions[user_id] = True

    await update.message.reply_text(
        f"💼 Для начала работы решите капчу:\n\n"
        f"🔢 {question} = ?\n\n"
        f"Отправьте ответ числом:"
    )


async def check_captcha(update: Update, context: ContextTypes.DEFAULT_TYPE):
    user_id = update.effective_user.id
    user_text = update.message.text

    try:
        user_answer = int(user_text)
        correct_answer = game_bot.captcha_challenges.get(user_id)

        if user_answer == correct_answer:
            # Капча пройдена - начисляем награду
            await process_work_reward(update, context)
            del game_bot.captcha_challenges[user_id]
            del game_bot.work_sessions[user_id]
        else:
            await update.message.reply_text("❌ Неправильный ответ! Попробуйте снова:")
    except ValueError:
        await update.message.reply_text("❌ Отправьте число!")


async def process_work_reward(update: Update, context: ContextTypes.DEFAULT_TYPE):
    user_id = update.effective_user.id
    user_data = game_bot.get_user_data(user_id)

    # Заработок в зависимости от уровня
    base_earn = random.randint(40, 70)
    level_bonus = user_data['level'] * 5
    earned = base_earn + level_bonus

    # Бонус за локацию
    location_bonus = {
        '🏢 Офис': 20,
        '🏠 Дом': 0,
        '🏪 Магазин': 10
    }
    earned += location_bonus.get(user_data['location'], 0)

    user_data['money'] += earned
    user_data['energy'] = max(0, user_data.get('energy', 100) - 20)
    user_data['last_work'] = datetime.now().isoformat()

    # Добавляем опыт
    xp_earned = 15
    level_up = game_bot.add_xp(user_id, xp_earned)

    game_bot.save_data()

    message = (f"✅ Капча пройдена! Работа выполнена!\n"
               f"💵 Заработано: {earned} ₽\n"
               f"⚡ -20 энергии\n"
               f"⭐ +{xp_earned} опыта")

    if level_up:
        message += f"\n🎉 Поздравляем! Вы достигли {user_data['level']} уровня!"

    await update.message.reply_text(message, reply_markup=get_main_keyboard(user_id))


# ========== ОСНОВНЫЕ ФУНКЦИИ ==========
async def taxi_menu_message(update: Update, context: ContextTypes.DEFAULT_TYPE):
    user_id = update.effective_user.id
    user_data = game_bot.get_user_data(user_id)

    locations = {
        "🏠 Дом": 10,
        "🏢 Офис": 20,
        "🏪 Магазин": 15,
        "🎮 Аркада": 25,
        "🌳 Парк": 12,
        "⚔️ Арена": 30,
        "🏥 Больница": 18
    }

    keyboard = []
    for location, price in locations.items():
        if location != user_data['location']:  # Не показываем текущую локацию
            keyboard.append([InlineKeyboardButton(f"{location} - {price} ₽", callback_data=f"taxi_{location}")])

    reply_markup = InlineKeyboardMarkup(keyboard)
    await update.message.reply_text("🚕 Выберите место назначения:", reply_markup=reply_markup)


async def take_taxi(update: Update, context: ContextTypes.DEFAULT_TYPE):
    query = update.callback_query
    await query.answer()

    user_id = query.from_user.id
    user_data = game_bot.get_user_data(user_id)
    destination = query.data.split('_')[1]

    locations = {
        "🏠 Дом": 10,
        "🏢 Офис": 20,
        "🏪 Магазин": 15,
        "🎮 Аркада": 25,
        "🌳 Парк": 12,
        "⚔️ Арена": 30,
        "🏥 Больница": 18
    }

    price = locations.get(destination, 20)

    if user_data['money'] < price:
        await query.edit_message_text(f"❌ Недостаточно денег! Нужно {price} ₽")
        return

    # Проверка кулдауна такси
    current_time = datetime.now()
    if user_data.get('last_taxi'):
        last_taxi = datetime.fromisoformat(user_data['last_taxi'])
        if (current_time - last_taxi).seconds < 60:
            await query.edit_message_text("⏳ Вы недавно уже ездили на такси! Подождите 1 минуту.")
            return

    user_data['money'] -= price
    user_data['location'] = destination
    user_data['last_taxi'] = current_time.isoformat()

    # Добавляем опыт
    level_up = game_bot.add_xp(user_id, 5)

    game_bot.save_data()

    message = f"🚕 Вы доехали до {destination}!\n💵 Потрачено: {price} ₽\n⭐ +5 опыта"
    if level_up:
        message += f"\n🎉 Поздравляем! Вы достигли {user_data['level']} уровня!"

    await query.edit_message_text(message)
    # Обновляем клавиатуру с новыми действиями для локации
    await query.message.reply_text("Исследуйте новую локацию!", reply_markup=get_main_keyboard(user_id))


async def quests_menu_message(update: Update, context: ContextTypes.DEFAULT_TYPE):
    quests = [
        {"name": "📦 Доставить посылку", "reward": 100, "xp": 20, "energy": 15},
        {"name": "🛒 Сделать покупки", "reward": 80, "xp": 15, "energy": 10},
        {"name": "🔍 Найти кота", "reward": 150, "xp": 25, "energy": 20},
        {"name": "📚 Отнести документы", "reward": 120, "xp": 18, "energy": 12}
    ]

    keyboard = []
    for quest in quests:
        keyboard.append([InlineKeyboardButton(
            f"{quest['name']} - {quest['reward']} ₽",
            callback_data=f"quest_{quests.index(quest)}"
        )])

    reply_markup = InlineKeyboardMarkup(keyboard)
    await update.message.reply_text("🎯 Доступные квесты:", reply_markup=reply_markup)


async def start_quest(update: Update, context: ContextTypes.DEFAULT_TYPE):
    query = update.callback_query
    await query.answer()

    user_id = query.from_user.id
    user_data = game_bot.get_user_data(user_id)
    quest_index = int(query.data.split('_')[1])

    quests = [
        {"name": "📦 Доставить посылку", "reward": 100, "xp": 20, "energy": 15},
        {"name": "🛒 Сделать покупки", "reward": 80, "xp": 15, "energy": 10},
        {"name": "🔍 Найти кота", "reward": 150, "xp": 25, "energy": 20},
        {"name": "📚 Отнести документы", "reward": 120, "xp": 18, "energy": 12}
    ]

    quest = quests[quest_index]

    # Проверка энергии
    energy = user_data.get('energy', 100)
    if energy < quest['energy']:
        await query.edit_message_text(f"❌ Недостаточно энергии! Нужно {quest['energy']} ⚡")
        return

    # Шанс успеха зависит от уровня
    success_chance = 0.7 + (user_data['level'] * 0.02)
    success_chance = min(success_chance, 0.95)

    if random.random() < success_chance:
        user_data['money'] += quest['reward']
        user_data['energy'] = max(0, energy - quest['energy'])
        user_data['quests_completed'] = user_data.get('quests_completed', 0) + 1
        level_up = game_bot.add_xp(user_id, quest['xp'])

        message = (f"✅ Квест '{quest['name']}' выполнен успешно!\n"
                   f"💵 Награда: {quest['reward']} ₽\n"
                   f"⚡ -{quest['energy']} энергии\n"
                   f"⭐ +{quest['xp']} опыта")
        if level_up:
            message += f"\n🎉 Поздравляем! Вы достигли {user_data['level']} уровня!"
    else:
        user_data['energy'] = max(0, energy - quest['energy'] // 2)
        message = f"❌ Квест '{quest['name']}' провален! Потеряно {quest['energy'] // 2} ⚡"

    game_bot.save_data()
    await query.edit_message_text(message)
    await query.message.reply_text("Выберите действие:", reply_markup=get_main_keyboard(user_id))


async def profile_message(update: Update, context: ContextTypes.DEFAULT_TYPE):
    user_id = update.effective_user.id
    user_data = game_bot.get_user_data(user_id)

    profile_text = (f"👤 Профиль {update.effective_user.first_name}\n\n"
                    f"💵 Деньги: {user_data['money']} ₽\n"
                    f"❤️ Здоровье: {user_data['health']}/100\n"
                    f"⚡ Энергия: {user_data.get('energy', 100)}/100\n"
                    f"🎯 Уровень: {user_data['level']}\n"
                    f"⭐ Опыт: {user_data['xp']}/{user_data['level'] * 100}\n"
                    f"📍 Локация: {user_data['location']}\n"
                    f"⚔️ PvP: {user_data.get('pvp_wins', 0)} побед / {user_data.get('pvp_losses', 0)} поражений\n"
                    f"🎯 Квестов выполнено: {user_data.get('quests_completed', 0)}\n"
                    f"🎒 Инвентарь: {len(user_data['inventory'])} предметов")

    await update.message.reply_text(profile_text, reply_markup=get_main_keyboard(user_id))


async def stats_message(update: Update, context: ContextTypes.DEFAULT_TYPE):
    user_id = update.effective_user.id
    user_data = game_bot.get_user_data(user_id)

    total_users = len(game_bot.user_data)
    total_money = sum(data['money'] for data in game_bot.user_data.values())
    avg_level = sum(data['level'] for data in game_bot.user_data.values()) / total_users if total_users > 0 else 0

    stats_text = (f"📊 Глобальная статистика:\n\n"
                  f"👥 Всего игроков: {total_users}\n"
                  f"💰 Общая денежная масса: {total_money} ₽\n"
                  f"📈 Средний уровень: {avg_level:.1f}\n\n"
                  f"🏆 Топ игроков:\n")

    # Топ 5 игроков по уровню
    top_players = sorted(game_bot.user_data.items(),
                         key=lambda x: (x[1]['level'], x[1]['xp']),
                         reverse=True)[:5]

    for i, (player_id, data) in enumerate(top_players, 1):
        stats_text += f"{i}. Ур. {data['level']} - {data.get('name', 'Игрок')}\n"

    await update.message.reply_text(stats_text, reply_markup=get_main_keyboard(user_id))


async def inventory_message(update: Update, context: ContextTypes.DEFAULT_TYPE):
    user_id = update.effective_user.id
    user_data = game_bot.get_user_data(user_id)

    if not user_data['inventory']:
        await update.message.reply_text("🎒 Ваш инвентарь пуст!", reply_markup=get_main_keyboard(user_id))
        return

    inventory_text = "🎒 Ваш инвентарь:\n\n"
    for item in user_data['inventory']:
        inventory_text += f"• {item}\n"

    await update.message.reply_text(inventory_text, reply_markup=get_main_keyboard(user_id))


async def shop_menu_message(update: Update, context: ContextTypes.DEFAULT_TYPE):
    user_id = update.effective_user.id
    user_data = game_bot.get_user_data(user_id)

    items = [
        {"name": "💊 Энергетик", "price": 30, "effect": "energy", "value": 30},
        {"name": "❤️ Аптечка", "price": 50, "effect": "health", "value": 30},
        {"name": "🛡️ Броня", "price": 100, "effect": "item", "value": "Броня"},
        {"name": "⚔️ Меч", "price": 120, "effect": "item", "value": "Меч"},
        {"name": "🎒 Рюкзак", "price": 80, "effect": "item", "value": "Рюкзак"}
    ]

    keyboard = []
    for item in items:
        keyboard.append([InlineKeyboardButton(
            f"{item['name']} - {item['price']} ₽",
            callback_data=f"buy_{items.index(item)}"
        )])

    reply_markup = InlineKeyboardMarkup(keyboard)
    await update.message.reply_text("🏪 Магазин - выберите товар:", reply_markup=reply_markup)


async def buy_item(update: Update, context: ContextTypes.DEFAULT_TYPE):
    query = update.callback_query
    await query.answer()

    user_id = query.from_user.id
    user_data = game_bot.get_user_data(user_id)
    item_index = int(query.data.split('_')[1])

    items = [
        {"name": "💊 Энергетик", "price": 30, "effect": "energy", "value": 30},
        {"name": "❤️ Аптечка", "price": 50, "effect": "health", "value": 30},
        {"name": "🛡️ Броня", "price": 100, "effect": "item", "value": "Броня"},
        {"name": "⚔️ Меч", "price": 120, "effect": "item", "value": "Меч"},
        {"name": "🎒 Рюкзак", "price": 80, "effect": "item", "value": "Рюкзак"}
    ]

    item = items[item_index]

    if user_data['money'] >= item['price']:
        user_data['money'] -= item['price']

        if item['effect'] == 'energy':
            user_data['energy'] = min(100, user_data.get('energy', 100) + item['value'])
            message = f"✅ Куплен {item['name']}! +{item['value']} ⚡"
        elif item['effect'] == 'health':
            user_data['health'] = min(100, user_data['health'] + item['value'])
            message = f"✅ Куплен {item['name']}! +{item['value']} ❤️"
        else:
            user_data['inventory'].append(item['value'])
            message = f"✅ Куплен {item['name']}! Добавлен в инвентарь."

        game_bot.save_data()
    else:
        message = "❌ Недостаточно денег для покупки!"

    await query.edit_message_text(message)
    await query.message.reply_text("Выберите действие:", reply_markup=get_main_keyboard(user_id))


async def sell_menu_message(update: Update, context: ContextTypes.DEFAULT_TYPE):
    user_id = update.effective_user.id
    user_data = game_bot.get_user_data(user_id)

    if not user_data['inventory']:
        await update.message.reply_text("❌ В инвентаре нет предметов для продажи!")
        return

    keyboard = []
    for item in user_data['inventory']:
        price = len(item) * 10  # Цена зависит от длины названия
        keyboard.append([InlineKeyboardButton(f"{item} - {price} ₽", callback_data=f"sell_{item}")])

    reply_markup = InlineKeyboardMarkup(keyboard)
    await update.message.reply_text("💵 Выберите предмет для продажи:", reply_markup=reply_markup)


async def sell_item(update: Update, context: ContextTypes.DEFAULT_TYPE):
    query = update.callback_query
    await query.answer()

    user_id = query.from_user.id
    user_data = game_bot.get_user_data(user_id)
    item_name = query.data.split('_')[1]

    if item_name in user_data['inventory']:
        user_data['inventory'].remove(item_name)
        price = len(item_name) * 10
        user_data['money'] += price
        game_bot.save_data()
        message = f"✅ Продано {item_name} за {price} ₽"
    else:
        message = "❌ Предмет не найден в инвентаре!"

    await query.edit_message_text(message)
    await query.message.reply_text("Выберите действие:", reply_markup=get_main_keyboard(user_id))


async def start_fishing(update: Update, context: ContextTypes.DEFAULT_TYPE):
    user_id = update.effective_user.id
    user_data = game_bot.get_user_data(user_id)

    energy = user_data.get('energy', 100)
    if energy < 10:
        await update.message.reply_text("❌ Недостаточно энергии для рыбалки!")
        return

    user_data['energy'] = max(0, energy - 10)
    catch_chance = random.random()

    if catch_chance < 0.6:  # 60% шанс поймать рыбу
        fish_types = [
            {"name": "🐟 Маленькая рыбка", "price": 15},
            {"name": "🐠 Яркая рыбка", "price": 25},
            {"name": "🦈 Большая рыба", "price": 40},
            {"name": "🎣 Редкая рыба", "price": 60}
        ]
        fish = random.choice(fish_types)
        user_data['money'] += fish['price']
        user_data['inventory'].append(fish['name'])
        message = f"🎣 Вы поймали {fish['name']} и продали за {fish['price']} ₽!"
    elif catch_chance < 0.8:  # 20% шанс найти сокровище
        treasure = random.randint(50, 100)
        user_data['money'] += treasure
        message = f"💎 Вы нашли сокровище! +{treasure} ₽"
    else:  # 20% шанс ничего не поймать
        message = "🎣 К сожалению, вы ничего не поймали."

    game_bot.add_xp(user_id, 8)
    game_bot.save_data()

    await update.message.reply_text(message, reply_markup=get_main_keyboard(user_id))


async def show_arena_ranking(update: Update, context: ContextTypes.DEFAULT_TYPE):
    user_id = update.effective_user.id

    # Топ 5 игроков по победам в PvP
    top_fighters = sorted(game_bot.user_data.items(),
                          key=lambda x: x[1].get('pvp_wins', 0),
                          reverse=True)[:5]

    ranking_text = "🏆 Рейтинг бойцов:\n\n"
    for i, (player_id, data) in enumerate(top_fighters, 1):
        wins = data.get('pvp_wins', 0)
        losses = data.get('pvp_losses', 0)
        total = wins + losses
        win_rate = wins / max(1, total) * 100
        ranking_text += f"{i}. {data.get('name', 'Боец')} - {wins} побед ({win_rate:.1f}%)\n"

    await update.message.reply_text(ranking_text, reply_markup=get_main_keyboard(user_id))


async def pharmacy_menu(update: Update, context: ContextTypes.DEFAULT_TYPE):
    user_id = update.effective_user.id

    keyboard = [
        [InlineKeyboardButton("💊 Малое зелье здоровья - 30 ₽", callback_data="potion_small")],
        [InlineKeyboardButton("❤️ Большое зелье здоровья - 60 ₽", callback_data="potion_large")],
        [InlineKeyboardButton("⚡ Энергетик - 25 ₽", callback_data="potion_energy")]
    ]
    reply_markup = InlineKeyboardMarkup(keyboard)

    await update.message.reply_text("💊 Аптека - выберите зелье:", reply_markup=reply_markup)


async def buy_potion(update: Update, context: ContextTypes.DEFAULT_TYPE):
    query = update.callback_query
    await query.answer()

    user_id = query.from_user.id
    user_data = game_bot.get_user_data(user_id)
    potion_type = query.data.split('_')[1]

    potions = {
        'small': {'price': 30, 'health': 30, 'name': 'Малое зелье здоровья'},
        'large': {'price': 60, 'health': 60, 'name': 'Большое зелье здоровья'},
        'energy': {'price': 25, 'energy': 25, 'name': 'Энергетик'}
    }

    potion = potions[potion_type]

    if user_data['money'] >= potion['price']:
        user_data['money'] -= potion['price']

        if 'health' in potion:
            user_data['health'] = min(100, user_data['health'] + potion['health'])
            message = f"✅ Куплено {potion['name']}! +{potion['health']} ❤️"
        else:
            user_data['energy'] = min(100, user_data.get('energy', 100) + potion['energy'])
            message = f"✅ Куплен {potion['name']}! +{potion['energy']} ⚡"

        game_bot.save_data()
    else:
        message = "❌ Недостаточно денег!"

    await query.edit_message_text(message)
    await query.message.reply_text("Выберите действие:", reply_markup=get_main_keyboard(user_id))


# ========== УЛУЧШЕННАЯ PvP СИСТЕМА ==========
async def pvp_menu_message(update: Update, context: ContextTypes.DEFAULT_TYPE):
    user_id = update.effective_user.id
    user_data = game_bot.get_user_data(user_id)

    if user_data['location'] != '⚔️ Арена':
        keyboard = [
            [InlineKeyboardButton("🚕 Поехать на Арену", callback_data="taxi_⚔️ Арена")],
            [InlineKeyboardButton("❌ Отмена", callback_data="back_main")]
        ]
        reply_markup = InlineKeyboardMarkup(keyboard)

        await update.message.reply_text(
            "❌ PvP доступен только на Арене! Хотите поехать?",
            reply_markup=reply_markup
        )
        return

    # Ищем противников с разными уровнями сложности
    all_users = list(game_bot.user_data.keys())
    if len(all_users) < 2:
        await update.message.reply_text("❌ Не найдено противников для PvP!")
        return

    user_id_str = str(user_id)
    opponents = [uid for uid in all_users if uid != user_id_str and game_bot.user_data[uid]['health'] > 0]

    if not opponents:
        await update.message.reply_text("❌ Не найдено противников для PvP!")
        return

    # Сортируем противников по разнице в уровнях
    user_level = user_data['level']
    opponents_with_diff = []

    for opponent_id in opponents:
        opponent_data = game_bot.user_data[opponent_id]
        level_diff = abs(user_level - opponent_data['level'])
        opponents_with_diff.append((opponent_id, level_diff, opponent_data))

    # Выбираем противников разной сложности
    easy_opponents = [o for o in opponents_with_diff if o[1] >= 3]  # Легкие (разница ≥ 3)
    medium_opponents = [o for o in opponents_with_diff if 1 <= o[1] <= 2]  # Средние
    hard_opponents = [o for o in opponents_with_diff if o[1] == 0]  # Сложные (равные по уровню)

    # Создаем меню выбора сложности
    keyboard = []

    if easy_opponents:
        opp_id, diff, opp_data = random.choice(easy_opponents)
        keyboard.append([InlineKeyboardButton(
            f"🥉 Новичок (ур. {opp_data['level']}) - 30 ₽",
            callback_data=f"pvp_easy_{opp_id}"
        )])

    if medium_opponents:
        opp_id, diff, opp_data = random.choice(medium_opponents)
        keyboard.append([InlineKeyboardButton(
            f"🥈 Опытный (ур. {opp_data['level']}) - 50 ₽",
            callback_data=f"pvp_medium_{opp_id}"
        )])

    if hard_opponents:
        opp_id, diff, opp_data = random.choice(hard_opponents)
        keyboard.append([InlineKeyboardButton(
            f"🥇 Мастер (ур. {opp_data['level']}) - 80 ₽",
            callback_data=f"pvp_hard_{opp_id}"
        )])

    keyboard.append([InlineKeyboardButton("🔙 Назад", callback_data="back_main")])

    reply_markup = InlineKeyboardMarkup(keyboard)

    await update.message.reply_text(
        f"⚔️ *Выберите противника:*\n\n"
        f"🥉 *Новичок* - Легкий бой, маленькая награда\n"
        f"🥈 *Опытный* - Средняя сложность, хорошая награда\n"
        f"🥇 *Мастер* - Сложный бой, большая награда\n\n"
        f"Ваш уровень: {user_level}",
        parse_mode='Markdown',
        reply_markup=reply_markup
    )


async def start_pvp_battle(update: Update, context: ContextTypes.DEFAULT_TYPE):
    query = update.callback_query
    await query.answer()

    user_id = query.from_user.id
    user_data = game_bot.get_user_data(user_id)

    data_parts = query.data.split('_')
    difficulty = data_parts[1]
    opponent_id = data_parts[2]

    # Ставки в зависимости от сложности
    bets = {
        'easy': 30,
        'medium': 50,
        'hard': 80
    }

    bet = bets.get(difficulty, 50)
    opponent_data = game_bot.user_data[opponent_id]

    # Проверка денег
    if user_data['money'] < bet or opponent_data['money'] < bet:
        await query.edit_message_text("❌ У одного из игроков недостаточно денег для PvP!")
        return

    # Сохраняем данные о битве
    context.user_data['pvp_opponent'] = opponent_id
    context.user_data['pvp_bet'] = bet
    context.user_data['pvp_difficulty'] = difficulty

    # Бонусы в зависимости от сложности
    difficulty_bonuses = {
        'easy': {'user_bonus': 1.2, 'opponent_bonus': 0.8},
        'medium': {'user_bonus': 1.0, 'opponent_bonus': 1.0},
        'hard': {'user_bonus': 0.8, 'opponent_bonus': 1.2}
    }

    bonus = difficulty_bonuses.get(difficulty, {'user_bonus': 1.0, 'opponent_bonus': 1.0})
    context.user_data['pvp_bonus'] = bonus

    keyboard = [
        [InlineKeyboardButton("⚔️ Атаковать", callback_data="pvp_attack")],
        [InlineKeyboardButton("🛡️ Защищаться", callback_data="pvp_defend")],
        [InlineKeyboardButton("💥 Супер удар", callback_data="pvp_special")],
        [InlineKeyboardButton("🏃 Бежать", callback_data="back_main")]
    ]
    reply_markup = InlineKeyboardMarkup(keyboard)

    await query.edit_message_text(
        f"⚔️ *Начало битвы!*\n\n"
        f"👤 *Противник:* Уровень {opponent_data['level']}\n"
        f"❤️ *Здоровье:* {opponent_data['health']}/100\n"
        f"💵 *Ставка:* {bet} ₽\n"
        f"🎯 *Сложность:* {['Новичок', 'Опытный', 'Мастер'][['easy', 'medium', 'hard'].index(difficulty)]}\n\n"
        f"*Выберите действие:*",
        parse_mode='Markdown',
        reply_markup=reply_markup
    )


async def pvp_attack(update: Update, context: ContextTypes.DEFAULT_TYPE):
    query = update.callback_query
    await query.answer()

    user_id = query.from_user.id
    user_data = game_bot.get_user_data(user_id)
    opponent_id = context.user_data.get('pvp_opponent')
    bet = context.user_data.get('pvp_bet', 50)
    difficulty = context.user_data.get('pvp_difficulty', 'medium')
    bonus = context.user_data.get('pvp_bonus', {'user_bonus': 1.0, 'opponent_bonus': 1.0})

    if not opponent_id:
        await query.edit_message_text("❌ Противник не найден!")
        return

    opponent_data = game_bot.user_data[opponent_id]

    # Расчет урона с учетом бонусов сложности
    user_damage = int((random.randint(15, 25) + user_data['level']) * bonus['user_bonus'])
    opponent_damage = int((random.randint(15, 25) + opponent_data['level']) * bonus['opponent_bonus'])

    # Бой
    user_data['health'] -= opponent_damage
    opponent_data['health'] -= user_damage

    if user_data['health'] <= 0 and opponent_data['health'] <= 0:
        # Ничья
        result = "🤝 *Ничья!* Оба игрока проиграли."
        user_data['health'] = 1
        opponent_data['health'] = 1
        user_data['pvp_losses'] = user_data.get('pvp_losses', 0) + 1
        opponent_data['pvp_losses'] = opponent_data.get('pvp_losses', 0) + 1
    elif user_data['health'] <= 0:
        # Проигрыш
        result = "❌ *Вы проиграли!*"
        user_data['money'] -= bet
        opponent_data['money'] += bet
        user_data['health'] = 1
        user_data['pvp_losses'] = user_data.get('pvp_losses', 0) + 1
        opponent_data['pvp_wins'] = opponent_data.get('pvp_wins', 0) + 1
    elif opponent_data['health'] <= 0:
        # Победа
        result = "🎉 *Вы победили!*"
        user_data['money'] += bet
        opponent_data['money'] -= bet
        opponent_data['health'] = 1
        user_data['pvp_wins'] = user_data.get('pvp_wins', 0) + 1
        opponent_data['pvp_losses'] = opponent_data.get('pvp_losses', 0) + 1

        # Награда за победу в зависимости от сложности
        xp_rewards = {'easy': 30, 'medium': 50, 'hard': 80}
        xp_earned = xp_rewards.get(difficulty, 50)
        level_up = game_bot.add_xp(user_id, xp_earned)

        if level_up:
            result += f"\n🎉 *Поздравляем!* Вы достигли {user_data['level']} уровня!"
    else:
        # Бой продолжается
        result = (f"⚔️ *Бой продолжается!*\n"
                  f"💥 Вы нанесли *{user_damage}* урона\n"
                  f"🛡️ Получили *{opponent_damage}* урона")

    game_bot.save_data()

    battle_text = (f"{result}\n\n"
                   f"❤️ *Ваше здоровье:* {max(1, user_data['health'])}/100\n"
                   f"❤️ *Здоровье противника:* {max(1, opponent_data['health'])}/100")

    await query.edit_message_text(battle_text, parse_mode='Markdown')

    if user_data['health'] > 0 and opponent_data['health'] > 0:
        # Продолжаем бой
        keyboard = [
            [InlineKeyboardButton("⚔️ Атаковать", callback_data="pvp_attack")],
            [InlineKeyboardButton("🛡️ Защищаться", callback_data="pvp_defend")],
            [InlineKeyboardButton("💥 Супер удар", callback_data="pvp_special")],
            [InlineKeyboardButton("🏃 Бежать", callback_data="back_main")]
        ]
        reply_markup = InlineKeyboardMarkup(keyboard)
        await query.message.reply_text("*Выберите действие:*", parse_mode='Markdown', reply_markup=reply_markup)
    else:
        await query.message.reply_text("Выберите действие:", reply_markup=get_main_keyboard(user_id))


async def pvp_defend(update: Update, context: ContextTypes.DEFAULT_TYPE):
    query = update.callback_query
    await query.answer()

    user_id = query.from_user.id
    user_data = game_bot.get_user_data(user_id)
    opponent_id = context.user_data.get('pvp_opponent')

    if not opponent_id:
        await query.edit_message_text("❌ Противник не найден!")
        return

    opponent_data = game_bot.user_data[opponent_id]

    # При защите получаем меньше урона
    user_damage = random.randint(10, 20) + user_data['level']
    opponent_damage = random.randint(5, 15) + opponent_data['level'] // 2  # Меньше урона при защите

    user_data['health'] -= opponent_damage
    opponent_data['health'] -= user_damage

    result = (f"🛡️ *Вы защищаетесь!*\n"
              f"💥 Вы нанесли *{user_damage}* урона\n"
              f"🛡️ Получили всего *{opponent_damage}* урона")

    game_bot.save_data()

    battle_text = (f"{result}\n\n"
                   f"❤️ *Ваше здоровье:* {max(1, user_data['health'])}/100\n"
                   f"❤️ *Здоровье противника:* {max(1, opponent_data['health'])}/100")

    await query.edit_message_text(battle_text, parse_mode='Markdown')

    if user_data['health'] > 0 and opponent_data['health'] > 0:
        keyboard = [
            [InlineKeyboardButton("⚔️ Атаковать", callback_data="pvp_attack")],
            [InlineKeyboardButton("🛡️ Защищаться", callback_data="pvp_defend")],
            [InlineKeyboardButton("💥 Супер удар", callback_data="pvp_special")],
            [InlineKeyboardButton("🏃 Бежать", callback_data="back_main")]
        ]
        reply_markup = InlineKeyboardMarkup(keyboard)
        await query.message.reply_text("*Выберите действие:*", parse_mode='Markdown', reply_markup=reply_markup)
    else:
        await query.message.reply_text("Выберите действие:", reply_markup=get_main_keyboard(user_id))


async def pvp_special(update: Update, context: ContextTypes.DEFAULT_TYPE):
    query = update.callback_query
    await query.answer()

    user_id = query.from_user.id
    user_data = game_bot.get_user_data(user_id)
    opponent_id = context.user_data.get('pvp_opponent')

    if not opponent_id:
        await query.edit_message_text("❌ Противник не найден!")
        return

    # Проверка энергии для супер удара
    if user_data.get('energy', 100) < 30:
        await query.answer("❌ Недостаточно энергии для супер удара!", show_alert=True)
        return

    opponent_data = game_bot.user_data[opponent_id]

    # Супер удар - больше урона, но тратит энергию
    user_data['energy'] = max(0, user_data['energy'] - 30)
    user_damage = random.randint(25, 40) + user_data['level'] * 2
    opponent_damage = random.randint(15, 25) + opponent_data['level']

    user_data['health'] -= opponent_damage
    opponent_data['health'] -= user_damage

    result = (f"💥 *СУПЕР УДАР!*\n"
              f"💥 Вы нанесли *{user_damage}* урона!\n"
              f"🛡️ Получили *{opponent_damage}* урона\n"
              f"⚡ Потрачено 30 энергии")

    game_bot.save_data()

    battle_text = (f"{result}\n\n"
                   f"❤️ *Ваше здоровье:* {max(1, user_data['health'])}/100\n"
                   f"❤️ *Здоровье противника:* {max(1, opponent_data['health'])}/100")

    await query.edit_message_text(battle_text, parse_mode='Markdown')

    if user_data['health'] > 0 and opponent_data['health'] > 0:
        keyboard = [
            [InlineKeyboardButton("⚔️ Атаковать", callback_data="pvp_attack")],
            [InlineKeyboardButton("🛡️ Защищаться", callback_data="pvp_defend")],
            [InlineKeyboardButton("💥 Супер удар", callback_data="pvp_special")],
            [InlineKeyboardButton("🏃 Бежать", callback_data="back_main")]
        ]
        reply_markup = InlineKeyboardMarkup(keyboard)
        await query.message.reply_text("*Выберите действие:*", parse_mode='Markdown', reply_markup=reply_markup)
    else:
        await query.message.reply_text("Выберите действие:", reply_markup=get_main_keyboard(user_id))


# ========== ДОПОЛНИТЕЛЬНЫЕ ФУНКЦИИ ==========
async def taxi_to_home(update: Update, context: ContextTypes.DEFAULT_TYPE):
    user_id = update.effective_user.id
    user_data = game_bot.get_user_data(user_id)

    if user_data['location'] == '🏠 Дом':
        await update.message.reply_text("Вы уже дома!", reply_markup=get_main_keyboard(user_id))
        return

    price = 10

    if user_data['money'] < price:
        await update.message.reply_text(f"❌ Недостаточно денег! Нужно {price} ₽")
        return

    user_data['money'] -= price
    user_data['location'] = '🏠 Дом'
    user_data['last_taxi'] = datetime.now().isoformat()

    game_bot.add_xp(user_id, 3)
    game_bot.save_data()

    await update.message.reply_text(
        f"🚕 Вы доехали до дома!\n💵 Потрачено: {price} ₽\n⭐ +3 опыта",
        reply_markup=get_main_keyboard(user_id)
    )


async def play_slots(update: Update, context: ContextTypes.DEFAULT_TYPE):
    user_id = update.effective_user.id
    user_data = game_bot.get_user_data(user_id)

    if user_data['money'] < 5:
        await update.message.reply_text("❌ Недостаточно денег для игры в слоты! Нужно 5 ₽")
        return

    user_data['money'] -= 5

    # Генерация слотов
    symbols = ["🍒", "🍋", "🍊", "🍇", "🔔", "⭐", "7️⃣"]
    result = [random.choice(symbols) for _ in range(3)]

    # Проверка выигрыша
    if result[0] == result[1] == result[2]:
        if result[0] == "7️⃣":
            prize = 100
            win_text = "🎰 ДЖЕКПОТ! ТРИ СЕМЕРКИ!"
        else:
            prize = 25
            win_text = f"🎰 ВЫИГРЫШ! ТРИ {result[0]}"
        user_data['money'] += prize
    elif result[0] == result[1] or result[1] == result[2]:
        prize = 10
        win_text = "🎰 Малый выигрыш! Два одинаковых символа"
        user_data['money'] += prize
    else:
        prize = 0
        win_text = "🎰 К сожалению, вы не выиграли"

    game_bot.save_data()

    await update.message.reply_text(
        f"{win_text}\n\n"
        f"🎰 | {' | '.join(result)} |\n\n"
        f"💵 Ставка: 5 ₽\n"
        f"💰 Выигрыш: {prize} ₽\n"
        f"💵 Баланс: {user_data['money']} ₽",
        reply_markup=get_main_keyboard(user_id)
    )


# ========== ОБРАБОТЧИК CALLBACK'ОВ ==========
async def button_handler(update: Update, context: ContextTypes.DEFAULT_TYPE):
    query = update.callback_query
    data = query.data

    if data.startswith("taxi_"):
        await take_taxi(update, context)
    elif data.startswith("quest_"):
        await start_quest(update, context)
    elif data.startswith("pvp_easy_") or data.startswith("pvp_medium_") or data.startswith("pvp_hard_"):
        await start_pvp_battle(update, context)
    elif data == "pvp_attack":
        await pvp_attack(update, context)
    elif data == "pvp_defend":
        await pvp_defend(update, context)
    elif data == "pvp_special":
        await pvp_special(update, context)
    elif data.startswith("buy_"):
        await buy_item(update, context)
    elif data.startswith("sell_"):
        await sell_item(update, context)
    elif data.startswith("potion_"):
        await buy_potion(update, context)
    elif data == "back_main":
        user_id = query.from_user.id
        await query.edit_message_text("Главное меню:")
        await query.message.reply_text("Выберите действие:", reply_markup=get_main_keyboard(user_id))


def main():
    application = Application.builder().token(BOT_TOKEN).build()

    # Обработчики команд
    application.add_handler(CommandHandler("start", start))
    application.add_handler(CommandHandler("help", help_command))
    application.add_handler(CommandHandler("pvp", pvp_command))
    application.add_handler(MessageHandler(filters.TEXT & ~filters.COMMAND, handle_text))
    application.add_handler(CallbackQueryHandler(button_handler))

    # Запуск бота
    print("🚀 Бот запущен с улучшенной PvP системой и умными кнопками!")
    application.run_polling()


if __name__ == '__main__':
    main()
