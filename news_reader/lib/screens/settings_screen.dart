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
        padding: const EdgeInsets.all(16),
        children: [
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              const Text('Dark Mode'),
              ShadSwitch(
                value: false, 
                onChanged: (val) {
                  // Implement theme switching
                },
              ),
            ],
          ),
          const SizedBox(height: 16),
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              const Text('Notifications'),
              ShadSwitch(
                value: true, 
                onChanged: (val) {},
              ),
            ],
          ),
          const Divider(),
          const ListTile(
            title: Text('About'),
            subtitle: Text('News Reader App v1.0.0'),
          ),
        ],
      ),
    );
  }
}