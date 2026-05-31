#!/usr/bin/env python3

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

DOCS = [
    "docs/01-project-map.md",
    "docs/02-product-definition.md",
    "docs/03-current-state.md",
    "docs/07-next-actions.md",
    "docs/09-scratchbook.md",
]

OUTPUT_DIR = ROOT / "operator" / "restart-prompts"
OUTPUT_FILE = OUTPUT_DIR / "restart-prompt.md"

EXCLUDED_DIRS = {
    ".git",
    ".venv",
    "venv",
    "__pycache__",
    "node_modules",
    "certs",
    "operator/restart_prompts",
}

EXCLUDED_FILES = {
    ".DS_Store",
}

EXCLUDED_SUFFIXES = {
    ".pyc",
}

INSTRUCTIONS = """\
Act as an advisory senior software engineer and product/system design reviewer for this project.

Start from the project documentation and repository structure included below. Treat the docs as the source of truth unless implementation files show otherwise.

Working rules:
- Do not invent implementation details.
- Do not rely on memory from previous chats.
- Use only the files or text I provide in this chat.
- If something is missing, ask for the specific file or clarification needed.
- Keep the MVP narrow.
- Prefer small, readable documentation and implementation steps.
- Separate current facts, accepted decisions, open questions, and next actions.
- Do not push new features before the existing implementation is documented.
- If docs and code disagree, call out the mismatch instead of silently resolving it.
- When proposing doc changes, keep them concise and maintainable.
- Avoid turning docs into essays.
- Use the repository tree only to understand file layout and request relevant files. Do not infer implementation behavior from file names alone.

Documentation conventions:
- `03-current-state.md` should describe what is true now.
- `04-decisions.md` should record accepted decisions only.
- `05-architecture.md` should describe the actual current architecture and clearly mark future or non-MVP items.
- `06-implementation-log.md` should record chronological work history.
- `07-next-actions.md` should stay short and readable.
- `08-open-questions.md` should capture unresolved questions.
- `09-scratchbook.md` should not become a permanent source of truth.
- Do not put the ordered task list inside `03-current-state.md`.

Project workflow:
1. Read the documentation below in order.
2. Use the repository tree to understand where relevant files live.
3. Identify the current project mode and next small action from the docs.
4. Review `09-scratchbook.md` for useful notes, obsolete notes, or unresolved thoughts.
5. Ask for the smallest set of implementation files needed for the next action.
6. Review those files against the docs.
7. Propose minimal documentation updates.
8. Only after docs match the current system, help resume implementation.

Start by telling me:
- what the current project mode appears to be
- what the next small action appears to be
- whether anything in `09-scratchbook.md` should move into another doc
- which files you need next, if any
"""


def is_excluded(path: Path) -> bool:
    relative = path.relative_to(ROOT).as_posix()

    if relative in EXCLUDED_DIRS:
        return True

    if path.name in EXCLUDED_DIRS:
        return True

    if path.name in EXCLUDED_FILES:
        return True

    if path.suffix in EXCLUDED_SUFFIXES:
        return True

    return False


def read_file(path: Path) -> str:
    if not path.exists():
        return f"[Missing file: {path.relative_to(ROOT)}]"

    return path.read_text(encoding="utf-8").strip()


def visible_children(path: Path) -> list[Path]:
    children = [child for child in path.iterdir() if not is_excluded(child)]
    return sorted(children, key=lambda p: (not p.is_dir(), p.name.lower()))


def build_tree(path: Path, prefix: str = "") -> list[str]:
    lines: list[str] = []
    children = visible_children(path)

    for index, child in enumerate(children):
        is_last = index == len(children) - 1
        connector = "└── " if is_last else "├── "
        lines.append(f"{prefix}{connector}{child.name}")

        if child.is_dir():
            extension = "    " if is_last else "│   "
            lines.extend(build_tree(child, prefix + extension))

    return lines


def repository_tree() -> str:
    lines = ["."]
    lines.extend(build_tree(ROOT))
    return "\n".join(lines)


def build_prompt() -> str:
    parts = [INSTRUCTIONS.strip(), "\n\n# Project documentation\n"]

    for relative_path in DOCS:
        path = ROOT / relative_path
        content = read_file(path)

        parts.append(f"\n\n## `{relative_path}`\n\n")
        parts.append(content)

    parts.append("\n\n# Repository structure\n\n")
    parts.append(
        "Generated from the current working tree. Generated/local-only directories may be excluded.\n\n"
    )
    parts.append("```text\n")
    parts.append(repository_tree())
    parts.append("\n```")

    return "".join(parts)


def main() -> None:
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

    prompt = build_prompt()
    OUTPUT_FILE.write_text(prompt, encoding="utf-8")

    print(f"Wrote restart prompt to: {OUTPUT_FILE.relative_to(ROOT)}")


if __name__ == "__main__":
    main()