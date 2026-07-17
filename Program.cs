using System;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace ZenJiggle;

internal static class Program
{
    [STAThread]
    private static void Main()
    {
        // Prevent duplicate instances stacking multiple tray icons.
        using var singleInstance = new Mutex(true, "ZenJiggle.SingleInstance.Mutex", out bool isNew);
        if (!isNew)
        {
            return;
        }

        Application.EnableVisualStyles();
        Application.SetCompatibleTextRenderingDefault(false);
        Application.Run(new TrayApplicationContext());
    }
}

internal sealed class TrayApplicationContext : ApplicationContext
{
    // ES_CONTINUOUS informs Windows the state should persist until explicitly cleared.
    // ES_SYSTEM_REQUIRED prevents the system idling to sleep.
    // ES_DISPLAY_REQUIRED prevents the display (and therefore the screensaver / lock) from kicking in.
    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern uint SetThreadExecutionState(uint esFlags);

    [DllImport("user32.dll")]
    private static extern bool DestroyIcon(IntPtr hIcon);

    private const uint ES_CONTINUOUS = 0x80000000;
    private const uint ES_SYSTEM_REQUIRED = 0x00000001;
    private const uint ES_DISPLAY_REQUIRED = 0x00000002;

    private readonly NotifyIcon _trayIcon;
    private readonly ToolStripMenuItem _pauseItem;
    private readonly Icon _activeIcon;
    private readonly Icon _pausedIcon;
    private readonly System.Windows.Forms.Timer _keepAliveTimer;
    private bool _isPaused;

    public TrayApplicationContext()
    {
        _activeIcon = CreateDotIcon(Color.FromArgb(46, 204, 113));  // green = zen active
        _pausedIcon = CreateDotIcon(Color.FromArgb(231, 76, 60));   // red = paused

        var menu = new ContextMenuStrip();
        _pauseItem = new ToolStripMenuItem("Pause", null, OnTogglePause);
        menu.Items.Add(_pauseItem);
        menu.Items.Add(new ToolStripSeparator());
        menu.Items.Add(new ToolStripMenuItem("Exit", null, OnExit));

        _trayIcon = new NotifyIcon
        {
            Icon = _activeIcon,
            ContextMenuStrip = menu,
            Visible = true,
            Text = "ZenJiggle — Active"
        };
        _trayIcon.MouseDoubleClick += (_, e) =>
        {
            if (e.Button == MouseButtons.Left)
            {
                TogglePause();
            }
        };

        // Refresh periodically as cheap insurance against anything resetting the flag.
        _keepAliveTimer = new System.Windows.Forms.Timer { Interval = 30_000 };
        _keepAliveTimer.Tick += (_, _) => ApplyExecutionState();
        _keepAliveTimer.Start();

        ApplyExecutionState();
    }

    private void ApplyExecutionState()
    {
        if (!_isPaused)
        {
            SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);
        }
    }

    private void OnTogglePause(object? sender, EventArgs e) => TogglePause();

    private void TogglePause()
    {
        _isPaused = !_isPaused;

        if (_isPaused)
        {
            SetThreadExecutionState(ES_CONTINUOUS); // release override, restore normal idle behavior
            _trayIcon.Icon = _pausedIcon;
            _trayIcon.Text = "ZenJiggle — Paused";
            _pauseItem.Text = "Resume";
        }
        else
        {
            ApplyExecutionState();
            _trayIcon.Icon = _activeIcon;
            _trayIcon.Text = "ZenJiggle — Active";
            _pauseItem.Text = "Pause";
        }
    }

    private void OnExit(object? sender, EventArgs e)
    {
        SetThreadExecutionState(ES_CONTINUOUS); // release override before quitting
        _keepAliveTimer.Stop();
        _trayIcon.Visible = false;
        _trayIcon.Dispose();
        _activeIcon.Dispose();
        _pausedIcon.Dispose();
        Application.Exit();
    }

    private static Icon CreateDotIcon(Color color)
    {
        using var bmp = new Bitmap(32, 32);
        using (var g = Graphics.FromImage(bmp))
        {
            g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.AntiAlias;
            g.Clear(Color.Transparent);
            using var brush = new SolidBrush(color);
            g.FillEllipse(brush, 2, 2, 28, 28);
            using var pen = new Pen(Color.White, 2);
            g.DrawEllipse(pen, 2, 2, 28, 28);
        }

        IntPtr hIcon = bmp.GetHicon();
        try
        {
            using var temp = Icon.FromHandle(hIcon);
            return (Icon)temp.Clone();
        }
        finally
        {
            DestroyIcon(hIcon);
        }
    }
}
