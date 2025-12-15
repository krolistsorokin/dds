import 'package:flutter/material.dart' hide Scaffold, AppBar, Card, IconButton, Theme, CircularProgressIndicator;
import 'package:go_router/go_router.dart';
import 'package:shadcn_ui/shadcn_ui.dart';
import 'screens/home_screen.dart';
import 'screens/article_screen.dart';
import 'screens/settings_screen.dart';
import 'models/article.dart';

void main() {
  runApp(const MyApp());
}

final _router = GoRouter(
  initialLocation: '/',
  routes: [
    GoRoute(
      path: '/',
      builder: (context, state) => const HomeScreen(),
    ),
    GoRoute(
      path: '/article/:id',
      builder: (context, state) {
        final article = state.extra as Article?;
        if (article == null) {
          // Fallback or error
          return const ShadApp(home: Center(child: Text('Article not found')));
        }
        return ArticleScreen(article: article);
      },
    ),
    GoRoute(
      path: '/settings',
      builder: (context, state) => const SettingsScreen(),
    ),
  ],
);

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return ShadApp.router(
      title: 'News Reader',
      routerConfig: _router,
      theme: ShadThemeData(
        brightness: Brightness.light,
        colorScheme: const ShadSlateColorScheme.light(),
      ),
    );
  }
}
