#include "stdafx.h"
#include "WeaselTSF.h"

STDMETHODIMP WeaselTSF::OnInitDocumentMgr(ITfDocumentMgr* pDocMgr) {
  return S_OK;
}

STDMETHODIMP WeaselTSF::OnUninitDocumentMgr(ITfDocumentMgr* pDocMgr) {
  return S_OK;
}

STDMETHODIMP WeaselTSF::OnSetFocus(ITfDocumentMgr* pDocMgrFocus,
                                   ITfDocumentMgr* pDocMgrPrevFocus) {
  _InitTextEditSink(pDocMgrFocus);

  // focus moved inside this thread (another window or document of the same
  // app): the server side badge may belong to the window that just lost focus,
  // so state this window's wish even when its own state did not change
  _tipForce = true;

  com_ptr<ITfDocumentMgr> pCandidateListDocumentMgr;
  com_ptr<ITfContext> pTfContext = _GetUIContextDocument();
  if ((nullptr != pTfContext) &&
      SUCCEEDED(pTfContext->GetDocumentMgr(&pCandidateListDocumentMgr))) {
    if (pCandidateListDocumentMgr != pDocMgrFocus) {
      _HideUI();
      _HideTip();
    } else {
      _ShowUI();
      _ShowTip();
    }
  } else if (pDocMgrFocus != nullptr) {
    // no UI context yet in this process, but a document of ours gained focus
    _ShowTip();
  }

  return S_OK;
}

STDMETHODIMP WeaselTSF::OnPushContext(ITfContext* pContext) {
  return S_OK;
}

STDMETHODIMP WeaselTSF::OnPopContext(ITfContext* pContext) {
  return S_OK;
}

BOOL WeaselTSF::_InitThreadMgrEventSink() {
  ITfSource* pSource;
  if (_pThreadMgr->QueryInterface(IID_ITfSource, (void**)&pSource) != S_OK)
    return FALSE;
  if (pSource->AdviseSink(IID_ITfThreadMgrEventSink,
                          (ITfThreadMgrEventSink*)this,
                          &_dwThreadMgrEventSinkCookie) != S_OK) {
    _dwThreadMgrEventSinkCookie = TF_INVALID_COOKIE;
    pSource->Release();
    return FALSE;
  }
  pSource->Release();
  return TRUE;
}

void WeaselTSF::_UninitThreadMgrEventSink() {
  ITfSource* pSource;
  if (_dwThreadMgrEventSinkCookie == TF_INVALID_COOKIE)
    return;
  if (SUCCEEDED(_pThreadMgr->QueryInterface(IID_ITfSource, (void**)&pSource))) {
    pSource->UnadviseSink(_dwThreadMgrEventSinkCookie);
    pSource->Release();
  }
  _dwThreadMgrEventSinkCookie = TF_INVALID_COOKIE;
}
