import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:shadcn_ui/shadcn_ui.dart';
import 'package:url_launcher/url_launcher.dart';
import '../data/services/news_service.dart';

class ArticleDetailScreen extends ConsumerWidget {
  final String articleId;

  const ArticleDetailScreen({super.key, required this.articleId});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final articleAsync = ref.watch(articleDetailProvider(articleId));

    return Scaffold(
      appBar: AppBar(
        title: const Text('Article'),
      ),
      body: articleAsync.when(
        data: (article) {
          if (article == null) {
            return const Center(child: Text('Article not found'));
          }
          return SingleChildScrollView(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                if (article.urlToImage.isNotEmpty)
                  Image.network(
                    article.urlToImage,
                    width: double.infinity,
                    height: 250,
                    fit: BoxFit.cover,
                  ),
                Padding(
                  padding: const EdgeInsets.all(16.0),
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Text(
                        article.title,
                        style: ShadTheme.of(context).textTheme.h3,
                      ),
                      const SizedBox(height: 8),
                      Text(
                        '${article.sourceName} • ${article.publishedAt.toLocal().toString().split(' ')[0]}',
                        style: ShadTheme.of(context).textTheme.muted,
                      ),
                      const SizedBox(height: 16),
                      Text(
                        article.description,
                        style: ShadTheme.of(context).textTheme.large,
                      ),
                      const SizedBox(height: 16),
                      Text(
                        article.content,
                        style: ShadTheme.of(context).textTheme.p,
                      ),
                      const SizedBox(height: 24),
                      SizedBox(
                        width: double.infinity,
                        child: ShadButton(
                          onPressed: () async {
                            final uri = Uri.parse(article.url);
                            if (await canLaunchUrl(uri)) {
                              await launchUrl(uri);
                            }
                          },
                          child: const Text('Read Full Article'),
                        ),
                      ),
                    ],
                  ),
                ),
              ],
            ),
          );
        },
        loading: () => const Center(child: CircularProgressIndicator()),
        error: (err, stack) => Center(child: Text('Error: $err')),
      ),
    );
  }
}
