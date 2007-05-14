typedef enum {
  INVOKE_KBD_HIDE,
  INVOKE_KBD_SHOW,
} InvokerEvent;

void protocol_send_event (InvokerEvent e);
