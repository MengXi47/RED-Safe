"""Template tags for including Vite-built assets."""

from __future__ import annotations

import json
from functools import lru_cache
from pathlib import Path
from typing import Iterable

from django import template
from django.conf import settings
from django.templatetags.static import static
from django.utils.safestring import mark_safe

register = template.Library()

MANIFEST_NAME = "manifest.json"
FRONTEND_STATIC_DIR = Path(__file__).resolve().parents[1] / "static" / "frontend"


@lru_cache(maxsize=1)
def _load_manifest() -> dict[str, dict[str, object]]:
    manifest_path = FRONTEND_STATIC_DIR / MANIFEST_NAME
    if not manifest_path.exists():
        return {}
    with manifest_path.open(encoding="utf-8") as manifest_file:
        return json.load(manifest_file)


def _build_static_path(asset_path: str) -> str:
    return static(f"frontend/{asset_path}")


def _render_css_links(css_files: Iterable[str]) -> str:
    tags = []
    for href in css_files:
        tags.append(f'<link rel="stylesheet" href="{_build_static_path(href)}" />')
    return "\n".join(tags)


def _render_script(src: str, module: bool = True) -> str:
    attr = " type=\"module\"" if module else ""
    return f'<script{attr} src="{_build_static_path(src)}"></script>'


@register.simple_tag
def vite_scripts(entry: str = "src/main.ts") -> str:
    """Render script/link tags for a given Vite entry file."""

    manifest = _load_manifest()

    if manifest and entry in manifest:
        chunk = manifest[entry]
        tags: list[str] = []
        css_files = chunk.get("css", [])
        if css_files:
            tags.append(_render_css_links(css_files))
        file_path = chunk.get("file")
        if file_path:
            tags.append(_render_script(file_path))
        for dep in chunk.get("imports", []):
            dep_chunk = manifest.get(dep)
            if dep_chunk and dep_chunk.get("file"):
                tags.append(_render_script(dep_chunk["file"]))
        return mark_safe("\n".join(tags))

    # Fallback: assume dev server
    dev_server = getattr(settings, "VITE_DEV_SERVER", "http://localhost:5173")
    return mark_safe(
        "\n".join(
            [
                f'<script type="module" src="{dev_server}/@vite/client"></script>',
                f'<script type="module" src="{dev_server}/{entry}"></script>'
            ]
        )
    )
