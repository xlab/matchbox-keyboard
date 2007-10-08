typedef enum {
  INVOKE_KBD_HIDE,
  INVOKE_KBD_SHOW,
  INVOKE_KBD_TOGGLE,
} InvokerEvent;

void protocol_send_event (InvokerEvent e);
