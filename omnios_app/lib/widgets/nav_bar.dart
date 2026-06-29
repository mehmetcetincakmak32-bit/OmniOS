import 'package:flutter/material.dart';

class NavBar extends StatelessWidget {
  final String currentMode;
  final VoidCallback onModeToggle;
  final VoidCallback onHome;
  final VoidCallback onBack;

  const NavBar({
    super.key,
    required this.currentMode,
    required this.onModeToggle,
    required this.onHome,
    required this.onBack,
  });

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 24, vertical: 8),
      decoration: const BoxDecoration(
        color: Color(0xFF0A0A23),
        border: Border(top: BorderSide(color: Color(0xFF1A1A3E))),
      ),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          IconButton(
            icon: const Icon(Icons.circle_outlined, color: Colors.white, size: 22),
            onPressed: onHome,
          ),
          Container(width: 4, height: 4, decoration: const BoxDecoration(
            color: Color(0xFF1A1A3E), shape: BoxShape.circle,
          )),
          GestureDetector(
            onTap: onModeToggle,
            child: Container(
              padding: const EdgeInsets.symmetric(horizontal: 14, vertical: 6),
              decoration: BoxDecoration(
                border: Border.all(color: const Color(0xFF64FFDA)),
                borderRadius: BorderRadius.circular(20),
              ),
              child: Text(
                currentMode == 'normal' ? '◉ Flow' : '◉ Normal',
                style: const TextStyle(color: Color(0xFF64FFDA), fontSize: 12, fontWeight: FontWeight.w600),
              ),
            ),
          ),
          Container(width: 4, height: 4, decoration: const BoxDecoration(
            color: Color(0xFF1A1A3E), shape: BoxShape.circle,
          )),
          IconButton(
            icon: const Icon(Icons.chevron_left, color: Colors.white, size: 22),
            onPressed: onBack,
          ),
        ],
      ),
    );
  }
}
