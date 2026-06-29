import 'package:flutter/material.dart';

class StatusBar extends StatelessWidget {
  const StatusBar({super.key});

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 20, vertical: 6),
      color: const Color(0xFF0D0D24),
      child: const Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Row(
            children: [
              Text('12:00', style: TextStyle(color: Colors.white, fontSize: 12, fontWeight: FontWeight.w500)),
              SizedBox(width: 6),
              Text('OmniOS', style: TextStyle(color: Color(0xFF64FFDA), fontSize: 8, fontWeight: FontWeight.w700)),
            ],
          ),
          Row(
            children: [
              Text('●', style: TextStyle(color: Color(0xFF4CAF50), fontSize: 10)),
              SizedBox(width: 4),
              Text('☁', style: TextStyle(color: Colors.white, fontSize: 10)),
              SizedBox(width: 4),
              Text('⚡', style: TextStyle(color: Color(0xFFFFEB3B), fontSize: 10)),
            ],
          ),
        ],
      ),
    );
  }
}
