#pragma once

class IInputConverter
{
public:
  // Returns 0 on failure. Takes the raw DirectInput scancode and resolves both
  // the virtual key and the character under the active OS keyboard layout.
  virtual wchar_t VkCodeToChar(uint8_t scanCode,
                               bool capitalLetters) noexcept = 0;
};
