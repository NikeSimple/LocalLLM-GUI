Руководство разработчика
========================

Настройка среды
---------------

1. Установите MSYS2 UCRT64;
2. Установите зависимости:

.. code-block:: bash

   pacman -S mingw-w64-ucrt-x86_64-gcc
   pacman -S mingw-w64-ucrt-x86_64-cmake
   pacman -S mingw-w64-ucrt-x86_64-qt6-base
   pacman -S mingw-w64-ucrt-x86_64-qt6-svg
   pacman -S mingw-w64-ucrt-x86_64-sqlite3

Сборка
------

.. code-block:: bash

   mkdir build && cd build
   cmake .. -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=/ucrt64
   cmake --build .

Запуск тестов
-------------

.. code-block:: bash

   cd build
   ctest --output-on-failure

Всего 58 тестов: модульные, интеграционные, границы, безопасность.

Структура проекта
-----------------

::

   LocalLLM-GUI/
   ├── gui/        — интерфейс
   ├── core/       — бизнес-логика (ApiClient, FileHandler)
   ├── db/         — доступ к данным (репозитории, DatabaseManager)
   ├── config/     — SettingsManager
   ├── resources/  — QSS, иконки, QRC
   ├── tests/      — тесты
   ├── docs/       — документация
   └── sql/        — схема БД

Сборка документации
-------------------

.. code-block:: bash

   # 1. Сгенерировать XML из комментариев C++
   doxygen Doxyfile

   # 2. Собрать HTML из Sphinx
   source docs/venv/bin/activate
   sphinx-build -b html docs/source docs/_build/html

   # 3. Открыть в браузере
   start docs/_build/html/index.html

Передача проекта
----------------

Новому разработчику нужно:

1. Клонировать репозиторий: ``git clone https://github.com/NikeSimple/LocalLLM-GUI``
2. Прочитать README.md и этот документ;
3. Установить зависимости через pacman (см. выше);
4. Собрать проект и запустить тесты;
5. Собрать документацию и открыть её в браузере;
6. Посмотреть Issues на GitHub — там текущие задачи.
