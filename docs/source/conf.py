# Конфигурация Sphinx для проекта LocalLLM-GUI
# -*- coding: utf-8 -*-

project = 'LocalLLM-GUI'
copyright = '2026, Михайлов Богдан Михайлович'
author = 'Михайлов Богдан Михайлович'
release = '1.0.0'

extensions = ['breathe']

# Путь к XML, сгенерированному Doxygen
breathe_projects = {
    "LocalLLM-GUI": "../doxygen/xml"
}
breathe_default_project = "LocalLLM-GUI"

templates_path = ['_templates']
exclude_patterns = []

language = 'ru'

html_theme = 'alabaster'
html_static_path = ['_static']
html_title = 'LocalLLM-GUI — документация'

highlight_language = 'cpp'
