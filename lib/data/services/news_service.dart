import 'package:flutter_riverpod/flutter_riverpod.dart';
import '../models/article.dart';

abstract class NewsService {
  Future<List<Article>> getTopHeadlines();
  Future<List<Article>> searchNews(String query);
  Future<Article?> getArticleById(String id);
}

class MockNewsService implements NewsService {
  @override
  Future<List<Article>> getTopHeadlines() async {
    await Future.delayed(const Duration(seconds: 1)); // Simulate network delay
    return _articles;
  }

  final List<Article> _articles = [
      Article(
        id: '1',
        title: 'Flutter is Awesome',
        description: 'Why Flutter is the best framework for cross-platform development.',
        content: 'Flutter provides a fast development cycle with hot reload...',
        url: 'https://flutter.dev',
        urlToImage: 'https://storage.googleapis.com/cms-storage-bucket/a9d6ce81aee44ae017ee.png',
        publishedAt: DateTime.now().subtract(const Duration(hours: 2)),
        sourceName: 'Flutter Dev',
      ),
      Article(
        id: '2',
        title: 'Dart 3.0 Released',
        description: 'The new Dart version brings records, patterns, and class modifiers.',
        content: 'Dart 3.0 is a major release that changes how we write Dart code...',
        url: 'https://dart.dev',
        urlToImage: 'https://dart.dev/assets/dash-cover.png',
        publishedAt: DateTime.now().subtract(const Duration(hours: 5)),
        sourceName: 'Dart News',
      ),
      Article(
        id: '3',
        title: 'AI Revolution',
        description: 'How AI is changing the landscape of software development.',
        content: 'AI tools like Gemini and Copilot are assisting developers...',
        url: 'https://example.com/ai',
        urlToImage: 'https://via.placeholder.com/400x200',
        publishedAt: DateTime.now().subtract(const Duration(days: 1)),
        sourceName: 'Tech Daily',
      ),
      Article(
        id: '4',
        title: 'Global Economy Update',
        description: 'Markets are reacting to the latest inflation data.',
        content: 'Stock markets showed mixed results today as investors...',
        url: 'https://example.com/economy',
        urlToImage: 'https://via.placeholder.com/400x200',
        publishedAt: DateTime.now().subtract(const Duration(days: 1)),
        sourceName: 'Finance World',
      ),
    ];

  @override
  Future<List<Article>> searchNews(String query) async {
    await Future.delayed(const Duration(milliseconds: 500));
    return _articles.where((article) => 
      article.title.toLowerCase().contains(query.toLowerCase()) || 
      article.description.toLowerCase().contains(query.toLowerCase())
    ).toList();
  }

  @override
  Future<Article?> getArticleById(String id) async {
    await Future.delayed(const Duration(milliseconds: 200));
    try {
      return _articles.firstWhere((article) => article.id == id);
    } catch (e) {
      return null;
    }
  }
}

final newsServiceProvider = Provider<NewsService>((ref) => MockNewsService());

final topHeadlinesProvider = FutureProvider<List<Article>>((ref) async {
  final service = ref.watch(newsServiceProvider);
  return service.getTopHeadlines();
});

final articleDetailProvider = FutureProvider.family<Article?, String>((ref, id) async {
  final service = ref.watch(newsServiceProvider);
  return service.getArticleById(id);
});

final searchNewsProvider = FutureProvider.family<List<Article>, String>((ref, query) async {
  final service = ref.watch(newsServiceProvider);
  return service.searchNews(query);
});
