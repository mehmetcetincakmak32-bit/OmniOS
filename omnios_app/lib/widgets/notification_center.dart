import 'package:flutter/material.dart';

class NotificationItem {
  final String appName;
  final String appIcon;
  final String title;
  final String body;
  final String time;
  final bool isIOS;
  final bool isRead;

  const NotificationItem({
    required this.appName,
    required this.appIcon,
    required this.title,
    required this.body,
    required this.time,
    this.isIOS = false,
    this.isRead = false,
  });
}

class NotificationCenter extends StatelessWidget {
  final List<NotificationItem> notifications;
  final VoidCallback? onClearAll;

  const NotificationCenter({
    super.key,
    required this.notifications,
    this.onClearAll,
  });

  static const List<NotificationItem> sampleNotifications = [
    NotificationItem(
      appName: 'WhatsApp', appIcon: '💬',
      title: 'Ali Yılmaz', body: 'Merhaba, yarın görüşelim mi?',
      time: '2dk önce',
    ),
    NotificationItem(
      appName: 'Gmail', appIcon: '✉️',
      title: '3 yeni e-posta', body: 'Toplantı hatırlatması, rapor ve fatura',
      time: '15dk önce',
    ),
    NotificationItem(
      appName: 'Instagram', appIcon: '📸',
      title: 'Bildirim', body: 'ahmet_42 fotoğrafını beğendi',
      time: '1s önce', isIOS: true,
    ),
    NotificationItem(
      appName: 'Twitter', appIcon: '🐦',
      title: 'Trendler', body: '#OmniOS gündemde',
      time: '30dk önce', isRead: true,
    ),
    NotificationItem(
      appName: 'YouTube', appIcon: '▶️',
      title: 'Yeni video', body: 'Kanalına abone olan yeni bir video yükledi',
      time: '1s önce', isIOS: true, isRead: true,
    ),
    NotificationItem(
      appName: 'Calendar', appIcon: '📅',
      title: 'Yarın: Proje Teslimi', body: 'Saat 10:00 - Ofis',
      time: '3s önce',
    ),
  ];

  @override
  Widget build(BuildContext context) {
    if (notifications.isEmpty) {
      return Container(
        color: const Color(0xFF0A0A23),
        child: const Center(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Text('🔔', style: TextStyle(fontSize: 48)),
              SizedBox(height: 12),
              Text('Bildirim yok', style: TextStyle(fontSize: 14, color: Color(0xFF666666))),
            ],
          ),
        ),
      );
    }

    return Container(
      color: const Color(0xFF0A0A23),
      child: Column(
        children: [
          _buildHeader(),
          Expanded(
            child: ListView.builder(
              padding: const EdgeInsets.symmetric(horizontal: 12),
              itemCount: notifications.length + 1,
              itemBuilder: (context, index) {
                if (index == 0) return _buildDateGroup('Bugün');
                return _buildNotificationCard(notifications[index - 1]);
              },
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildHeader() {
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 12),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          const Text('Bildirim Merkezi',
            style: TextStyle(fontSize: 16, fontWeight: FontWeight.bold, color: Colors.white)),
          if (onClearAll != null)
            GestureDetector(
              onTap: onClearAll,
              child: const Text('Temizle',
                style: TextStyle(fontSize: 12, color: Color(0xFF64FFDA))),
            ),
        ],
      ),
    );
  }

  Widget _buildDateGroup(String label) {
    return Padding(
      padding: const EdgeInsets.only(left: 4, top: 4, bottom: 6),
      child: Text(label,
        style: const TextStyle(fontSize: 11, color: Color(0xFF888888), fontWeight: FontWeight.w600)),
    );
  }

  Widget _buildNotificationCard(NotificationItem item) {
    return Container(
      margin: const EdgeInsets.only(bottom: 8),
      padding: const EdgeInsets.all(12),
      decoration: BoxDecoration(
        color: item.isRead ? const Color(0xFF0A0A23) : const Color(0xFF0D0D24),
        borderRadius: BorderRadius.circular(12),
        border: Border.all(
          color: item.isRead ? const Color(0xFF1A1A3E) : const Color(0xFF2A2A5E),
        ),
      ),
      child: Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Container(
            width: 36, height: 36,
            decoration: BoxDecoration(
              color: const Color(0xFF16213E),
              borderRadius: BorderRadius.circular(8),
            ),
            child: Center(child: Text(item.appIcon, style: const TextStyle(fontSize: 18))),
          ),
          const SizedBox(width: 10),
          Expanded(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Row(
                  children: [
                    Text(item.appName,
                      style: const TextStyle(fontSize: 11, fontWeight: FontWeight.w600, color: Colors.white)),
                    const Spacer(),
                    if (item.isIOS)
                      Container(
                        margin: const EdgeInsets.only(right: 4),
                        padding: const EdgeInsets.symmetric(horizontal: 4, vertical: 1),
                        decoration: BoxDecoration(
                          color: const Color(0xFF007AFF).withValues(alpha: 0.2),
                          borderRadius: BorderRadius.circular(3),
                        ),
                        child: const Text('iOS', style: TextStyle(fontSize: 7, color: Color(0xFF007AFF))),
                      ),
                    Text(item.time, style: const TextStyle(fontSize: 9, color: Color(0xFF666666))),
                  ],
                ),
                const SizedBox(height: 2),
                Text(item.title,
                  style: const TextStyle(fontSize: 12, fontWeight: FontWeight.w500, color: Color(0xFFE0E0E0))),
                const SizedBox(height: 1),
                Text(item.body,
                  style: const TextStyle(fontSize: 10, color: Color(0xFF888888)),
                  maxLines: 2, overflow: TextOverflow.ellipsis),
              ],
            ),
          ),
        ],
      ),
    );
  }
}
