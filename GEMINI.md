# News Reader App

## Overview
A news reader application built with Flutter, using Riverpod for state management, GoRouter for navigation, and Shadcn UI for the design system.

## Tech Stack
- **Framework**: Flutter
- **Language**: Dart
- **State Management**: flutter_riverpod
- **Navigation**: go_router
- **UI Library**: shadcn_ui (Flutter port)
- **Networking**: dio
- **Icons**: lucide_icons_flutter (via shadcn_ui)

## Features
- **Top Headlines**: View the latest news articles.
- **Article Detail**: Read full article content with image and link to source.
- **Search**: Search for articles by keyword.
- **Bookmarks**: Placeholder for saving articles (UI implemented).
- **Settings**: Placeholder for app settings (UI implemented).

## Architecture
- `lib/core`: Routing and Theme configuration.
- `lib/data`: Data models and services (Mock News Service).
- `lib/screens`: UI screens (Home, Search, Detail, etc.).
- `lib/widgets`: Reusable widgets (ArticleCard, ScaffoldWithNavBar).

## Setup
1. Run `flutter pub get`.
2. Run `flutter run`.