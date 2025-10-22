# Полный гайд: ReactJS + Drogon на Ubuntu

Этот гайд охватывает полную установку C++ Drogon фреймворка и ReactJS с интеграцией на Ubuntu. В папке `Hackaton2025` уже находится готовый проект с двумя папками: `frontend` (ReactJS) и `backend` (Drogon).

- **Папка установки Drogon:** `drogon_dev`
- **Рабочая папка проекта:** `Hackaton2025` (уже готовая)
- **Структура проекта:**
  - `Hackaton2025/frontend/` — React приложение
  - `Hackaton2025/backend/` — Drogon C++ сервер
- **Порт Drogon:** 80 (требуется sudo для запуска)
- **Платформа:** Ubuntu 20.04 LTS / 22.04 LTS / 24.04 LTS

---

## Часть 1: Подготовка системы

### Шаг 1.1: Обновить системные пакеты

```bash
sudo apt update
```

```bash
sudo apt upgrade -y
```

### Шаг 1.2: Установить зависимости

Установить build-essential:

```bash
sudo apt install -y build-essential
```

Установить CMake:

```bash
sudo apt install -y cmake
```

Установить Git:

```bash
sudo apt install -y git
```

Установить pkg-config:

```bash
sudo apt install -y pkg-config
```

Установить GCC:

```bash
sudo apt install -y gcc
```

Установить G++:

```bash
sudo apt install -y g++
```

Установить OpenSSL dev:

```bash
sudo apt install -y libssl-dev
```

Установить zlib dev:

```bash
sudo apt install -y zlib1g-dev
```

Установить c-ares dev (для асинхронных DNS):

```bash
sudo apt install -y libc-ares-dev
```

Установить UUID dev:

```bash
sudo apt install -y uuid-dev
```

Установить OpenSSL:

```bash
sudo apt install -y openssl
```

### Шаг 1.3: Установить Node.js и npm для ReactJS

Установить Node.js:

```bash
sudo apt install -y nodejs
```

Установить npm:

```bash
sudo apt install -y npm
```

**Проверить установку:**

```bash
node --version
```

```bash
npm --version
```

---

## Часть 2: Установка Drogon Framework

### Шаг 2.1: Создать папку установки

```bash
mkdir -p ~/drogon_dev
```

```bash
cd ~/drogon_dev
```

### Шаг 2.2: Клонировать репозиторий Drogon

```bash
git clone https://github.com/drogonframework/drogon.git
```

```bash
cd drogon
```

Загрузить подмодули:

```bash
git submodule update --init
```

### Шаг 2.3: Собрать Drogon из исходников

Создать папку build:

```bash
mkdir build
```

Перейти в build:

```bash
cd build
```

Запустить CMake:

```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
```

Скомпилировать (может занять 5-10 минут) и установить в систему:

```bash
make && sudo make install
```

### Шаг 2.4: Проверить установку Drogon

Найти путь к drogon_ctl:

```bash
which drogon_ctl
```

Проверить версию:

```bash
drogon_ctl --version
```

**Результат должен показать путь к `drogon_ctl` и его версию.**

---

## Часть 3: Подготовка проекта Hackaton2025

### Шаг 3.1: Перейти в папку проекта

```bash
cd ~/Hackaton2025
```

### Шаг 3.2: Структура проекта

Проверить структуру:

```bash
ls -la ~/Hackaton2025
```

Должно быть:

```
Hackaton2025/
├── frontend/           (ReactJS приложение)
│   ├── package.json
│   ├── src/
│   ├── public/
│   └── ...
└── backend/            (Drogon C++ сервер)
    ├── CMakeLists.txt
    ├── config.json
    ├── main.cc
    ├── controllers/
    └── ...
```

---

## Часть 4: Установка и сборка Frontend (React)

### Шаг 4.1: Перейти в папку frontend

```bash
cd ~/Hackaton2025/frontend
```

### Шаг 4.2: Установить зависимости React

```bash
npm install
```

### Шаг 4.3: Собрать React для продакшена

```bash
npm run build
```

Это создаст папку `build/` с готовыми HTML/JS/CSS файлами.

### Шаг 4.4: Вернуться в корневую папку

```bash
cd ..
```

---

## Часть 5: Конфигурация Backend (Drogon)

### Шаг 5.1: Перейти в папку backend

```bash
cd ~/Hackaton2025/backend
```

**Ключевые параметры:**
- `document_root` — путь к React build файлам (`../frontend/build`)
- `port` — 80 (стандартный HTTP порт)
- `address` — 0.0.0.0 (доступно со всех интерфейсов)

Сохранить файл: нажать `Ctrl+X`, затем `Y`, затем `Enter`.

---

## Часть 6: Сборка Backend (Drogon)

### Шаг 6.1: Создать папку build

В папке backend:

```bash
cd ~/Hackaton2025/backend
```

Создать build папку:

```bash
mkdir -p build
```

### Шаг 6.2: Собрать проект CMake

Перейти в build:

```bash
cd build
```

Запустить CMake:

```bash
cmake ..
```

Скомпилировать проект:

```bash
make -j$(nproc)
```

Проверить создание исполняемого файла:

```bash
ls -la ./Hackaton2025
```

### Шаг 6.3: Дать права на использование порта 80

Для использования портов ниже 1024 в Linux требуются привилегии. Выполните следующую команду:

```bash
sudo setcap CAP_NET_BIND_SERVICE=+eip ./Hackaton2025
```

Проверить установку прав:

```bash
getcap ./Hackaton2025
```

**Должно вывести:** `./Hackaton2025 = cap_net_bind_service+eip`

### Шаг 6.4: Вернуться в папку backend

```bash
cd ..
```

---

## Часть 7: Запуск проекта

### Вариант 1: Запуск через скрипт run.sh (РЕКОМЕНДУЕТСЯ)

Создайте скрипт запуска `run.sh` в корне `Hackaton2025`:

```bash
cd ~/Hackaton2025
```

Создать файл скрипта:

```bash
nano run.sh
```

Вставить содержимое:

```bash
#!/bin/bash

# Цвета для вывода
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Функция для вывода сообщений
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Переходим в корень проекта
cd "$(dirname "$0")"

log_info "Запуск Hackaton2025..."
log_info "Сборка React..."

# Собираем React
cd frontend
npm run build > /dev/null 2>&1
if [ $? -eq 0 ]; then
    log_success "React успешно собран"
else
    log_error "Ошибка при сборке React"
    exit 1
fi

cd ..

log_info "Сборка Drogon (C++)..."

# Собираем Drogon
cd backend/build
cmake .. > /dev/null 2>&1
make -j$(nproc) > /dev/null 2>&1
if [ $? -eq 0 ]; then
    log_success "Drogon успешно собран"
else
    log_error "Ошибка при сборке Drogon"
    exit 1
fi

cd ..

# Проверяем права на использование порта 80
if ! getcap ./build/Hackaton2025 | grep -q "cap_net_bind_service"; then
    log_info "Установка прав на использование порта 80..."
    sudo setcap CAP_NET_BIND_SERVICE=+eip ./build/Hackaton2025
    if [ $? -eq 0 ]; then
        log_success "Права установлены"
    else
        log_error "Не удалось установить права"
        exit 1
    fi
fi

log_info "Запуск Drogon сервера..."
log_success "Приложение доступно по адресу: http://localhost"
log_info "Для остановки нажмите Ctrl+C"

# Запускаем Drogon
./build/Hackaton2025
```

Сохранить файл: нажать `Ctrl+X`, затем `Y`, затем `Enter`.

Дать права на запуск скрипта:

```bash
chmod +x run.sh
```

Запустить проект:

```bash
./run.sh
```

---

### Вариант 2: Запуск вручную (для разработки)

Собрать React:

```bash
cd ~/Hackaton2025/frontend
npm run build
cd ..
```

Собрать Drogon:

```bash
cd backend/build
cmake ..
make -j$(nproc)
cd ..
```

Установить права (один раз):

```bash
sudo setcap CAP_NET_BIND_SERVICE=+eip ./build/Hackaton2025
```

Запустить сервер:

```bash
./build/Hackaton2025
```

---

### Вариант 3: Запуск в фоне

Запустить сервер в фоне:

```bash
cd ~/Hackaton2025/backend/build
./Hackaton2025 &
cd ~
```

### Шаг 7.1: Проверить, что сервер запущен

Проверить основную страницу:

```bash
curl http://localhost
```

Открыть в браузере:

```
http://localhost
```

---

### Перезапуск сервера

Убить текущий процесс:

```bash
killall Hackaton2025
```

Запустить новый процесс:

```bash
./build/Hackaton2025
```

Или используйте скрипт `run.sh`:

```bash
cd ~/Hackaton2025
./run.sh
```

---

## Troubleshooting

### ❌ Ошибка: `cmake: command not found`

**Решение:**

```bash
sudo apt install -y cmake
```

### ❌ Ошибка: `error: 'uuid/uuid.h' file not found`

**Решение:**

```bash
sudo apt install -y uuid-dev
```

Пересобрать Drogon:

```bash
cd ~/drogon_dev/drogon/build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
sudo make install
```

### ❌ Ошибка: `g++: command not found`

**Решение:**

```bash
sudo apt install -y build-essential
```

```bash
sudo apt install -y g++
```

### ❌ Ошибка при npm: `EACCES: permission denied`

**Решение (быстро):**

```bash
sudo npm install -g npm
```

**Решение (без sudo, правильно):**

```bash
mkdir ~/.npm-global
```

```bash
npm config set prefix '~/.npm-global'
```

```bash
export PATH=~/.npm-global/bin:$PATH
```

```bash
echo 'export PATH=~/.npm-global/bin:$PATH' >> ~/.bashrc
```

```bash
source ~/.bashrc
```

### ❌ Ошибка: `Port 80 is already in use`

**Решение 1: Найти и кильнуть процесс (безопасность не подразумевается)**

Найти процесс:

```bash
sudo lsof -i :80
```

Убить процесс (замените PID на реальное значение):

```bash
sudo kill -9 <PID>
```

**Решение 2: Изменить порт в config.json**

Открыть config.json:

```bash
nano ~/Hackaton2025/backend/config.json
```

Найти и изменить:

```json
"listeners": [
  {
    "port": 8080
  }
]
```

Пересобрать и перезапустить.

### ❌ Ошибка: `Permission denied: ./Hackaton2025`

**Проблема:** Нет прав на запуск порта 80

**Решение:**

```bash
cd ~/Hackaton2025/backend/build
sudo setcap CAP_NET_BIND_SERVICE=+eip ./Hackaton2025
getcap ./Hackaton2025
```

**Должно вывести:** `./Hackaton2025 = cap_net_bind_service+eip`

Затем запустить БЕЗ sudo:

```bash
./Hackaton2025
```

### ❌ Ошибка: `cmake version 3.13 or greater is required`

**Решение:**

Удалить старую версию:

```bash
sudo apt remove -y cmake
```

Установить новую:

```bash
sudo apt install -y cmake
```

Проверить версию:

```bash
cmake --version
```

### ❌ React компоненты не обновляются в браузере

**Решение:**

Очистить кеш браузера: нажать `Ctrl+Shift+Delete`

Или пересобрать React:

```bash
cd ~/Hackaton2025/frontend
npm run build
cd ..
```

Перезагрузить браузер: `Ctrl+Shift+R`

### ❌ Ошибка при git: `submodule update failed`

**Решение:**

```bash
cd ~/drogon_dev/drogon
```

```bash
git submodule sync
```

```bash
git submodule update --init --recursive
```

### ❌ Drogon не запускается: `error while loading shared libraries`

**Решение:**

```bash
sudo ldconfig
```

```bash
cd ~/Hackaton2025/backend/build
./Hackaton2025
```

### ❌ `make: *** [Makefile] Error 2` при сборке

**Решение:**

```bash
cd ~/drogon_dev/drogon/build
```

```bash
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++ ..
```

```bash
make clean
```

```bash
make -j$(nproc)
```

```bash
sudo make install
```

### ❌ `npm run build` не работает

**Решение:**

```bash
cd ~/Hackaton2025/frontend
```

```bash
rm -rf node_modules
```

```bash
npm cache clean --force
```

```bash
npm install
```

```bash
npm run build
```

### ❌ Скрипт run.sh не выполняется

**Решение:**

```bash
chmod +x ~/Hackaton2025/run.sh
```

```bash
bash ~/Hackaton2025/run.sh

---

## Структура проекта Hackaton2025

```
~/Hackaton2025/
├── run.sh                          (Скрипт для запуска проекта)
├── frontend/                       (React приложение)
│   ├── package.json
│   ├── package-lock.json
│   ├── src/
│   │   ├── App.jsx
│   │   ├── components/
│   │   └── ...
│   ├── public/
│   ├── build/                      (Собранные статические файлы)
│   │   ├── index.html
│   │   ├── static/
│   │   └── ...
│   └── node_modules/
│
└── backend/                        (Drogon C++ сервер)
    ├── CMakeLists.txt              (C++ конфигурация)
    ├── config.json                 (Drogon конфигурация с портом 80)
    ├── main.cc                     (Главный файл C++)
    ├── controllers/                (C++ контроллеры API)
    ├── models/
    ├── views/
    ├── plugins/
    ├── build/                      (Собранный проект)
    │   └── Hackaton2025            (Исполняемый файл)
    └── ...
```

---

## Архитектура приложения

```
┌─────────────────────────────────────────────┐
│  Browser (localhost:80)                     │
│  ┌───────────────────────────────────────┐  │
│  │ React Frontend (SPA)                  │  │
│  │ - Компоненты                         │  │
│  │ - State Management                    │  │
│  │ - HTTP requests (fetch/axios)         │  │
│  └───────────────────────────────────────┘  │
└─────────────────────────────────────────────┘
         ↓ HTTP/JSON ↑
┌─────────────────────────────────────────────┐
│  Drogon C++ Backend (localhost:80)          │
│  ┌───────────────────────────────────────┐  │
│  │ Controllers (API endpoints)           │  │
│  │ - /api/...                            │  │
│  │ - Business Logic                      │  │
│  ├───────────────────────────────────────┤  │
│  │ Static Files Serving (React)          │  │
│  │ - ../frontend/build/index.html        │  │
│  │ - JS, CSS, images                     │  │
│  └───────────────────────────────────────┘  │
└─────────────────────────────────────────────┘
```

---

## Git workflow для команды

### Коллега работает с React (frontend)

```bash
cd ~/Hackaton2025
git fetch origin
git checkout main
git pull origin main
git checkout -b feat/react-component
cd frontend
# Редактирование файлов в src/
npm run build
cd ..
git add frontend/
git commit -m "feat: новые React компоненты"
git push origin feat/react-component
# Создать Pull Request на GitHub
```

### Вы работаете с Drogon (backend)

```bash
cd ~/Hackaton2025
git fetch origin
git checkout main
git pull origin main
git checkout -b feat/api-endpoint
cd backend
# Редактирование файлов в controllers/
cd build
cmake ..
make -j$(nproc)
cd ..
git add backend/
git commit -m "feat: новые API endpoints"
git push origin feat/api-endpoint
# Создать Pull Request на GitHub
```

---

## Быстрый старт

**Один раз: установка всех зависимостей**

```bash
sudo apt update && sudo apt upgrade -y
sudo apt install -y build-essential cmake git pkg-config gcc g++ libssl-dev zlib1g-dev libc-ares-dev uuid-dev openssl nodejs npm
mkdir -p ~/drogon_dev && cd ~/drogon_dev
git clone https://github.com/drogonframework/drogon.git && cd drogon
git submodule update --init && mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release .. && make -j$(nproc) && sudo make install
```

**Подготовка проекта (один раз)**

```bash
cd ~/Hackaton2025/frontend && npm install
npm run build && cd ..
cd backend && mkdir -p build && cd build && cmake .. && make -j$(nproc)
sudo setcap CAP_NET_BIND_SERVICE=+eip ./Hackaton2025 && cd ..
```

**Запуск проекта**

```bash
cd ~/Hackaton2025 && ./run.sh
```

**Доступ:** `http://localhost`

---

Готово! Ваше приложение ReactJS + Drogon успешно установлено и готово к разработке на Ubuntu.
