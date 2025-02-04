/*
    SPDX-FileCopyrightText: 1997 Matthias Kalle Dalheimer <kalle@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kworkspace_export.h"

namespace KWorkSpace
{
/**
 * The possible values for the @p confirm parameter of requestShutDown().
 */
enum ShutdownConfirm {
    /**
     * Obey the user's confirmation setting.
     */
    ShutdownConfirmDefault = -1,
    /**
     * Don't confirm, shutdown without asking.
     */
    ShutdownConfirmNo = 0,
    /**
     * Always confirm, ask even if the user turned it off.
     */
    ShutdownConfirmYes = 1,
};

/**
 * The possible values for the @p sdtype parameter of requestShutDown().
 */
enum ShutdownType {
    /**
     * Select previous action or the default if it's the first time.
     */
    ShutdownTypeDefault = -1,
    /**
     * Only log out
     */
    ShutdownTypeNone = 0,
    /**
     * Log out and reboot the machine.
     */
    ShutdownTypeReboot = 1,
    /**
     * Log out and halt the machine.
     */
    ShutdownTypeHalt = 2,
    /**
     * Temporary brain damage. Don't use. Same as ShutdownTypeNone
     */
    // KDE5: kill this
    ShutdownTypeLogout = 3,
};

/**
 * The possible values for the @p sdmode parameter of requestShutDown().
 */
// KDE5: this seems fairly useless
enum ShutdownMode {
    /**
     * Select previous mode or the default if it's the first time.
     */
    ShutdownModeDefault = -1,
    /**
     * Schedule a shutdown (halt or reboot) for the time all active sessions
     * have exited.
     */
    ShutdownModeSchedule = 0,
    /**
     * Shut down, if no sessions are active. Otherwise do nothing.
     */
    ShutdownModeTryNow = 1,
    /**
     * Force shutdown. Kill any possibly active sessions.
     */
    ShutdownModeForceNow = 2,
    /**
     * Pop up a dialog asking the user what to do if sessions are still active.
     */
    ShutdownModeInteractive = 3,
};

/**
 * Asks the session manager to shut the session down.
 *
 * Using @p confirm == ShutdownConfirmYes or @p sdtype != ShutdownTypeDefault or
 * @p sdmode != ShutdownModeDefault causes the use of ksmserver's DCOP
 * interface. The remaining two combinations use the standard XSMP and
 * will work with any session manager compliant with it.
 *
 * @param confirm Whether to ask the user if he really wants to log out.
 * ShutdownConfirm
 * @param sdtype The action to take after logging out. ShutdownType
 * @param sdmode If/When the action should be taken. ShutdownMode
 * @deprecated
 */
KWORKSPACE_EXPORT void
requestShutDown(ShutdownConfirm confirm = ShutdownConfirmDefault, ShutdownType sdtype = ShutdownTypeDefault, ShutdownMode sdmode = ShutdownModeDefault);

/**
 * Used to check whether a requestShutDown call with the same arguments
 * has any chance of succeeding.
 *
 * For example, if KDE's own session manager cannot be contacted, we can't
 * demand that the computer be shutdown, or force a confirmation dialog.
 *
 * Even if we can access the KDE session manager, the system or user
 * configuration may prevent the user from requesting a shutdown or
 * reboot.
 * @deprecated
 */
KWORKSPACE_EXPORT bool
canShutDown(ShutdownConfirm confirm = ShutdownConfirmDefault, ShutdownType sdtype = ShutdownTypeDefault, ShutdownMode sdmode = ShutdownModeDefault);

/**
 * Used to check whether a shutdown is currently in progress
 */
KWORKSPACE_EXPORT bool isShuttingDown();

/**
 * Propagates the network address of the session manager in the
 * SESSION_MANAGER environment variable so that child processes can
 * pick it up.
 *
 * If SESSION_MANAGER isn't defined yet, the address is searched in
 * $HOME/.KSMserver.
 *
 * This function is called by clients that are started outside the
 * session ( i.e. before ksmserver is started), but want to launch
 * other processes that should participate in the session.  Examples
 * are kdesktop or kicker.
 */
KWORKSPACE_EXPORT void propagateSessionManager();
}
