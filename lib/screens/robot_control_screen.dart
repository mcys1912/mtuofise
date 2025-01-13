import 'package:flutter/material.dart';
import 'package:flutter_joystick/flutter_joystick.dart';
import 'package:firebase_database/firebase_database.dart';
import '../utils/responsive_helper.dart';

class RobotControlScreen extends StatefulWidget {
  const RobotControlScreen({super.key});

  @override
  State<RobotControlScreen> createState() => _RobotControlScreenState();
}

class _RobotControlScreenState extends State<RobotControlScreen> {
  final _database = FirebaseDatabase.instance.ref();
  List<double> _servoAngles = [90, 90, 90, 90];
  bool _isDragging = false;
  Map<int, Map<String, double>> _lastJoystickPositions = {
    0: {'x': 0, 'y': 0},
    2: {'x': 0, 'y': 0},
  };

  @override
  void initState() {
    super.initState();
    _listenToFirebase();
  }

  void _listenToFirebase() {
    _database.child('robot_position').onValue.listen((event) {
      final data = event.snapshot.value as Map?;
      if (data != null && !_isDragging) {
        setState(() {
          _servoAngles = [
            data['servo1']?.toDouble() ?? 90,
            data['servo2']?.toDouble() ?? 90,
            data['servo3']?.toDouble() ?? 90,
            data['servo4']?.toDouble() ?? 90,
          ];
        });
      }
    });
  }

  void _updateServoAngle(int index, double angle) {
    setState(() {
      _servoAngles[index] = angle;
    });

    // Slider değiştiğinde Firebase'e yaz
    _database.child('robot_position').set({
      'servo1': _servoAngles[0],
      'servo2': _servoAngles[1],
      'servo3': _servoAngles[2],
      'servo4': _servoAngles[3],
      'timestamp': ServerValue.timestamp,
    });
  }

  Widget _buildJoystickSection() {
    final joystickSize = ResponsiveHelper.getJoystickSize(context);

    return Container(
      decoration: BoxDecoration(
        gradient: LinearGradient(
          begin: Alignment.topLeft,
          end: Alignment.bottomRight,
          colors: [
            Colors.blue.shade100,
            Colors.blue.shade50,
          ],
        ),
        borderRadius: BorderRadius.circular(20),
      ),
      padding: const EdgeInsets.all(20),
      child: ResponsiveHelper.isMobile(context)
          ? Column(
              children: [
                _buildJoystickPair(joystickSize),
              ],
            )
          : _buildJoystickPair(joystickSize),
    );
  }

  Widget _buildJoystickPair(double size) {
    return Row(
      mainAxisAlignment: MainAxisAlignment.spaceEvenly,
      children: [
        _buildJoystickColumn(0, size, "Sol Kontrol\n(Motor 1-2)"),
        _buildJoystickColumn(2, size, "Sağ Kontrol\n(Motor 3-4)"),
      ],
    );
  }

  Widget _buildJoystickColumn(int baseIndex, double size, String label) {
    return Column(
      children: [
        Text(
          label,
          textAlign: TextAlign.center,
          style: TextStyle(
            fontSize: ResponsiveHelper.isMobile(context) ? 16 : 20,
            fontWeight: FontWeight.bold,
            color: Colors.blue.shade700,
          ),
        ),
        const SizedBox(height: 10),
        Container(
          width: size,
          height: size,
          decoration: BoxDecoration(
            color: Colors.white.withOpacity(0.9),
            borderRadius: BorderRadius.circular(size / 2),
            boxShadow: [
              BoxShadow(
                color: Colors.black.withOpacity(0.1),
                blurRadius: 10,
                spreadRadius: 2,
              ),
            ],
          ),
          child: Joystick(
            mode: JoystickMode.all,
            period: const Duration(milliseconds: 100),
            onStickDragStart: () {
              setState(() {
                _isDragging = true;
              });
            },
            listener: (details) {
              if (_isDragging) {
                double yAngle = -details.y * 90 + 90;
                double xAngle = details.x * 90 + 90;

                setState(() {
                  _servoAngles[baseIndex] = yAngle;
                  _servoAngles[baseIndex + 1] = xAngle;
                });

                // Firebase'e anlık yaz
                _database.child('robot_position').update({
                  'servo1': _servoAngles[0],
                  'servo2': _servoAngles[1],
                  'servo3': _servoAngles[2],
                  'servo4': _servoAngles[3],
                  'timestamp': ServerValue.timestamp,
                });
              }
            },
            onStickDragEnd: () {
              setState(() {
                _isDragging = false;
              });

              // Joystick bırakıldığında Firebase'e son değerleri yaz
              _database.child('robot_position').update({
                'servo1': _servoAngles[0],
                'servo2': _servoAngles[1],
                'servo3': _servoAngles[2],
                'servo4': _servoAngles[3],
                'timestamp': ServerValue.timestamp,
              });
            },
          ),
        ),
        const SizedBox(height: 10),
        Row(
          mainAxisSize: MainAxisSize.min,
          children: [
            Column(
              children: [
                Text(
                  'Motor ${baseIndex + 1}',
                  style: TextStyle(
                    fontSize: ResponsiveHelper.isMobile(context) ? 12 : 14,
                    color: Colors.blue.shade700,
                  ),
                ),
                Text(
                  '${_servoAngles[baseIndex].toStringAsFixed(1)}°',
                  style: TextStyle(
                    fontSize: ResponsiveHelper.isMobile(context) ? 14 : 16,
                    fontWeight: FontWeight.bold,
                  ),
                ),
              ],
            ),
            const SizedBox(width: 20),
            Column(
              children: [
                Text(
                  'Motor ${baseIndex + 2}',
                  style: TextStyle(
                    fontSize: ResponsiveHelper.isMobile(context) ? 12 : 14,
                    color: Colors.blue.shade700,
                  ),
                ),
                Text(
                  '${_servoAngles[baseIndex + 1].toStringAsFixed(1)}°',
                  style: TextStyle(
                    fontSize: ResponsiveHelper.isMobile(context) ? 14 : 16,
                    fontWeight: FontWeight.bold,
                  ),
                ),
              ],
            ),
          ],
        ),
      ],
    );
  }

  Widget _buildSliders() {
    return Container(
      width: ResponsiveHelper.getWidth(context),
      padding: const EdgeInsets.all(16),
      decoration: BoxDecoration(
        gradient: LinearGradient(
          begin: Alignment.topLeft,
          end: Alignment.bottomRight,
          colors: [
            Colors.blue.shade50,
            Colors.white,
          ],
        ),
        borderRadius: BorderRadius.circular(20),
        boxShadow: [
          BoxShadow(
            color: Colors.black.withOpacity(0.1),
            blurRadius: 10,
            spreadRadius: 2,
          ),
        ],
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Padding(
            padding: const EdgeInsets.only(left: 16, bottom: 16),
            child: Text(
              'Hassas Kontrol',
              style: TextStyle(
                fontSize: ResponsiveHelper.isMobile(context) ? 18 : 22,
                fontWeight: FontWeight.bold,
                color: Colors.blue.shade700,
              ),
            ),
          ),
          for (int i = 0; i < 4; i++)
            Container(
              margin: const EdgeInsets.symmetric(vertical: 8),
              padding: const EdgeInsets.all(12),
              decoration: BoxDecoration(
                color: Colors.white,
                borderRadius: BorderRadius.circular(15),
                boxShadow: [
                  BoxShadow(
                    color: Colors.black.withOpacity(0.05),
                    blurRadius: 5,
                    spreadRadius: 1,
                  ),
                ],
              ),
              child: Row(
                children: [
                  Text(
                    'Motor ${i + 1}: ',
                    style: TextStyle(
                      fontSize: ResponsiveHelper.isMobile(context) ? 14 : 16,
                      fontWeight: FontWeight.w500,
                      color: Colors.blue.shade700,
                    ),
                  ),
                  Expanded(
                    child: Slider(
                      value: _servoAngles[i],
                      min: 0,
                      max: 180,
                      activeColor: Colors.blue.shade400,
                      inactiveColor: Colors.blue.shade100,
                      onChanged: (value) => _updateServoAngle(i, value),
                    ),
                  ),
                  Container(
                    width: 50,
                    alignment: Alignment.center,
                    child: Text(
                      '${_servoAngles[i].toStringAsFixed(1)}°',
                      style: TextStyle(
                        fontSize: ResponsiveHelper.isMobile(context) ? 14 : 16,
                        fontWeight: FontWeight.bold,
                        color: Colors.blue.shade700,
                      ),
                    ),
                  ),
                ],
              ),
            ),
        ],
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Container(
        decoration: BoxDecoration(
          gradient: LinearGradient(
            begin: Alignment.topLeft,
            end: Alignment.bottomRight,
            colors: [
              Colors.blue.shade100,
              Colors.white,
              Colors.blue.shade50,
            ],
          ),
        ),
        child: SafeArea(
          child: Column(
            children: [
              Container(
                decoration: BoxDecoration(
                  gradient: LinearGradient(
                    begin: Alignment.topLeft,
                    end: Alignment.bottomRight,
                    colors: [
                      Colors.blue.shade400,
                      Colors.blue.shade300,
                    ],
                  ),
                  borderRadius: const BorderRadius.only(
                    bottomLeft: Radius.circular(25),
                    bottomRight: Radius.circular(25),
                  ),
                  boxShadow: [
                    BoxShadow(
                      color: Colors.black.withOpacity(0.1),
                      blurRadius: 10,
                      spreadRadius: 2,
                    ),
                  ],
                ),
                child: AppBar(
                  title: Row(
                    mainAxisAlignment: MainAxisAlignment.center,
                    children: [
                      Icon(
                        Icons.precision_manufacturing_rounded,
                        color: Colors.white,
                        size: ResponsiveHelper.isMobile(context) ? 24 : 28,
                      ),
                      const SizedBox(width: 10),
                      Text(
                        'Robot Kontrol',
                        style: TextStyle(
                          fontWeight: FontWeight.bold,
                          fontSize:
                              ResponsiveHelper.isMobile(context) ? 20 : 24,
                          color: Colors.white,
                          letterSpacing: 1,
                        ),
                      ),
                    ],
                  ),
                  centerTitle: true,
                  backgroundColor: Colors.transparent,
                  elevation: 0,
                  shape: const RoundedRectangleBorder(
                    borderRadius: BorderRadius.only(
                      bottomLeft: Radius.circular(25),
                      bottomRight: Radius.circular(25),
                    ),
                  ),
                ),
              ),
              Expanded(
                child: SingleChildScrollView(
                  padding: const EdgeInsets.all(16),
                  child: Column(
                    children: [
                      _buildJoystickSection(),
                      const SizedBox(height: 20),
                      _buildSliders(),
                    ],
                  ),
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
