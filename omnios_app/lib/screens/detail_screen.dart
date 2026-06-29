import 'package:flutter/material.dart';
import '../models/app_item.dart';
import '../services/compatibility_service.dart';

class DetailScreen extends StatelessWidget {
  final AppItem app;
  final VoidCallback onBack;

  const DetailScreen({super.key, required this.app, required this.onBack});

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
    final color = _platformColor();
    final bundleId = CompatibilityService.getBundleId(app);
    final runtime = CompatibilityService.getRuntimeInfo(app);

    return Container(
      color: const Color(0xFF0A0A23),
      child: Column(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          Text(app.emoji, style: const TextStyle(fontSize: 64)),
          const SizedBox(height: 12),
          Text(app.name, style: const TextStyle(fontSize: 20, fontWeight: FontWeight.w600, color: Colors.white)),
          const SizedBox(height: 4),
          Text(app.platformLabel, style: TextStyle(fontSize: 11, color: color, fontWeight: FontWeight.w500)),
          const SizedBox(height: 16),
          Container(
            margin: const EdgeInsets.symmetric(horizontal: 40),
            padding: const EdgeInsets.all(14),
            decoration: BoxDecoration(
              color: const Color(0xFF0D0D24),
              borderRadius: BorderRadius.circular(12),
              border: Border.all(color: const Color(0xFF1A1A3E)),
            ),
            child: Column(
              children: [
                _infoRow('📌 Uygulama', app.name),
                _infoRow('🌐 Platform', app.platformLabel, valueColor: color),
                _infoRow('⚡ Durum', '● Aktif', valueColor: const Color(0xFF4CAF50)),
                _infoRow('🔧 Runtime', runtime),
                _infoRow('📦 Bundle ID', bundleId),
              ],
            ),
          ),
          const SizedBox(height: 20),
          GestureDetector(
            onTap: onBack,
            child: Container(
              padding: const EdgeInsets.symmetric(horizontal: 28, vertical: 8),
              decoration: BoxDecoration(
                color: const Color(0xFF16213E),
                borderRadius: BorderRadius.circular(10),
              ),
              child: const Text('✖ Ana Ekrana Dön', style: TextStyle(color: Colors.white, fontSize: 12)),
            ),
          ),
        ],
      ),
    );
  }

  Widget _infoRow(String label, String value, {Color? valueColor}) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 4),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Text(label, style: const TextStyle(fontSize: 11, color: Color(0xFF888888))),
          Text(value, style: TextStyle(fontSize: 11, color: valueColor ?? const Color(0xFFE0E0E0), fontWeight: FontWeight.w500)),
        ],
      ),
    );
  }
}
