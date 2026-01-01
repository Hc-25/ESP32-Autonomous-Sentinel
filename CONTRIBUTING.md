# Contributing to ESP32 Autonomous Sentinel

Thank you for your interest in contributing! This document provides guidelines for contributing to this project.

## 🤝 How to Contribute

### Reporting Bugs

1. Check if the bug has already been reported in [Issues](https://github.com/Hc-25/ESP32-Autonomous-Sentinel/issues)
2. If not, create a new issue with:
   - Clear title and description
   - Steps to reproduce
   - Expected vs actual behavior
   - Environment details (ESP-IDF version, hardware, etc.)
   - Relevant logs or screenshots

### Suggesting Enhancements

1. Check existing issues and discussions
2. Create a new issue with:
   - Clear use case description
   - Proposed solution
   - Alternative approaches considered
   - Impact on existing functionality

### Pull Requests

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Make your changes following the code style guidelines
4. Test thoroughly on actual hardware
5. Commit with clear messages (`git commit -m 'Add: Amazing feature'`)
6. Push to your fork (`git push origin feature/AmazingFeature`)
7. Open a Pull Request

## 📝 Code Style Guidelines

### C++ Code

- Use modern C++17 features
- Follow ESP-IDF coding conventions
- Use meaningful variable and function names
- Add comments for complex logic
- Keep functions focused and small

Example:
```cpp
/**
 * @brief Brief description of function
 * 
 * Detailed description if needed
 * 
 * @param param1 Description of parameter
 * @return Description of return value
 */
esp_err_t doSomething(int param1) {
    // Implementation
}
```

### File Organization

- Header files: `.hpp` extension
- Implementation: `.cpp` extension
- One class per file (generally)
- Include guards in all headers

### Naming Conventions

- Classes: `PascalCase`
- Functions: `camelCase`
- Variables: `snake_case` or `camelCase`
- Constants: `UPPER_SNAKE_CASE`
- Namespaces: `lowercase`

## 🧪 Testing

- Test on actual ESP32-S3 hardware
- Verify all power modes work correctly
- Check memory usage (heap, stack)
- Test edge cases (WiFi failure, SD card errors, etc.)
- Verify Telegram notifications

## 📚 Documentation

- Update README.md if adding features
- Add inline comments for complex code
- Update configuration examples
- Include usage examples

## 🔍 Code Review Process

1. Automated checks must pass
2. At least one maintainer review required
3. All conversations must be resolved
4. Testing on hardware encouraged

## 📋 Commit Message Format

Use clear, descriptive commit messages:

```
Type: Brief description (50 chars max)

Detailed explanation if needed (wrap at 72 chars)

- Bullet points for multiple changes
- Reference issues: Fixes #123
```

Types:
- `Add:` New feature
- `Fix:` Bug fix
- `Update:` Modify existing functionality
- `Remove:` Delete code/features
- `Refactor:` Code restructuring
- `Docs:` Documentation changes
- `Test:` Testing updates

## 🚫 What Not to Commit

- Credentials (`credentials.hpp`)
- Build artifacts (`build/`, `sdkconfig`)
- IDE-specific files
- Personal test files
- Binary files (unless necessary)

## ⚖️ License

By contributing, you agree that your contributions will be licensed under the MIT License.

## 💬 Questions?

- Open a [Discussion](https://github.com/Hc-25/ESP32-Autonomous-Sentinel/discussions)
- Join our community chat (if available)
- Contact maintainers

---

Thank you for contributing! 🎉
