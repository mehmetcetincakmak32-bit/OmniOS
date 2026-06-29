import 'package:flutter/material.dart';
import '../models/app_item.dart';
import '../models/app_data.dart';

class FlowScreen extends StatefulWidget {
  final Function(AppItem) onAppLaunch;

  const FlowScreen({super.key, required this.onAppLaunch});

  @override
  State<FlowScreen> createState() => _FlowScreenState();
}

class _FlowScreenState extends State<FlowScreen> {
  String _time = '';
  String _date = '';

  @override
  void initState() {
    super.initState();
    _updateDateTime();
  }

  void _updateDateTime() {
    final now = DateTime.now();
    setState(() {
      _time = '${now.hour.toString().padLeft(2, '0')}:${now.minute.toString().padLeft(2, '0')}';
      const days = ['Pazartesi', 'Salı', 'Çarşamba', 'Perşembe', 'Cuma', 'Cumartesi', 'Pazar'];
      const months = ['Ocak', 'Şubat', 'Mart', 'Nisan', 'Mayıs', 'Haziran', 'Temmuz', 'Ağustos', 'Eylül', 'Ekim', 'Kasım', 'Aralık'];
      _date = '${days[now.weekday - 1]}, ${now.day} ${months[now.month - 1]} ${now.year}';
    });
  }

  @override
  Widget build(BuildContext context) {
    final allApps = AppData.getAll()..shuffle();
    final suggested = allApps.take(4).toList();

    return Container(
      color: const Color(0xFF0A0A23),
      child: Column(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          const Spacer(flex: 2),
          Text(_time, style: const TextStyle(fontSize: 56, fontWeight: FontWeight.w300, color: Colors.white, letterSpacing: -2)),
          const SizedBox(height: 2),
          Text(_date, style: const TextStyle(fontSize: 12, color: Color(0xFF666666))),
          const SizedBox(height: 24),
          _buildGestures(),
          const SizedBox(height: 24),
          _buildSuggestions(suggested),
          const Spacer(),
          const Text('↑ → ↓ ← • Jestlerle keşfet',
            style: TextStyle(fontSize: 9, color: Color(0xFF333333)),
          ),
          const SizedBox(height: 12),
        ],
      ),
    );
  }

  Widget _buildGestures() {
    final gestures = [
      ('⬆️', 'Yukarı Çek', 'Uygulama Menüsü', const Color(0xFFFF6B6B)),
      ('➡️', 'Sağa Kaydır', 'Son Uygulamalar', const Color(0xFF4ECDC4)),
      ('⬅️', 'Sola Kaydır', 'Bildirimler', const Color(0xFF45B7D1)),
      ('⬇️', 'Aşağı Kaydır', 'Hızlı Ayarlar', const Color(0xFF96CEB4)),
      ('✌️', 'Çift Dokun', 'Ana Ekran', const Color(0xFFFFEAA7)),
    ];

    return Padding(
      padding: const EdgeInsets.symmetric(horizontal: 20),
      child: Wrap(
        spacing: 6,
        runSpacing: 6,
        alignment: WrapAlignment.center,
        children: gestures.map((g) => Container(
          padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 8),
          decoration: BoxDecoration(
            color: const Color(0xFF0D0D24),
            borderRadius: BorderRadius.circular(12),
            border: Border.all(color: const Color(0xFF1A1A3E)),
          ),
          child: Column(
            children: [
              Text(g.$1, style: const TextStyle(fontSize: 20)),
              Text(g.$2, style: TextStyle(fontSize: 9, color: g.$4, fontWeight: FontWeight.w600)),
              Text(g.$3, style: const TextStyle(fontSize: 8, color: Color(0xFF555555))),
            ],
          ),
        )).toList(),
      ),
    );
  }

  Widget _buildSuggestions(List<AppItem> apps) {
    return Padding(
      padding: const EdgeInsets.symmetric(horizontal: 20),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          const Text('⚡ Önerilen Uygulamalar',
            style: TextStyle(color: Color(0xFF64FFDA), fontSize: 10, fontWeight: FontWeight.w600, letterSpacing: 0.5),
          ),
          const SizedBox(height: 8),
          Row(
            children: apps.map((app) => Expanded(
              child: GestureDetector(
                onTap: () => widget.onAppLaunch(app),
                child: Container(
                  margin: const EdgeInsets.symmetric(horizontal: 3),
                  padding: const EdgeInsets.symmetric(vertical: 10),
                  decoration: BoxDecoration(
                    color: const Color(0xFF0D0D24),
                    borderRadius: BorderRadius.circular(12),
                    border: Border.all(color: const Color(0xFF1A1A3E)),
                  ),
                  child: Column(
                    children: [
                      Text(app.emoji, style: const TextStyle(fontSize: 22)),
                      Text(app.name, style: const TextStyle(fontSize: 8, color: Color(0xFFAAAAAA))),
                    ],
                  ),
                ),
              ),
            )).toList(),
          ),
        ],
      ),
    );
  }
}
