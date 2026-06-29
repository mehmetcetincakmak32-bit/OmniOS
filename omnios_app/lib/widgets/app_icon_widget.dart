import 'package:flutter/material.dart';
import '../models/app_item.dart';

class AppIconWidget extends StatelessWidget {
  final AppItem app;
  final VoidCallback onTap;

  const AppIconWidget({super.key, required this.app, required this.onTap});

  Color _platformColor() {
    switch (app.platform) {
      case AppPlatform.android:
        return const Color(0xFF3DDC84);
      case AppPlatform.ios:
        return const Color(0xFF007AFF);
      case AppPlatform.cross:
        return const Color(0xFFFF6B6B);
    }
  }

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onTap: onTap,
      child: Container(
        padding: const EdgeInsets.symmetric(vertical: 8, horizontal: 4),
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            Container(
              width: 52, height: 52,
              decoration: BoxDecoration(
                color: const Color(0xFF16213E),
                borderRadius: BorderRadius.circular(14),
              ),
              child: Center(child: Text(app.emoji, style: const TextStyle(fontSize: 26))),
            ),
            const SizedBox(height: 3),
            Text(app.name,
              style: const TextStyle(fontSize: 9, color: Color(0xFFC0C0D0)),
              textAlign: TextAlign.center,
              overflow: TextOverflow.ellipsis,
            ),
            Text('●',
              style: TextStyle(fontSize: 6, color: _platformColor()),
            ),
          ],
        ),
      ),
    );
  }
}
