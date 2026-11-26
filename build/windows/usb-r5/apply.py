from pathlib import Path
import subprocess

overlay = Path(__file__).resolve().parent
repo = overlay.parents[2]
desktop = repo / "desktop-apps"
subprocess.run(["git", "-C", str(desktop), "apply", "--ignore-space-change", str(overlay / "desktop-apps.patch")], check=True)
for filename in ("w4portable.cpp", "w4portable.h"):
    (desktop / "win-linux/src" / filename).write_bytes((overlay / filename).read_bytes())

defines = desktop / "win-linux/src/prop/defines_p.h"
text = defines.read_text(encoding="utf-8")
anchor = "#ifdef _WIN32\n"
if text.count(anchor) != 1:
    raise RuntimeError("Expected one Windows settings block")
replacement = '''#ifdef _WIN32
#include "../w4portable.h"
#undef GET_REGISTRY_USER
#define GET_REGISTRY_USER(variable) \\
    auto variable##_w4_owner = W4Portable::createSettings(QSettings::UserScope); \\
    QSettings &variable = *variable##_w4_owner;
#undef GET_REGISTRY_SYSTEM
#define GET_REGISTRY_SYSTEM(variable) \\
    auto variable##_w4_owner = W4Portable::createSettings(QSettings::SystemScope); \\
    QSettings &variable = *variable##_w4_owner;
'''
defines.write_text(text.replace(anchor, replacement), encoding="utf-8", newline="\n")
