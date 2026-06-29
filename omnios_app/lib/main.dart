import 'package:flutter/material.dart';
import 'models/app_item.dart';
import 'widgets/status_bar.dart';
import 'widgets/nav_bar.dart';
import 'screens/normal_screen.dart';
import 'screens/flow_screen.dart';
import 'screens/detail_screen.dart';

void main() {
  runApp(const OmniOSApp());
}

class OmniOSApp extends StatelessWidget {
  const OmniOSApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'OmniOS',
      debugShowCheckedModeBanner: false,
      theme: ThemeData.dark().copyWith(
        scaffoldBackgroundColor: const Color(0xFF0A0A23),
      ),
      home: const PhoneFrame(),
    );
  }
}

class PhoneFrame extends StatefulWidget {
  const PhoneFrame({super.key});

  @override
  State<PhoneFrame> createState() => _PhoneFrameState();
}

class _PhoneFrameState extends State<PhoneFrame> {
  String _currentMode = 'normal';
  String _currentScreen = 'normal';
  AppItem? _selectedApp;

  void _toggleMode() {
    setState(() {
      _currentMode = _currentMode == 'normal' ? 'flow' : 'normal';
      _currentScreen = _currentMode;
      _selectedApp = null;
    });
  }

  void _launchApp(AppItem app) {
    setState(() {
      _selectedApp = app;
      _currentScreen = 'detail';
    });
  }

  void _goHome() {
    setState(() {
      _currentScreen = _currentMode;
      _selectedApp = null;
    });
  }

  void _goBack() {
    if (_currentScreen == 'detail') {
      _goHome();
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Column(
        children: [
          const StatusBar(),
          Expanded(
            child: _buildScreen(),
          ),
          NavBar(
            currentMode: _currentMode,
            onModeToggle: _toggleMode,
            onHome: _goHome,
            onBack: _goBack,
          ),
        ],
      ),
    );
  }

  Widget _buildScreen() {
    if (_currentScreen == 'detail' && _selectedApp != null) {
      return DetailScreen(app: _selectedApp!, onBack: _goHome);
    }
    if (_currentMode == 'flow') {
      return FlowScreen(onAppLaunch: _launchApp);
    }
    return NormalScreen(onAppLaunch: _launchApp);
  }
}
