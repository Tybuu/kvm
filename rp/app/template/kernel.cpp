//
// kernel.cpp
//
#include "kernel.h"
#include <circle/usb/usbkeyboard.h>

static const char FromKernel[] = "kernel";

CKernel::CKernel(void)
    : // m_Screen(m_Options.GetWidth(), m_Options.GetHeight()),
      m_Timer(&m_Interrupt), m_Logger(m_Options.GetLogLevel(), &m_Timer),
      m_USBHCI(&m_Interrupt, &m_Timer, TRUE) {}

CKernel::~CKernel(void) {}

boolean CKernel::Initialize(void) {
  boolean bOK = TRUE;

  if (bOK) {
    bOK = m_Serial.Initialize(115200);
  }

  if (bOK) {
    CDevice *pTarget =
        m_DeviceNameService.GetDevice(m_Options.GetLogDevice(), FALSE);
    bOK = m_Logger.Initialize(pTarget);
  }

  if (bOK) {
    bOK = m_Interrupt.Initialize();
  }

  if (bOK) {
    bOK = m_Timer.Initialize();
  }

  if (bOK) {
    m_USBHCI.Initialize();
  }

  return bOK;
}

TShutdownMode CKernel::Run(void) {
  m_Logger.Write(FromKernel, LogNotice, "Hello World");
  while (true) {
    m_Logger.Write(FromKernel, LogNotice, "Scanning for Devices");
    m_ActLED.On();
    bool bUpdated = m_USBHCI.UpdatePlugAndPlay();
    if (bUpdated) {
      m_Logger.Write(FromKernel, LogNotice, "Device Plugged in!");
    }
    m_Timer.MsDelay(500);
    m_ActLED.Off();
    m_Timer.MsDelay(500);
  }

  return ShutdownHalt;
}
