import 'package:flutter/material.dart';

class ResponsiveHelper {
  static bool isMobile(BuildContext context) =>
      MediaQuery.of(context).size.width < 600;

  static bool isTablet(BuildContext context) =>
      MediaQuery.of(context).size.width >= 600 &&
      MediaQuery.of(context).size.width < 1200;

  static bool isDesktop(BuildContext context) =>
      MediaQuery.of(context).size.width >= 1200;

  static double getWidth(BuildContext context) {
    if (isMobile(context)) return MediaQuery.of(context).size.width * 0.9;
    if (isTablet(context)) return MediaQuery.of(context).size.width * 0.7;
    return MediaQuery.of(context).size.width * 0.5;
  }

  static double getSplashIconSize(BuildContext context) {
    if (isMobile(context)) return 200;
    if (isTablet(context)) return 300;
    return 400;
  }

  static double getJoystickSize(BuildContext context) {
    if (isMobile(context)) return 150;
    if (isTablet(context)) return 200;
    return 250;
  }
}
