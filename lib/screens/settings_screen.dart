import 'package:flutter/material.dart';
import 'package:shadcn_ui/shadcn_ui.dart';

class SettingsScreen extends StatelessWidget {
  const SettingsScreen({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Settings'),
      ),
      body: ListView(
        children: [
          ListTile(
            title: const Text('Dark Mode'),
            trailing: ShadSwitch(
              value: false,
              onChanged: (value) {
                // TODO: Implement theme switching
              },
            ),
          ),
          const Divider(),
          ListTile(
            title: const Text('About'),
            subtitle: const Text('News Reader App v1.0.0'),
          ),
        ],
      ),
    );
  }
}
