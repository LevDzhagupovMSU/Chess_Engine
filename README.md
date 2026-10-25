# Chess Engine and WebSocket Server

Небольшой шахматный проект на C++17: движок оформлен как отдельная статическая
библиотека, сервер принимает ходы по WebSocket и играет за чёрных, а frontend
показывает доску и историю партии.

## Структура проекта

- `chess-engine/` — библиотека движка: доска, фигуры, ходы, состояние партии,
  правила и поиск хода ИИ.
- `server/` — WebSocket-сервер, сериализация сообщений и серверные тесты.
- `frontend/` — статический HTML/CSS/JavaScript клиент.
- `chess-engine/tests/` — тесты правил фигур, доски, партии и оценки позиции.
- `chess-engine/benchmarks/` — benchmark поиска и скрипт сравнения результатов.

Основные типы движка находятся в namespace `ce`. Класс `ce::Game` управляет
состоянием партии и легальностью ходов; `ce::ChessBoard` хранит позицию. Для
подключения движка в C++ используйте CMake target `chess_engine` и заголовки из
`chess-engine/include/`.

## Требования

- CMake 3.20 или новее.
- Компилятор с поддержкой C++17.
- Boost.System.
- `nlohmann_json`.
- GoogleTest для сборки тестов (включены по умолчанию через `BUILD_TESTING`).

Например, в Ubuntu зависимости можно установить так:

```sh
sudo apt install build-essential cmake libboost-system-dev nlohmann-json3-dev libgtest-dev
```

## Сборка

Из корня репозитория:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j2
```

Release-конфигурация используется по умолчанию. Для движка и сервера заданы
флаги оптимизации `-O2` (MSVC: `/O2`) и предупреждения компилятора. Исполняемый
сервер находится по пути `build/server/chess_server_app`.

Чтобы собрать проект без тестовых зависимостей:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
cmake --build build -j2
```

Запуск тестов:

```sh
ctest --test-dir build --output-on-failure
```

## Запуск

Запустите сервер:

```sh
./build/server/chess_server_app
```

Сервер слушает `ws://127.0.0.1:8080`. В другом терминале раздайте frontend:

```sh
python3 -m http.server 8000 --directory frontend
```

Откройте <http://127.0.0.1:8000>, оставьте указанный адрес WebSocket и нажмите
«Подключиться». Игрок играет белыми, движок — чёрными. Новая партия начинает
новую серверную сессию; после нажатия кнопки нужно подключиться заново.

## Протокол WebSocket

Каждое сообщение — JSON. Номера клеток идут построчно от верхнего левого угла:
`0` — `a8`, `7` — `h8`, `56` — `a1`, `63` — `h1`.

Клиент отправляет ход белых:

```json
{"type":"player","move":[52,36]}
```

При превращении пешки можно передать `promotion`: `queen`, `rook`, `bishop` или
`knight`. Если поле отсутствует, выбирается ферзь.

Сервер отвечает ходом ИИ:

```json
{"type":"ai","move":[6,21]}
```

Непроменённое по умолчанию превращение в ферзя не требует дополнительного поля;
для остальных фигур сервер передаёт `promotion`. Нелегальный ход отклоняется
сообщением `{"type":"bad move","move":[-1,-1]}`. Окончание партии передаётся
отдельно: `{"type":"result","result":"win"}`, где `result` может быть
`win`, `lost`, `draw` или `stalemate`.

## Benchmark поиска

Соберите benchmark отдельной целью и запустите фиксированные позиции на глубинах
1 и 2:

```sh
cmake --build build --target chess_engine_search_benchmark -j2
./build/chess-engine/chess_engine_search_benchmark --iterations 7 --warmup 2
```

Программа печатает CSV с медианой, минимумом и максимумом времени поиска.
Подробности и сравнение двух CSV описаны в
[`chess-engine/benchmarks/README.md`](chess-engine/benchmarks/README.md).
