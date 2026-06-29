import 'package:flutter/material.dart';
import '../models/app_item.dart';
import '../services/runtime/runtime_manager.dart';
import '../services/gesture_service.dart';
import '../services/state_provider.dart';

class SettingsScreen extends StatelessWidget {
  final OmniOSState state;

  const SettingsScreen({super.key, required this.state});

  @override
  Widget build(BuildContext context) {
    return Container(
      color: const Color(0xFF0A0A23),
      child: ListView(
        padding: const EdgeInsets.all(16),
        children: [
          _sectionTitle('Sistem'),
          _settingTile('Mod', state.currentMode.name.toUpperCase(),
              onTap: () => state.toggleMode()),
          _settingTile('Sistem Durumu', state.systemState.name),
          _settingTile('Aktif Process', '${state.processCount}'),
          _settingTile('Toplam Uygulama', '${AppData.getAll().length}'),
          _settingTile('Calisma Suresi', SystemService.uptime),
          const SizedBox(height: 16),
          _sectionTitle('Runtime Servisleri'),
          _runtimeTile('Android Runtime (ART)',
              'API 35 • v4.0', const Color(0xFF3DDC84)),
          _runtimeTile('iOS Runtime (UIKit)',
              'API 18 • v3.2', const Color(0xFF007AFF)),
          _runtimeTile('OmniOS Native',
              'v1.0', const Color(0xFFFF6B6B)),
          const SizedBox(height: 16),
          _sectionTitle('Jestler (${state.isFlowMode ? "Flow" : "Normal"} Mod)'),
          ...GestureService.getHints(isFlow: state.isFlowMode).map((g) =>
            _gestureTile(g.gesture, g.description)),
          const SizedBox(height: 16),
          if (state.runningApps.isNotEmpty) ...[
            _sectionTitle('Calisan Uygulamalar'),
            ...state.runningApps.map((app) => _runningAppTile(app)),
          ],
          const SizedBox(height: 16),
          _sectionTitle('Son Eventler'),
          ...state.getRecentEvents(10).map((e) =>
            Padding(
              padding: const EdgeInsets.symmetric(vertical: 1),
              child: Text(e,
                style: const TextStyle(fontSize: 9, color: Color(0xFF666666)),
              ),
            )),
        ],
      ),
    );
  }

  Widget _sectionTitle(String title) {
    return Padding(
      padding: const EdgeInsets.only(top: 8, bottom: 4),
      child: Text(title,
        style: const TextStyle(
          color: Color(0xFF64FFDA),
          fontSize: 11,
          fontWeight: FontWeight.w600,
          letterSpacing: 0.5,
        ),
      ),
    );
  }

  Widget _settingTile(String label, String value, {VoidCallback? onTap}) {
    return Container(
      margin: const EdgeInsets.symmetric(vertical: 2),
      padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 10),
      decoration: BoxDecoration(
        color: const Color(0xFF0D0D24),
        borderRadius: BorderRadius.circular(8),
        border: Border.all(color: const Color(0xFF1A1A3E)),
      ),
      child: InkWell(
        onTap: onTap,
        child: Row(
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: [
            Text(label, style: const TextStyle(fontSize: 11, color: Color(0xFFCCCCCC))),
            Text(value, style: const TextStyle(fontSize: 11, color: Color(0xFF64FFDA), fontWeight: FontWeight.w500)),
          ],
        ),
      ),
    );
  }

  Widget _runtimeTile(String name, String version, Color color) {
    return Container(
      margin: const EdgeInsets.symmetric(vertical: 2),
      padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 10),
      decoration: BoxDecoration(
        color: const Color(0xFF0D0D24),
        borderRadius: BorderRadius.circular(8),
        border: Border.all(color: color.withOpacity(0.3)),
      ),
      child: Row(
        children: [
          Container(width: 8, height: 8, decoration: BoxDecoration(
            color: color, shape: BoxShape.circle,
          )),
          const SizedBox(width: 8),
          Text(name, style: const TextStyle(fontSize: 11, color: Color(0xFFCCCCCC))),
          const Spacer(),
          Text(version, style: TextStyle(fontSize: 10, color: color)),
        ],
      ),
    );
  }

  Widget _gestureTile(String gesture, String desc) {
    return Container(
      margin: const EdgeInsets.symmetric(vertical: 1),
      padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 6),
      child: Row(
        children: [
          Text('⬡', style: TextStyle(fontSize: 8, color: const Color(0xFF4ECDC4))),
          const SizedBox(width: 8),
          Text(gesture, style: const TextStyle(fontSize: 10, color: Color(0xFFAAAAAA))),
          const Spacer(),
          Text(desc, style: const TextStyle(fontSize: 10, color: Color(0xFF666666))),
        ],
      ),
    );
  }

  Widget _runningAppTile(AppItem app) {
    return Container(
      margin: const EdgeInsets.symmetric(vertical: 2),
      padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 8),
      decoration: BoxDecoration(
        color: const Color(0xFF0D0D24),
        borderRadius: BorderRadius.circular(8),
        border: Border.all(color: const Color(0xFF1A1A3E)),
      ),
      child: Row(
        children: [
          Text(app.emoji, style: const TextStyle(fontSize: 16)),
          const SizedBox(width: 8),
          Text(app.name, style: const TextStyle(fontSize: 11, color: Color(0xFFCCCCCC))),
          const Spacer(),
          Container(
            padding: const EdgeInsets.symmetric(horizontal: 6, vertical: 2),
            decoration: BoxDecoration(
              color: const Color(0xFF4CAF50).withOpacity(0.2),
              borderRadius: BorderRadius.circular(4),
            ),
            child: Text('● AKTIF', style: TextStyle(fontSize: 7, color: const Color(0xFF4CAF50))),
          ),
        ],
      ),
    );
  }
}
