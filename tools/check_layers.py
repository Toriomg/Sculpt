#!/usr/bin/env python3
"""Validate layer dependency rules across all non-vendor header files.

Layer map:
  L1  Core/glhead.hpp
  L2  Platform/CoreUtils/, Platform/Graphics/, Platform/System/, Platform/Layers/
  L3  Core/, Renderer/, AssetManager/
  L4  Editor/
  L5  Platform/Application.hpp/.cpp
"""

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC  = ROOT / "src"

# Order matters: most-specific prefix first.
LAYER_TABLE: list[tuple[str, int]] = [
    ("Core/glhead.hpp",       1),
    ("Platform/CoreUtils/",   2),
    ("Platform/Graphics/",    2),
    ("Platform/System/",      2),
    ("Platform/Layers/",      2),
    ("Platform/Jobs/",        2),
    ("Core/",                 3),
    ("Renderer/",             3),
    ("AssetManager/",         3),
    ("Editor/",               4),
    ("Platform/Application.", 5),
]

def layer_of(hpp: Path) -> int | None:
    rel = hpp.relative_to(SRC).as_posix()
    for prefix, n in LAYER_TABLE:
        if rel == prefix or rel.startswith(prefix):
            return n
    return None

# L1 includes are always allowed from any layer.
L1_INCLUDES = {"Core/glhead.hpp"}

# Documented architectural exceptions: (source rel path, include substring).
# Add a comment explaining why each exception exists.
EXCEPTIONS: set[tuple[str, str]] = {
    # CameraComponent stores Camera by value — intentional per CLAUDE.md because
    # Camera has no Core/ dependencies and extracting it would add complexity.
    ("Core/Components/Component.hpp", "Renderer/Camera.hpp"),
}

# (source dir prefix, forbidden include substring, human-readable reason)
# Editor (L4) may include L2 System/Layers/CoreUtils directly — only L2 Graphics
# is forbidden (must be accessed through L3 wrappers like Renderer/Material.hpp).
FORBIDDEN: list[tuple[str, str, str]] = [
    # L2 Graphics must not reach into L3+
    ("Platform/Graphics/", "Core/Systems/",    "L2 Graphics → L3 Core"),
    ("Platform/Graphics/", "Core/Components/", "L2 Graphics → L3 Core"),
    ("Platform/Graphics/", "Core/Scene.",      "L2 Graphics → L3 Core"),
    ("Platform/Graphics/", "Core/Entity.",     "L2 Graphics → L3 Core"),
    ("Platform/Graphics/", "Renderer/",        "L2 Graphics → L3 Renderer"),
    ("Platform/Graphics/", "AssetManager/",    "L2 Graphics → L3 AssetManager"),
    ("Platform/Graphics/", "Editor/",          "L2 Graphics → L4 Editor"),
    # L3 Core headers must use forward declarations for sibling L3 modules
    ("Core/",              "Renderer/",        "L3 Core header → L3 Renderer (use forward declaration)"),
    ("Core/",              "AssetManager/",    "L3 Core header → L3 AssetManager (use forward declaration)"),
    # L4 Editor must not include L2 Graphics directly (only through L3 wrappers)
    ("Editor/",            "Platform/Graphics/", "L4 Editor → L2 Graphics (must go through an L3 wrapper)"),
    # L2 Jobs must not reach into L3+
    ("Platform/Jobs/",     "Core/Systems/",    "L2 Jobs → L3 Core"),
    ("Platform/Jobs/",     "Core/Components/", "L2 Jobs → L3 Core"),
    ("Platform/Jobs/",     "Core/Scene.",      "L2 Jobs → L3 Core"),
    ("Platform/Jobs/",     "Core/Entity.",     "L2 Jobs → L3 Core"),
    ("Platform/Jobs/",     "Renderer/",        "L2 Jobs → L3 Renderer"),
    ("Platform/Jobs/",     "AssetManager/",    "L2 Jobs → L3 AssetManager"),
    ("Platform/Jobs/",     "Editor/",          "L2 Jobs → L4 Editor"),
]

INCLUDE_RE = re.compile(r'^\s*#\s*include\s+"([^"]+)"', re.MULTILINE)


def check_file(hpp: Path) -> list[str]:
    rel = hpp.relative_to(SRC).as_posix()
    text = hpp.read_text(encoding="utf-8", errors="replace")
    errors: list[str] = []

    for match in INCLUDE_RE.finditer(text):
        inc = match.group(1)
        if inc in L1_INCLUDES:
            continue  # L1 is always allowed
        if (rel, inc) in EXCEPTIONS:
            continue  # documented architectural exception
        line_no = text.count("\n", 0, match.start()) + 1

        for src_prefix, tgt_substr, reason in FORBIDDEN:
            if rel.startswith(src_prefix) and tgt_substr in inc:
                errors.append(
                    f"  {hpp.relative_to(ROOT)}:{line_no}: {reason}\n"
                    f"    #include \"{inc}\""
                )

    return errors


def main() -> int:
    headers = sorted(
        p for p in SRC.rglob("*.hpp") if "vendor" not in p.parts
    )

    all_errors: list[str] = []
    for hpp in headers:
        all_errors.extend(check_file(hpp))

    if all_errors:
        print("Layer dependency violations found:\n")
        print("\n".join(all_errors))
        print(f"\n{len(all_errors)} violation(s) in {len(headers)} headers checked.")
        return 1

    print(f"Layer check passed — {len(headers)} headers clean.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
