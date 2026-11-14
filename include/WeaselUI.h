#pragma once
#include <WeaselIPCData.h>
#include <functional>
#include <memory>
#include <string>
#include <windows.h>
#include <wrl.h>

namespace weasel {
using namespace Microsoft::WRL;

template <typename T>
using an = std::shared_ptr<T>;
template <typename T>
using the = std::unique_ptr<T>;
typedef std::function<
    void(size_t* const, size_t* const, bool* const, bool* const)>
    UICallbackFunc;

// 常驻中英状态徽标的图标种类。跨越 IPC 传递，客户端决定显示哪一种，服务端负责绘制。
enum StatusTipIcon {
  STATUS_TIP_NONE = -1,
  STATUS_TIP_DISABLED = 0,
  STATUS_TIP_ASCII,
  STATUS_TIP_ZH,
  STATUS_TIP_FULL,
  STATUS_TIP_HALF,
};

// (show, icon_kind)：客户端把徽标的显示决定交给服务进程
typedef std::function<void(bool, int)> TipCallbackFunc;

enum ClientCapabilities {
  INLINE_PREEDIT_CAPABLE = 1,
};

class UIImpl;
class UI {
 public:
  UI();
  virtual ~UI();
  // 创建输入法界面
  bool Create(HWND parent, bool preview_mode = false);
  // 销毁界面
  void Destroy(bool full = false);
  // 界面显隐
  void Show();
  void Hide();
  // 常驻中英状态提示（style/ascii_tip_persistent）
  // 徽标窗口由服务进程创建，客户端经 tipCallback 交出显示决定；
  // owner_pid 是持有徽标的客户端进程，徽标据此在进程消失时自动隐藏
  void ShowTip(int icon_kind = STATUS_TIP_NONE, DWORD owner_pid = 0);
  void HideTip();
  void ShowWithTimeout(size_t millisec);
  BOOL IsCountingDown() const;
  BOOL IsShown() const;
  // 重绘界面
  void Refresh();
  // Reattach an undragged preview window to its parent dialog.
  void RepositionPreview();
  // 置输入焦点位置（光标跟随时移动候选窗）但不重绘
  void UpdateInputPosition(RECT const& rc);
  // 更新界面显示内容
  void Update(Context const& ctx, Status const& status);
  Context& ctx() { return ctx_; }
  Status& status() { return status_; }
  UIStyle& style() { return style_; }
  UIStyle& ostyle() { return ostyle_; }
  bool GetIsReposition();
  UICallbackFunc& uiCallback() { return _uiCallback; }
  TipCallbackFunc& tipCallback() { return _tipCallback; }
  // compatibility shim for existing call sites
  void SetUICallBack(const UICallbackFunc& func) { _uiCallback = func; }
  void SetCallback(const UICallbackFunc& func) { _uiCallback = func; }
  void SetTipCallback(const TipCallbackFunc& func) { _tipCallback = func; }
  bool& InServer() { return in_server_; }
  HWND hwnd();

 private:
  the<UIImpl> pimpl_;
  Context ctx_;
  Status status_;
  UIStyle style_;
  UIStyle ostyle_;
  bool in_server_;
  UICallbackFunc _uiCallback;
  TipCallbackFunc _tipCallback;
};
}  // namespace weasel
