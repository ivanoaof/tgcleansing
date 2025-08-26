#Tgcleaner

 > A program for clearing message history in selected Telegram chats, written in C++ using **TDLib**.


**Features**

- Authorization via Telegram (with 2FA support).
- Getting a list of chats.
- Clearing message history in a selected chat.
- Automatically saves configuration (`api_id`, `api_hash`, encryption key) in `config.json`.

**Dependencies**
- [TDLib](  https://github.com/tdlib/td?tab=readme-ov-file#using-cxx)
- [nlohmann/json](https://github.com/nlohmann/json)
- C++17 and above

**Building**
```
g++ main.cpp -std=c++17 -ltdjson -o tgcleaner
```

**Usage**

1. Run the program

```

./tgcleaner

```

2. Enter your phone number, confirmation code, and (if necessary) 2FA password.
3. Select a chat from the list.
4. Messages from the selected chat will be deleted.

**Configuration**

When you first run the program, it will ask you to enter:

- `api_id`

- `api_hash`

- database password

This data is stored in `config.json`.

Программа для очистки истории сообщений в выбранных чатах Telegram, написанная на C++ с использованием **TDLib**.

**Возможности**

- Авторизация через Telegram (с поддержкой 2FA).
- Получение списка чатов.
- Очистка истории сообщений в выбранном чате.
- Автоматическое сохранение конфигурации (`api_id`, `api_hash`, ключ шифрования) в `config.json`.

**Зависимости**

- [TDLib](https://github.com/tdlib/td?tab=readme-ov-file#using-cxx)
- [nlohmann/json](https://github.com/nlohmann/json)
- C++17 и выше

**Сборка**

```
g++ main.cpp -std=c++17 -ltdjson -o tgcleaner
```

**Использование**
1. Запустите программу

```
./tgcleaner
```
2. Введите телефон, код подтверждения и (при необходимости) 2FA пароль.
    
3. Выберите чат из списка.
    
4. Сообщения из выбранного чата будут удалены.

**Конфигурация**
При первом запуске программа попросит ввести:
- `api_id`
- `api_hash`
- пароль для базы данных.
- Эти данные сохраняются в `config.json`.