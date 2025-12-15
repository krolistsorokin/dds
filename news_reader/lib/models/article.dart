class Article {
  final String id;
  final String title;
  final String description;
  final String imageUrl;
  final String content;
  final DateTime publishedAt;
  final String author;
  final String category;

  Article({
    required this.id,
    required this.title,
    required this.description,
    required this.imageUrl,
    required this.content,
    required this.publishedAt,
    required this.author,
    required this.category,
  });

  factory Article.fromJson(Map<String, dynamic> json) {
    return Article(
      id: json['id'] ?? '',
      title: json['title'] ?? '',
      description: json['description'] ?? '',
      imageUrl: json['imageUrl'] ?? 'https://via.placeholder.com/150',
      content: json['content'] ?? '',
      publishedAt: DateTime.parse(json['publishedAt'] ?? DateTime.now().toIso8601String()),
      author: json['author'] ?? 'Unknown',
      category: json['category'] ?? 'General',
    );
  }
}
