import 'package:flutter/material.dart';
import 'package:shadcn_ui/shadcn_ui.dart';

class BookmarksScreen extends StatelessWidget {
  const BookmarksScreen({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Saved Articles'),
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            const Icon(LucideIcons.bookmark, size: 64, color: Colors.grey),
            const SizedBox(height: 16),
            Text('No saved articles yet', style: ShadTheme.of(context).textTheme.h4),
            const SizedBox(height: 8),
            Text('Articles you bookmark will appear here.', style: ShadTheme.of(context).textTheme.muted),
          ],
        ),
      ),
    );
  }
}