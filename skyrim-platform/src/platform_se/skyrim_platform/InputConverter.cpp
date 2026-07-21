#include "InputConverter.h"

wchar_t InputConverter::VkCodeToChar(uint8_t scanCode,
                                     bool capitalLetters) noexcept
{
  // https://github.com/cefsharp/CefSharp/issues/2143
  // https://gist.github.com/jankurianski/5b56b9e36526606bcf175747c592e1c8
  // https://stackoverflow.com/questions/6929275/how-to-convert-a-virtual-key-code-to-a-character-according-to-the-current-keyboa/6949520#6949520
  // https://github.com/adobe/webkit/blob/master/Source/WebCore/platform/chromium/KeyboardCodes.h

  // Resolve the active keyboard layout: if the user cycled one explicitly
  // (Shift+Alt → SwitchLayout) use it; otherwise follow whatever layout the
  // foreground window uses in the OS. Using the foreground layout (instead of
  // the game thread's default, which is US) is what makes non-US layouts —
  // e.g. Spanish/Latin-American `?`, `ñ`, `¿` — type correctly in the chat.
  HKL hkl = (HKL)this->keyboardLayout;
  if (!hkl) {
    hkl = GetKeyboardLayout(
      GetWindowThreadProcessId(GetForegroundWindow(), nullptr));
  }

  // Derive the virtual key from the scancode under THIS layout so the VK and the
  // translated character stay consistent (MapVirtualKeyA in the caller uses the
  // thread layout, which may differ).
  UINT virtualKeyCode = MapVirtualKeyExW(scanCode, MAPVK_VSC_TO_VK, hkl);
  if (!virtualKeyCode)
    return 0;

  wchar_t buf[128] = { 0 };

  std::array<uint8_t, 256> keyboardState;
  keyboardState.fill(0x00);
  if (capitalLetters) {
    keyboardState[VK_SHIFT] = 0xff;
  }

  // https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-tounicode
  int res = ToUnicodeEx(virtualKeyCode, scanCode, keyboardState.data(), buf,
                        std::size(buf), 0, hkl);
  if (res != 1)
    return 0;
  return buf[0];
}

void InputConverter::SwitchLayout() noexcept
{
  if (this->keyboardLayouts.empty()) {
    const int n = GetKeyboardLayoutList(0, nullptr);
    if (n <= 0)
      return;
    this->keyboardLayouts.resize(n);
    const int copied =
      GetKeyboardLayoutList(n, (HKL*)this->keyboardLayouts.data());
    if (copied != n)
      return;
  } else {
    ++this->currentLangId;
  }
  if (this->currentLangId >= this->keyboardLayouts.size()) {
    this->currentLangId = 0;
  }
  this->keyboardLayout = this->keyboardLayouts[this->currentLangId];
}
