#include "mode_manager.h"
#include "servo_controller.h"
#include "firebase_app.h"

// MODE VALUES
// - auto
// - manual
static String lastCmd = "";

void managerLoop(unsigned long now)
{
    String mode = getCachedMode();
    String cmd = getCachedCommand();

    if (mode == "auto")
    {
        controlLidAuto(now);
        return;
    }

    if (mode == "manual")
    {
        if (cmd != lastCmd)
        {
            if (cmd == "open")
            {
                openLid();
            }
            else if (cmd == "close")
            {
                closeLid();
            }

            lastCmd = cmd;
        }
    }
}