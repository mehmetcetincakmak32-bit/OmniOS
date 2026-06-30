import 'dart:convert';
import 'package:flutter/services.dart';

class PlatformBridge {
  static const BasicMessageChannel _channel = BasicMessageChannel(
    'omnios/system', JSONMessageCodec());

  static final PlatformBridge _instance = PlatformBridge._();
  factory PlatformBridge() => _instance;
  PlatformBridge._();

  bool _initialized = false;

  Future<void> initialize() async {
    if (_initialized) return;
    _channel.setMessageHandler(_handleNativeMessage);
    _initialized = true;
  }

  Future<Map<String, dynamic>?> send(String method, [dynamic args]) async {
    try {
      final message = <String, dynamic>{'method': method};
      if (args != null) message['args'] = args;
      final result = await _channel.send(message);
      if (result is Map) {
        return result.cast<String, dynamic>();
      }
      if (result is String) {
        return jsonDecode(result) as Map<String, dynamic>;
      }
    } catch (_) {}
    return null;
  }

  Future<Map<String, dynamic>?> getSystemInfo() =>
      send('getSystemInfo');

  Future<String?> getDeviceName() async {
    final r = await send('getDeviceName');
    return r?['name'] as String?;
  }

  Future<int?> requestCapability(String capability) async {
    final r = await send('requestCapability', capability);
    return r?['tokenId'] as int?;
  }

  Future<void> sendNotification(Map<String, dynamic> notification) async {
    await send('sendNotification', notification);
  }

  Future<dynamic> _handleNativeMessage(dynamic message) async {
    if (message is Map) {
      final method = message['method'] as String?;
      if (method == 'onNotification') {
        // handle incoming notifications from native
      }
    }
    return null;
  }
}
