# News Reader App

A Flutter-based news reader application.

## Tech Stack
- **Framework**: Flutter
- **Language**: Dart
- **UI Library**: shadcn_ui (Flutter port)
- **Navigation**: go_router
- **State Management**: StatefulWidgets (Mock Service)
- **Network**: http (installed but currently using mock), cached_network_image

## Features
- **Home**: List of news articles with category filter.
- **Article Detail**: Full view of article content.
- **Settings**: Theme toggle (UI only), notifications toggle (UI only).

## Architecture
- `lib/models`: Data models (`Article`).
- `lib/services`: Data providers (`NewsService`).
- `lib/widgets`: Reusable UI components (`NewsCard`).
- `lib/screens`: App screens (`HomeScreen`, `ArticleScreen`, `SettingsScreen`).

## Setup
1. `flutter pub get`
2. `flutter run`
