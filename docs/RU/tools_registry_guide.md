# Руководство по ToolsRegistry

`ToolsRegistry` — это центральный компонент Cremniy для управления всеми инструментами, справочниками и вкладками. Он обеспечивает автоматическое наполнение меню и создание объектов без жестких связей между компонентами.

---

## Типы инструментов

В системе существует три типа инструментов (определены в `ToolKind`):

1.  **FileTab**: Вкладка, привязанная к открытому файлу (например, Hex-вьювер, Редактор кода). Отображается в `ToolsTabWidget`.
2.  **Window**: Независимое окно инструмента (например, Калькулятор, Конвертер данных). Отображается в меню "Tools -> Window Tools".
3.  **Reference**: Информационный модуль или справочник (например, Таблица ASCII). Отображается в меню "References".

---

## Регистрация модуля

Для регистрации модуля не нужно править `ToolsRegistry.cpp`. Каждый модуль регистрирует себя сам в своем `.cpp` файле с помощью статической переменной.

### 1. Добавление Вкладки (FileTab)

Для вкладок, которые должны открываться для **всех** файлов (Always):

```cpp
// В начале .cpp файла плагина
static const bool registeredMyTab =
    registerAlwaysFileTool<MyTabClass>(
        "unique_id",          // ID плагина
        "Display Name",       // Имя в меню
        FileToolOrder::Code   // Порядок отображения (из перечисления FileToolOrder)
    );
```

### 2. Добавление Окна (Window)

Для независимых инструментов, открывающихся в отдельном окне:

```cpp
// В начале .cpp файла инструмента
static const bool registeredMyWindow =
    registerWindowTool("unique_id", "Calculator", [](QWidget* parent) {
        auto* dlg = new MyWindowDialog(parent);
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        dlg->show();
    });
```

### 3. Добавление Справочника (Reference)

Для информационных окон:

```cpp
// В начале .cpp файла справочника
static const bool registeredMyRef =
    registerReferenceTool<MyRefWindow>(
        "unique_id",         // ID плагина
        "ASCII Table"        // Имя в меню "References"
    );
```

---

## Важные требования к классам

1.  **Конструктор**:
    *   Для `FileTab`: Должен иметь конструктор `MyTab(FileDataBuffer* buffer)`.
    *   Для `Window`/`Reference`: Должен принимать `(QWidget* parent)`.
2.  **Наследование**:
    *   Вкладки файлов должны наследоваться от `ToolTab`.
    *   Окна/Справочники должны наследоваться от `QWidget` или `QDialog`.
3.  **Имена и Иконки**:
    *   Для вкладок обязательно переопределить методы `name()` и `icon()`.

---

## Как это работает под капотом

*   Меню (`ToolsMenu`, `ReferencesMenu`) при создании запрашивают у `ToolsRegistry` список доступных дескрипторов.
*   При клике на пункт меню вызывается `ToolsRegistry::instance().openWindowTool(...)`.
*   Регистрация происходит до запуска `main()` благодаря инициализации статических переменных.
