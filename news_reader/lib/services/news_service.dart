import '../models/article.dart';

class NewsService {
  Future<List<Article>> getArticles({String category = 'All'}) async {
    // Simulate network delay
    await Future.delayed(const Duration(seconds: 1));

    List<Article> allArticles = [
      Article(
        id: '1',
        title: 'Flutter 4.0 Released',
        description: 'The new version of Flutter brings massive performance improvements.',
        imageUrl: 'https://picsum.photos/seed/flutter/400/200',
        content: 'Full content about Flutter 4.0...',
        publishedAt: DateTime.now().subtract(const Duration(hours: 2)),
        author: 'Tech Daily',
        category: 'Tech',
      ),
      Article(
        id: '2',
        title: 'Global Markets Rally',
        description: 'Stock markets around the world are showing positive signs.',
        imageUrl: 'https://picsum.photos/seed/finance/400/200',
        content: 'Detailed analysis of global markets...',
        publishedAt: DateTime.now().subtract(const Duration(hours: 5)),
        author: 'Finance Weekly',
        category: 'Business',
      ),
      Article(
        id: '3',
        title: 'New AI Model Unveiled',
        description: 'A revolutionary AI model was announced today.',
        imageUrl: 'https://picsum.photos/seed/ai/400/200',
        content: 'In-depth look at the new AI model...',
        publishedAt: DateTime.now().subtract(const Duration(days: 1)),
        author: 'AI Insider',
        category: 'Tech',
      ),
       Article(
        id: '4',
        title: 'Championship Finals',
        description: 'The finals are set for next weekend.',
        imageUrl: 'https://picsum.photos/seed/sports/400/200',
        content: 'Sports commentary...',
        publishedAt: DateTime.now().subtract(const Duration(days: 2)),
        author: 'Sports Center',
        category: 'Sports',
      ),
    ];

    if (category == 'All') {
      return allArticles;
    } else {
      return allArticles.where((a) => a.category == category).toList();
    }
  }

  Future<Article> getArticleById(String id) async {
    await Future.delayed(const Duration(milliseconds: 500));
    // Re-create the list or fetch properly in real app
    // For mock, just return a dummy if not found or re-generate
    return Article(
        id: id,
        title: 'Article $id',
        description: 'Description for article $id',
        imageUrl: 'https://picsum.photos/seed/$id/400/200',
        content: 'Full content for article $id...',
        publishedAt: DateTime.now(),
        author: 'Author',
        category: 'General',
    );
  }
}
