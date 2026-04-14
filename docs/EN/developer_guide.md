# Developer Guide

The project uses a decentralized modular architecture based on the **ToolsRegistry**.

## Project structure

- `app/` - main windows and application logic
- `core/` - base interfaces and core logic (File management, Settings, Registry)
- `Tools/` - functional modules: tabs, windows, and references
- `libs/` - shared libraries used across the project
- `ui/` - common UI components

---

## Tool Kinds

Cremniy supports three types of tools (defined in `ToolKind`):

1.  **FileTab**: A tab tied to an open file (e.g., Hex viewer, Code editor). Displayed in the main workspace.
2.  **Window**: An independent tool window (e.g., Calculator, Data Converter). Found in "Tools -> Window Tools" menu.
3.  **Reference**: Informational modules (e.g., ASCII Table). Found in "References" menu.

---

## Registering a Module

Modules register themselves automatically using static initialization. There is no need to modify core registry files.

### 1. Adding a File Tab

Tabs inherit from `ToolTab` and get access to `FileDataBuffer`.

```cpp
// In the plugin's .cpp file
static const bool registeredMyTab =
    registerAlwaysFileTool<MyTabClass>(
        "unique_id",          // System ID
        "Display Name",       // Menu name
        FileToolOrder::Code   // Display order
    );
```

### 2. Adding a Window Tool

Independent tools for general tasks.

```cpp
// In the tool's .cpp file
static const bool registeredMyWindow =
    registerWindowTool("unique_id", "Calculator", [](QWidget* parent) {
        auto* dlg = new MyWindowDialog(parent);
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        dlg->show();
    });
```

### 3. Adding a Reference Tool

Static informational windows.

```cpp
// In the reference's .cpp file
static const bool registeredMyRef =
    registerReferenceTool<MyRefWindow>(
        "unique_id",
        "ASCII Table"
    );
```

---

## Rules

- Modules **must not depend** on each other directly.
- Use the **central registry** to open windows or create tabs.
- All dependencies are managed **through CMake**.
- All data operations must go through **FileDataBuffer** to ensure synchronization between tabs.
