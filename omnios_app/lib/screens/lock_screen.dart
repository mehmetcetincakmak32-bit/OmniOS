import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

class LockScreen extends StatefulWidget {
  final VoidCallback onUnlock;

  const LockScreen({super.key, required this.onUnlock});

  @override
  State<LockScreen> createState() => _LockScreenState();
}

class _LockScreenState extends State<LockScreen> {
  String _time = '';
  String _date = '';
  final List<int> _pin = [];
  final int _pinLength = 4;

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

  void _onDigit(int digit) {
    if (_pin.length >= _pinLength) return;
    setState(() => _pin.add(digit));
    HapticFeedback.lightImpact();
    if (_pin.length == _pinLength) {
      Future.delayed(const Duration(milliseconds: 200), widget.onUnlock);
    }
  }

  void _onDelete() {
    if (_pin.isEmpty) return;
    setState(() => _pin.removeLast());
    HapticFeedback.selectionClick();
  }

  @override
  Widget build(BuildContext context) {
    return Container(
      decoration: const BoxDecoration(
        gradient: LinearGradient(
          begin: Alignment.topCenter,
          end: Alignment.bottomCenter,
          colors: [Color(0xFF0A0A23), Color(0xFF1A1A2E)],
        ),
      ),
      child: Column(
        children: [
          const Spacer(flex: 3),
          Text(_time, style: const TextStyle(fontSize: 72, fontWeight: FontWeight.w200, color: Colors.white, letterSpacing: -3)),
          const SizedBox(height: 4),
          Text(_date, style: const TextStyle(fontSize: 14, color: Color(0xFF888899))),
          const Spacer(flex: 2),
          _buildPinDots(),
          const SizedBox(height: 40),
          _buildKeypad(),
          const Spacer(flex: 2),
          Row(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Icon(Icons.flashlight_on_outlined, color: Colors.white54, size: 20),
              const SizedBox(width: 40),
              Icon(Icons.camera_alt_outlined, color: Colors.white54, size: 20),
            ],
          ),
          const SizedBox(height: 30),
        ],
      ),
    );
  }

  Widget _buildPinDots() {
    return Row(
      mainAxisAlignment: MainAxisAlignment.center,
      children: List.generate(_pinLength, (i) {
        final filled = i < _pin.length;
        return Container(
          margin: const EdgeInsets.symmetric(horizontal: 8),
          width: 12,
          height: 12,
          decoration: BoxDecoration(
            shape: BoxShape.circle,
            color: filled ? Colors.white : Colors.transparent,
            border: Border.all(color: filled ? Colors.white : Colors.white38, width: 1.5),
          ),
        );
      }),
    );
  }

  Widget _buildKeypad() {
    const digits = [
      [1, 2, 3],
      [4, 5, 6],
      [7, 8, 9],
      [-1, 0, -2],
    ];

    return Column(
      children: digits.map((row) => Padding(
        padding: const EdgeInsets.symmetric(vertical: 4),
        child: Row(
          mainAxisAlignment: MainAxisAlignment.center,
          children: row.map((d) {
            if (d == -1) return const SizedBox(width: 76);
            if (d == -2) {
              return GestureDetector(
                onTap: _onDelete,
                child: Container(
                  width: 76, height: 76,
                  alignment: Alignment.center,
                  child: const Icon(Icons.backspace_outlined, color: Colors.white54, size: 24),
                ),
              );
            }
            return GestureDetector(
              onTap: () => _onDigit(d),
              child: Container(
                width: 76, height: 76,
                margin: const EdgeInsets.symmetric(horizontal: 8),
                decoration: const BoxDecoration(
                  shape: BoxShape.circle,
                  color: Color(0xFF1A1A3E),
                ),
                child: Center(
                  child: Text('$d',
                    style: const TextStyle(fontSize: 28, color: Colors.white, fontWeight: FontWeight.w300),
                  ),
                ),
              ),
            );
          }).toList(),
        ),
      )).toList(),
    );
  }
}
