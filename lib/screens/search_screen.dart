import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:shadcn_ui/shadcn_ui.dart';
import '../data/services/news_service.dart';
import '../widgets/article_card.dart';

class SearchQueryNotifier extends Notifier<String> {
  @override
  String build() => '';

  void set(String value) {
    state = value;
  }
}

final searchQueryProvider = NotifierProvider<SearchQueryNotifier, String>(SearchQueryNotifier.new);

class SearchScreen extends ConsumerWidget {
  const SearchScreen({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final query = ref.watch(searchQueryProvider);
    final searchResultsAsync = ref.watch(searchNewsProvider(query));

    return Scaffold(
      appBar: AppBar(
        title: const Text('Search'),
      ),
      body: Column(
        children: [
          Padding(
            padding: const EdgeInsets.all(16.0),
            child: ShadInput(
              placeholder: const Text('Search news...'),
              onChanged: (value) {
                ref.read(searchQueryProvider.notifier).set(value);
              },
            ),
          ),
          Expanded(
            child: query.isEmpty
                ? const Center(child: Text('Type to search'))
                : searchResultsAsync.when(
                    data: (articles) {
                      if (articles.isEmpty) {
                        return const Center(child: Text('No results found'));
                      }
                      return ListView.builder(
                        itemCount: articles.length,
                        itemBuilder: (context, index) {
                          return ArticleCard(article: articles[index]);
                        },
                      );
                    },
                    loading: () => const Center(child: CircularProgressIndicator()),
                    error: (err, stack) => Center(child: Text('Error: $err')),
                  ),
          ),
        ],
      ),
    );
  }
}