# 🚀 Roadmap

## 🎯 Цель проекта
Создать **единый инструмент для системного программирования**, устраняющий необходимость использования разрозненных решений и упрощающий процесс разработки

## 📦 Текущая версия
[v0.1.2](https://github.com/Cremniy-Project/cremniy/releases/tag/v0.1.2) — базовая среда разработки с:
- редактором кода (полный набор низкоуровневых языков)
- HEX-редактором (просмотр байтов в RAW формате)
- дизассемблером (может использовать `objdump` и `radare2`)
- калькулятором для преобразования в разные системы счисления
- и связанностью инструментов

## 🛠 Ближайшие задачи

> ⚠️ На текущий релиз (v0.1.3) все усилия должны быть сосредоточены на **редакторе кода** и **критических багах**. PR, не связанные с этими задачами, временно **не будут приниматься**.

### 🐞 Баги

- [ ] [Баг: Неправильное отображение байтов в HEX-Editor на MacOS](https://github.com/Cremniy-Project/cremniy/issues/43)
- [ ] [Баг: Не правильно работает синтаксис Makefile](https://github.com/Cremniy-Project/cremniy/issues/41)
- [ ] [Баг: Отображение полоски в HEX-Editor на Windows](https://github.com/Cremniy-Project/cremniy/issues/33)

### ✨ Улучшения и новые задачи

- [ ] [Реализация кастомного QPlainText для Code Editor](https://github.com/Cremniy-Project/cremniy/issues/56)
- [ ] ~~[Оптимизировать хранение данных у QHexView](https://github.com/Cremniy-Project/cremniy/issues/57)~~
- [ ] ~~[Улучшение дизайна Disassembler](https://github.com/Cremniy-Project/cremniy/issues/55)~~
- [ ] ~~[Добавить многоязычность](https://github.com/Cremniy-Project/cremniy/issues/67)~~
- [ ] ~~[Основы базовой архитектуры для обеспечения расширяемости](https://github.com/Cremniy-Project/cremniy/issues/29)~~
- [ ] ~~[Сборка и запуск проекта пользователя](https://github.com/Cremniy-Project/cremniy/issues/6)~~
- [ ] ~~[Использование иконок Breeze для файлов в QTreeView](https://github.com/Cremniy-Project/cremniy/issues/72)~~
- [ ] ~~[Реализовать StatusBar](https://github.com/Cremniy-Project/cremniy/issues/73)~~
- [ ] ~~[Поиск строки по всем файлам проекта](https://github.com/Cremniy-Project/cremniy/issues/76)~~
- [ ] ~~[Закрепление вкладки файла (FileTab)](https://github.com/Cremniy-Project/cremniy/issues/75)~~
- [ ] ~~[Перемещение файлов по директориям в QTreeView](https://github.com/Cremniy-Project/cremniy/issues/77)~~
- [ ] ~~[Отображение чисел в разных системах счисления при наведении](https://github.com/Cremniy-Project/cremniy/issues/28)~~
- [ ] ~~[Работа с Git](https://github.com/Cremniy-Project/cremniy/issues/42)~~

## 🔮 Будущие планы

### v0.1.3

- Новый улучшенный редактор кода
- Использование общего буффера для хранения файла в редакторе кода
- Использование общего буффера для хранения файла в HEX-редакторе

### v0.2.0

- Расширяемая архитектура
- Отладчик
- Просмотр памяти
