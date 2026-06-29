import 'package:flutter/material.dart';
import '../models/app_item.dart';
import '../models/app_data.dart';
import '../widgets/app_icon_widget.dart';

class NormalScreen extends StatefulWidget {
  final Function(AppItem) onAppLaunch;

  const NormalScreen({super.key, required this.onAppLaunch});

  @override
  State<NormalScreen> createState() => _NormalScreenState();
}

class _NormalScreenState extends State<NormalScreen> {
  String _filter = 'all';

  @override
  Widget build(BuildContext context) {
    return Column(
      children: [
        _buildFilterBar(),
        Expanded(child: _buildAppList()),
      ],
    );
  }

  Widget _buildFilterBar() {
    return Padding(
      padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 6),
      child: Row(
        children: [
          _filterChip('📱 Tümü', 'all'),
          const SizedBox(width: 6),
          _filterChip('🤖 Android', 'android'),
          const SizedBox(width: 6),
          _filterChip('🍎 iOS', 'ios'),
        ],
      ),
    );
  }

  Widget _filterChip(String label, String value) {
    final active = _filter == value;
    return GestureDetector(
      onTap: () => setState(() => _filter = value),
      child: Container(
        padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 4),
        decoration: BoxDecoration(
          color: active ? const Color(0xFF64FFDA) : Colors.transparent,
          borderRadius: BorderRadius.circular(12),
          border: Border.all(
            color: active ? const Color(0xFF64FFDA) : const Color(0xFF2A2A5E),
          ),
        ),
        child: Text(label,
          style: TextStyle(
            fontSize: 10,
            fontWeight: active ? FontWeight.w600 : FontWeight.w500,
            color: active ? const Color(0xFF0A0A23) : const Color(0xFF888888),
          ),
        ),
      ),
    );
  }

  Widget _buildAppList() {
    if (_filter == 'all') return _buildAllSections();
    final platform = _filter == 'android' ? AppPlatform.android : AppPlatform.ios;
    final apps = AppData.getByPlatform(platform);
    return _buildAppGrid(apps, _filter == 'android' ? '🤖 Android' : '🍎 iOS');
  }

  Widget _buildAllSections() {
    final sections = [
      ('🌐 Platformlar Arası', AppData.crossApps),
      ('📱 Android', AppData.androidApps),
      ('📱 iOS', AppData.iosApps),
    ];
    return ListView.builder(
      padding: const EdgeInsets.only(bottom: 12),
      itemCount: sections.length,
      itemBuilder: (context, index) {
        final title = sections[index].$1;
        final apps = sections[index].$2;
        return Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Padding(
              padding: const EdgeInsets.only(left: 12, top: 12, bottom: 4),
              child: Text(title,
                style: const TextStyle(
                  color: Color(0xFF64FFDA),
                  fontSize: 11,
                  fontWeight: FontWeight.w600,
                  letterSpacing: 0.5,
                ),
              ),
            ),
            _buildAppGrid(apps, title),
          ],
        );
      },
    );
  }

  Widget _buildAppGrid(List<AppItem> apps, String title) {
    return Padding(
      padding: const EdgeInsets.symmetric(horizontal: 4),
      child: Wrap(
        spacing: 0,
        runSpacing: 0,
        children: apps.map((app) => SizedBox(
          width: (MediaQuery.of(context).size.width - 8) / 4,
          child: AppIconWidget(
            app: app,
            onTap: () => widget.onAppLaunch(app),
          ),
        )).toList(),
      ),
    );
  }
}
