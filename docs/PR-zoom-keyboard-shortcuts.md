# Pull Request: Zoom by Ctrl+Plus / Ctrl+Minus in code editor

## Description

Adds keyboard shortcuts for zoom in/out in the code editor, in addition to the existing Ctrl+scroll wheel behavior.

## Changes

- **File:** `src/components/codeeditor/src/QCodeEditor.cpp`
- **Change:** Handle `Ctrl+Plus` / `Ctrl+Equal` (zoom in) and `Ctrl+Minus` (zoom out) in `QCodeEditor::keyPressEvent`.
- **Behavior:** Same logic as in `wheelEvent` (Ctrl+scroll): updates `scaleFactor`, font size (12pt × scaleFactor), tab stop distance, and refreshes the viewport.

## Shortcuts

| Shortcut      | Action   |
|---------------|----------|
| **Ctrl+Plus** / **Ctrl+=** | Zoom in  |
| **Ctrl+Minus**            | Zoom out |

*(Ctrl+scroll wheel zoom remains unchanged.)*

## Testing

1. Open the code editor (e.g. any source file).
2. Press **Ctrl+Plus** or **Ctrl+=** — font should increase.
3. Press **Ctrl+Minus** — font should decrease.
4. Confirm tab width and line number area scale correctly.

## Checklist

- [x] Change is minimal (single block in `keyPressEvent`).
- [x] No new dependencies.
- [x] Follows existing zoom logic from `wheelEvent`.
- [x] No linter errors.
