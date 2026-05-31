# LiveWallpaper

LiveWallpaper - это экспериментальные живые обои для Linux/X11. Приложение создает desktop-окно на весь экран, рисует поверх него частицы через SDL2 и меняет их поведение по данным системы и погоды:

- цветовая палитра зависит от текущей температуры;
- направление и скорость ветра влияют на движение частиц;
- загрузка CPU меняет скорость анимации;
- яркость фона плавно меняется в течение суток.

Проект написан на C++17 и собирается через CMake.

## Возможности

- Интеграция с X11 desktop window: окно помечается как `_NET_WM_WINDOW_TYPE_DESKTOP` и опускается ниже обычных окон.
- Рендеринг частиц через SDL2.
- Получение погоды из Open-Meteo API без API-ключа.
- Конфигурация через JSON.
- Установка systemd user service для автозапуска.
- Unit-тесты на GoogleTest.
- CI для сборки, тестов и проверки через Valgrind.

## Планы развития

Проект находится в активной разработке. Возможные направления развития:

- Поддержка других платформ и графических окружений помимо Linux/X11.
- Реакция обоев на звук: изменение движения, цвета или интенсивности эффектов в зависимости от аудиосигнала.
- Улучшение архитектуры: более явное разделение рендеринга, источников данных, эффектов и конфигурации.
- Применение подходящих паттернов проектирования там, где они упростят расширение проекта.
- Расширение визуальных эффектов, настроек и пользовательского интерфейса.

## Требования

Проект рассчитан на Linux с X11.

Зависимости:

- CMake 3.14+
- компилятор с поддержкой C++17
- SDL2
- X11
- libcurl
- nlohmann/json
- systemd user services для автозапуска
- Valgrind, если нужны profiling/memcheck targets

Для Ubuntu/Debian:

```bash
sudo apt update
sudo apt install -y cmake g++ libsdl2-dev libx11-dev libcurl4-openssl-dev nlohmann-json3-dev valgrind
```

## Сборка

```bash
cmake -B build
cmake --build build
```

Запуск из директории проекта:

```bash
./build/LiveWallpaper
```

При первом запуске, если `config.json` не найден, приложение создаст пользовательский конфиг на основе `config.default.json`:

```text
~/.config/livewallpaper/config.json
```

## Установка

В проекте есть скрипт установки:

```bash
./install.sh
```

Он:

1. собирает проект с `BUILD_TESTING=OFF`;
2. устанавливает бинарный файл в `/usr/local/bin/LiveWallpaper`;
3. устанавливает ресурсы в `/usr/local/share/livewallpaper`;
4. копирует `livewallpaper.service` в `~/.config/systemd/user/`;
5. включает и запускает user service.

Чтобы только включить уже установленный сервис:

```bash
/usr/local/share/livewallpaper/install.sh --enable
```

## Удаление

```bash
/usr/local/share/livewallpaper/uninstall.sh
```

Скрипт остановит и отключит сервис, удалит установленный бинарный файл и данные из `/usr/local/share/livewallpaper`. Перед удалением пользовательского конфига он спросит подтверждение.

Чтобы только отключить сервис:

```bash
/usr/local/share/livewallpaper/uninstall.sh --disable
```

## Конфигурация

Пример `config.json`:

```json
{
    "latitude": 55.0,
    "longitude": 37.0,
    "weather_update_interval": 1,
    "fps": 30,
    "particle_count": 100,
    "particle_lifetime": 1000
}
```

Поля:

| Поле | Описание |
| --- | --- |
| `latitude` | Широта для запроса погоды |
| `longitude` | Долгота для запроса погоды |
| `weather_update_interval` | Интервал обновления погоды в минутах, минимум `1` |
| `fps` | Целевая частота кадров, минимум `1` |
| `particle_count` | Количество частиц на экране |
| `particle_lifetime` | Время жизни частицы в кадрах/тиках обновления, минимум `1` |

Если отдельные поля отсутствуют, используются значения по умолчанию. Если числовые ограничения нарушены, приложение автоматически поднимает значение до минимума.

## Тесты

Сборка с тестами включена по умолчанию через `BUILD_TESTING`:

```bash
cmake -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

GoogleTest загружается через CMake `FetchContent`.

## Valgrind

Если Valgrind найден в системе, CMake добавляет дополнительные targets:

```bash
cmake --build build --target valgrind-memcheck
cmake --build build --target valgrind-massif
cmake --build build --target valgrind-callgrind
```

`valgrind-massif` дополнительно генерирует `massif.txt`.

## CI

GitHub Actions workflow находится в `.github/workflows/ci.yml`. Он запускается на `push` и `pull_request`, устанавливает зависимости, собирает проект, выполняет тесты и запускает Valgrind memcheck для тестового бинарника.

## Структура проекта

```text
src/
  config/          загрузка и валидация JSON-конфига
  data/            системные данные и клиент погоды
  particles/       модель и обновление частиц
  signal_handler/  обработка сигналов завершения
  window/          создание desktop-окна через X11
tests/             unit-тесты
```

## Ограничения

- Приложение ориентировано на X11. На Wayland окружениях поведение зависит от поддержки XWayland и оконного менеджера.
- Для получения погоды нужен доступ к `https://api.open-meteo.com`.
- Сервис по умолчанию запускает `/usr/local/bin/LiveWallpaper`, поэтому при нестандартном `CMAKE_INSTALL_PREFIX` service-файл может потребовать правки.
