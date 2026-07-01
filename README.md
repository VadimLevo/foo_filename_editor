# File Name Editor (foo_filename_editor)

[English](#english) | [Русский](#русский)

![File Name Editor](screenshot%20v1.1.png)
---

## English

Advanced file renamer component for **foobar2000** media player. This plugin helps you rename your physical audio files on your hard drive based on their metadata tags, while ensuring a seamless, smart, and fully automated workflow.

### Key Features
* **Smart Track Number Padding**: Automatically scans all selected tracks. If the album contains 100 or more tracks, it pads numbers to 3 digits (`001`, `002`), otherwise it defaults to 2 digits (`01`, `02`) dynamically.
* **Intelligent Compilation (Various Artists) Detection**: Automatically checks `%album artist%` and `%artist%` tags. If it detects a compilation, it seamlessly applies your custom VA formatting template.
* **Advanced Case Conversion**: Supports *Title Case*, *Smart Title Case* (keeps short words like *a, an, the, and, with* lowercase), *Sentence case*, *ALL CAPS*, and *lowercase*.
* **Roman Numerals Preservation**: Intelligently detects standalone Roman numerals (e.g., `Part iii`) and forces them to uppercase (`Part III`) across all case conversion modes (except lowercase). Includes protection against false positives (words like *MIX*, *DIV*).
* **Extended Unicode & Ligature Support**: Works perfectly with Windows native APIs, preserving and processing complex characters like `æ`, `œ`, `ß`, and language-specific diacritics.
* **Safe Formatting Menu**: Quick insertion menu for popular tags and safely escaped syntax characters like `'('`, `')'`, `'['`, `']'`, and single/double quotes (`'`, `''`).
* **Seamless Playlist Synchronization & Reload**: Automatically replaces dead file links in all open playlists with the newly renamed paths. Instantly reloads file tags and durations in the background—no broken links, no manual reloading required!
* **Robust Error Handling**: If a file cannot be renamed (e.g., if it is locked or currently playing), the plugin bypasses it without crashing and provides a detailed summary report of successful and failed files.
* **Persistent Settings**: Remembers your custom single/VA templates, history list, checkboxes status, and even your preferred window column widths between sessions.


### 🎵 M3U/M3U8 Playlist Generation (v1.1)
The plugin now includes built-in support for generating playlist files automatically during the renaming process:
* **Save M3U Checkbox**: Automatically creates a playlist file in the top-level directory of your processed files.
* **Custom Naming Templates**: Use standard foobar2000 title formatting syntax (like `%album%` or `%artist% - %album%`) to dynamically name your playlist file.
* **EXTINF Metadata**: Optional toggle to write standard `#EXTM3U` and `#EXTINF` headers containing track durations and formatted titles (`%artist% - %title%`).
* **Smart Conflict Resolution & Overwrite Toggle**: Prevents accidental loss of data. If overwrite is disabled, the plugin safely creates incremented filenames like `Playlist (1).m3u`.
* **UTF-8 Compatibility**: Playlists are written with a proper UTF-8 BOM, ensuring seamless support for non-ASCII characters, symbols, and ligatures.

### 🔧 Patch v1.1.1
* **Case-only Renaming Fix:** Resolved an issue where changing only the capitalization of a filename (e.g., from `TRACK.flac` to `Track.flac`) would be ignored or fail on Windows/NTFS filesystems due to their case-preserving but case-insensitive nature. The plugin now uses a safe two-step transit renaming process to handle these specific cases smoothly.

---

## Русский

Продвинутый компонент переименования файлов для медиаплеера **foobar2000**. Этот плагин помогает переименовывать физические аудиофайлы на жестком диске на основе их метатегов, обеспечивая при этом умный и полностью автоматизированный рабочий процесс.

### Ключевые возможности
* **Умное выравнивание номеров треков (Авто-паддинг)**: Автоматически сканирует все выделенные треки. Если альбом содержит 100 или более файлов, он дополняет номера до 3 цифр (`001`, `002`), в противном случае — до 2 цифр (`01`, `02`).
* **Интеллектуальное определение сборников (Various Artists)**: Автоматически проверяет теги `%album artist%` и `%artist%`. Если обнаруживается сборник, плагин сам подставляет ваш сохраненный шаблон для VA.
* **Продвинутая смена регистра**: Поддерживает режимы *Title Case*, *Smart title Case* (оставляет служебные слова наподобие *a, an, the, and, with* строчными), *Sentence case*, *ALL CAPS* и *lowercase*.
* **Корректная обработка римских цифр**: Умный алгоритм определяет отдельно стоящие римские цифры (например, `Part iii`) и переводит их в верхний регистр (`Part III`) во всех режимах (кроме lowercase). Защищен от ложных срабатываний на обычных словах (таких как *MIX*, *DIV*).
* **Полноценная поддержка Юникода и лигатур**: Благодаря интеграции с Windows API, плагин корректно обрабатывает сложные символы (`æ`, `œ`, `ß`) и специфические диакритические знаки различных языков.
* **Безопасное меню подстановки**: Быстрое меню для вставки популярных тегов и корректно экранированных символов пунктуации, таких как `'('`, `')'`, `'['`, `']'`, а также одинарных/двойных кавычек (`'`, `''`).
* **Бесшовная интеграция с плейлистами**: Автоматически заменяет старые пути файлов на новые во всех открытых плейлистах. Сразу же запускает фоновое чтение метаданных (Reload Info) — никаких «мертвых» ссылок и неопознанных треков.
* **Надежная обработка ошибок**: Если файл заблокирован или проигрывается в данный момент, плагин аккуратно пропустит его, не прерывая работу, а в конце выдаст подробный отчет со списком успешных и проблемных файлов.
* **Сохранение состояния**: Плагин намертво запоминает ваши шаблоны для Single/VA, историю ввода, состояние чекбоксов и даже ширину столбцов таблицы между запусками.


### 🎵 Генерация плейлистов M3U/M3U8 (v1.1)
В плагин добавлена встроенная поддержка автоматического создания файлов плейлистов прямо в процессе переименования:
* **Чекбокс Save M3U**: Автоматически создает файл плейлиста в корневой папке обрабатываемых треков.
* **Кастомные шаблоны имен**: Использование стандартного синтаксиса форматирования foobar2000 (например, `%album%` или `%artist% - %album%`) для динамического именования файла плейлиста.
* **Метаданные EXTINF**: Опциональный переключатель для записи стандартных заголовков `#EXTM3U` и строк `#EXTINF`, содержащих длительность треков и отформатированные имена вида `%artist% - %title%`.
* **Умное разрешение конфликтов и перезапись**: Предотвращает случайную потерю данных. Если перезапись отключена, плагин безопасно создает файлы с инкрементом в имени, например: `Playlist (1).m3u`.
* **Совместимость с UTF-8**: Плейлисты записываются со строгим UTF-8 BOM, что гарантирует корректное отображение любых кириллических символов, лигатур и спецсимволов.

### 🔧 Патч v1.1.1
* **Исправление работы с регистром:** Решена проблема, из-за которой изменение только регистра букв в имени файла (например, с `TRACK.flac` на `Track.flac`) игнорировалось операционной системой Windows (NTFS). Теперь плагин автоматически распознает такие ситуации и использует безопасное двухэтапное переименование через временный файл, гарантируя точное применение выбранной капитализации.
